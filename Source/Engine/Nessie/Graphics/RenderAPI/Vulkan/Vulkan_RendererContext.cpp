// Vulkan_RendererContext.cpp
#include "Graphics/RendererContext.h"

#ifdef NES_RENDER_API_VULKAN
#include "Application/Application.h"
#include "Debug/CheckedCast.h"

#ifdef NES_WINDOW_API_GLFW
#include "GLFW/glfw3.h"
#endif

#ifdef NES_WINDOW_API_GLFW
	static std::vector<const char*> GLFW_GetRequiredExtensions();
	static vk::SurfaceKHR GLFW_CreateSurface(vk::Instance pInstance, GLFWwindow* pWindow);
#else
#error "RendererContext not setup to handle current Window API!"
#endif

namespace nes
{
	bool RendererContext::Init(Window* pWindow, const ApplicationProperties& props, const ConfigOptions& options)
    {
    	// [TODO]: This configuration object should be a part of some
    	// Device Specification that is loaded from Data.
    	// For now, I am just going to be using defaults to get things going.
        auto requiredExtensions = GLFW_GetRequiredExtensions();
        
        // Vulkan must first have an instance created, which will act as the
        // DLL context for the app
        auto instBuilder = vkb::InstanceBuilder()
            .set_engine_name("Nessie")
            //.set_engine_version(props.m_engineVersion.m_value)
            .set_app_name(props.m_appName.c_str())
            .set_app_version(props.m_appVersion)
    		.enable_extensions(requiredExtensions)
            .require_api_version(1, 3, 0);
        
#if NES_VULKAN_DEBUG
        {
			// [TODO]: Allow this to be changed.
			m_debugLogFunc = options.m_debugLogFunc;
            instBuilder.set_debug_callback(DebugLogCallback); 
            instBuilder.set_debug_callback_user_data_pointer(this);

            // If a user is providing a logging function, assume they want validation checks
            instBuilder.request_validation_layers(true);
        }
#endif

        auto instResult = instBuilder.build();
        if (!instResult)
        {
            NES_ERROR(vulkan::kLogTag, "Failed to initialize Vulkan! Failed to build vkb::Instance!");
            return false;
        }

        m_vkbInstance = instResult.value();

        // Create the Surface that we are going to render to.
        m_displaySurface = GLFW_CreateSurface(m_vkbInstance.instance, checked_cast<GLFWwindow*>(pWindow->GetNativeWindowHandle()));
        if (!m_displaySurface)
        {
            NES_ERROR(vulkan::kLogTag, "Failed to initialize Vulkan! Failed to create Surface!");
            return false;
        }

        // Now that we have a surface we can look for the physical device which can draw to this surface

        auto selector = vkb::PhysicalDeviceSelector(m_vkbInstance)
            .set_surface(m_displaySurface);

    	// [TODO]: Remove in favor of having a minimum spec for Physical Device.
        if (options.m_configureDeviceFunc)
            options.m_configureDeviceFunc(selector);

        auto selectResult = selector.select();
        if (!selectResult)
        {
            NES_ERROR(vulkan::kLogTag, "Failed to initialize Vulkan! Failed to select Physical Device!");
            return false;
        }
        m_vkbPhysicalDevice = selectResult.value();

        // Finally, we can create a logical device using the physical device, all our commands will go through the logical device

        auto deviceResult = vkb::DeviceBuilder(m_vkbPhysicalDevice).build();
        if (!deviceResult)
        {
            NES_ERROR(vulkan::kLogTag, "Failed to initialize Vulkan! Failed to build Logical Device!");
            return false;
        }

        m_vkbDevice = deviceResult.value();

        // Create the graphics and present Queues.

        m_graphicsQueue = m_vkbDevice.get_queue(vkb::QueueType::graphics).value();
        if (!m_graphicsQueue)
        {
            NES_ERROR(vulkan::kLogTag, "Failed to initialize Vulkan! Failed get Graphics Queue!");
            return false;
        }

        m_presentQueue = m_vkbDevice.get_queue(vkb::QueueType::present).value();
        if (!m_presentQueue)
        {
            NES_ERROR(vulkan::kLogTag, "Failed to initialize Vulkan! Failed get Present Queue!");
            return false;
        }

        vk::Device device(m_vkbDevice.device);

        // Now with basic device setup out of the way we need to finish creating the objects
        // that will allow us to issue rendering commands to the window that will be displayed

        // One command pool will house all the graphics commands we issue
        m_graphicsCommandPool = device.createCommandPool(
            vk::CommandPoolCreateInfo()
                .setQueueFamilyIndex(m_vkbDevice.get_queue_index(vkb::QueueType::graphics).value())
                .setFlags(vk::CommandPoolCreateFlagBits::eResetCommandBuffer)
        );

        if (!m_graphicsCommandPool)
        {
            NES_ERROR(vulkan::kLogTag, "Failed to initialize Vulkan! Failed to create Graphics Command Pool!");
            return false;
        }

        // A descriptor pool holds all the types of descriptor sets (shader uniforms) we will be allocating
        // to set shader values.
        m_descriptorPool = device.createDescriptorPool(
            vk::DescriptorPoolCreateInfo()
                .setFlags(vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet)
                .setMaxSets(options.m_maxDescriptorSets)		// [TODO]: This should be part of a Renderer Specification 
                .setPoolSizes(options.m_descriptorPoolSizes)	// [TODO]: This should be part of a Renderer Specification 
        );
        
        if (!m_descriptorPool)
        {
            NES_ERROR(vulkan::kLogTag, "Failed to initialize Vulkan! Failed to create Descriptor Pool!");
            return false;
        }

        // A pipeline cache is not required but can speed up the creation of duplicate pipelines
        m_pipelineCache = device.createPipelineCache(
            vk::PipelineCacheCreateInfo()
        );

        // Create the synchronization primitives used to organize our multi-buffered rendering
        for (auto& frame : m_frames)
        {
            frame.m_isImageAvailable = device.createSemaphore(vk::SemaphoreCreateInfo());
            frame.m_isRenderFinished = device.createSemaphore(vk::SemaphoreCreateInfo());
            frame.m_inUse = device.createFence(
                vk::FenceCreateInfo()
                    .setFlags(vk::FenceCreateFlagBits::eSignaled)
            );
        }
        m_currentFrameIndex = 0;

        // Normally we would query the physical device to find out a good depth/stencil format
        // that matches what we want, but 24-bits of depth and 8-bits of stencil information
        // is widely supported / fairly standardized so it should be supported on all gpus.
        if (options.m_enableDepthStencilBuffer)
            m_depthFormat = vk::Format::eD24UnormS8Uint;
        else
            m_depthFormat = vk::Format::eUndefined;

        // Build the swapchain to set up our output framebuffers and any related resources like render passes
        if (!RebuildSwapchain())
            return false;
            
        return true;
    }

	//----------------------------------------------------------------------------------------------------
	///		@brief : Shutdown the Vulkan Instance, cleaning up all resources. This is required to be called at
	///             the end of the Application's lifetime.
	//----------------------------------------------------------------------------------------------------
    void RendererContext::Shutdown()
    {
        if (const vk::Device device = m_vkbDevice.device)
        {
            device.waitIdle();
            m_graphicsPipelines.clear();

            if (m_displayRenderPass)
            {
                device.destroyRenderPass(m_displayRenderPass);
                m_displayRenderPass = vk::RenderPass();
            }
            
            std::vector<vk::CommandBuffer> commandBuffers;
            commandBuffers.reserve(m_framebuffers.size());
            std::vector<VkImageView> imageViews;
            imageViews.reserve(m_framebuffers.size());
            for (const auto& fbData : m_framebuffers)
            {
                if (fbData.m_imageView)
                    imageViews.push_back(fbData.m_imageView);
                
                if (fbData.m_framebuffer)
                    device.destroyFramebuffer(fbData.m_framebuffer);
                
                if (fbData.m_commandBuffer)
                    commandBuffers.push_back(fbData.m_commandBuffer);
            }

            m_framebuffers.clear();
            m_currentFramebufferIndex = 0;
            
            if (!commandBuffers.empty())
                device.freeCommandBuffers(m_graphicsCommandPool, commandBuffers);
            if (!imageViews.empty())
                m_vkbSwapchain.destroy_image_views(imageViews);

            if (m_depthStencilImage)
            {
                DestroyImageAndView(m_depthStencilImage, m_depthStencilView);
                m_depthStencilImage = vk::Image();
                m_depthStencilView = vk::ImageView();
            }

            for (auto& frame : m_frames)
            {
                if (frame.m_isImageAvailable)
                {
                    device.destroySemaphore(frame.m_isImageAvailable);
                    frame.m_isImageAvailable = vk::Semaphore();
                }

                if (frame.m_isRenderFinished)
                {
                    device.destroySemaphore(frame.m_isRenderFinished);
                    frame.m_isRenderFinished = vk::Semaphore();
                }

                if (frame.m_inUse)
                {
                    device.destroyFence(frame.m_inUse);
                    frame.m_inUse = vk::Fence();
                }
            }
            m_currentFrameIndex = 0;

            if (m_graphicsCommandPool)
            {
                device.destroyCommandPool(m_graphicsCommandPool);
                m_graphicsCommandPool = vk::CommandPool();
            }

            if (m_descriptorPool)
            {
                device.destroyDescriptorPool(m_descriptorPool);
                m_descriptorPool = vk::DescriptorPool();
            }

            if (m_pipelineCache)
            {
                device.destroyPipelineCache(m_pipelineCache);
                m_pipelineCache = vk::PipelineCache();
            }
        }

        vkb::destroy_swapchain(m_vkbSwapchain);
        vkb::destroy_device(m_vkbDevice);
        m_vkbPhysicalDevice = vkb::PhysicalDevice();

        if (const vk::Instance instance = m_vkbInstance.instance)
        {
            if (m_displaySurface)
            {
                instance.destroySurfaceKHR(m_displaySurface);
                m_displaySurface = vk::SurfaceKHR();
            }
        }

        vkb::destroy_instance(m_vkbInstance);
    }

    
    //----------------------------------------------------------------------------------------------------
    //		NOTES:
    //      I would like to abstract issuing Render Commands further. This should be thought of more as an internal
    //      function.
    //		
    ///		@brief : Starts the next frame. Must be bookended with a call to EndFrame(). Returns references to
    ///             both the command buffer and framebuffer of the current frame to be used to issue Render Commands.
    ///		@returns : False if we have to rebuild the Swapchain.
    //----------------------------------------------------------------------------------------------------
    bool RendererContext::BeginFrame(vk::CommandBuffer& commandBuffer, vk::Framebuffer& framebuffer)
    {
        if (!m_vkbSwapchain.swapchain)
        {
            RebuildSwapchain();
            return false;
        }

        auto& currentFrame = m_frames[m_currentFrameIndex];
        
        // Wait for our the frame to complete before we start changing it
        // The return value is cast to void because we don't care what it is
        (void)GetDevice().waitForFences(1, &currentFrame.m_inUse, true, UINT64_MAX);
        
        // Get next image to render into
        uint32_t imageIndex = 0;
        auto acquireResult = GetDevice().acquireNextImageKHR(m_vkbSwapchain.swapchain, UINT64_MAX,
            currentFrame.m_isImageAvailable, {}, &imageIndex);
        if (acquireResult == vk::Result::eErrorOutOfDateKHR)
        {
            RebuildSwapchain();
            return false;
        }
        m_currentFramebufferIndex = imageIndex;
        auto& targetFramebuffer = m_framebuffers[m_currentFramebufferIndex];

        // make sure our next framebuffer is not in use
        if (targetFramebuffer.m_inUse)
        {
            // The return value is cast to void because we don't care what it is
            (void)GetDevice().waitForFences(1, &targetFramebuffer.m_inUse, true, UINT64_MAX);
        }

        targetFramebuffer.m_inUse = currentFrame.m_inUse;
        targetFramebuffer.m_commandBuffer.reset();
        targetFramebuffer.m_commandBuffer.begin(vk::CommandBufferBeginInfo());

        commandBuffer = targetFramebuffer.m_commandBuffer;
        framebuffer = targetFramebuffer.m_framebuffer;
        return true;
    }

    //----------------------------------------------------------------------------------------------------
    //		NOTES:
    //		
    ///		@brief : Submits the current frame to the GPU. Must be preceded by a call to BeginFrame(). 
    //----------------------------------------------------------------------------------------------------
    void RendererContext::EndFrame()
    {
        auto& currentFrame = m_frames[m_currentFrameIndex];
        auto& targetFramebuffer = m_framebuffers[m_currentFramebufferIndex];

        targetFramebuffer.m_commandBuffer.end();
        vk::Semaphore waitSemaphores[] =
        {
            currentFrame.m_isImageAvailable,
        };
        vk::PipelineStageFlags waitFlags[] =
        {
            vk::PipelineStageFlagBits::eColorAttachmentOutput,
        };
        vk::Semaphore signalSemaphores[] =
        {
            currentFrame.m_isRenderFinished,
        };

        vk::SubmitInfo submitInfo;
        submitInfo.pWaitSemaphores = waitSemaphores;
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitDstStageMask = waitFlags;
        submitInfo.pSignalSemaphores = signalSemaphores;
        submitInfo.signalSemaphoreCount = 1;

        vk::CommandBuffer buffers[] = { targetFramebuffer.m_commandBuffer };
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = buffers;

        [[maybe_unused]] auto resetResult = GetDevice().resetFences(1, &currentFrame.m_inUse);
        [[maybe_unused]] auto submitResult = GetGraphicsQueue().submit(1, &submitInfo, currentFrame.m_inUse);
    	
        // Present
        //
        // The present command should wait until rendering is finished and this is ensured
        // by having it wait until the semaphores bundled with the submit are signaled

        vk::PresentInfoKHR presentInfo;
        presentInfo.pWaitSemaphores = signalSemaphores;
        presentInfo.waitSemaphoreCount = 1;

        vk::SwapchainKHR swapchains[] = { m_vkbSwapchain.swapchain };
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = swapchains;
        presentInfo.pImageIndices = &m_currentFramebufferIndex;

        auto presentResult = GetPresentQueue().presentKHR(presentInfo);
        if (presentResult == vk::Result::eErrorOutOfDateKHR)
        {
            RebuildSwapchain();
            return;
        }

        m_currentFrameIndex = (m_currentFrameIndex + 1) % kMaxPendingFrames;
    }

    //----------------------------------------------------------------------------------------------------
    //		NOTES:
    //		
    ///		@brief :  For times when you need to execute commands on the GPU that do not result in
    ///	              graphical output, like uploading data when creating a resource, this method
    ///	              accepts a function that will be provided a command buffer to write commands into
    ///	              which will be executed immediately after the function returns.
    ///	              This method will wait for the commands to finish executing before returning.
    ///		@param generateCommands : Function that takes in a command buffer to write commands into. 
    ///		@returns : Returns false if no command buffers are available.
    //----------------------------------------------------------------------------------------------------
    bool RendererContext::ExecuteCommands(std::function<void(vk::CommandBuffer&)>&& generateCommands)
    {
        // Create our command buffer to record copy commands
        vk::CommandBufferAllocateInfo cbInfo;
        cbInfo.commandBufferCount = 1;
        cbInfo.commandPool = m_graphicsCommandPool;
        auto commandBuffers = GetDevice().allocateCommandBuffers(cbInfo);
        if (commandBuffers.empty())
            return false;

        vk::CommandBufferBeginInfo beginInfo;
        beginInfo.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;
        commandBuffers[0].begin(beginInfo);
        generateCommands(commandBuffers[0]);
        commandBuffers[0].end();

        vk::Fence fence = GetDevice().createFence(vk::FenceCreateInfo());
        vk::SubmitInfo submitInfo;
        submitInfo.setCommandBuffers(commandBuffers);
        m_graphicsQueue.submit(submitInfo, fence);

        (void)GetDevice().waitForFences(1, &fence, true, UINT64_MAX);
        GetDevice().destroyFence(fence);
        GetDevice().freeCommandBuffers(m_graphicsCommandPool, commandBuffers);
        return true;
    }

    vk::CommandBuffer RendererContext::CreateSecondaryCommandBuffer()
    {
		vk::CommandBufferAllocateInfo cbInfo;
		cbInfo.commandBufferCount = 1;
		cbInfo.level = vk::CommandBufferLevel::eSecondary;
		cbInfo.commandPool = m_graphicsCommandPool;
		
		auto commandBuffers = GetDevice().allocateCommandBuffers(cbInfo);
		if (commandBuffers.empty())
			return nullptr;

		return commandBuffers[0];
    }

    //----------------------------------------------------------------------------------------------------
    //		NOTES:
    //		
    ///		@brief : Create a buffer given a full info struct, optionally copying initial data to it.
    ///	             The info struct must be configured properly if initial data is provided and the initial data must
    ///	             be the exact size of the buffer. This function will not perform any safety checks.
    ///	             See the other helpers for safer versions.
    //----------------------------------------------------------------------------------------------------
    vk::Buffer RendererContext::CreateBuffer(const vk::BufferCreateInfo& createInfo, const void* pInitialData)
    {
        vk::Buffer buffer = GetDevice().createBuffer(createInfo);
        if (!buffer)
            return {};

        vk::MemoryRequirements memoryReq = GetDevice().getBufferMemoryRequirements(buffer);
        vk::MemoryPropertyFlags memoryFlags{};
        if (!pInitialData)
            memoryFlags |= vk::MemoryPropertyFlagBits::eDeviceLocal;
        else
            memoryFlags |= vk::MemoryPropertyFlagBits::eHostVisible;

        vk::DeviceMemory bufferMemory = AllocateMemory(memoryReq, memoryFlags);
        if (!bufferMemory)
        {
            GetDevice().destroyBuffer(buffer);
            return {};
        }

        GetDevice().bindBufferMemory(buffer, bufferMemory, 0);

        if (pInitialData)
        {
            void* pMapped = GetDevice().mapMemory(bufferMemory, 0, memoryReq.size);
            if (pMapped)
            {
                memset(pMapped, 0, memoryReq.size);
                memcpy(pMapped, pInitialData, createInfo.size);
                GetDevice().flushMappedMemoryRanges(
                    vk::MappedMemoryRange()
                    .setMemory(bufferMemory)
                    .setOffset(0)
                    .setSize(VK_WHOLE_SIZE)
                );
                GetDevice().unmapMemory(bufferMemory);
            }
        }

        m_bufferMemoryMap[buffer] = bufferMemory;
        return buffer;
    }

    //----------------------------------------------------------------------------------------------------
    //		NOTES:
    //		
    ///		@brief : Create a buffer given its usage and size, optionally copying initial data to it.
    ///	            This method will create the buffer with TransferDst usage if initial data is supplied. 
    //----------------------------------------------------------------------------------------------------
    vk::Buffer RendererContext::CreateBuffer(const vk::BufferUsageFlags usage, const vk::DeviceSize size, const void* pInitialData)
    {
        vk::BufferCreateInfo createInfo{};
        createInfo.size = size;
        createInfo.usage = usage;
        createInfo.sharingMode = vk::SharingMode::eExclusive;

        // Ensure the buffer is being marked as a transfer destination so we can copy the initial data
        if (!pInitialData)
            createInfo.usage |= vk::BufferUsageFlagBits::eTransferDst;

        return CreateBuffer(createInfo, pInitialData);
    }

    //----------------------------------------------------------------------------------------------------
    //		NOTES:
    //		
    ///		@brief : Destroy a buffer created with a CreateBuffer method.
    ///	            This will lookup the associated memory object and free it. 
    //----------------------------------------------------------------------------------------------------
    void RendererContext::DestroyBuffer(vk::Buffer buffer)
    {
        GetDevice().destroyBuffer(buffer);

        auto bufferIter = m_bufferMemoryMap.find(buffer);
        if (bufferIter != m_bufferMemoryMap.end())
        {
            GetDevice().freeMemory(bufferIter->second);
            m_bufferMemoryMap.erase(bufferIter);
        }
    }

    //----------------------------------------------------------------------------------------------------
    //		NOTES:
    //		
    ///		@brief : Look up the memory objet associated with the buffer.
    //----------------------------------------------------------------------------------------------------
    vk::DeviceMemory RendererContext::GetBufferMemoryHandle(vk::Buffer buffer) const
    {
        auto bufferIter = m_bufferMemoryMap.find(buffer);
        if (bufferIter != m_bufferMemoryMap.end())
            return bufferIter->second;
        return nullptr;
    }

    //----------------------------------------------------------------------------------------------------
    //		NOTES:
    //		
    ///		@brief : Create an image given its info structure and optionally some initial data to upload to it.
    ///	            This method does not provide any checks to ensure the info struct was set up correctly.
    ///	            Use one of the other helper methods for better safety. 
    //----------------------------------------------------------------------------------------------------
    vk::Image RendererContext::CreateImage(const vk::ImageCreateInfo& imageInfo, const void* pInitialData,
                                         vk::DeviceSize initialDataSize)
    {
        const vk::Image image = GetDevice().createImage(imageInfo);
        if (!image)
            return nullptr;

        auto memoryReq = GetDevice().getImageMemoryRequirements(image);
        vk::DeviceMemory imageMemory = AllocateMemory(memoryReq, vk::MemoryPropertyFlagBits::eDeviceLocal);
        if (!imageMemory)
        {
            GetDevice().destroyImage(image);
            return nullptr;
        }

        GetDevice().bindImageMemory(image, imageMemory, 0);

        if (pInitialData && !UploadImageData(image, imageInfo.extent, pInitialData, initialDataSize, imageInfo.arrayLayers))
        {
            GetDevice().destroyImage(image);
            GetDevice().freeMemory(imageMemory);
            return nullptr;
        }

        m_imageMemoryMap[image] = imageMemory;
        return image;
    }

    //----------------------------------------------------------------------------------------------------
    //		NOTES:
    //		
    ///		@brief : Create an image given its info structure and create a view with a configuration derived
    ///             from the image.
    //----------------------------------------------------------------------------------------------------
    std::tuple<vk::Image, vk::ImageView> RendererContext::CreateImageAndView(const vk::ImageCreateInfo& imageInfo,
                                                                           const void* pInitialData, vk::DeviceSize initialDataSize)
    {
        vk::Image image = CreateImage(imageInfo, pInitialData, initialDataSize);
        if (!image)
            return {};

        vk::ImageViewCreateInfo viewInfo{};
        viewInfo.image = image;
        viewInfo.format = imageInfo.format;
        viewInfo.subresourceRange.layerCount = imageInfo.arrayLayers;
        viewInfo.subresourceRange.levelCount = imageInfo.mipLevels;
        viewInfo.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;

        // Mild hack to support detect depth/stencil images
        if (imageInfo.usage & vk::ImageUsageFlagBits::eDepthStencilAttachment)
            viewInfo.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eDepth | vk::ImageAspectFlagBits::eStencil;

        // Mild hack to treat an array of 6 2D layers as a cubemap
        if (imageInfo.imageType == vk::ImageType::e2D && imageInfo.arrayLayers == 6)
            viewInfo.viewType = vk::ImageViewType::eCube;
        // Detect standard image types
        else if (imageInfo.imageType == vk::ImageType::e1D)
            viewInfo.viewType = (imageInfo.arrayLayers > 1) ? vk::ImageViewType::e1DArray : vk::ImageViewType::e1D;
        else if (imageInfo.imageType == vk::ImageType::e2D)
            viewInfo.viewType = (imageInfo.arrayLayers > 1) ? vk::ImageViewType::e2DArray : vk::ImageViewType::e2D;
        else if (imageInfo.imageType == vk::ImageType::e3D)
            viewInfo.viewType = vk::ImageViewType::e3D;

        vk::ImageView imageView = GetDevice().createImageView(viewInfo);
        if (!imageView)
        {
            DestroyImage(image);
            return {};
        }

        return { image, imageView };
    }

    //----------------------------------------------------------------------------------------------------
    //		NOTES:
    //		
    ///		@brief : Create an Image and ImageView for use as a 2D sampled texture.
    //----------------------------------------------------------------------------------------------------
    std::tuple<vk::Image, vk::ImageView> RendererContext::CreateTexture2DImageAndView(vk::Extent2D extents,
                                                                                    vk::Format format, const void* pInitialData, vk::DeviceSize initialDataSize)
    {
        vk::ImageUsageFlags usage = vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled;
        if (pInitialData)
            usage |= vk::ImageUsageFlagBits::eTransferDst;
        
        return CreateImageAndView(
            vk::ImageCreateInfo()
                .setImageType(vk::ImageType::e2D)
                .setExtent(vk::Extent3D(extents, 1))
                .setMipLevels(1)
                .setArrayLayers(1)
                .setFormat(format)
                .setUsage(usage),
            pInitialData,
            initialDataSize
        );
    }

    //----------------------------------------------------------------------------------------------------
    //		NOTES:
    //		
    ///		@brief : Create a 6-layered image to be used as a cubemap. The initial data has each image for
    ///             each face, one after the other.
    //----------------------------------------------------------------------------------------------------
    std::tuple<vk::Image, vk::ImageView> RendererContext::CreateCubemapImageAndView(vk::Extent2D extents,
                                                                                  vk::Format format, const void* pInitialData, vk::DeviceSize initialDataSize)
    {
        vk::ImageUsageFlags usage = vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled;
        if (pInitialData)
            usage |= vk::ImageUsageFlagBits::eTransferDst;
        
        return CreateImageAndView(
            vk::ImageCreateInfo()
                .setImageType(vk::ImageType::e2D)
                .setExtent(vk::Extent3D(extents, 1))
                .setMipLevels(1)
                .setArrayLayers(6)
                .setFormat(format)
                .setUsage(usage)
                .setFlags(vk::ImageCreateFlagBits::eCubeCompatible),
            pInitialData,
            initialDataSize
        );
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Destroy an image, also freeing its associated memory object 
    //----------------------------------------------------------------------------------------------------
    void RendererContext::DestroyImage(vk::Image image)
    {
        GetDevice().destroyImage(image);

        auto imageIter = m_imageMemoryMap.find(image);
        if (imageIter != m_imageMemoryMap.end())
        {
            GetDevice().freeMemory(imageIter->second);
            m_imageMemoryMap.erase(imageIter);
        }
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Helper to also destroy an image view at the same time
    //----------------------------------------------------------------------------------------------------
    void RendererContext::DestroyImageAndView(vk::Image image, vk::ImageView view)
    {
        GetDevice().destroyImageView(view);
        DestroyImage(image);
    }

    //----------------------------------------------------------------------------------------------------
    //		NOTES:
    //		
    ///		@brief : Lookup the memory object associated with the image 
    //----------------------------------------------------------------------------------------------------
    vk::DeviceMemory RendererContext::GetImageMemoryHandle(vk::Image image) const
    {
        auto imageIter = m_imageMemoryMap.find(image);
        if (imageIter != m_imageMemoryMap.end())
            return imageIter->second;
        return nullptr;
    }

    //----------------------------------------------------------------------------------------------------
    //		NOTES:
    //		
    ///		@brief : Helper to upload data to an image using a staging buffer
    //----------------------------------------------------------------------------------------------------
    bool RendererContext::UploadImageData(vk::Image image, vk::Extent3D extents, const void* pData,
                                        vk::DeviceSize dataSize, uint32_t layerCount)
    {
        vk::Buffer stagingBuffer = GetDevice().createBuffer(
	    vk::BufferCreateInfo()
		    .setSize(dataSize)
		    .setUsage(vk::BufferUsageFlagBits::eTransferSrc)
		    .setSharingMode(vk::SharingMode::eExclusive)
        );
        if (!stagingBuffer)
	        return false;

        auto memoryReq = GetDevice().getBufferMemoryRequirements(stagingBuffer);
        vk::DeviceMemory stagingBufferMemory = AllocateMemory(memoryReq, vk::MemoryPropertyFlagBits::eHostVisible);
        if (!stagingBufferMemory)
        {
	        GetDevice().destroyBuffer(stagingBuffer);
	        return false;
        }

        GetDevice().bindBufferMemory(stagingBuffer, stagingBufferMemory, 0);
        void* pMapped = GetDevice().mapMemory(stagingBufferMemory, 0, memoryReq.size);
        memset(pMapped, 0, memoryReq.size);
        memcpy(pMapped, pData, dataSize);
        GetDevice().flushMappedMemoryRanges(
	        vk::MappedMemoryRange()
		        .setMemory(stagingBufferMemory)
		        .setOffset(0)
		        .setSize(VK_WHOLE_SIZE)
        );
        GetDevice().unmapMemory(stagingBufferMemory);

        ExecuteCommands([&image, &extents, &stagingBuffer, layerCount](vk::CommandBuffer& commandBuffer)
        {
	        vk::ImageMemoryBarrier barrier{};
	        barrier.image = image;
	        barrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
	        barrier.subresourceRange.baseMipLevel = 0;
	        barrier.subresourceRange.levelCount = 1;
	        barrier.subresourceRange.baseArrayLayer = 0;
	        barrier.subresourceRange.layerCount = layerCount;

	        // Ensure the image is transformed on the GPU into a format that is ready to be transferred into
	        barrier.oldLayout = vk::ImageLayout::eUndefined;
	        barrier.newLayout = vk::ImageLayout::eTransferDstOptimal;
	        barrier.srcAccessMask = {};
	        barrier.dstAccessMask = vk::AccessFlagBits::eTransferWrite;
	        commandBuffer.pipelineBarrier(
		        vk::PipelineStageFlagBits::eTopOfPipe,
		        vk::PipelineStageFlagBits::eTransfer,
		        {}, {}, {}, { barrier });

	        // Now perform the copy from our staging buffer
	        vk::BufferImageCopy copyInfo{};
	        copyInfo.imageSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
	        copyInfo.imageSubresource.mipLevel = 0;
	        copyInfo.imageSubresource.baseArrayLayer = 0;
	        copyInfo.imageSubresource.layerCount = layerCount;
	        copyInfo.imageExtent = extents;
	        commandBuffer.copyBufferToImage(stagingBuffer, image, barrier.newLayout, 1, &copyInfo);

	        // Transform the image back into a layout ready for use by the shader
	        barrier.oldLayout = vk::ImageLayout::eTransferDstOptimal;
	        barrier.newLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
	        barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
	        barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;
	        commandBuffer.pipelineBarrier(
		        vk::PipelineStageFlagBits::eTransfer,
		        vk::PipelineStageFlagBits::eFragmentShader,
		        {}, {}, {}, { barrier });
        });

        GetDevice().destroyBuffer(stagingBuffer);
        GetDevice().freeMemory(stagingBufferMemory);
        return true;
    }

    //----------------------------------------------------------------------------------------------------
    //		NOTES:
    //		
    ///		@brief : Create a GraphicsPipeline object. The RenderPass will automatically be set to the display's RenderPass
    ///	            when it is available if the provided RenderPass is null or matches the current display's RenderPass.
    ///	            See GraphicsPipelineConfig for more details.
    //----------------------------------------------------------------------------------------------------
    std::shared_ptr<RendererContext::GraphicsPipeline> RendererContext::CreatePipeline(const GraphicsPipelineConfig& config)
    {
        // if the render pass in the config is null, it is for use with the display render pass
        // so do not create if the RP is not available
        vk::Pipeline pipeline;
        vk::PipelineLayout layout;

        if (!config.m_renderPass)
        {
            if (auto pass = GetDisplayRenderPass())
            {
                GraphicsPipelineConfig overrideConfig = config;
                overrideConfig.m_renderPass = pass;
                std::tie(pipeline, layout) = CreatePipelineAndLayout(overrideConfig);
            }
        }
        else
        {
            std::tie(pipeline, layout) = CreatePipelineAndLayout(config);
        }

        auto wrapper = std::make_shared<GraphicsPipeline>();
        std::get<0>(*wrapper) = pipeline;
        std::get<1>(*wrapper) = layout;
        std::get<2>(*wrapper) = config;
        m_graphicsPipelines.push_back(wrapper);
        return wrapper;
    }

    //----------------------------------------------------------------------------------------------------
    //		NOTES:
    //		
    ///		@brief : This is a lightweight helper to create the raw Vulkan handles instead of a managed
    ///             GraphicsPipeline object. 
    //----------------------------------------------------------------------------------------------------
    std::tuple<vk::Pipeline, vk::PipelineLayout> RendererContext::CreatePipelineAndLayout(
        const GraphicsPipelineConfig& config)
    {
        auto layoutCreateInfo = vk::PipelineLayoutCreateInfo()
	        .setPushConstantRanges(config.m_shaderPushConstants)
	        .setSetLayouts(config.m_shaderUniforms);

        vk::PipelineLayout layout = GetDevice().createPipelineLayout(layoutCreateInfo);

        auto inputAssemblyState = vk::PipelineInputAssemblyStateCreateInfo()
	        .setTopology(config.m_topology);

        auto vertexInputState = vk::PipelineVertexInputStateCreateInfo()
	        .setVertexBindingDescriptions(config.m_vertexBindings)
	        .setVertexAttributeDescriptions(config.m_vertexAttributes);

        auto rasterState = vk::PipelineRasterizationStateCreateInfo()
	        .setCullMode(config.m_cullMode)
	        .setFrontFace(config.m_frontFace)
	        .setPolygonMode(config.m_polygonMode)
	        .setLineWidth(1);

        auto depthStencilState = vk::PipelineDepthStencilStateCreateInfo()
	        .setDepthTestEnable(config.m_depthTestEnable)
	        .setDepthWriteEnable(config.m_depthWriteEnable)
	        .setDepthCompareOp(config.m_depthCompareOp);

        auto colorBlendState = vk::PipelineColorBlendStateCreateInfo()
	        .setLogicOp(vk::LogicOp::eCopy)
	        .setAttachments(config.m_colorBlendStates);

        auto dynamicState = vk::PipelineDynamicStateCreateInfo()
	        .setDynamicStates(config.m_dynamicStates);

    	auto multisampleState = vk::PipelineMultisampleStateCreateInfo();

        auto viewportState = vk::PipelineViewportStateCreateInfo();

        // If no viewports defined, use the display's viewport
        const std::array<const vk::Viewport, 1> defaultViewports = { m_displayViewport };
        viewportState.setViewports(config.m_viewports.empty() ? vk::ArrayProxyNoTemporaries<const vk::Viewport>(defaultViewports) : config.m_viewports);

        // If no scissors defined, match viewport
        const std::array<const vk::Rect2D, 1> defaultScissors =
        {
	        vk::Rect2D{
		        vk::Offset2D{ static_cast<int32_t>(viewportState.pViewports->x), static_cast<int32_t>(viewportState.pViewports->y) },
		        vk::Extent2D{ static_cast<uint32_t>(viewportState.pViewports->width), static_cast<uint32_t>(viewportState.pViewports->height) }
	        }
        };
        viewportState.setScissors(config.m_scissors.empty() ? vk::ArrayProxyNoTemporaries<const vk::Rect2D>(defaultScissors) : config.m_scissors);


        auto pipelineCreateInfo = vk::GraphicsPipelineCreateInfo()
	        .setLayout(layout)
	        .setPInputAssemblyState(&inputAssemblyState)
	        .setPVertexInputState(&vertexInputState)
	        .setStages(config.m_shaderStages)
	        .setPRasterizationState(&rasterState)
	        .setPDepthStencilState(&depthStencilState)
	        .setPColorBlendState(&colorBlendState)
	        .setPViewportState(&viewportState)
    		.setPMultisampleState(&multisampleState)
	        .setPDynamicState(&dynamicState)
	        .setRenderPass(config.m_renderPass);

        const auto& [result, pipeline] = GetDevice().createGraphicsPipeline(GetPipelineCache(), pipelineCreateInfo);
        if (result != vk::Result::eSuccess)
        {
	        GetDevice().destroyPipelineLayout(layout);
	        return {};
        }

        return { pipeline, layout };
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Destroy the managed GraphicsPipeline object and null out the pointer.
    //----------------------------------------------------------------------------------------------------
    void RendererContext::DestroyPipeline(std::shared_ptr<GraphicsPipeline>& pipeline)
    {
        if (!pipeline)
            return;

        if (auto& pipelineRef = std::get<0>(*pipeline))
        {
            GetDevice().destroyPipeline(pipelineRef);
            pipelineRef = vk::Pipeline();
        }
        if (auto& layout = std::get<1>(*pipeline))
        {
            GetDevice().destroyPipelineLayout(layout);
            layout = vk::PipelineLayout();
        }

        pipeline.reset();
    }

    //----------------------------------------------------------------------------------------------------
    //		NOTES:
    //		
    ///		@brief : Helper to create a DescriptorSet pairing for use with a constant buffer.
    //----------------------------------------------------------------------------------------------------
    RendererContext::ShaderUniform RendererContext::CreateUniformForBuffer(int binding, vk::Buffer buffer,
                                                                       vk::DeviceSize size, vk::DeviceSize offset, vk::ShaderStageFlags stages)
    {
        std::array layoutBindings =
        {
            vk::DescriptorSetLayoutBinding()
                .setBinding(binding)
                .setStageFlags(stages)
                .setDescriptorType(vk::DescriptorType::eUniformBuffer)
                .setDescriptorCount(1)
        };

        vk::DescriptorSetLayout layout = GetDevice().createDescriptorSetLayout(
            vk::DescriptorSetLayoutCreateInfo()
                .setBindings(layoutBindings)
        );
        if (!layout)
            return {};

        auto sets = GetDevice().allocateDescriptorSets(
            vk::DescriptorSetAllocateInfo()
                .setDescriptorPool(m_descriptorPool)
                .setSetLayouts(layout)
                .setDescriptorSetCount(1)
        );
        if (sets.empty())
        {
            GetDevice().destroyDescriptorSetLayout(layout);
            return {};
        }

        auto bufferInfo = vk::DescriptorBufferInfo()
            .setBuffer(buffer)
            .setOffset(offset)
            .setRange(size);

        auto initialUpdate = vk::WriteDescriptorSet()
            .setDescriptorCount(1)
            .setDescriptorType(vk::DescriptorType::eUniformBuffer)
            .setDstBinding(binding)
            .setDstSet(sets[0])
            .setBufferInfo(bufferInfo);

        GetDevice().updateDescriptorSets(initialUpdate, {});

        return static_cast<ShaderUniform>(std::make_tuple(sets[0], layout));
    }

    //----------------------------------------------------------------------------------------------------
    //		NOTES:
    //		
    ///		@brief : Helper to create a DescriptorSet pairing for use with an image for sampling
    //----------------------------------------------------------------------------------------------------
    RendererContext::ShaderUniform RendererContext::CreateUniformForImage(int binding, vk::ImageView view,
                                                                      vk::Sampler sampler, vk::ImageLayout imageLayout, vk::ShaderStageFlags stages)
    {
        std::array layoutBindings =
        {
            vk::DescriptorSetLayoutBinding()
                .setBinding(binding)
                .setStageFlags(stages)
                .setDescriptorType(vk::DescriptorType::eCombinedImageSampler)
                .setDescriptorCount(1)
        };

        vk::DescriptorSetLayout layout = GetDevice().createDescriptorSetLayout(
            vk::DescriptorSetLayoutCreateInfo()
                .setBindings(layoutBindings)
        );
        if (!layout)
            return {};

        auto sets = GetDevice().allocateDescriptorSets(
            vk::DescriptorSetAllocateInfo()
                .setDescriptorPool(m_descriptorPool)
                .setSetLayouts(layout)
                .setDescriptorSetCount(1)
        );
        if (sets.empty())
        {
            GetDevice().destroyDescriptorSetLayout(layout);
            return {};
        }

        auto imageInfo = vk::DescriptorImageInfo()
            .setImageView(view)
            .setSampler(sampler)
            .setImageLayout(imageLayout);

        auto initialUpdate = vk::WriteDescriptorSet()
            .setDescriptorCount(1)
            .setDescriptorType(vk::DescriptorType::eCombinedImageSampler)
            .setDstBinding(binding)
            .setDstSet(sets[0])
            .setImageInfo(imageInfo);

        GetDevice().updateDescriptorSets(initialUpdate, {});

        return static_cast<ShaderUniform>(std::make_tuple(sets[0], layout));
    }

    //----------------------------------------------------------------------------------------------------
    //		NOTES:
    //		
    ///		@brief : Destroy the shader uniform resources
    //----------------------------------------------------------------------------------------------------
    void RendererContext::DestroyUniform(ShaderUniform uniform)
    {
        if (auto set = uniform.GetSet())
            GetDevice().freeDescriptorSets(m_descriptorPool, { set });
        if (auto layout = uniform.GetLayout())
            GetDevice().destroyDescriptorSetLayout(layout);
    }

    //----------------------------------------------------------------------------------------------------
    //		NOTES:
    //		
    ///		@brief : Create a RenderPass that has a single color output of the given format.
    ///             Layout defaults to being presentable, but you may want to use
    ///	            eShaderReadOnlyOptimal if drawing to a texture that will later be used in a shader.
    //----------------------------------------------------------------------------------------------------
    vk::RenderPass RendererContext::CreateColorOnlyRenderPass(vk::Format colorFormat, vk::ImageLayout colorFinalLayout)
    {
        // An attachment describes an image that is written into by our rendering operations
        //	This is typically represented as an output from the fragment shader stage
        //	Like bindings, attachments are numerically referenced in the shader and
        //	if unspecified, attachment 0 is assumed.
        //	The layout here describes the data layout of the image that should be expected when
        //	all rendering is finished.
        auto colorAttachment = vk::AttachmentReference()
	        .setAttachment(0)
	        .setLayout(vk::ImageLayout::eColorAttachmentOptimal);

        // An attachment description describes more information about how color (or similar) data
        //	should be interpreted by the output and blending stage, so this is where the expected
        //	color format is defined as well as what to do when reading/writing (load/store) to the
        //	attachment. Final layout here describes the format the image should be transformed into
        //	when all output and blending operations are complete. For example, we want an attachment
        //	that is going to be displayed to the screen (the backbuffer/framebuffer) in ePresentSrcKHR.
        auto colorAttachmentDescription = vk::AttachmentDescription()
	        .setFormat(colorFormat)
	        .setFinalLayout(colorFinalLayout)
	        .setSamples(vk::SampleCountFlagBits::e1)
	        .setLoadOp(vk::AttachmentLoadOp::eClear)
	        .setStoreOp(vk::AttachmentStoreOp::eStore)
	        .setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
	        .setStencilStoreOp(vk::AttachmentStoreOp::eDontCare);

        // A subpass defines what attachments are used when a multi-pass rendering operation is performed.
        //	But for our purposes we'll only have the required minimum of one subpass.
        auto subpassDesc = vk::SubpassDescription()
	        .setPipelineBindPoint(vk::PipelineBindPoint::eGraphics)
	        .setColorAttachments(colorAttachment);

        // Each subpass should declare its dependencies for synchronization purposes.
        // The way this is done is by declaring what data access is performed between the particular stages
        //	that the subpass operates upon.
        //	Again, since we just have one subpass we use the special value of VK_SUBPASS_EXTERNAL.
        //	We're only operating on the attachments during the color output stages and need write access to
        //	write new color information but also read access to perform color blending.
        auto subpassDep = vk::SubpassDependency()
	        .setSrcSubpass(VK_SUBPASS_EXTERNAL)
	        .setSrcStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
	        .setDstStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
	        .setDstAccessMask(vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite);

        return GetDevice().createRenderPass(vk::RenderPassCreateInfo()
	        .setAttachments(colorAttachmentDescription)
	        .setSubpasses(subpassDesc)
	        .setDependencies(subpassDep)
        );
    }

    //----------------------------------------------------------------------------------------------------
    //		NOTES:
    //		
    ///		@brief : Creates both a Color and DepthStencil capable RenderPass.
    //----------------------------------------------------------------------------------------------------
    vk::RenderPass RendererContext::CreateColorAndDepthRenderPass(vk::Format colorFormat, vk::Format depthFormat,
                                                                vk::ImageLayout colorFinalLayout)
    {
        // Set up a color attachment, see CreateColorRenderPass for more details
        auto colorAttachment = vk::AttachmentReference()
	        .setAttachment(0)
	        .setLayout(vk::ImageLayout::eColorAttachmentOptimal);
        auto colorAttachmentDescription = vk::AttachmentDescription()
	        .setFormat(colorFormat)
	        .setFinalLayout(colorFinalLayout)
	        .setSamples(vk::SampleCountFlagBits::e1)
	        .setLoadOp(vk::AttachmentLoadOp::eClear)
	        .setStoreOp(vk::AttachmentStoreOp::eStore)
	        .setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
	        .setStencilStoreOp(vk::AttachmentStoreOp::eDontCare);

        // For depth, we set up another attachment but in the next slot
        // The difference here is that we will not be presenting this attachment to the screen
        //	so both the initial layout and final layout are the same.
        auto depthStencilAttachment = vk::AttachmentReference()
	        .setAttachment(1)
	        .setLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal);
        auto depthStencilAttachmentDescription = vk::AttachmentDescription()
	        .setFormat(depthFormat)
	        .setFinalLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal)
	        .setSamples(vk::SampleCountFlagBits::e1)
	        .setLoadOp(vk::AttachmentLoadOp::eClear)
	        .setStoreOp(vk::AttachmentStoreOp::eStore)
	        .setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
	        .setStencilStoreOp(vk::AttachmentStoreOp::eDontCare);

        // Subpass information is very similar to that of a typical color-only pass but we
        //	must also call out that the depth/stencil buffer will be accessed.
        auto subpassDesc = vk::SubpassDescription()
	        .setPipelineBindPoint(vk::PipelineBindPoint::eGraphics)
	        .setColorAttachments(colorAttachment)
	        .setPDepthStencilAttachment(&depthStencilAttachment);
        // The depth/stencil buffer is used as part of fragment testing, so those stages need to be included
        auto subpassDep = vk::SubpassDependency()
	        .setSrcSubpass(VK_SUBPASS_EXTERNAL)
	        .setSrcStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput | vk::PipelineStageFlagBits::eEarlyFragmentTests | vk::PipelineStageFlagBits::eLateFragmentTests)
	        .setDstStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput | vk::PipelineStageFlagBits::eEarlyFragmentTests | vk::PipelineStageFlagBits::eLateFragmentTests)
	        .setDstAccessMask(vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite | vk::AccessFlagBits::eDepthStencilAttachmentWrite | vk::AccessFlagBits::eDepthStencilAttachmentRead);

        std::array attachmentDescriptions = { colorAttachmentDescription, depthStencilAttachmentDescription };

        return GetDevice().createRenderPass(vk::RenderPassCreateInfo()
	        .setAttachments(attachmentDescriptions)
	        .setSubpasses(subpassDesc)
	        .setDependencies(subpassDep)
        );
    }

    //----------------------------------------------------------------------------------------------------
    //		NOTES:
    //		
    ///		@brief : Creates a collection of resources to use as a render target, given the size and formats
    ///             of the color and/or depth values. At least one format needs to be specified as images
    ///             will not be created for eUndefined.
    //----------------------------------------------------------------------------------------------------
    RendererContext::RenderTarget RendererContext::CreateRenderTarget(vk::Extent2D extents, vk::Format colorFormat,
                                                                  vk::Format depthFormat)
    {
        if (colorFormat == vk::Format::eUndefined && depthFormat == vk::Format::eUndefined)
	        return {};

        RenderTarget target;
        std::vector<vk::AttachmentDescription> attachments;
        std::array<vk::AttachmentReference, 1> colorReferences;
        std::optional<vk::AttachmentReference> depthReference;

        // Subpass dependencies call out how the attachments will be accessed between the different
        // stages of the pipeline being executed. For a render target that will also be used in a shader,
        // we call out two modes of access:
        std::array dependencies =
        {
	        // First, starting from before the subpass is executed (VK_SUBPASS_EXTERNAL)
	        // we will need to call out what the old (src) stage access was and what the
	        // new (dst) access will be when the subpass is active. What is accessed and how is based on
	        // what attachments are created, so these will be defined later
	        vk::SubpassDependency()
		        .setSrcSubpass(VK_SUBPASS_EXTERNAL)
		        .setDstSubpass(0)
		        .setSrcStageMask(vk::PipelineStageFlagBits::eNone)
		        .setDstStageMask(vk::PipelineStageFlagBits::eNone)
		        .setSrcAccessMask(vk::AccessFlagBits::eNone)
		        .setDstAccessMask(vk::AccessFlagBits::eNone),
	        // Once the subpass is done, we call out any access changes that should occur after completion
	        // so the access is set up properly for when the attachments are later used by a shader for input
	        vk::SubpassDependency()
		        .setSrcSubpass(0)
		        .setDstSubpass(VK_SUBPASS_EXTERNAL)
		        .setSrcStageMask(vk::PipelineStageFlagBits::eNone)
		        .setDstStageMask(vk::PipelineStageFlagBits::eNone)
		        .setSrcAccessMask(vk::AccessFlagBits::eNone)
		        .setDstAccessMask(vk::AccessFlagBits::eNone),
        };

        if (colorFormat != vk::Format::eUndefined)
        {
	        auto [image, view] = CreateImageAndView(
		        vk::ImageCreateInfo()
			        .setFormat(colorFormat)
			        .setExtent(vk::Extent3D(extents, 1))
			        .setImageType(vk::ImageType::e2D)
			        .setInitialLayout(vk::ImageLayout::eUndefined)
			        .setUsage(vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eColorAttachment)
			        .setSamples(vk::SampleCountFlagBits::e1)
			        .setSharingMode(vk::SharingMode::eExclusive)
			        .setTiling(vk::ImageTiling::eLinear)
			        .setMipLevels(1)
			        .setArrayLayers(1)
			);
            
	        if (!image)
	        {
		        DestroyRenderTarget(target);
		        return {};
	        }

	        target.m_images.push_back(image);
	        target.m_views.push_back(view);

	        colorReferences[0]
		        .setAttachment(static_cast<uint32_t>(attachments.size()))
		        .setLayout(vk::ImageLayout::eColorAttachmentOptimal);

	        attachments.push_back(vk::AttachmentDescription()
		        .setFormat(colorFormat)
		        .setSamples(vk::SampleCountFlagBits::e1)
		        .setLoadOp(vk::AttachmentLoadOp::eClear)
		        .setStoreOp(vk::AttachmentStoreOp::eStore)
		        .setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
		        .setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
		        .setInitialLayout(vk::ImageLayout::eUndefined)
		        .setFinalLayout(vk::ImageLayout::eShaderReadOnlyOptimal)
	        );

	        // We're dealing with colors, so once we're past the fragment shader, we'll be setting up
	        // the attachments to be used as a color output, which needs read/write access.
	        dependencies[0].srcStageMask |= vk::PipelineStageFlagBits::eFragmentShader;
	        dependencies[0].dstStageMask |= vk::PipelineStageFlagBits::eColorAttachmentOutput;
	        dependencies[0].dstAccessMask |= vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite;

	        // Once we're done with the subpass, we're done with color output, so we change the access
	        // back to what's appropriate for when the attachments get used as shader inputs, so
	        // that means the fragment shader will need read access.
	        dependencies[1].srcStageMask |= vk::PipelineStageFlagBits::eColorAttachmentOutput;
	        dependencies[1].dstStageMask |= vk::PipelineStageFlagBits::eFragmentShader;
	        dependencies[1].srcAccessMask |= vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite;
	        dependencies[1].dstAccessMask |= vk::AccessFlagBits::eShaderRead;
        }

        if (depthFormat != vk::Format::eUndefined)
        {
	        auto [image, view] = CreateImageAndView(
		        vk::ImageCreateInfo()
			        .setFormat(depthFormat)
			        .setExtent(vk::Extent3D(extents, 1))
			        .setImageType(vk::ImageType::e2D)
			        .setInitialLayout(vk::ImageLayout::eUndefined)
			        .setUsage(vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eColorAttachment)
			        .setSamples(vk::SampleCountFlagBits::e1)
			        .setSharingMode(vk::SharingMode::eExclusive)
			        .setTiling(vk::ImageTiling::eLinear)
			        .setMipLevels(1)
			        .setArrayLayers(1)
	        );
	        if (!image)
	        {
		        DestroyRenderTarget(target);
		        return {};
	        }

	        target.m_images.push_back(image);
	        target.m_views.push_back(view);

	        depthReference = vk::AttachmentReference()
		        .setAttachment(static_cast<uint32_t>(attachments.size()))
		        .setLayout(vk::ImageLayout::eColorAttachmentOptimal);

	        attachments.push_back(vk::AttachmentDescription()
		        .setFormat(depthFormat)
		        .setSamples(vk::SampleCountFlagBits::e1)
		        .setLoadOp(vk::AttachmentLoadOp::eClear)
		        .setStoreOp(vk::AttachmentStoreOp::eStore)
		        .setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
		        .setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
		        .setInitialLayout(vk::ImageLayout::eUndefined)
		        .setFinalLayout(vk::ImageLayout::eShaderReadOnlyOptimal)
	        );

	        // We're dealing with depth values, so unlike color we need to mark using the attachments
	        // for depth read/write when we do fragment testing.
	        dependencies[0].srcStageMask |= vk::PipelineStageFlagBits::eFragmentShader;
	        dependencies[0].dstStageMask |= vk::PipelineStageFlagBits::eEarlyFragmentTests | vk::PipelineStageFlagBits::eLateFragmentTests;
	        dependencies[0].dstAccessMask |= vk::AccessFlagBits::eDepthStencilAttachmentRead | vk::AccessFlagBits::eDepthStencilAttachmentWrite;

	        // Once we're done with the subpass we'll want to read the values in the shader
	        // so mark that the fragment shader should be able to read it.
	        dependencies[1].srcStageMask |= vk::PipelineStageFlagBits::eEarlyFragmentTests | vk::PipelineStageFlagBits::eLateFragmentTests;
	        dependencies[1].dstStageMask |= vk::PipelineStageFlagBits::eFragmentShader;
	        dependencies[1].srcAccessMask |= vk::AccessFlagBits::eDepthStencilAttachmentRead | vk::AccessFlagBits::eDepthStencilAttachmentWrite;
	        dependencies[1].dstAccessMask |= vk::AccessFlagBits::eShaderRead;
        }

        auto subpass = vk::SubpassDescription()
	        .setPipelineBindPoint(vk::PipelineBindPoint::eGraphics)
	        .setColorAttachments(colorReferences)
	        .setPDepthStencilAttachment(depthReference.has_value() ? &depthReference.value() : nullptr);

        target.m_renderPass = GetDevice().createRenderPass(
	        vk::RenderPassCreateInfo()
		        .setAttachments(attachments)
		        .setDependencies(dependencies)
		        .setSubpasses(subpass)
        );
        if (!target.m_renderPass)
        {
	        DestroyRenderTarget(target);
	        return {};
        }

        target.m_framebuffer = GetDevice().createFramebuffer(
	        vk::FramebufferCreateInfo()
		        .setRenderPass(target.m_renderPass)
		        .setAttachments(target.m_views)
		        .setWidth(extents.width)
		        .setHeight(extents.height)
		        .setLayers(1)
        );
        if (!target.m_framebuffer)
        {
	        DestroyRenderTarget(target);
	        return {};
        }

        return target;
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Destroy all the created resources for the render target 
    //----------------------------------------------------------------------------------------------------
    void RendererContext::DestroyRenderTarget(RenderTarget target)
    {
        if (target.m_framebuffer)
            GetDevice().destroyFramebuffer(target.m_framebuffer);
        if (target.m_renderPass)
            GetDevice().destroyRenderPass(target.m_renderPass);

        auto imageIter = target.m_images.begin();
        auto viewIter = target.m_views.begin();
        for (; imageIter != target.m_images.end() && viewIter != target.m_views.end();
            ++imageIter, ++viewIter)
        {
            DestroyImageAndView(*imageIter, *viewIter);
        }
    }

    //----------------------------------------------------------------------------------------------------
    //		NOTES:
    //		
    ///		@brief : Resolve memory requirements into the type of memory that should be allocated.
    ///     @returns : Returns -1 in the event that no memory satisfies the requirements.
    //----------------------------------------------------------------------------------------------------
    int RendererContext::FindMemoryTypeIndex(vk::MemoryRequirements req, vk::MemoryPropertyFlags flags)
    {
        for (uint32_t typeIndex = 0; typeIndex < m_vkbPhysicalDevice.memory_properties.memoryTypeCount; ++typeIndex)
        {
            if (req.memoryTypeBits & (0x1 << typeIndex))
            {
                vk::MemoryPropertyFlags memoryFlags(m_vkbPhysicalDevice.memory_properties.memoryTypes[typeIndex].propertyFlags);
                if ((flags & memoryFlags) == flags)
                {
                    return static_cast<int>(typeIndex);
                }
            }
        }
        
        return -1;
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Allocate device memory that fulfills the provided requirements 
    //----------------------------------------------------------------------------------------------------
    vk::DeviceMemory RendererContext::AllocateMemory(vk::MemoryRequirements req, vk::MemoryPropertyFlags flags)
    {
        return GetDevice().allocateMemory(
            vk::MemoryAllocateInfo()
                .setAllocationSize(req.size)
                .setMemoryTypeIndex(FindMemoryTypeIndex(req, flags))
        );
    }

    //----------------------------------------------------------------------------------------------------
    //		NOTES:
    //		
    ///		@brief : There are a number of reasons a swapchain can become invalidated (like resizing the window)
    ///	            so we organize the recreation into a single method.
    ///		@returns : 
    //----------------------------------------------------------------------------------------------------
    bool RendererContext::RebuildSwapchain()
    {
        auto device = GetDevice();
		device.waitIdle();

		auto oldSwapchainImageFormat = static_cast<vk::Format>(m_vkbSwapchain.image_format);

		// Destroy the old per-frame resources as they will be recreated
		{
			std::vector<vk::CommandBuffer> commandBuffers;
			commandBuffers.reserve(m_framebuffers.size());
			std::vector<VkImageView> imageViews;
			imageViews.reserve(m_framebuffers.size());
			for (const auto& fbData : m_framebuffers)
			{
				if (fbData.m_imageView)
					imageViews.push_back(fbData.m_imageView);
				if (fbData.m_framebuffer)
					device.destroyFramebuffer(fbData.m_framebuffer);
				if (fbData.m_commandBuffer)
					commandBuffers.push_back(fbData.m_commandBuffer);
			}
			m_framebuffers.clear();
			m_currentFramebufferIndex = 0;

			if (!commandBuffers.empty())
				device.freeCommandBuffers(m_graphicsCommandPool, commandBuffers);
			if (!imageViews.empty())
				m_vkbSwapchain.destroy_image_views(imageViews);
		}

		auto buildResult = vkb::SwapchainBuilder(m_vkbDevice)
			.set_old_swapchain(m_vkbSwapchain)
			.build();
		if (!buildResult)
			return false;

		m_vkbSwapchain = buildResult.value();

		m_displayViewport.width = static_cast<float>(m_vkbSwapchain.extent.width);
		m_displayViewport.height = static_cast<float>(m_vkbSwapchain.extent.height);
		m_displayViewport.minDepth = 0.0f;
		m_displayViewport.maxDepth = 1.0f;

		// recreate depth image as the swapchain size might have changed
		if (m_depthFormat != vk::Format::eUndefined)
		{
			if (m_depthStencilImage)
			{
				DestroyImageAndView(m_depthStencilImage, m_depthStencilView);
				m_depthStencilImage = vk::Image();
				m_depthStencilView = vk::ImageView();
			}

			std::tie(m_depthStencilImage, m_depthStencilView) = CreateImageAndView(
				vk::ImageCreateInfo()
					.setFormat(m_depthFormat)
					.setUsage(vk::ImageUsageFlagBits::eDepthStencilAttachment | vk::ImageUsageFlagBits::eSampled)
					.setExtent(vk::Extent3D(m_vkbSwapchain.extent, 1))
					.setImageType(vk::ImageType::e2D)
					.setMipLevels(1)
					.setArrayLayers(1)
			);
		}

		// If the color format if our swapchain has changed, rebuild the associated render pass
		// We could just unconditionally rebuild, too, but since the format is unlikely to change
		// this will save having to rebuild any associated pipeline objects.
		if (!m_displayRenderPass || static_cast<vk::Format>(m_vkbSwapchain.image_format) != oldSwapchainImageFormat)
		{
			if (m_displayRenderPass)
			{
				// Destroy any pipeline objects that are referencing the old render pass

				std::unordered_set<vk::Pipeline> pendingDestroyPipelines;
				std::unordered_set<vk::PipelineLayout> pendingDestroyLayouts;

				erase_if(m_graphicsPipelines, [](const std::weak_ptr<GraphicsPipeline>& p) -> bool { return p.expired(); });
				for (auto& ptr : m_graphicsPipelines)
				{
					auto pipeline = ptr.lock();
					if (auto& renderPass = std::get<2>(*pipeline).m_renderPass;
						!renderPass || renderPass == m_displayRenderPass)
					{
						if (auto& pipelineRef = std::get<0>(*pipeline))
						{
							pendingDestroyPipelines.insert(pipelineRef);
							pipelineRef = vk::Pipeline();
						}
						if (auto& layout = std::get<1>(*pipeline))
						{
							pendingDestroyLayouts.insert(layout);
							layout = vk::PipelineLayout();
						}
						renderPass = vk::RenderPass();
					}
				}

				for (const auto& pipeline : pendingDestroyPipelines)
					device.destroyPipeline(pipeline);
				for (const auto& layout : pendingDestroyLayouts)
					device.destroyPipelineLayout(layout);
			}

			device.destroyRenderPass(m_displayRenderPass);
			m_displayRenderPass = m_depthFormat != vk::Format::eUndefined ?
				CreateColorAndDepthRenderPass(static_cast<vk::Format>(m_vkbSwapchain.image_format), m_depthFormat) :
				CreateColorOnlyRenderPass(static_cast<vk::Format>(m_vkbSwapchain.image_format));
			if (!m_displayRenderPass)
				return false;

			// Recreate any pipelines that output to the screen (swapchain) as they need the corresponding
			// RenderPass object
			erase_if(m_graphicsPipelines, [](const auto& p) -> bool { return p.expired(); });
			for (auto& ptr : m_graphicsPipelines)
			{
				auto pipeline = ptr.lock();
				if (auto& renderPass = std::get<2>(*pipeline).m_renderPass; !renderPass)
				{
					auto& pipelineRef = std::get<0>(*pipeline);
					auto& layout = std::get<1>(*pipeline);
					renderPass = m_displayRenderPass;
					std::tie(pipelineRef, layout) = CreatePipelineAndLayout(pipeline->GetConfig());
				}
			}
		}
			
		// Create the framebuffer objects associated with each image in our swapchain

		auto imageViews = m_vkbSwapchain.get_image_views().value();
		m_framebuffers.reserve(imageViews.size());

		auto frameCommandBuffers = device.allocateCommandBuffers(
			vk::CommandBufferAllocateInfo()
				.setCommandPool(m_graphicsCommandPool)
				.setCommandBufferCount(static_cast<uint32_t>(imageViews.size()))
		);

		std::vector<vk::ImageView> imageAttachments = { vk::ImageView() };

		if (m_depthStencilView)
			imageAttachments.push_back(m_depthStencilView);

		for (vk::ImageView imageView : imageViews)
		{
			imageAttachments[0] = imageView;

			auto framebuffer = device.createFramebuffer(
				vk::FramebufferCreateInfo()
					.setAttachments(imageAttachments)
					.setRenderPass(m_displayRenderPass)
					.setWidth(m_vkbSwapchain.extent.width)
					.setHeight(m_vkbSwapchain.extent.height)
					.setLayers(1)
			);
			if (!framebuffer)
				return false;
			
			m_framebuffers.push_back({
				.m_framebuffer = framebuffer,
				.m_imageView = imageView,
				.m_commandBuffer = frameCommandBuffers.back()
			});
			
			frameCommandBuffers.pop_back();
		}
		m_currentFramebufferIndex = 0;

		return true;
    }

    //----------------------------------------------------------------------------------------------------
    //		NOTES:
    //		
    ///		@brief : Static DebugLogCallback function that calls into the set m_debugLogFunction. 
    //----------------------------------------------------------------------------------------------------
    VkBool32 RendererContext::DebugLogCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                             VkDebugUtilsMessageTypeFlagsEXT messageTypes, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
                                             void* pUserData)
    {
    	auto self = static_cast<RendererContext*>(pUserData);
    	self->m_debugLogFunc(messageSeverity, messageTypes, pCallbackData);
    	return VK_FALSE;
    }
}

//----------------------------------------------------------------------------------------------------
///		@brief : Get the required extensions for GLFW to work with Vulkan.
//----------------------------------------------------------------------------------------------------
std::vector<const char*> GLFW_GetRequiredExtensions()
{
	uint32_t glfwExtensionCount = 0;
	const char** pGLFWExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
	
	std::vector<const char*> extensions(pGLFWExtensions, pGLFWExtensions + glfwExtensionCount);
	return extensions;
}

//----------------------------------------------------------------------------------------------------
//		NOTES:
//		
///		@brief : Create a Surface for Vulkan to render to.
///		@param pInstance : Instance that will be rendering to the surface.
///		@param pWindow : GLFW window to create the surface for.
///		@returns : True on success, false on failure.
//----------------------------------------------------------------------------------------------------
vk::SurfaceKHR GLFW_CreateSurface(vk::Instance pInstance, GLFWwindow* pWindow)
{
	NES_ASSERT(pWindow != nullptr);
	NES_ASSERT(pInstance != nullptr);
        
	VkSurfaceKHR pSurface = nullptr;
	NES_VULKAN_MUST_PASS(glfwCreateWindowSurface(pInstance, pWindow, nullptr, &pSurface));
	return pSurface;
}

namespace nes::internal
{
	VkBool32 DefaultDebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		[[maybe_unused]] VkDebugUtilsMessageTypeFlagsEXT messageType, [[maybe_unused]] const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData)
	{
		// If the message is important enough to show.
		if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) 
		{
			NES_ERROR(vulkan::kLogTag, "Validation Layer: ", pCallbackData->pMessage);
		}

		return VK_FALSE;
	}
}
#endif
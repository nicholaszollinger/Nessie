// Swapchain.cpp
#include "Swapchain.h"

#include <GLFW/glfw3.h>
#include "CommandPool.h"
#include "DeviceImage.h"
#include "DeviceQueue.h"
#include "RenderDevice.h"
#include "Descriptor.h"
#include "Vulkan/VulkanGLFW.h"

namespace nes
{
    Swapchain::~Swapchain()
    {
        DestroySwapchain();
    }

    EGraphicsResult Swapchain::Init(const SwapchainDesc& swapchainDesc)
    {
        // Ensure that we haven't been initialized yet.
        NES_GRAPHICS_ASSERT(m_device, m_surface == nullptr);

        m_pQueue = swapchainDesc.m_pDeviceQueue;
        m_pCommandPool = swapchainDesc.m_pCommandPool;
        NES_GRAPHICS_ASSERT(m_device, m_pQueue != nullptr);
        NES_GRAPHICS_ASSERT(m_device, m_pCommandPool != nullptr);
        
        const uint32 familyIndex = m_pQueue->GetFamilyIndex();

        // Create the surface.
        const NativeWindow& nativeWindow = swapchainDesc.m_pWindow->GetNativeWindow();
        m_pWindow = static_cast<GLFWwindow*>(nativeWindow.m_glfw);
        
        VkSurfaceKHR surface = vulkan::glfw::CreateSurface(m_device, m_pWindow);
        m_surface = vk::raii::SurfaceKHR(m_device, surface);

        // Check that the given device queue can be used to present to the surface.
        const VkBool32 presentSupported = m_device.GetVkPhysicalDevice().getSurfaceSupportKHR(familyIndex, *m_surface);
        if (!presentSupported)
        {
            NES_GRAPHICS_ERROR(m_device, "Selected Queue family {} cannot present to the surface! Swapchain creation failed!", familyIndex);
            return EGraphicsResult::Unsupported;
        }
        
        // Build the initial swapchain to the size of the Window.
        return BuildSwapchain(swapchainDesc.m_pWindow->GetResolution(), swapchainDesc.m_pWindow->IsVsyncEnabled());
    }

    EGraphicsResult Swapchain::OnResize(const UVec2 desiredWindowSize, const bool enableVsync)
    {
        // Wait for all frames to finish rendering before recreating the swapchain
        m_pQueue->WaitUntilIdle();

        m_frameResourceIndex = 0;
        m_needsRebuild = false;
        DestroySwapchain();
        return BuildSwapchain(desiredWindowSize, enableVsync);
    }

    EGraphicsResult Swapchain::AcquireNextImage()
    {
        NES_GRAPHICS_ASSERT(m_device, m_needsRebuild == false);

        // Get the frame resources for the current frame
        // We use m_currentFrame here because we want to ensure we don't overwrite resources
        // that are still in use by previous frames
        auto& frame = m_frameResources[m_frameResourceIndex];

        // Acquire the next image from the swapchain
        // This will signal frame.imageAvailable when the image is ready
        // and store the index of the acquired image in m_nextImageIndex
        auto [result, nextImageIndex] = m_swapchain.acquireNextImage(std::numeric_limits<uint64>::max(), frame.m_imageAvailable, nullptr);
        m_frameImageIndex = nextImageIndex;
        
        switch (result)
        {
            case vk::Result::eSuccess:
            case vk::Result::eSuboptimalKHR:         // Still valid for presentation.
                break;

            case vk::Result::eErrorOutOfDateKHR:  // The swapchain is no longer compatible with the surface and needs to be recreated
                m_needsRebuild = true;
                break;
                
            default:
                NES_GRAPHICS_WARN(m_device, "Failed to acquire swapchain image: Vulkan Error: {}", vk::to_string(result));
                break;
        }
        
        return ConvertVkResultToGraphics(static_cast<VkResult>(result));
    }

    void Swapchain::PresentFrame()
    {
        // Lock the Queue mutex.
        std::lock_guard lock(m_pQueue->GetMutex());
        
        // Get the frame resources for the current image
        // We use m_nextImageIndex here because we want to signal the semaphore
        // associated with the image we just finished rendering
        auto& frame = m_frameResources[m_frameImageIndex];

        const vk::PresentInfoKHR presentInfo = vk::PresentInfoKHR()
            .setWaitSemaphoreCount(1)
            .setWaitSemaphores(*frame.m_renderFinished)
            .setSwapchainCount(1)
            .setSwapchains(*m_swapchain)
            .setPImageIndices(&m_frameImageIndex)
            .setPResults(nullptr);

        const vk::Result result = m_pQueue->GetVkQueue().presentKHR(presentInfo);
        
        if (result == vk::Result::eErrorOutOfDateKHR)
        {
            m_needsRebuild = true;
        }
        else
        {
            NES_GRAPHICS_ASSERT(m_device, result == vk::Result::eSuccess || result == vk::Result::eSuboptimalKHR);
        }

        // Advance to the next frame in the swapchain
        m_frameResourceIndex = (m_frameResourceIndex + 1) % m_maxFramesInFlight;
    }

    void Swapchain::SetDebugName(const std::string& name)
    {
        vk::SwapchainKHR swapchain = *m_swapchain;
        m_device.SetDebugNameToTrivialObject(swapchain, name);

        vk::SurfaceKHR surface = *m_surface;
        m_device.SetDebugNameToTrivialObject(surface, "Swapchain Surface");
    }

    EGraphicsResult Swapchain::BuildSwapchain(const UVec2 /*desiredWindowSize*/, const bool enableVsync)
    {
        const auto& physicalDevice = m_device.GetVkPhysicalDevice();
        auto surfaceCapabilities = physicalDevice.getSurfaceCapabilitiesKHR(m_surface);
        std::vector<vk::SurfaceFormatKHR> availableFormats = physicalDevice.getSurfaceFormatsKHR(m_surface);
        std::vector<vk::PresentModeKHR> availablePresentModes = physicalDevice.getSurfacePresentModesKHR(m_surface);

        // Choose the best available surface format and present mode
        m_swapchainImageFormat = SelectSwapSurfaceFormat(availableFormats);
        const vk::PresentModeKHR presentMode = SelectSwapPresentMode(availablePresentModes, enableVsync);
        m_swapchainExtent = SelectSwapExtent(surfaceCapabilities);

        auto minImageCount = math::Max(3u, surfaceCapabilities.minImageCount);
        minImageCount = ( surfaceCapabilities.maxImageCount > 0 && minImageCount > surfaceCapabilities.maxImageCount ) ? surfaceCapabilities.maxImageCount : minImageCount;

        vk::SwapchainCreateInfoKHR swapChainCreateInfo = vk::SwapchainCreateInfoKHR()
            .setSurface(m_surface)
            .setMinImageCount(minImageCount)
            .setImageFormat(m_swapchainImageFormat)
            .setImageColorSpace(vk::ColorSpaceKHR::eSrgbNonlinear)
            .setImageArrayLayers(1)
            .setImageUsage(vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eTransferDst)
            .setImageExtent(m_swapchainExtent)
            .setImageSharingMode(vk::SharingMode::eExclusive)
            .setCompositeAlpha(vk::CompositeAlphaFlagBitsKHR::eOpaque)
            .setPresentMode(presentMode)
            .setPreTransform(surfaceCapabilities.currentTransform)
            .setClipped(true);

        m_swapchain = vk::raii::SwapchainKHR(m_device, swapChainCreateInfo);
        
        // Create the image resources for the swapchain.
        CreateImages();

        // We can get more images than the minimum requested.
        // We still need to get a handle for each image in the swapchain
        // (because vkAcquireNextImageKHR can return an index to each image),
        // so adjust m_maxFramesInFlight.
        m_maxFramesInFlight = static_cast<uint32>(m_images.size());
        
        // Create the image views for the image resources.
        CreateImageViews();

        // Create the frame resources:
        CreateFrameResources();
        
        return EGraphicsResult::Success;
    }

    void Swapchain::DestroySwapchain()
    {
        // Destroy current descriptors
        for (auto* pDescriptor : m_imageViews)
        {
            m_device.FreeResource(pDescriptor);
        }
        m_imageViews.clear();
        
        // Destroy current image resources (this will not destroy the vk::Images, as they are owned by the swapchain).
        for (auto* pImage : m_images)
        {
            m_device.FreeResource(pImage);
        }
        m_images.clear();

        // raii type cleans up the swapchain object.
        m_swapchain = nullptr;

        // This will clean up all resources, since we use the vk::raii objects.
        m_frameResources.clear();
        
    }

    vk::Format Swapchain::SelectSwapSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& availableFormats) const
    {
        const auto formatIt = std::ranges::find_if(availableFormats, [](const auto& format)
            {
                return format.format == vk::Format::eB8G8R8A8Srgb && format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear;
            });

        // If no preferred format is available, then return the first available.
        return formatIt != availableFormats.end() ? formatIt->format : availableFormats[0].format;
    }
    
    vk::PresentModeKHR Swapchain::SelectSwapPresentMode(const std::vector<vk::PresentModeKHR>& availablePresentModes, const bool vSyncEnabled) const
    {
        if (vSyncEnabled)
        {
            return vk::PresentModeKHR::eFifo;
        }

        bool mailboxSupported = false;
        bool immediateSupported = false;

        for (const vk::PresentModeKHR mode : availablePresentModes)
        {
            // If this is the preferred mode, return it.
            if (mode == m_preferredVsyncOffMode)
                return mode;

            if (mode == vk::PresentModeKHR::eMailbox)
                mailboxSupported = true;

            if (mode == vk::PresentModeKHR::eImmediate)
                immediateSupported = true;
        }

        // Immediate is preferred for low latency.
        if (immediateSupported)
            return vk::PresentModeKHR::eImmediate;

        if (mailboxSupported)
            return vk::PresentModeKHR::eMailbox;
        
        return vk::PresentModeKHR::eFifo;
    }

    vk::Extent2D Swapchain::SelectSwapExtent(const vk::SurfaceCapabilitiesKHR& capabilities) const
    {
        if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
            return capabilities.currentExtent;

        // Get the current width and height.
        int width, height;
        glfwGetFramebufferSize(m_pWindow, &width, &height);

        return
        {
            math::Clamp<uint32>(width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width),
            math::Clamp<uint32>(height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height)
        };
    }

    void Swapchain::CreateImages()
    {
        for (auto* pImage : m_images)
        {
            m_device.FreeResource(pImage);
        }
        m_images.clear();

        auto vkImages = m_swapchain.getImages();
        m_images.resize(vkImages.size());
        for (uint32 i = 0; i < vkImages.size(); i++)
        {
            ImageDesc desc = {};
            desc.m_format = GetFormat(static_cast<uint32>(m_swapchainImageFormat));
            desc.m_type = EImageType::Image2D;
            desc.m_width = m_swapchainExtent.width;
            desc.m_height = m_swapchainExtent.height;
            desc.m_depth = 1;
            desc.m_mipCount = 1;
            desc.m_layerCount = 1;
            desc.m_sampleCount = 1;

            DeviceImage* pImage = Allocate<DeviceImage>(m_device.GetAllocationCallbacks(), m_device);
            pImage->Init(vkImages[i], desc);
            m_images[i] = pImage;
        }
    }

    void Swapchain::CreateImageViews()
    {
        // Destroy current descriptors
        for (auto* pDescriptor : m_imageViews)
        {
            m_device.FreeResource(pDescriptor);
        }
        m_imageViews.clear();

        Image2DViewDesc desc{};
        desc.m_format = GetFormat(static_cast<uint32>(m_swapchainImageFormat));
        desc.m_viewType = EImage2DViewType::ColorAttachment;
        desc.m_baseMipLevel = 0;
        desc.m_mipCount = 1;
        desc.m_baseLayer = 0;
        desc.m_layerCount = 1;
        
        // Create the new descriptors:
        for (const auto* pImage : m_images)
        {
            //imageViewCreateInfo.image = image;
            desc.m_pImage = pImage;
            Descriptor* pView = nullptr;
            
            m_device.CreateResource<Descriptor>(pView, desc);
            m_imageViews.emplace_back(pView);
        }
    }

    void Swapchain::CreateFrameResources()
    {
        // raii types handle destruction.
        m_frameResources.clear();

        constexpr vk::SemaphoreCreateInfo kSemaphoreInfo = vk::SemaphoreCreateInfo();
        
        m_frameResources.resize(m_images.size());
        for (size_t i = 0; i < m_images.size(); ++i)
        {
            m_frameResources[i].m_renderFinished = vk::raii::Semaphore(m_device, kSemaphoreInfo, m_device.GetVkAllocationCallbacks());
            m_frameResources[i].m_imageAvailable = vk::raii::Semaphore(m_device, kSemaphoreInfo, m_device.GetVkAllocationCallbacks());
        }
    }
}

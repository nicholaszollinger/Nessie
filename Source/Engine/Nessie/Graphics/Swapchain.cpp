// Swapchain.cpp
#include "Swapchain.h"

#include <vulkan/vk_enum_string_helper.h>

#include "CommandPool.h"
#include "DeviceQueue.h"
#include "RenderDevice.h"

namespace nes
{
    Swapchain::~Swapchain()
    {
        DestroySwapchain();

        // Destroy the surface
        if (m_surface)
        {
            vkDestroySurfaceKHR(m_device, m_surface, m_device.GetVkAllocationCallbacks());
            m_surface = nullptr;
        }
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
    #if NES_PLATFORM_WINDOWS
        if (nativeWindow.m_windows.m_hwnd)
        {
            VkWin32SurfaceCreateInfoKHR win32SurfaceInfo = {VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR};
            win32SurfaceInfo.hwnd = static_cast<HWND>(nativeWindow.m_windows.m_hwnd);
            
            NES_VK_FAIL_RETURN(m_device, vkCreateWin32SurfaceKHR(m_device, &win32SurfaceInfo, m_device.GetVkAllocationCallbacks(), &m_surface));   
        }
    #else
    #error "Unsupported Swapchain platform!"
    #endif

        // Check that the given device queue can be used to present to the surface.
        {
            VkBool32 supportsPresent = VK_FALSE;
            const VkResult result = vkGetPhysicalDeviceSurfaceSupportKHR(m_device, familyIndex, m_surface, &supportsPresent);
        
            if (supportsPresent != VK_TRUE)
            {
                NES_GRAPHICS_ERROR(m_device, "Selected Queue family {} cannot present to the surface! Swapchain creation failed!", familyIndex);
                return EGraphicsResult::InitializationFailed;
            }
            if (result != VK_SUCCESS)
            {
                return ConvertVkResultToGraphics(result);
            }
        }
        
        // Build the initial swapchain to the size of the Window.
        return BuildSwapchain(swapchainDesc.m_pWindow->GetResolution(), swapchainDesc.m_pWindow->IsVsyncEnabled());
    }

    EGraphicsResult Swapchain::OnResize(const UVec2 desiredWindowSize, const bool enableVsync)
    {
        // Wait for all frames to finish rendering before recreating the swapchain
        vkQueueWaitIdle(m_pQueue->GetHandle());

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
        VkResult result = vkAcquireNextImageKHR(m_device, m_swapchain, std::numeric_limits<uint64>::max(), frame.m_imageAvailable, nullptr, &m_frameImageIndex);

        switch (result)
        {
            case VK_SUCCESS:
            case VK_SUBOPTIMAL_KHR:         // Still valid for presentation.
                break;

            case VK_ERROR_OUT_OF_DATE_KHR:  // The swapchain is no longer compatible with the surface and needs to be recreated
                m_needsRebuild = true;
                break;
                
            default:
                NES_GRAPHICS_WARN(m_device, "Failed to acquire swapchain image: Vulkan Error: {}", string_VkResult(result));
                break;
        }
        
        return ConvertVkResultToGraphics(result);
    }

    void Swapchain::PresentFrame()
    {
        // Lock the Queue mutex.
        std::lock_guard lock(m_pQueue->GetMutex());
        
        // Get the frame resources for the current image
        // We use m_nextImageIndex here because we want to signal the semaphore
        // associated with the image we just finished rendering
        auto& frame = m_frameResources[m_frameImageIndex];

        const VkPresentInfoKHR presentInfo
        {
            .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
            .pNext = nullptr,
            .waitSemaphoreCount = 1,
            .pWaitSemaphores = &frame.m_renderFinished,
            .swapchainCount = 1,
            .pSwapchains = &m_swapchain,
            .pImageIndices = &m_frameImageIndex,
            .pResults = nullptr,
        };

        const VkResult result = vkQueuePresentKHR(*m_pQueue, &presentInfo);
        if (result == VK_ERROR_OUT_OF_DATE_KHR)
        {
            m_needsRebuild = true;
        }
        else
        {
            NES_GRAPHICS_ASSERT(m_device, result == VK_SUCCESS || result == VK_SUBOPTIMAL_KHR);
        }

        // Advance to the next frame in the swapchain
        m_frameResourceIndex = (m_frameResourceIndex + 1) % m_maxFramesInFlight;
    }

    void Swapchain::SetDebugName(const char* name)
    {
        m_device.SetDebugNameToTrivialObject(m_swapchain, name);
        m_device.SetDebugNameToTrivialObject(m_surface, "Swapchain Surface");
    }

    EGraphicsResult Swapchain::BuildSwapchain(const UVec2 desiredWindowSize, const bool enableVsync)
    {
        // Get the physical device's capabilities for the given surface:
        const VkPhysicalDeviceSurfaceInfo2KHR surfaceInfo2
        {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SURFACE_INFO_2_KHR,
            .surface = m_surface,
        };

        VkSurfaceCapabilities2KHR capabilities2{ .sType = VK_STRUCTURE_TYPE_SURFACE_CAPABILITIES_2_KHR };
        NES_VK_FAIL_RETURN(m_device, vkGetPhysicalDeviceSurfaceCapabilities2KHR(m_device, &surfaceInfo2, &capabilities2));

        uint32 formatCount;
        NES_VK_FAIL_RETURN(m_device, vkGetPhysicalDeviceSurfaceFormats2KHR(m_device, &surfaceInfo2, &formatCount, nullptr));
        std::vector<VkSurfaceFormat2KHR> formats(formatCount, { .sType = VK_STRUCTURE_TYPE_SURFACE_FORMAT_2_KHR });
        NES_VK_FAIL_RETURN(m_device, vkGetPhysicalDeviceSurfaceFormats2KHR(m_device, &surfaceInfo2, &formatCount, formats.data()));

        uint32 presentModeCount;
        NES_VK_FAIL_RETURN(m_device, vkGetPhysicalDeviceSurfacePresentModesKHR(m_device, m_surface, &presentModeCount, nullptr));
        std::vector<VkPresentModeKHR> presentModes(presentModeCount);
        NES_VK_FAIL_RETURN(m_device, vkGetPhysicalDeviceSurfacePresentModesKHR(m_device, m_surface, &presentModeCount, presentModes.data()));

        // Choose the best available surface format and present mode
        const VkSurfaceFormat2KHR surfaceFormat2 = SelectSwapSurfaceFormat(formats);
        const VkPresentModeKHR presentMode = SelectSwapPresentMode(presentModes, enableVsync);

        // Ensure a valid window dimension:
        const auto& surfaceCaps = capabilities2.surfaceCapabilities;
        const bool isValidWidth = desiredWindowSize.x >= surfaceCaps.minImageExtent.width && desiredWindowSize.x <= surfaceCaps.maxImageExtent.width;
        NES_GRAPHICS_RETURN_FAIL(m_device, isValidWidth, EGraphicsResult::InvalidArgument, "Invalid window width! Must be in range [{}, {}]", surfaceCaps.minImageExtent.width, surfaceCaps.maxImageExtent.width);
        const bool isValidHeight = desiredWindowSize.y >= surfaceCaps.minImageExtent.height && desiredWindowSize.y <= surfaceCaps.maxImageExtent.height;
        NES_GRAPHICS_RETURN_FAIL(m_device, isValidHeight, EGraphicsResult::InvalidArgument, "Invalid window height! Must be in range [{}, {}]", surfaceCaps.minImageExtent.width, surfaceCaps.maxImageExtent.width);
        
        // Set the number of images in flight, respecting the GPU's maxImageCount limit.
        // If maxImageCount is equal to 0, then there is no limit other than memory,
        // so don't change m_maxFramesInFlight.
        if (surfaceCaps.maxImageCount > 0)
        {
            m_maxFramesInFlight = math::Min(m_maxFramesInFlight, surfaceCaps.maxImageCount);
        }

        // Store the chosen image format
        m_imageFormat = surfaceFormat2.surfaceFormat.format;

        // Create the swapchain:
        const VkSwapchainCreateInfoKHR swapchainCreateInfo
        {
            .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
            .pNext = nullptr,
            .surface = m_surface,
            .minImageCount = m_maxFramesInFlight,
            .imageFormat = surfaceFormat2.surfaceFormat.format,
            .imageColorSpace = surfaceFormat2.surfaceFormat.colorSpace,
            .imageExtent = surfaceCaps.currentExtent,
            .imageArrayLayers = 1,
            .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
            .imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
            .preTransform = surfaceCaps.currentTransform,
            .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
            .presentMode = presentMode,
            .clipped = VK_TRUE,
        };
        NES_VK_FAIL_RETURN(m_device, vkCreateSwapchainKHR(m_device, &swapchainCreateInfo, m_device.GetVkAllocationCallbacks(), &m_swapchain));
        SetDebugName("Swapchain");

        // Retrieve swapchain images:
        uint32 imageCount;
        NES_VK_FAIL_RETURN(m_device, vkGetSwapchainImagesKHR(m_device, m_swapchain, &imageCount, nullptr));
        // We can get more images than the minimum requested.
        // We still need to get a handle for each image in the swapchain
        // (because vkAcquireNextImageKHR can return an index to each image),
        // so adjust m_maxFramesInFlight.
        NES_GRAPHICS_ASSERT(m_device, m_maxFramesInFlight <= imageCount);
        m_maxFramesInFlight = imageCount;
        std::vector<VkImage> swapImages(m_maxFramesInFlight);
        NES_VK_FAIL_RETURN(m_device, vkGetSwapchainImagesKHR(m_device, m_swapchain, &imageCount, swapImages.data()));

        // Store the swap chain images and create views for them.
        m_images.resize(m_maxFramesInFlight);
        VkImageViewCreateInfo imageViewCreateInfo
        {
            .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .pNext = nullptr,
            .viewType = VK_IMAGE_VIEW_TYPE_2D,
            .format = m_imageFormat,
            .components  = {.r = VK_COMPONENT_SWIZZLE_IDENTITY, .g = VK_COMPONENT_SWIZZLE_IDENTITY, .b = VK_COMPONENT_SWIZZLE_IDENTITY, .a = VK_COMPONENT_SWIZZLE_IDENTITY},
            .subresourceRange = {.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, .baseMipLevel = 0, .levelCount = 1, .baseArrayLayer = 0, .layerCount = 1},
        };

        for (uint32 i = 0; i < m_maxFramesInFlight; ++i)
        {
            m_images[i].m_image = swapImages[i];
            m_device.SetDebugNameToTrivialObject(m_images[i].m_image, fmt::format("Swapchain Image ({})", i));
            imageViewCreateInfo.image = m_images[i].m_image;
            NES_VK_FAIL_RETURN(m_device, vkCreateImageView(m_device, &imageViewCreateInfo, m_device.GetVkAllocationCallbacks(), &m_images[i].m_view));
            m_device.SetDebugNameToTrivialObject(m_images[i].m_view, fmt::format("Swapchain Image View ({})", i));
        }

        // Initialize the frame resources for each swapchain image.
        m_frameResources.resize(m_maxFramesInFlight);
        for (uint32 i = 0; i < m_maxFramesInFlight; ++i)
        {
            constexpr VkSemaphoreCreateInfo semaphoreCreateInfo {.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };
            NES_VK_FAIL_RETURN(m_device, vkCreateSemaphore(m_device, &semaphoreCreateInfo, m_device.GetVkAllocationCallbacks(), &m_frameResources[i].m_imageAvailable));
            m_device.SetDebugNameToTrivialObject(m_frameResources[i].m_imageAvailable, fmt::format("Swapchain Image Available ({})", i));
            NES_VK_FAIL_RETURN(m_device, vkCreateSemaphore(m_device, &semaphoreCreateInfo, m_device.GetVkAllocationCallbacks(), &m_frameResources[i].m_renderFinished));
            m_device.SetDebugNameToTrivialObject(m_frameResources[i].m_renderFinished, fmt::format("Swapchain Render Finished ({})", i));
        }

        // Transition all images to the present layout:
        {
            CommandBuffer* pBuffer = nullptr;
            m_pCommandPool->CreateCommandBuffer(pBuffer);
            
            ImageMemoryBarrierDesc imageMemoryBarrierDesc{};
            imageMemoryBarrierDesc.SetLayoutTransition(VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
            
            pBuffer->Begin();
            for (uint32 i = 0; i < m_maxFramesInFlight; ++i)
            {
                EGraphicsResult result = pBuffer->TransitionImageLayout(m_images[i].m_image, imageMemoryBarrierDesc);
                NES_GRAPHICS_RETURN_FAIL(m_device, result == EGraphicsResult::Success, EGraphicsResult::Failure, "Failed to transition swapchain image to present layout!");
            }
            pBuffer->End();
            
            // Submit to the Queue:
            m_pQueue->SubmitSingleTimeCommands(*pBuffer);
            m_device.FreeResource(pBuffer);
        }
        
        return EGraphicsResult::Success;
    }

    void Swapchain::DestroySwapchain()
    {
        vkDestroySwapchainKHR(m_device, m_swapchain, m_device.GetVkAllocationCallbacks());
        for (auto& frame : m_frameResources)
        {
            vkDestroySemaphore(m_device, frame.m_imageAvailable, m_device.GetVkAllocationCallbacks());
            vkDestroySemaphore(m_device, frame.m_renderFinished, m_device.GetVkAllocationCallbacks());
        }
        m_frameResources.clear();

        for (auto& image : m_images)
        {
            vkDestroyImageView(m_device, image.m_view, m_device.GetVkAllocationCallbacks());
        }
        m_images.clear();
    }

    VkSurfaceFormat2KHR Swapchain::SelectSwapSurfaceFormat(const std::vector<VkSurfaceFormat2KHR>& availableFormats) const
    {
        // If there's only one available format, and it's undefined, return a default format.
        if (availableFormats.size() == 1 && availableFormats[0].surfaceFormat.format == VK_FORMAT_UNDEFINED)
        {
            static constexpr VkSurfaceFormat2KHR kDefault
            {
                .sType = VK_STRUCTURE_TYPE_SURFACE_FORMAT_2_KHR,
                .pNext = nullptr,
                .surfaceFormat = { VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR },
            };
            return kDefault;
        }

        static constexpr VkSurfaceFormat2KHR preferredFormats[]
        {
            VkSurfaceFormat2KHR
            {
                .sType = VK_STRUCTURE_TYPE_SURFACE_FORMAT_2_KHR, .pNext = nullptr,
                .surfaceFormat = {VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR}
            },
            
            VkSurfaceFormat2KHR
            {
                .sType = VK_STRUCTURE_TYPE_SURFACE_FORMAT_2_KHR, .pNext = nullptr,
                .surfaceFormat = {VK_FORMAT_R8G8B8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR}
            },
        };

        for (const auto& preferredFormat : preferredFormats)
        {
            for (const auto& availableFormat : availableFormats)
            {
                // Return the first matching preferred format:
                if (availableFormat.surfaceFormat.format == preferredFormat.surfaceFormat.format
                    && availableFormat.surfaceFormat.colorSpace == preferredFormat.surfaceFormat.colorSpace)
                {
                    return availableFormat;
                }
            }
        }

        // If no preferred format is available, then return the first available.
        return availableFormats[0];
    }

    VkPresentModeKHR Swapchain::SelectSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes, const bool vSyncEnabled) const
    {
        if (vSyncEnabled)
        {
            return VK_PRESENT_MODE_FIFO_KHR;
        }

        bool mailboxSupported = false;
        bool immediateSupported = false;

        for (const VkPresentModeKHR mode : availablePresentModes)
        {
            // If this is the preferred mode, return it.
            if (mode == m_preferredVsyncOffMode)
                return mode;

            if (mode == VK_PRESENT_MODE_MAILBOX_KHR)
                mailboxSupported = true;

            if (mode == VK_PRESENT_MODE_IMMEDIATE_KHR)
                immediateSupported = true;
        }

        // Immediate is preferred for low latency.
        if (immediateSupported)
            return VK_PRESENT_MODE_IMMEDIATE_KHR;

        if (mailboxSupported)
            return VK_PRESENT_MODE_MAILBOX_KHR;
        
        return VK_PRESENT_MODE_FIFO_KHR;
    }
}

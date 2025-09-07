// Swapchain.h
#pragma once
#include "Descriptor.h"
#include "DeviceImage.h"

struct GLFWwindow;

//-------------------------------------------------------------------------------------------------
// Under development. I am in the process of abstracting the Vulkan API.
//-------------------------------------------------------------------------------------------------
namespace nes
{
    class Swapchain;
    class ApplicationWindow;
    
    //----------------------------------------------------------------------------------------------------
    /// @brief : Info for creating a swapchain.
    //----------------------------------------------------------------------------------------------------
    struct SwapchainDesc
    {
        ApplicationWindow*  m_pWindow = nullptr;        // Window that we are presenting to.
        DeviceQueue*        m_pDeviceQueue = nullptr;   // Device queue that will be used to submit present commands.
    };
    
    //----------------------------------------------------------------------------------------------------
    /// @brief : The swapchain is responsible for presenting rendered images to the screen.
    ///     It consists of multiple images (frames) that are cycled through for rendering and display.
    ///     The swapchain is created with a surface and optional vSync setting, with the window size
    ///     determined during its setup.
    ///
    ///     "Frames in flight" refers to the number of images being processed concurrently
    ///     (e.g., double buffering = 2, triple buffering = 3).
    ///
    ///     vSync enabled (FIFO mode) uses double buffering, while disabling vSync (MAILBOX mode) uses triple buffering.
    ///
    ///     The "current frame" is the frame currently being processed.
    ///     The "next image index" points to the swapchain image that will be rendered next, which might differ from the current frame's index.
    ///     If the window is resized or certain conditions are met, the swapchain needs to be recreated (`m_needsRebuild` flag).
    //----------------------------------------------------------------------------------------------------
    class Swapchain 
    {
    public:
        Swapchain(std::nullptr_t) {}
        Swapchain(const Swapchain&) = delete;
        Swapchain(Swapchain&& other) noexcept;
        Swapchain& operator=(std::nullptr_t);
        Swapchain& operator=(const Swapchain&) = delete;
        Swapchain& operator=(Swapchain&& other) noexcept;
        ~Swapchain();
        
        //----------------------------------------------------------------------------------------------------
        /// @brief : Create the swapchain.
        //----------------------------------------------------------------------------------------------------
        Swapchain(RenderDevice& device, const SwapchainDesc& swapchainDesc);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Must be called any time the Window is resized, or if the vsync setting changes.
        //----------------------------------------------------------------------------------------------------
        EGraphicsResult             OnResize(const UVec2 desiredWindowSize, const bool enableVsync = true);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Must be called any time the Window is resized, or if the vsync setting changes.
        //----------------------------------------------------------------------------------------------------
        EGraphicsResult             OnResize(const uint32 width, const uint32 height, const bool enableVsync = true) { return OnResize(UVec2(width, height), enableVsync); }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Prepares the command buffer for recording rendering commands.
        ///     This function handles the synchronization with the previous frame and acquires the next image
        ///     from the swapchain.
        //----------------------------------------------------------------------------------------------------
        EGraphicsResult             AcquireNextImage();

        //----------------------------------------------------------------------------------------------------
        /// @brief : Presents the rendered image to the screen. Advances to the next frame in the cycle.
        //----------------------------------------------------------------------------------------------------
        void                        PresentFrame();
        
        //----------------------------------------------------------------------------------------------------
        /// @brief : Forces the rebuild of the swapchain. 
        //----------------------------------------------------------------------------------------------------
        void                        RequestRebuild()                    { m_needsRebuild = true; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Check if the Swapchain needs to be rebuilt. 
        //----------------------------------------------------------------------------------------------------
        bool                        NeedsRebuild() const                { return m_needsRebuild; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the current image that we are rendering to. 
        //----------------------------------------------------------------------------------------------------
        const DeviceImage&          GetImage() const                    { return m_images[m_frameImageIndex]; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the current swapchain image.
        //----------------------------------------------------------------------------------------------------
        DeviceImage*                GetImage()                          { return &m_images[m_frameImageIndex]; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the descriptor for the current swapchain image. This is the image view. 
        //----------------------------------------------------------------------------------------------------
        const Descriptor&           GetImageDescriptor() const          { return m_imageViews[m_frameImageIndex]; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the current size of the swapchain.
        //----------------------------------------------------------------------------------------------------
        vk::Extent2D                GetExtent() const                   { return m_swapchainExtent;}
        
        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the image format used for the swapchain images.
        //----------------------------------------------------------------------------------------------------
        vk::Format                  GetImageFormat() const              { return m_swapchainImageFormat; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the max number of frames that can be processed concurrently. 
        //----------------------------------------------------------------------------------------------------
        uint32                      GetMaxFramesInFlight() const        { return m_maxFramesInFlight; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the current semaphore that will be signaled when the next image is ready for rendering.
        //----------------------------------------------------------------------------------------------------
        vk::Semaphore               GetImageAvailableSemaphore() const  { return m_frameResources[m_frameResourceIndex].m_imageAvailable; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the current semaphore that will be signaled when rendering to the image has finished.
        //----------------------------------------------------------------------------------------------------
        vk::Semaphore               GetRenderFinishedSemaphore() const  { return m_frameResources[m_frameResourceIndex].m_renderFinished; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Sets the debug name for both the swapchain and the surface. 
        //----------------------------------------------------------------------------------------------------
        void                        SetDebugName(const std::string& name);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Advanced use. Get the native vulkan object handle, and the type.
        //----------------------------------------------------------------------------------------------------
        NativeVkObject              GetNativeVkObject() const;

    private:
        //----------------------------------------------------------------------------------------------------
        /// @brief : Resources associated with each frame being processed. 
        //----------------------------------------------------------------------------------------------------
        struct FrameResources
        {
            vk::raii::Semaphore     m_imageAvailable = nullptr;  /// Signals when the image is ready for rendering.
            vk::raii::Semaphore     m_renderFinished = nullptr;  /// Signals when rendering is finished.
        };

        //----------------------------------------------------------------------------------------------------
        /// @brief : Select the format that is most common and supported by the physical device.
        //----------------------------------------------------------------------------------------------------
        vk::Format                  SelectSwapSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& availableFormats) const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Choose the present mode based on vSyncEnabled.
        ///     - The FIFO mode is the most common, and is used when vSync is enabled.
        ///     - The `m_preferredVsyncOffMode` is used when vSync is disabled and the mode is supported.
        ///     Otherwise:
        ///     - The IMMEDIATE mode is used when vSync is disabled, and is the best mode for low latency.
        ///     - The MAILBOX mode is used when vSync is disabled, and is the best mode for triple buffering.
        //----------------------------------------------------------------------------------------------------
        vk::PresentModeKHR          SelectSwapPresentMode(const std::vector<vk::PresentModeKHR>& availablePresentModes, const bool vSyncEnabled = true) const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Select the swap extent size based on the current window and surface capabilities. 
        //----------------------------------------------------------------------------------------------------
        vk::Extent2D                SelectSwapExtent(const vk::SurfaceCapabilitiesKHR& capabilities) const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Create the image resources from the swapchain images.
        //----------------------------------------------------------------------------------------------------
        void                        CreateImages();
        
        //----------------------------------------------------------------------------------------------------
        /// @brief : Create the image views for each of the swapchain images.
        //----------------------------------------------------------------------------------------------------
        void                        CreateImageViews();

        //----------------------------------------------------------------------------------------------------
        /// @brief : Create the frame sync resources.
        //----------------------------------------------------------------------------------------------------
        void                        CreateFrameResources();

        //----------------------------------------------------------------------------------------------------
        /// @brief : Build the swapchain and all of its resources, aside from the surface.
        //----------------------------------------------------------------------------------------------------
        EGraphicsResult             BuildSwapchain(const UVec2 desiredWindowSize, const bool enableVsync = true);
        
        //----------------------------------------------------------------------------------------------------
        /// @brief : Destroys the swapchain and all images/frame resources. 
        //----------------------------------------------------------------------------------------------------
        void                        DestroySwapchain();

    private:
        RenderDevice*               m_pDevice = nullptr;                                // The Render Device handle.
        DeviceQueue*                m_pQueue = nullptr;                                 // The queue used to submit command buffers to the GPU
        GLFWwindow*                 m_pWindow = nullptr;                                // Window that we render to.
        vk::raii::SwapchainKHR      m_swapchain = nullptr;                              // The swapchain object.
        vk::Format                  m_swapchainImageFormat = vk::Format::eUndefined;    // The image format for the swapchain images.
        vk::raii::SurfaceKHR        m_surface = nullptr;                                // The surface to present images to. Owned by the swapchain.
        std::vector<DeviceImage>    m_images{};                                         // Swapchain image resources.
        std::vector<Descriptor>     m_imageViews{};                                     // Swapchain image views resources. Recreated when the Swapchain is recreated.
        std::vector<FrameResources> m_frameResources{};                                 // Synchronization primitives for each frame.
        vk::Extent2D                m_swapchainExtent{};                                // Current size of the swapchain.
        uint32                      m_frameResourceIndex = 0;                           // Index of the current frame.
        uint32                      m_frameImageIndex = 0;                              // Index of the swapchain image we are rendering to.
        bool                        m_needsRebuild = false;                             // Flag indicating that the swapchain needs to be rebuilt.

        vk::PresentModeKHR          m_preferredVsyncOffMode = static_cast<vk::PresentModeKHR>(VK_PRESENT_MODE_MAX_ENUM_KHR); 
        uint32                      m_maxFramesInFlight = 3;    /// Best for most cases.
    };

    static_assert(DeviceObjectType<Swapchain>);
}
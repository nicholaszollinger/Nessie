// Swapchain.h
#pragma once
#include "GraphicsResource.h"

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
        ApplicationWindow*  m_pWindow = nullptr;        /// Window that we are presenting to.
        DeviceQueue*        m_pDeviceQueue = nullptr;   /// Device queue that will be used to submit present commands.
        CommandPool*        m_pCommandPool = nullptr;   /// The Command Pool used to perform the initial image layout transition to present.
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
    class Swapchain final : public GraphicsResource 
    {
    public:
        explicit                    Swapchain(RenderDevice& device) : GraphicsResource(device) {}
        virtual                     ~Swapchain() override;
        /* Copy Ctor */             Swapchain(const Swapchain&) = delete;
        /* Move Ctor */             Swapchain(Swapchain&&) noexcept = delete;
        /* Copy Assignment */       Swapchain& operator=(const Swapchain&) = delete;
        /* Move Assignment */       Swapchain& operator=(Swapchain&&) noexcept = delete;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Initialize the swapchain.
        //----------------------------------------------------------------------------------------------------
        EGraphicsResult             Init(const SwapchainDesc& swapchainDesc);

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
        /// @brief : Sets the debug name for both the swapchain and the surface. 
        //----------------------------------------------------------------------------------------------------
        virtual void                SetDebugName(const char* name) override;

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
        VkImage                     GetImage() const                    { return m_images[m_frameImageIndex].m_image; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the image view for the current image that we are rendering to.
        //----------------------------------------------------------------------------------------------------
        VkImageView                 GetImageView() const                { return m_images[m_frameImageIndex].m_view; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the image format used for the swapchain images.
        //----------------------------------------------------------------------------------------------------
        VkFormat                    GetImageFormat() const              { return m_imageFormat; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the max number of frames that can be processed concurrently. 
        //----------------------------------------------------------------------------------------------------
        uint32                      GetMaxFramesInFlight() const        { return m_maxFramesInFlight; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the current semaphore that will be signaled when the next image is ready for rendering.
        //----------------------------------------------------------------------------------------------------
        VkSemaphore                 GetImageAvailableSemaphore() const  { return m_frameResources[m_frameResourceIndex].m_imageAvailable; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the current semaphore that will be signaled when rendering to the image has finished.
        //----------------------------------------------------------------------------------------------------
        VkSemaphore                 GetRenderFinishedSemaphore() const  { return m_frameResources[m_frameResourceIndex].m_renderFinished; }

    private:
        //----------------------------------------------------------------------------------------------------
        /// @brief : Represents an image within the swapchain that can be rendered to. 
        //----------------------------------------------------------------------------------------------------
        struct SwapchainImage
        {
            VkImage                 m_image{};                  /// Image to render to.
            VkImageView             m_view{};                   /// Image view to access the image.
        };

        //----------------------------------------------------------------------------------------------------
        /// @brief : Resources associated with each frame being processed. 
        //----------------------------------------------------------------------------------------------------
        struct FrameResources
        {
            VkSemaphore             m_imageAvailable{};         /// Signals when the image is ready for rendering.
            VkSemaphore             m_renderFinished{};         /// Signals when rendering is finished.
        };

        //----------------------------------------------------------------------------------------------------
        /// @brief : Select the format that is most common and supported by the physical device.
        //----------------------------------------------------------------------------------------------------
        VkSurfaceFormat2KHR         SelectSwapSurfaceFormat(const std::vector<VkSurfaceFormat2KHR>& availableFormats) const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Choose the present mode based on vSyncEnabled.
        ///     - The FIFO mode is the most common, and is used when vSync is enabled.
        ///     - The `m_preferredVsyncOffMode` is used when vSync is disabled and the mode is supported.
        ///     Otherwise:
        ///     - The IMMEDIATE mode is used when vSync is disabled, and is the best mode for low latency.
        ///     - The MAILBOX mode is used when vSync is disabled, and is the best mode for triple buffering.
        //----------------------------------------------------------------------------------------------------
        VkPresentModeKHR            SelectSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes, const bool vSyncEnabled = true) const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Build the swapchain and all of its resources, aside from the surface.
        //----------------------------------------------------------------------------------------------------
        EGraphicsResult             BuildSwapchain(const UVec2 desiredWindowSize, const bool enableVsync = true);
        
        //----------------------------------------------------------------------------------------------------
        /// @brief : Destroys the swapchain and all images/frame resources. 
        //----------------------------------------------------------------------------------------------------
        void                        DestroySwapchain();

    private:
        DeviceQueue*                m_pQueue = nullptr;         /// The queue used to submit command buffers to the GPU
        VkSwapchainKHR              m_swapchain{};              /// The swapchain object.
        VkFormat                    m_imageFormat{};            /// The image format for the swapchain images.
        VkSurfaceKHR                m_surface{};                /// The surface to present images to. Owned by the swapchain.
        CommandPool*                m_pCommandPool{};           /// The command pool for the swapchain.
        std::vector<SwapchainImage> m_images{};                 /// The swapchain images and their views.
        std::vector<FrameResources> m_frameResources{};         /// Synchronization primitives for each frame.
        uint32                      m_frameResourceIndex = 0;   /// Index of the current frame.
        uint32                      m_frameImageIndex = 0;      /// Index of the swapchain image we are rendering to.
        bool                        m_needsRebuild = false;     /// Flag indicating that the swapchain needs to be rebuilt.

        VkPresentModeKHR            m_preferredVsyncOffMode = VK_PRESENT_MODE_MAX_ENUM_KHR;  /// Used if available. 
        uint32                      m_maxFramesInFlight = 3;    /// Best for most cases.
    };
}
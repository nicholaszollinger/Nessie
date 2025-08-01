// Renderer.h
#pragma once
#include <functional>
#include "RenderDevice.h"
#include "RenderCommandQueue.h"
#include "Nessie/Core/Color.h"
#include "Nessie/Core/Memory/StrongPtr.h"
#include "Nessie/Core/Thread/WorkerThread.h"

//-------------------------------------------------------------------------------------------------
// Under development. The Renderer is being refactored as I abstract the Vulkan API.
//-------------------------------------------------------------------------------------------------

namespace nes
{
    class Camera;
    class Material;
    class RenderCommandBuffer;
    class RenderPass;
    
    //----------------------------------------------------------------------------------------------------
    // The Renderer is responsible for:
    // - Managing default graphics resources.
    // - Managing the Swapchain.
    // - Static API to issue draw commands.
    //
    /// @brief : The Renderer is a higher level API to issue commands to the GPU (draw commands, creating
    /// resources, compute commands, etc.).
    //----------------------------------------------------------------------------------------------------
    class Renderer
    {
    public:
        //----------------------------------------------------------------------------------------------------
        /// @brief : Enqueue a command to free a render resource.
        //----------------------------------------------------------------------------------------------------
        template <typename FuncType>
        static void                 SubmitResourceFree(FuncType&& func);
        
        //----------------------------------------------------------------------------------------------------
        /// @brief : Advanced use. Get the RenderCommandQueue of a specific frame used to release render resources. 
        //----------------------------------------------------------------------------------------------------
        static RenderCommandQueue&  GetRenderResourceReleaseQueue(const uint32 index);
    
    public:
        /* Constructor */           Renderer(RenderDevice& device);
        /* Destructor */            ~Renderer();

        /// No Copy or Move.
        /* Copy Constructor */      Renderer(const Renderer&) = delete;
        /* Copy Assignment */       Renderer& operator=(const Renderer&) = delete;
        /* Move Constructor */      Renderer(Renderer&&) noexcept = delete;
        /* Move Assignment */       Renderer& operator=(Renderer&&) noexcept = delete;
        
        //----------------------------------------------------------------------------------------------------
        /// @brief : Initialize the Renderer. If the window is nullptr, then no presenting will be allowed,
        ///     and the swapchain will not be created.
        //----------------------------------------------------------------------------------------------------
        bool                        Init(ApplicationWindow* pWindow, const RendererDesc& rendererDesc);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Shutdown the renderer, cleaning up all resources. 
        //----------------------------------------------------------------------------------------------------
        void                        Shutdown();
        
        //----------------------------------------------------------------------------------------------------
        /// @brief : Begin a new render frame. Must be called before any render commands.
        //----------------------------------------------------------------------------------------------------
        [[nodiscard]] bool          BeginFrame();

        //----------------------------------------------------------------------------------------------------
        /// @brief : End the current render frame. Must be called after all render commands have been submitted.
        //----------------------------------------------------------------------------------------------------
        void                        EndFrame();

        //----------------------------------------------------------------------------------------------------
        /// @brief : Begin a new render frame with no presentation. 
        //----------------------------------------------------------------------------------------------------
        void                        BeginHeadlessFrame();

        //----------------------------------------------------------------------------------------------------
        /// @brief : End a render frame with no presentation.
        //----------------------------------------------------------------------------------------------------
        void                        EndHeadlessFrame();

        //----------------------------------------------------------------------------------------------------
        /// @brief : Whenever the window is resized or if the vsync status changes, the Swapchain needs to be
        ///     rebuilt. Calling this will ensure that the swapchain is rebuilt on the next call to BeginFrame.
        //----------------------------------------------------------------------------------------------------
        void                        RequestSwapchainRebuild();

        //---------------------------------------------------------------------------------------------------
        /// @brief : Get the current frame index.
        //----------------------------------------------------------------------------------------------------
        uint32                      GetCurrentFrameIndex() const            { return m_currentFrameIndex; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Blocks until the render thread has finished, and the GPU has finished processing.  
        //----------------------------------------------------------------------------------------------------
        void                        WaitForFrameCompletion();
        
        //----------------------------------------------------------------------------------------------------
        /// @brief : Returns how long the Render thread was waiting to render the next frame.
        /// @note : Only call while synced with the main thread.
        //----------------------------------------------------------------------------------------------------
        float                       GetRenderThreadWaitTime() const { return m_renderThreadWaitTime; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Returns how long it took the render thread to render the previous frame.
        /// @note : Only call while synced with the main thread.
        //----------------------------------------------------------------------------------------------------
        float                       GetRenderThreadWorkTime() const { return m_renderThreadWaitTime; }
    
    protected:
        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the singleton instance of the renderer. 
        //----------------------------------------------------------------------------------------------------
        static Renderer*            Get();

        //----------------------------------------------------------------------------------------------------
        /// @brief : Execute functions in m_resourceFreeQueues[m_currentFrameIndex];   
        //----------------------------------------------------------------------------------------------------
        void                        ProcessResourceFreeQueue();

        //----------------------------------------------------------------------------------------------------
        /// @brief : Resets the frame's command pool and preps the frame's command buffer for recording. 
        //----------------------------------------------------------------------------------------------------
        void                        BeginCommandRecording();

        //----------------------------------------------------------------------------------------------------
        /// @brief : Calculates the signal value for when this frame completes.
        ///     Signal value = current frame number + numFramesInFlight
        ///     Example with 3 frames in flight:
        ///         - Frame 0 signals value 3 (allowing Frame 3 to start when complete).
        ///         - Frame 1 signals value 4 (allowing Frame 4 to start when complete).
        //----------------------------------------------------------------------------------------------------
        void                        PrepareFrameToSignal(const uint32 numFramesInFlight);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Creates a command pool (long life) and command buffer for each frame in flight.
        //----------------------------------------------------------------------------------------------------
        void                        CreateFrameSubmissionResources(const uint32 numFramesInFlight);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Clear the waitSemaphores, signalSemaphores, and command buffers from the previous frame.  
        //----------------------------------------------------------------------------------------------------
        void                        ClearPreviousFrameSubmissionData();

        //----------------------------------------------------------------------------------------------------
        /// @brief : Submit the current frame's command buffer to the Submission Queue. 
        //----------------------------------------------------------------------------------------------------
        void                        SubmitFrameCommands();

        //----------------------------------------------------------------------------------------------------
        /// @brief : Increments the circular m_currentFrameIndex to the next frame.
        //----------------------------------------------------------------------------------------------------
        void                        AdvanceToNextFrame(const uint32 numFramesInFlight);
    
    protected:
        static constexpr uint32                 kHeadlessFramesInFlight = 5;
        
        //----------------------------------------------------------------------------------------------------
        // [TODO]: In the future, I might have multiple command pools/buffers for each required device queue?
        //         I am thinking about a dedicated compute queue or dedicated transfer queue.
        /// @brief : Contains a dedicated command pool and buffer for rendering commands.
        //----------------------------------------------------------------------------------------------------
        struct FrameData
        {
            CommandPool*                        m_pCommandPool{};                           /// The Command Pool used for recording commands for this frame.
            CommandBuffer*                      m_pCommandBuffer{};                         /// The Command Buffer that contains this frame's rendering commands.
            uint64                              m_frameNumber{};                            /// Timeline value for synchronization (increases each frame).
        };
        
        RenderDevice&                           m_device;                                   /// Device to create graphics resources.
        ApplicationWindow*                      m_pWindow;                                  /// Window that we are rendering to.
        std::vector<RenderCommandQueue>         m_resourceFreeQueues;                       /// Command queues specific for freeing resources.

        DeviceQueue*                            m_pRenderSubmissionQueue = nullptr;
        CommandPool*                            m_pPresentCommandPool = nullptr;            /// Command pool owned by the main thread. 
        Swapchain*                              m_pSwapchain = nullptr;                     /// Object that manages the target framebuffer to render to.
        std::vector<FrameData>                  m_frames{};                                 /// Collection of per-frame resources to support multiple frames in flight.
        VkSemaphore                             m_frameTimelineSemaphore{};                 /// Timeline semaphore used to synchronize CPU submission and GPU completion.
        uint32                                  m_currentFrameIndex = 0;                    /// The current frame index in the frame data array (cycles through like a ring).

        // TODO: These should be wrapped, not the Vulkan Types.
        std::vector<VkSemaphoreSubmitInfo>      m_waitSemaphores;                           /// Possible extra wait semaphores.
        std::vector<VkSemaphoreSubmitInfo>      m_signalSemaphores;                         /// Possible extra frame signal semaphores.
        std::vector<VkCommandBufferSubmitInfo>  m_commandBuffers;                           /// Possible extra frame command buffers.
        
        // Performance Values
        float                                   m_renderThreadWorkTime;                     /// The amount of time the render thread was processing commands (ms).
        float                                   m_renderThreadWaitTime;                     /// The amount of time the render thread was waiting for the main thread (ms).
    };

    template <typename FuncType>
    void Renderer::SubmitResourceFree(FuncType&& func)
    {
        auto renderCommand = [](void* ptr)
        {
            // Call the function.
            auto pFunc = static_cast<FuncType*>(ptr);
            (*pFunc)();

            // Destroy the object.
            pFunc->~FuncType();
        };
        
        // if (IsRenderThread())
        // {
        //     const uint32 index = RT_GetCurrentFrameIndex();
        //     auto storageBuffer = GetRenderResourceReleaseQueue(index).Allocate(renderCommand, sizeof(func));
        //     new (storageBuffer) FuncType(std::forward<FuncType>((FuncType&&)func));
        // }
        //else
        {
            const uint32 frameIndex = Get()->GetCurrentFrameIndex();
            Submit([renderCommand, func, frameIndex]()
            {
                auto storageBuffer = GetRenderResourceReleaseQueue(frameIndex).Allocate(renderCommand, sizeof(func));
                new (storageBuffer) FuncType(std::forward<FuncType>((FuncType&&)func));
            });
        }
    }
}

// Renderer.h
#pragma once
#include <functional>
#include "RenderDevice.h"
#include "RenderCommandQueue.h"
#include "Nessie/Core/Color.h"
#include "Nessie/Core/Memory/StrongPtr.h"
#include "Nessie/Core/Thread/WorkerThread.h"
#include "DeviceQueue.h"
#include "CommandPool.h"
#include "Swapchain.h"

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
    /// @brief : Class containing information about the current frame, including the current image in the swapchain
    ///     to render to.
    //----------------------------------------------------------------------------------------------------
    class RenderFrameContext
    {
        friend class Renderer;
        
    public:
        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the current extent of the swapchain image. 
        //----------------------------------------------------------------------------------------------------
        const vk::Extent2D&         GetSwapchainExtent() const          { return m_swapchainExtent; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Returns a viewport that encompasses the entire swapchain image.
        //----------------------------------------------------------------------------------------------------
        Viewport                    GetSwapchainViewport() const        { return Viewport(m_swapchainExtent.width, m_swapchainExtent.height); }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the current swapchain image.
        //----------------------------------------------------------------------------------------------------
        const DeviceImage&          GetSwapchainImage() const           { return *m_swapchainImage; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the descriptor for the swapchain image. This can be used to set the swapchain image k
        ///     as a render target for a series of render calls.
        //----------------------------------------------------------------------------------------------------
        const Descriptor&           GetSwapchainImageDescriptor() const { return *m_swapchainImageDescriptor; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the current frame index. This will be a value between [0, Renderer::MaxFramesInFlight()].
        //----------------------------------------------------------------------------------------------------
        uint32                      GetFrameIndex() const               { return m_frameIndex; }
        
    private:
        const DeviceImage*          m_swapchainImage = nullptr;
        const Descriptor*           m_swapchainImageDescriptor = nullptr;
        vk::Extent2D                m_swapchainExtent{};
        uint32                      m_frameIndex = 0;
    };
    
    //----------------------------------------------------------------------------------------------------
    /// @brief : The Renderer is responsible for managing the Render Frame execution, the Swapchain, and
    ///  Command submission.
    //----------------------------------------------------------------------------------------------------
    class Renderer
    {
    public:
        using RecordCommandsFunc = std::function<void(RenderDevice& device, CommandBuffer& /*buffer*/)>;
        
        //----------------------------------------------------------------------------------------------------
        /// @brief : Enqueue a command to free a render resource.
        //----------------------------------------------------------------------------------------------------
        template <typename FuncType>
        static void                 SubmitResourceFree(FuncType&& func);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the Render Device object. This contains the Vulkan Instance, Device, Physical Device,
        ///     and Allocator utilities.
        //----------------------------------------------------------------------------------------------------
        static RenderDevice&        GetDevice();

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the maximum number of frames in flight.
        //----------------------------------------------------------------------------------------------------
        static uint32               GetMaxFramesInFlight();    

        //----------------------------------------------------------------------------------------------------
        /// @brief : Creates a temporary command buffer, records the given commands and submits them to the
        ///     graphics queue. This will block until complete.
        //----------------------------------------------------------------------------------------------------
        static void                 ExecuteImmediateCommands(const RecordCommandsFunc& func);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Creates and begins a temporary command buffer for recording commands.
        //----------------------------------------------------------------------------------------------------
        static CommandBuffer        BeginTempCommands();

        //----------------------------------------------------------------------------------------------------
        /// @brief : Submits a temporary command buffer created with BeginTempCommands(). This will block until
        /// complete.
        //----------------------------------------------------------------------------------------------------
        static void                 SubmitAndWaitTempCommands(CommandBuffer& cmdBuffer);
        
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
        /// @brief : Get the current frame's command buffer to record commands. 
        //----------------------------------------------------------------------------------------------------
        CommandBuffer&              GetCurrentCommandBuffer()               { return m_frames[m_currentFrameIndex].m_commandBuffer; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the context of the current render frame. This includes information about the current
        ///     swapchain image that we are rendering to.
        //----------------------------------------------------------------------------------------------------
        RenderFrameContext          GetRenderFrameContext() const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Blocks until the frame at the current frame index is completed. When rendering multiple frames,
        ///     this will be a previous frame that was submitted to the GPU. It ensures that we can render to the
        ///     current frame.
        //----------------------------------------------------------------------------------------------------
        void                        WaitForFrameCompletion();

        //----------------------------------------------------------------------------------------------------
        /// @brief : Wait until all frames have been completed.
        //----------------------------------------------------------------------------------------------------
        void                        WaitUntilAllFramesCompleted() const;
        
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
    
    private:
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
            CommandPool                         m_commandPool = nullptr;                   // The Command Pool used for recording commands for this frame.
            CommandBuffer                       m_commandBuffer = nullptr;                 // The Command Buffer that contains this frame's rendering commands.
            uint64                              m_frameNumber = 0;                          // Timeline value for synchronization (increases each frame).
        };
        
        RenderDevice&                           m_device;                                   // Device to create graphics resources.
        ApplicationWindow*                      m_pWindow;                                  // Window that we are rendering to.
        std::vector<RenderCommandQueue>         m_resourceFreeQueues;                       // Command queues specific for freeing resources.

        DeviceQueue*                            m_pRenderQueue = nullptr;                   // Device Queue that render commands are submitted to.
        CommandPool                             m_transientCommandPool = nullptr;           // Command Pool for creating temporary-usage command buffers, like staging resources.
        Swapchain                               m_swapchain = nullptr;                      // Object that manages the target framebuffer to render to.
        std::vector<FrameData>                  m_frames{};                                 // Collection of per-frame resources to support multiple frames in flight.
        vk::raii::Semaphore                     m_frameTimelineSemaphore = nullptr;         // Timeline semaphore used to synchronize CPU submission and GPU completion.
        uint32                                  m_currentFrameIndex = 0;                    // The current frame index in the frame data array (cycles through like a ring).
        
        std::vector<vk::SemaphoreSubmitInfo>      m_waitSemaphores;                         // Possible extra wait semaphores.
        std::vector<vk::SemaphoreSubmitInfo>      m_signalSemaphores;                       // Possible extra frame signal semaphores.
        std::vector<vk::CommandBufferSubmitInfo>  m_commandBuffers;                         // Possible extra frame command buffers.
        
        // Performance Values
        float                                   m_renderThreadWorkTime;                     // The amount of time the render thread was processing commands (ms).
        float                                   m_renderThreadWaitTime;                     // The amount of time the render thread was waiting for the main thread (ms).
    };

    template <typename FuncType>
    void Renderer::SubmitResourceFree(FuncType&& func)
    {
        const uint32 frameIndex = Get()->GetCurrentFrameIndex();
        
        // If the current frame is valid, add to the free queue.
        if (frameIndex < Get()->m_resourceFreeQueues.size())
        {
            auto renderCommand = [](void* ptr)
            {
                // Call the function.
                auto pFunc = static_cast<FuncType*>(ptr);
                (*pFunc)();

                // Destroy the object.
                pFunc->~FuncType();
            };
            
            auto storageBuffer = GetRenderResourceReleaseQueue(frameIndex).Allocate(renderCommand, sizeof(func));
            new (storageBuffer) FuncType(std::forward<FuncType>((FuncType&&)func));
        }
        
        // Otherwise, call it immediately. This can happen for resources destroyed after the Renderer has been closed.
        else
        {
            func();
        }
    }
}

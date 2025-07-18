// Renderer.h
#pragma once
#include <functional>
#include "Device.h"
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
    // - Managing the RenderThread (if multithreaded).
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
        // [TODO]: Render Pass
        //static void                BeginRenderPass(StrongPtr<RenderCommandBuffer> pCommandBuffer, StrongPtr<RenderPass> pRenderPass, bool explicitClear = false);
        //static void                EndRenderPass(StrongPtr<RenderCommandBuffer> pCommandBuffer);

        // [TODO]: Draw Calls 
        //static void                RenderGeometry(StrongPtr<RenderCommandBuffer> pCommandBuffer, StrongPtr<Pipeline> pPipeline, StrongPtr<Material> pMaterial, StrongPtr<VertexBuffer> pVertexBuffer, StrongPtr<IndexBuffer> pIndexBuffer, const Mat44& transform, uint32 indexCount = 0);

        // [TODO]: Get Default Resources

        // [TODO]: Shaders

        //----------------------------------------------------------------------------------------------------
        /// @brief : Enqueue a command to run on the GPU. 
        //----------------------------------------------------------------------------------------------------
        template <typename FuncType>
        static void                 Submit(FuncType&& func);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Enqueue a command to free a render resource.
        //----------------------------------------------------------------------------------------------------
        template <typename FuncType>
        static void                 SubmitResourceFree(FuncType&& func);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Returns true if the current executing thread is the Render Thread.
        //----------------------------------------------------------------------------------------------------
        static bool                 IsRenderThread();
        
        //----------------------------------------------------------------------------------------------------
        /// @brief : Advanced use. Get the RenderCommandQueue of a specific frame used to release render resources. 
        //----------------------------------------------------------------------------------------------------
        static RenderCommandQueue&  GetRenderResourceReleaseQueue(const uint32 index);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Advanced use, call only on the Render Thread.
        ///     Get the current frame index that we are rendering to.
        //----------------------------------------------------------------------------------------------------
        static uint32               RT_GetCurrentFrameIndex();
    
    public:
        Renderer();
        ~Renderer();

        /// No Copy or Move.
        Renderer(const Renderer&) = delete;
        Renderer& operator=(const Renderer&) = delete;
        Renderer(Renderer&&) noexcept = delete;
        Renderer& operator=(Renderer&&) noexcept = delete;
        
        //----------------------------------------------------------------------------------------------------
        /// @brief : Initialize the Renderer.
        //----------------------------------------------------------------------------------------------------
        bool                        Init(const RendererDesc& config);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Shutdown the renderer, cleaning up all resources. 
        //----------------------------------------------------------------------------------------------------
        void                        Shutdown();

        //----------------------------------------------------------------------------------------------------
        /// @brief : Begin a new render frame. Must be called before any render commands.
        //----------------------------------------------------------------------------------------------------
        void                        BeginFrame();

        //----------------------------------------------------------------------------------------------------
        /// @brief : End the current render frame. Must be called after all render commands have been submitted.
        //----------------------------------------------------------------------------------------------------
        void                        EndFrame();

        //----------------------------------------------------------------------------------------------------
        /// @brief : Handle the target window being resized, or changing it's vsync setting. 
        //----------------------------------------------------------------------------------------------------
        void                        UpdateSwapchain(const uint32 width, const uint32 height, const bool vsyncEnabled);

        //---------------------------------------------------------------------------------------------------
        /// @brief : Get the current frame index.
        //----------------------------------------------------------------------------------------------------
        uint32                      GetCurrentFrameIndex() const            { return m_currentFrameIndex; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Render a single frame. This will block until complete. 
        //----------------------------------------------------------------------------------------------------
        void                        RenderSingleFrame();

        //----------------------------------------------------------------------------------------------------
        /// @brief : Signals the Render thread to begin working on the previous frame.
        //----------------------------------------------------------------------------------------------------
        void                        SignalRenderThread();

        //----------------------------------------------------------------------------------------------------
        /// @brief : Swap the command queues.
        //----------------------------------------------------------------------------------------------------
        void                        SwapCommandQueues();

        //----------------------------------------------------------------------------------------------------
        /// @brief : Blocks until the render thread has finished.  
        //----------------------------------------------------------------------------------------------------
        void                        WaitUntilRenderComplete();
        
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
        enum class ERenderThreadInstruction : uint8
        {
            Init,
            RenderPreviousFrame,
            Terminate,
        };
        
        using RenderThread = WorkerThread<ERenderThreadInstruction>;
        
        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the singleton instance of the renderer. 
        //----------------------------------------------------------------------------------------------------
        static Renderer*            Get();

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the RenderCommandQueue to submit commands to. 
        //----------------------------------------------------------------------------------------------------
        RenderCommandQueue&         GetRenderCommandSubmissionQueue() const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the RenderCommandQueue to execute commands from. 
        //----------------------------------------------------------------------------------------------------
        RenderCommandQueue&         GetRenderCommandExecuteQueue() const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Process incoming instructions for the render thread.
        //----------------------------------------------------------------------------------------------------
        bool                        RT_ProcessInstruction(const ERenderThreadInstruction instruction);

        //----------------------------------------------------------------------------------------------------
        /// @brief : RenderThread function. Execute render commands stored on the previous frame. 
        //----------------------------------------------------------------------------------------------------
        bool                        RT_RenderPreviousFrame();
    
    protected:
        RenderThread                m_renderThread;                             /// Worker thread that executes render commands.
        RendererDesc                m_desc;                                     /// Settings passed from the Application to initialize the Renderer.
        RenderCommandQueue*         m_resourceFreeQueues;                       /// Command queues specific for freeing resources.
        RenderCommandQueue*         m_commandQueues[2]{};                       /// Two buffers, one to write commands to, one to execute commands from. 
        std::atomic<uint32>         m_commandQueueWriteIndex{0};          /// Index of the command queue that we are submitting commands to.
        Swapchain*                  m_pSwapchain = nullptr;                     /// Object that manages the target framebuffer to render to.
        uint32                      m_currentFrameIndex = 0;                    /// 

        // Performance Values
        float                       m_renderThreadWorkTime;                     /// The amount of time the render thread was processing commands (ms).
        float                       m_renderThreadWaitTime;                     /// The amount of time the render thread was waiting for the main thread (ms).
    };
    
    template <typename FuncType>
    void Renderer::Submit(FuncType&& func)
    {
        auto renderCommand = [](void* ptr)
        {
            // Call the function.
            auto pFunc = static_cast<FuncType*>(ptr);
            (*pFunc)();

            // Destroy the object.
            pFunc->~FuncType();
        };

        auto storageBuffer = Get()->GetRenderCommandSubmissionQueue().Allocate(renderCommand, sizeof(func));
        new (storageBuffer) FuncType(std::forward<FuncType>(func));
    }

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
        
        if (IsRenderThread())
        {
            const uint32 index = RT_GetCurrentFrameIndex();
            auto storageBuffer = GetRenderResourceReleaseQueue(index).Allocate(renderCommand, sizeof(func));
            new (storageBuffer) FuncType(std::forward<FuncType>((FuncType&&)func));
        }
        else
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

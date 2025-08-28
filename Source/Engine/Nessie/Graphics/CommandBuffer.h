// CommandBuffer.h
#pragma once
#include "Barriers.h"
#include "DeviceAsset.h"
#include "GraphicsCommon.h"

namespace nes
{
    class CommandPool;

    //----------------------------------------------------------------------------------------------------
    /// @brief : A Command Buffer is used to record commands that are then submitted to a DeviceQueue.
    //----------------------------------------------------------------------------------------------------
    class CommandBuffer final : public DeviceAsset
    {
    public:
        explicit                CommandBuffer(RenderDevice& device) : DeviceAsset(device) {}
        
        //----------------------------------------------------------------------------------------------------
        /// @brief : Do not call directly; this will be called by CommandPool::CreateCommandBuffer().
        //----------------------------------------------------------------------------------------------------
        EGraphicsResult         Init(CommandPool* pPool, vk::raii::CommandBuffer&& cmdBuffer);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Set a debug name for this command buffer. 
        //----------------------------------------------------------------------------------------------------
        virtual void            SetDebugName(const std::string& name) override;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Begin recording commands to this command buffer.
        //----------------------------------------------------------------------------------------------------
        void                    Begin();

        //----------------------------------------------------------------------------------------------------
        /// @brief : End recording commands to the buffer. The Command Buffer is now ready to be submitted to
        ///     a device queue. If there was an error during recording, this will return the invalid result
        ///     here and the Command Buffer will return to the invalid state (before Begin()).
        //----------------------------------------------------------------------------------------------------
        void                    End();

        //----------------------------------------------------------------------------------------------------
        /// @brief : Set a debug label for a set of commands. Must be ended with CommandBuffer::EndCommandLabel().
        //----------------------------------------------------------------------------------------------------
        void                    BeginCommandLabel(const std::string& label, const LinearColor& color = LinearColor::White());

        //----------------------------------------------------------------------------------------------------
        /// @brief : End the usage of a debug label for a set of commands. Must have called CommandBuffer::BeginCommandLabel().
        //----------------------------------------------------------------------------------------------------
        void                    EndCommandLabel();
        
        //----------------------------------------------------------------------------------------------------
        /// @brief : Transition an image from one layout to another. In the pipeline, the image must be in
        ///     the correct layout to be used.
        //----------------------------------------------------------------------------------------------------
        void                    TransitionImageLayout(vk::Image image, ImageMemoryBarrierDesc& barrierDesc) const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Begin rendering to a set of render targets. Must be followed with EndRendering().
        //----------------------------------------------------------------------------------------------------
        void                    BeginRendering(const RenderTargetsDesc& targetsDesc);

        //----------------------------------------------------------------------------------------------------
        /// @brief : End rendering will remove the 
        //----------------------------------------------------------------------------------------------------
        void                    EndRendering();

        //----------------------------------------------------------------------------------------------------
        /// @brief : Sets pixel information of the current render targets to given clear values.
        ///     - This can be used to 'clear the screen'.
        //----------------------------------------------------------------------------------------------------
        void                    ClearRenderTargets(const vk::ArrayProxy<ClearDesc>& clearDescs, const vk::ArrayProxy<vk::Rect2D>& clearRegions = {});

        //----------------------------------------------------------------------------------------------------
        /// @brief : Bind a pipeline.
        //----------------------------------------------------------------------------------------------------
        void                    BindPipeline(const Pipeline& pipeline);
        
        //----------------------------------------------------------------------------------------------------
        /// @brief : Bind the layout for a pipeline.
        //----------------------------------------------------------------------------------------------------
        void                    BindPipelineLayout(const PipelineLayout& pipelineLayout);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Set one or more viewports for the current pipeline. A Viewport determines what region
        ///     of the framebuffer to render to. An offset = (0, 0) and an extent = (imageWidth, imageHeight)
        ///     will render to the entire framebuffer. 
        //----------------------------------------------------------------------------------------------------
        void                    SetViewports(const vk::ArrayProxy<Viewport>& viewports);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Set one or more scissor rectangles. This rect defines which region pixels will actually
        ///     be stored on the output framebuffer. The rasterizer will discard any pixels outside the rect.
        ///     They function as a filter rather than a transformation.
        ///
        ///     To allow the full image to be rendered to, set the offset = (0, 0) and the extent = (imageWith, imageHeight). 
        //----------------------------------------------------------------------------------------------------
        void                    SetScissors(const vk::ArrayProxy<Scissor>& scissors);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Submit a set of vertices to be drawn - the vertices are either directly in the shader for
        ///     simple cases, or map to the bound vertex buffer. 
        //----------------------------------------------------------------------------------------------------
        void                    Draw(const DrawDesc& draw);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Submit a set of indices to draw from the bound vertex buffer. 
        //----------------------------------------------------------------------------------------------------
        void                    DrawIndexed(const DrawIndexedDesc& draw);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the Vulkan command buffer type.
        //----------------------------------------------------------------------------------------------------
        const vk::raii::CommandBuffer& GetVkCommandBuffer() const { return m_buffer; }

    public:
        //----------------------------------------------------------------------------------------------------
        /// @brief : Helper class that will set a label for commands within the given scope.
        //----------------------------------------------------------------------------------------------------
        class ScopedCommandLabel
        {
        public:
            ScopedCommandLabel(CommandBuffer& buffer, const std::string& label, const LinearColor& color = LinearColor::White());
            ~ScopedCommandLabel();
            
        private:
            CommandBuffer* m_commandBuffer = nullptr;
        };
    
    private:
        vk::raii::CommandBuffer m_buffer = nullptr;             // The Vulkan Command Buffer object.
        CommandPool*            m_pCommandPool = nullptr;       // The command pool that created this buffer.
        
        const Pipeline*         m_pPipeline = nullptr;          // The currently bound pipeline.
        const PipelineLayout*   m_pPipelineLayout = nullptr;    // The currently bound pipeline layout.
        const Descriptor*       m_depthStencil = nullptr;       // The current depth-stencil target for rendering commands.
        uint16                  m_renderLayerCount = 0;         // Number of image layers that we are rendering to.
        uint16                  m_renderWidth = 0;              // The width of the render area in pixels.
        uint16                  m_renderHeight = 0;             // The height of the render area in pixels. 
    };
}

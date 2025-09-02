// CommandBuffer.h
#pragma once
#include "Barriers.h"
#include "DeviceObject.h"
#include "GraphicsCommon.h"

namespace nes
{
    class CommandPool;

    //----------------------------------------------------------------------------------------------------
    /// @brief : A Command Buffer is used to record commands that are then submitted to a DeviceQueue.
    ///     Command Buffers are created with a Command Pool.
    //----------------------------------------------------------------------------------------------------
    class CommandBuffer
    {
    public:
        CommandBuffer(std::nullptr_t) {}
        CommandBuffer(const CommandBuffer&) = delete;
        CommandBuffer(CommandBuffer&& other) noexcept;
        CommandBuffer& operator=(std::nullptr_t);
        CommandBuffer& operator=(const CommandBuffer&) = delete;
        CommandBuffer& operator=(CommandBuffer&& other) noexcept;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Set a debug name for this command buffer. 
        //----------------------------------------------------------------------------------------------------
        void                    SetDebugName(const std::string& name);

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
        /// @brief : Copy the data from one device buffer to another.
        //----------------------------------------------------------------------------------------------------
        void                    CopyBuffer(const CopyBufferDesc& desc);

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
        /// @brief : Set the index buffer to use for the next DrawIndexed() call.
        //----------------------------------------------------------------------------------------------------
        void                    BindIndexBuffer(const IndexBufferDesc& desc);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Bind the vertex buffers used for the next draw call. 
        //----------------------------------------------------------------------------------------------------
        void                    BindVertexBuffers(const vk::ArrayProxy<nes::VertexBufferDesc>& buffers, const uint32 firstBinding = 0);  

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

        //----------------------------------------------------------------------------------------------------
        /// @brief : Advanced use. Get the native vulkan object handle, and the type.
        //----------------------------------------------------------------------------------------------------
        NativeVkObject          GetNativeVkObject() const;

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
        friend class CommandPool;
        
        //----------------------------------------------------------------------------------------------------
        /// @brief : Constructor called by the Command Pool. 
        //----------------------------------------------------------------------------------------------------
        CommandBuffer(RenderDevice& device, CommandPool& pool, vk::raii::CommandBuffer&& cmdBuffer);
    
    private:
        vk::raii::CommandBuffer m_buffer = nullptr;             // The Vulkan Command Buffer object.
        CommandPool*            m_pCommandPool = nullptr;       // The command pool that created this buffer.
        RenderDevice*           m_pDevice = nullptr;
        
        const Pipeline*         m_pPipeline = nullptr;          // The currently bound pipeline.
        const PipelineLayout*   m_pPipelineLayout = nullptr;    // The currently bound pipeline layout.
        const Descriptor*       m_depthStencil = nullptr;       // The current depth-stencil target for rendering commands.
        uint16                  m_renderLayerCount = 0;         // Number of image layers that we are rendering to.
        uint16                  m_renderWidth = 0;              // The width of the render area in pixels.
        uint16                  m_renderHeight = 0;             // The height of the render area in pixels. 
    };

    static_assert(DeviceObjectType<CommandBuffer>);
}

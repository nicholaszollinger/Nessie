// Renderer.h
#pragma once
#include "Mesh.h"
#include "RendererContext.h"
#include "Application/ApplicationWindow.h"
#include "Core/Generic/Color.h"

// [TODO]: Remove when API is set up to hide direct calls.
#include <imgui.h>

namespace GAP311 { class VulkanShaderLibrary; }

namespace nes
{
    NES_DEFINE_LOG_TAG(kRendererLogTag, "Renderer", Info);
    
    class RendererContext;
    using GraphicsPipelinePtr = std::shared_ptr<RendererContext::GraphicsPipeline>;

    //----------------------------------------------------------------------------------------------------
    /// @brief : The Renderer is responsible for drawing geometry to the window, managing resources
    ///     and operations directly with the GPU (if the system has one).
    //----------------------------------------------------------------------------------------------------
    class Renderer
    {
    public:
        using ImmediateCommandFunc = std::function<void(RendererContext& context)>;
    
    private:
        Renderer() = default;
        ~Renderer() = default;
        
    public:
        // No Copy or Move
        Renderer(const Renderer&) = delete;
        Renderer& operator=(const Renderer&) = delete;
        Renderer(Renderer&&) = delete;
        Renderer& operator=(Renderer&&) = delete;

    public:
        // BUFFERS:
        [[nodiscard]] static vk::Buffer CreateIndexBuffer(const void* pIndices, const size_t dataTypeSize, const size_t count);
        [[nodiscard]] static vk::Buffer CreateVertexBuffer(const void* pVertexData, const size_t vertexTypeSize, const size_t count);
        [[nodiscard]] static vk::Buffer CreateIndexBuffer(const std::vector<uint32_t>& bufferData);
        static void                     DestroyBuffer(vk::Buffer& buffer);
        static void                     UpdateBuffer(vk::Buffer buffer, const size_t offset, const size_t typeSize, const void* pData);

        // PIPELINE
        //----------------------------------------------------------------------------------------------------
        ///	@brief : Create a Graphics Pipeline that describes how to draw passed in geometry. You need to
        ///     bind the Pipeline before making draw calls.
        /// @param config : Configuration parameters for the Pipeline, including what Shaders to use.
        ///	@returns : Shared pointer to the new Pipeline.
        //----------------------------------------------------------------------------------------------------
        static GraphicsPipelinePtr      CreatePipeline(const nes::GraphicsPipelineConfig& config);
        static void                     DestroyPipeline(GraphicsPipelinePtr& pPipeline);
        static void                     BindGraphicsPipeline(GraphicsPipelinePtr& pPipeline);
        static void                     BindDescriptorSets(const GraphicsPipelinePtr& pPipeline, const vk::PipelineBindPoint bindPoint, const vk::ArrayProxy<const vk::DescriptorSet>& descriptorSets);

        // SHADERS:
        //----------------------------------------------------------------------------------------------------
        ///	@brief : Returns the Shader Module with the given name. In Debug Mode, if compilation is enabled,
        ///     the shader will be compiled on the fly if not already present.
        ///	@param shaderName : Name of the Shader
        ///	@returns : If nullptr, then no shader module with that name exists.
        //----------------------------------------------------------------------------------------------------
        static vk::ShaderModule         GetShader(const std::string& shaderName);

        //----------------------------------------------------------------------------------------------------
        //	NOTES:
        // [Consider] This should probably be a member function on a Pipeline object.
        // Should I also save the currently bound pipeline?
        //		
        /// @brief : Push a shader constant to the pipeline.
        //----------------------------------------------------------------------------------------------------
        static void                     PushShaderConstant(std::shared_ptr<RendererContext::GraphicsPipeline> pPipeline, vk::ShaderStageFlagBits shaderStage, const uint32_t offset, const uint32_t size, const void* pValues);

        // UNIFORMS:
        [[nodiscard]] static vk::Buffer CreateUniformBuffer(const size_t uniformTypeSize, void* pInitialData = nullptr);
        [[nodiscard]] static RendererContext::ShaderUniform CreateUniformForBuffer(int binding, vk::Buffer buffer, vk::DeviceSize size, vk::DeviceSize offset = 0, vk::ShaderStageFlags stages = vk::ShaderStageFlagBits::eAllGraphics);
        static void                     DestroyUniform(RendererContext::ShaderUniform uniform);
        
        //----------------------------------------------------------------------------------------------------
        ///	@brief : Allow recording overlay draw commands. Must be done within a Renderpass and followed
        ///  up with a call to EndImGui().
        //----------------------------------------------------------------------------------------------------
        static void                     BeginImGui();
        
        //----------------------------------------------------------------------------------------------------
        ///	@brief : Submit the Draw data for the Overlay Draw Commands. Must be done within a Renderpass
        ///  and preceded by a call to BeginImGui();
        //----------------------------------------------------------------------------------------------------
        static void                     EndImGui();
        static void                     BeginRenderPass(const vk::Rect2D& displayArea, const vk::ClearValue clearValues[], const uint32_t clearValueCount);
        static void                     EndRenderPass();

        //----------------------------------------------------------------------------------------------------
        ///	@brief : Draw a single, non-instanced VertexBuffer.
        ///	@param vertexBuffer : The array of vertices to submit.
        ///	@param vertexCount : Number of vertices in the buffer.
        //----------------------------------------------------------------------------------------------------
        static void                     Draw(const vk::Buffer& vertexBuffer, const uint32_t vertexCount);

        //----------------------------------------------------------------------------------------------------
        ///	@brief : Draw a set number of vertices. This is used when you have an array of vertices already
        ///     set in the shader, or if the vertices are already bound. 
        //----------------------------------------------------------------------------------------------------
        static void                     Draw(const uint32_t vertexCount, const uint32_t instanceCount, const uint32_t firstVertex, const uint32_t firstInstance);

        //----------------------------------------------------------------------------------------------------
        ///	@brief : Draw a single non-instanced set of vertices, using an index buffer. 
        ///	@param vertexBuffer : Array of vertices to submit.
        ///	@param indexBuffer : Index buffer associated with the vertex buffer.
        ///	@param indexCount : Number of indices to draw.      
        //----------------------------------------------------------------------------------------------------
        static void                     DrawIndexed(const vk::Buffer& vertexBuffer, const vk::Buffer& indexBuffer, const uint32_t indexCount);
        
        //----------------------------------------------------------------------------------------------------
        //  Hack for now, until I properly delineate the API between the Context and the
        //  Renderer. This will be done later in the semester or the summer, most likely.
        //  For now, I am going to grab the wrapper directly.	
        /// @brief : Get the context that stores base information about the Renderer.
        //----------------------------------------------------------------------------------------------------
        static RendererContext&         GetContext();
        
    protected:
        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the static Renderer instance.  
        //----------------------------------------------------------------------------------------------------
        static Renderer&                Instance();

        
        bool                            Init(ApplicationWindow* pWindow, const ApplicationProperties& appProperties);
        void                            Shutdown();

        //----------------------------------------------------------------------------------------------------
        /// @brief : Blocks until the Renderer is available for commands.  
        //----------------------------------------------------------------------------------------------------
        void                            WaitUntilIdle() const;

        //----------------------------------------------------------------------------------------------------
        ///	@brief : Begin rendering a new frame. IMPORTANT! If this function returns false, it means that
        ///  we have to rebuild the swapchain and *all draw commands will fail* due to the current command
        ///  buffer and framebuffer begin null.
        //----------------------------------------------------------------------------------------------------
        bool                            BeginFrame();

        //----------------------------------------------------------------------------------------------------
        ///	@brief : End Rendering for the current Frame. NOTE: Right now, you have to start and end a render
        ///  pass before calling this function! Otherwise, the display image will not return to the
        ///  proper format.
        //----------------------------------------------------------------------------------------------------
        void                            EndFrame();

        //----------------------------------------------------------------------------------------------------
        /// @brief : Initialize ImGui. Must be called if any ImGui commands are called. 
        //----------------------------------------------------------------------------------------------------
        void                            InitializeImGui();

        //----------------------------------------------------------------------------------------------------
        /// @brief : Cleanup ImGui subsystem.
        //----------------------------------------------------------------------------------------------------
        void                            ShutdownImGui();

    private:
        friend class Application;
        RendererContext*                m_pRenderContext = nullptr;
        ApplicationWindow*              m_pWindow = nullptr;
        GAP311::VulkanShaderLibrary*    m_pShaderLibrary = nullptr;

        vk::CommandBuffer               m_commandBuffer{};  /// The current frame's command buffer to handle incoming draw calls.
        vk::Framebuffer                 m_frameBuffer{};    /// The current frame's frame buffer.
    };
}

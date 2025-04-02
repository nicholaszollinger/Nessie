// Renderer.h
#pragma once
//#include "OverlayRenderer.h"
#include "Mesh.h"
#include "RendererContext.h"
#include "Application/Window.h"
#include "Core/Generic/Color.h"
#include "Math/Matrix.h"

// [TODO]: Remove when API is set up to hide direct calls.
#include <imgui.h>

namespace GAP311 { class VulkanShaderLibrary; }

namespace nes
{
    class RendererContext;
    
    class Renderer
    {
    public:
        using ImmediateCommandFunc = std::function<void(RendererContext& context)>;
        
    private:
        friend class Application;
        RendererContext* m_pRenderContext = nullptr;
        Window* m_pWindow = nullptr;
        GAP311::VulkanShaderLibrary* m_pShaderLibrary = nullptr;
        //OverlayRenderer m_overlayRenderer{};

        // The Current Frame's Command and Frame Buffer
        // to handle incoming draw calls.
        vk::CommandBuffer m_commandBuffer{};
        vk::Framebuffer m_frameBuffer{};

    private:
        Renderer() = default;
        ~Renderer() = default;
        
    public:
        // No Copy or Move
        Renderer(const Renderer&) = delete;
        Renderer& operator=(const Renderer&) = delete;
        Renderer(Renderer&&) = delete;
        Renderer& operator=(Renderer&&) = delete;

        // Buffers:
        [[nodiscard]] static vk::Buffer CreateIndexBuffer(const void* pIndices, const size_t dataTypeSize, const size_t count);
        [[nodiscard]] static vk::Buffer CreateVertexBuffer(const void* pVertexData, const size_t vertexTypeSize, const size_t count);
        [[nodiscard]] static vk::Buffer CreateIndexBuffer(const std::vector<uint32_t>& bufferData);
        static void DestroyBuffer(vk::Buffer& buffer);
        static void UpdateBuffer(vk::Buffer buffer, const size_t offset, const size_t typeSize, const void* pData);

        // Pipeline:
        static std::shared_ptr<RendererContext::GraphicsPipeline> CreatePipeline(const nes::GraphicsPipelineConfig& config);
        static void DestroyPipeline(std::shared_ptr<RendererContext::GraphicsPipeline>& pPipeline);
        static void BindGraphicsPipeline(std::shared_ptr<RendererContext::GraphicsPipeline>& pPipeline);
        static void BindDescriptorSets(const std::shared_ptr<RendererContext::GraphicsPipeline>& pPipeline,
            const vk::PipelineBindPoint bindPoint, const vk::ArrayProxy<const vk::DescriptorSet>& descriptorSets);

        // Shaders:
        static vk::ShaderModule GetShader(const std::string& shaderName);
        static void PushShaderConstant(std::shared_ptr<RendererContext::GraphicsPipeline> pPipeline, vk::ShaderStageFlagBits shaderStage, const uint32_t offset, const uint32_t size, const void* pValues);

        // Uniforms:
        [[nodiscard]] static vk::Buffer CreateUniformBuffer(const size_t uniformTypeSize, void* pInitialData = nullptr);
        [[nodiscard]] static RendererContext::ShaderUniform CreateUniformForBuffer(int binding, vk::Buffer buffer, vk::DeviceSize size, vk::DeviceSize offset = 0, vk::ShaderStageFlags stages = vk::ShaderStageFlagBits::eAllGraphics);
        static void DestroyUniform(RendererContext::ShaderUniform uniform);
        
        // Drawing:
        static void BeginImGui();
        static void EndImGui();
        static void BeginRenderPass(const vk::Rect2D& displayArea, const vk::ClearValue clearValues[], const uint32_t clearValueCount);
        static void EndRenderPass();
        static void Draw(const vk::Buffer& vertexBuffer, const uint32_t vertexCount);
        static void Draw(const uint32_t vertexCount, const uint32_t instanceCount, const uint32_t firstVertex, const uint32_t firstInstance);
        static void DrawIndexed(const vk::Buffer& vertexBuffer, const vk::Buffer& indexBuffer, const uint32_t indexCount);

        // Hack for now, until I properly delineate the API between the Context and the
        // Renderer. This will be done later in the semester or the summer, most likely.
        // For now, I am going to grab the wrapper directly.
        static RendererContext& GetContext();
        
    protected:
        static Renderer& Instance();
        bool Init(Window* pWindow, const ApplicationProperties& appProperties);
        void Shutdown();
        void WaitUntilIdle() const;
        
        bool BeginFrame();
        void EndFrame();
        
        void InitializeImGui();
        void ShutdownImGui();
    };
}

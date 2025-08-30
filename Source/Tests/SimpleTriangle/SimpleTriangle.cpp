// SimpleTriangle.cpp
#include "SimpleTriangle.h"
#include "Nessie/Application/EntryPoint.h"
#include "Nessie/Application/Device/DeviceManager.h"
#include "Nessie/Asset/AssetManager.h"
#include "Nessie/Graphics/CommandBuffer.h"
#include "Nessie/Graphics/RenderDevice.h"
#include "Nessie/Graphics/Renderer.h"
#include "Nessie/Graphics/Shader.h"

bool SimpleTriangle::Internal_AppInit()
{
    // Load the Simple Triangle Shader
    {
        std::filesystem::path shaderPath = NES_SHADER_DIR;
        shaderPath /= "SimpleTriangle.spv";

        const auto result = nes::AssetManager::LoadSync<nes::Shader>(m_shaderID, shaderPath);
        if (result != nes::ELoadResult::Success)
        {
            NES_ERROR("Failed to load Shader!");
            return false;
        }
        NES_LOG("Shader Loaded Successfully!");
    }

    auto& device = nes::DeviceManager::GetRenderDevice();

    // Get a Transfer Queue
    {
        const auto result = device.GetQueue(nes::EQueueType::Transfer, 0, m_pTransferQueue);
        if (result != nes::EGraphicsResult::Success)
        {
            NES_ERROR("Failed to get transfer queue!");
            return false;
        }
    }
    
    // Create a Graphics Pipeline:
    {
        static const std::array<Vertex, 3> vertices =
        {
            Vertex{{0.0f, -0.5f}, {1.0f, 0.0f, 0.0f}},
            Vertex{{0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
            Vertex{{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}} 
        };
        
        // Attributes of the Vertex struct
        std::array<nes::VertexAttributeDesc, 2> attributes =
        {
            nes::VertexAttributeDesc(0, offsetof(Vertex, m_position), nes::EFormat::RG32_SFLOAT, 0),
            nes::VertexAttributeDesc(1, offsetof(Vertex, m_color), nes::EFormat::RGB32_SFLOAT, 0),
        };

        // A single stream of Vertex elements:
        nes::VertexStreamDesc vertexStreamDesc = nes::VertexStreamDesc()
            .SetStride(sizeof(Vertex));

        nes::VertexInputDesc vertexInputDesc = nes::VertexInputDesc()
            .SetAttributes(attributes)
            .SetStreams(vertexStreamDesc);

        // Create the Vertex Buffer:
        nes::BufferDesc vertexBufferDesc{};
        vertexBufferDesc.m_usage = nes::EBufferUsageBits::VertexBuffer;
        vertexBufferDesc.m_size = sizeof(Vertex) * vertices.size();
        
        nes::AllocateBufferDesc desc{};
        desc.m_desc = vertexBufferDesc;
        desc.m_location = nes::EMemoryLocation::HostUpload; // We are setting the vertex data from the GPU.
        m_vertexBuffer = nes::DeviceBuffer(device, desc);
        
        // [TODO]: Change this.
        m_vertexBuffer.CopyToBuffer(vertices.data());
        m_vertexBufferDesc = nes::VertexBufferDesc(&m_vertexBuffer, sizeof(Vertex), 0);
        
        // Create the Pipeline Layout:
        nes::PipelineLayoutDesc layoutDesc{};
        m_pipelineLayout = nes::PipelineLayout(device, layoutDesc);

        // Vertex Shader Stages:
        nes::AssetPtr<nes::Shader> triangleShader = nes::AssetManager::GetAsset<nes::Shader>(m_shaderID);
        if (!triangleShader)
        {
            NES_ERROR("Failed to create Pipeline! Shader not present!");
            return false;
        }

        const auto& byteCode = triangleShader->GetByteCode();
        nes::ShaderDesc vertStage
        {
            .m_stage = nes::EPipelineStageBits::VertexShader,
            .m_pByteCode = byteCode.data(),
            .m_size = byteCode.size(),
            .m_entryPointName = "vertMain",
        };
        nes::ShaderDesc fragStage
        {
            .m_stage = nes::EPipelineStageBits::FragmentShader,
            .m_pByteCode = byteCode.data(),
            .m_size = byteCode.size(),
            .m_entryPointName = "fragMain",
        };

        // Create the Pipeline:
        nes::GraphicsPipelineDesc pipelineDesc = nes::GraphicsPipelineDesc()
            .SetShaderStages({ vertStage, fragStage })
            .SetVertexInput(vertexInputDesc);

        NES_ASSERT(m_pipelineLayout != nullptr);
        m_pipeline = nes::Pipeline(device, m_pipelineLayout, pipelineDesc);
    }

    return true;
}

void SimpleTriangle::Internal_AppUpdate([[maybe_unused]] const float timeStep)
{
    // [TODO]: 
}

void SimpleTriangle::Internal_AppRender(nes::CommandBuffer& commandBuffer, const nes::RenderFrameContext& context)
{
    // Transition the swapchain image to color attachment optimal:
    {
        nes::ImageMemoryBarrierDesc transitionBarrierDesc = nes::ImageMemoryBarrierDesc()
            .SetLayoutTransition(vk::ImageLayout::eUndefined, vk::ImageLayout::eColorAttachmentOptimal)
            .SetStages(vk::PipelineStageFlagBits2::eTopOfPipe, vk::PipelineStageFlagBits2::eColorAttachmentOutput)
            .SetAccessFlags({}, vk::AccessFlagBits2::eColorAttachmentWrite);
    
        commandBuffer.TransitionImageLayout(context.GetSwapchainImage().GetVkImage(), transitionBarrierDesc);
    }
    
    // Set up the attachment for rendering:
    nes::RenderTargetsDesc renderTargetsDesc = nes::RenderTargetsDesc()
        .SetColorTargets(&context.GetSwapchainImageDescriptor());

    // Get the viewport and scissor that will encompass the entire image.
    const nes::Viewport viewport = context.GetSwapchainViewport();
    const nes::Scissor scissor(viewport);

    // Render a triangle using the pipeline.
    commandBuffer.BeginRendering(renderTargetsDesc);
    {
        nes::ClearDesc clearDesc = nes::ClearDesc()
            .SetColorValue({ 0.01f, 0.01f, 0.01f, 1.0f });
        commandBuffer.ClearRenderTargets(clearDesc);
        
        commandBuffer.BindPipeline(m_pipeline);
        commandBuffer.BindPipelineLayout(m_pipelineLayout);
        
        commandBuffer.SetViewports(viewport);
        commandBuffer.SetScissors(scissor);
        commandBuffer.BindVertexBuffers(m_vertexBufferDesc);

        // Draw 3 vertices:
        commandBuffer.Draw(3);
        
        commandBuffer.EndRendering();
    }

    // Transition the image to present layout:
    {
        nes::ImageMemoryBarrierDesc transitionBarrierDesc = nes::ImageMemoryBarrierDesc()
            .SetLayoutTransition(vk::ImageLayout::eColorAttachmentOptimal, vk::ImageLayout::ePresentSrcKHR)
            .SetStages(vk::PipelineStageFlagBits2::eColorAttachmentOutput, vk::PipelineStageFlagBits2::eBottomOfPipe)
            .SetAccessFlags(vk::AccessFlagBits2::eColorAttachmentWrite, {});
    
        commandBuffer.TransitionImageLayout(context.GetSwapchainImage().GetVkImage(), transitionBarrierDesc);
    }
}

void SimpleTriangle::Internal_AppShutdown()
{
    m_vertexBuffer = nullptr;
    m_pipeline = nullptr;
    m_pipelineLayout = nullptr;
}

std::unique_ptr<nes::Application> nes::CreateApplication(ApplicationDesc& outAppDesc, WindowDesc& outWindowDesc, RendererDesc& outRendererDesc)
{
    outAppDesc.SetApplicationName("Triangle")
        .SetIsHeadless(false);
       
    outWindowDesc.SetResolution(720, 720)
        .SetLabel("Triangle")
        .SetWindowMode(EWindowMode::Windowed)
        .EnableResize(true)
        .EnableVsync(false);

    outRendererDesc.EnableValidationLayer()
        .RequireQueueType(EQueueType::Graphics)
        .RequireQueueType(EQueueType::Transfer);

    return std::make_unique<SimpleTriangle>(outAppDesc);
}

NES_MAIN()
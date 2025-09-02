// Rectangle.cpp
#include "RectangleApp.h"
#include "Nessie/Application/EntryPoint.h"
#include "Nessie/Application/Device/DeviceManager.h"
#include "Nessie/Asset/AssetManager.h"
#include "Nessie/Graphics/CommandBuffer.h"
#include "Nessie/Graphics/DataUploader.h"
#include "Nessie/Graphics/RenderDevice.h"
#include "Nessie/Graphics/Renderer.h"
#include "Nessie/Graphics/Shader.h"

bool RectangleApp::Internal_AppInit()
{
    // Load the Simple Shader
    {
        std::filesystem::path shaderPath = NES_SHADER_DIR;
        shaderPath /= "Rectangle.spv";

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
        const std::vector<Vertex> vertices =
        {
            {{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},
            {{0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},
            {{0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},
            {{-0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}}
        };
        const uint64 vertexBufferSize = (sizeof(Vertex) * vertices.size());
        
        const std::vector<uint16> indices =
        {
            0, 1, 2, 2, 3, 0
        };
        const uint64 indexBufferSize = (sizeof(uint16) * indices.size());
        
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
        
        // Allocate the Geometry Buffer:
        // - This single device buffer will contain both the vertices and the indices. The indices are stored after the vertices.
        {
            nes::AllocateBufferDesc desc;
            desc.m_size = vertexBufferSize + indexBufferSize;
            desc.m_location = nes::EMemoryLocation::Device;
            desc.m_usage = nes::EBufferUsageBits::IndexBuffer | nes::EBufferUsageBits::VertexBuffer;
            m_geometryBuffer = nes::DeviceBuffer(device, desc);
            
            m_vertexBufferDesc = nes::VertexBufferDesc(&m_geometryBuffer, sizeof(Vertex), 0);
            m_indexBufferDesc = nes::IndexBufferDesc(&m_geometryBuffer, nes::EIndexType::U16, vertexBufferSize);
        }

        // Upload the vertex and index data to the buffer.
        {
            nes::CommandBuffer buffer = nes::Renderer::BeginTempCommands();
            nes::DataUploader uploader(device);
        
            // Vertex Buffer data
            nes::UploadBufferDesc vertexUpload;
            vertexUpload.m_pBuffer = &m_geometryBuffer;
            vertexUpload.m_pData = vertices.data();
            vertexUpload.m_uploadOffset = 0;
            vertexUpload.m_uploadSize = vertexBufferSize;
            uploader.AppendUploadBuffer(vertexUpload);

            // Index Buffer data
            nes::UploadBufferDesc indexUpload;
            indexUpload.m_pBuffer = &m_geometryBuffer;
            indexUpload.m_pData = indices.data();
            indexUpload.m_uploadOffset = vertexBufferSize;
            indexUpload.m_uploadSize = indexBufferSize;
            uploader.AppendUploadBuffer(indexUpload);
            
            uploader.RecordCommands(buffer);
            nes::Renderer::SubmitAndWaitTempCommands(buffer);

            // Release staging buffer resources.
            uploader.Destroy();
        }
        
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

void RectangleApp::Internal_AppUpdate([[maybe_unused]] const float timeStep)
{
    // nothing to do.
}

void RectangleApp::Internal_AppRender(nes::CommandBuffer& commandBuffer, const nes::RenderFrameContext& context)
{
    // Transition the swapchain image to color attachment optimal layout, so we can render to it:
    {
        nes::ImageMemoryBarrierDesc transitionBarrierDesc = nes::ImageMemoryBarrierDesc()
            .SetLayoutTransition(vk::ImageLayout::eUndefined, vk::ImageLayout::eColorAttachmentOptimal)
            .SetStages(vk::PipelineStageFlagBits2::eTopOfPipe, vk::PipelineStageFlagBits2::eColorAttachmentOutput)
            .SetAccessFlags({}, vk::AccessFlagBits2::eColorAttachmentWrite);
    
        commandBuffer.TransitionImageLayout(context.GetSwapchainImage().GetVkImage(), transitionBarrierDesc);
    }
    
    // Set the swapchain image as our color render target:
    nes::RenderTargetsDesc renderTargetsDesc = nes::RenderTargetsDesc()
        .SetColorTargets(&context.GetSwapchainImageDescriptor());

    // Get the viewport and scissor that will encompass the entire image.
    const nes::Viewport viewport = context.GetSwapchainViewport();
    const nes::Scissor scissor(viewport);

    // Render a rectangle using the pipeline.
    commandBuffer.BeginRendering(renderTargetsDesc);
    {
        // Clear the screen to a dark grey color:
        nes::ClearDesc clearDesc = nes::ClearDesc()
            .SetColorValue({ 0.01f, 0.01f, 0.01f, 1.0f });
        commandBuffer.ClearRenderTargets(clearDesc);

        // Set our pipeline and render area:
        commandBuffer.BindPipeline(m_pipeline);
        commandBuffer.BindPipelineLayout(m_pipelineLayout);
        commandBuffer.SetViewports(viewport);
        commandBuffer.SetScissors(scissor);
        
        // Draw the rectangle:
        commandBuffer.BindIndexBuffer(m_indexBufferDesc);
        commandBuffer.BindVertexBuffers(m_vertexBufferDesc);
        commandBuffer.DrawIndexed(6);

        // Finish.
        commandBuffer.EndRendering();
    }

    // Transition the swapchain image to present layout:
    {
        nes::ImageMemoryBarrierDesc transitionBarrierDesc = nes::ImageMemoryBarrierDesc()
            .SetLayoutTransition(vk::ImageLayout::eColorAttachmentOptimal, vk::ImageLayout::ePresentSrcKHR)
            .SetStages(vk::PipelineStageFlagBits2::eColorAttachmentOutput, vk::PipelineStageFlagBits2::eBottomOfPipe)
            .SetAccessFlags(vk::AccessFlagBits2::eColorAttachmentWrite, {});
    
        commandBuffer.TransitionImageLayout(context.GetSwapchainImage().GetVkImage(), transitionBarrierDesc);
    }
}

void RectangleApp::Internal_AppShutdown()
{
    m_geometryBuffer = nullptr;
    m_pipeline = nullptr;
    m_pipelineLayout = nullptr;
}

std::unique_ptr<nes::Application> nes::CreateApplication(ApplicationDesc& outAppDesc, WindowDesc& outWindowDesc, RendererDesc& outRendererDesc)
{
    outAppDesc.SetApplicationName("Rectangle")
        .SetIsHeadless(false);
       
    outWindowDesc.SetResolution(720, 720)
        .SetLabel("Rectangle")
        .SetWindowMode(EWindowMode::Windowed)
        .EnableResize(true)
        .EnableVsync(false);

    outRendererDesc.EnableValidationLayer()
        .RequireQueueType(EQueueType::Graphics)
        .RequireQueueType(EQueueType::Transfer);

    return std::make_unique<RectangleApp>(outAppDesc);
}

NES_MAIN()
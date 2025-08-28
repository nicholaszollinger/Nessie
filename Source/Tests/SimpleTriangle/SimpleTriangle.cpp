// SimpleTriangle.cpp
#include "SimpleTriangle.h"
#include "Nessie/Application/EntryPoint.h"
#include "Nessie/Application/Device/DeviceManager.h"
#include "Nessie/Asset/AssetManager.h"
#include "Nessie/Graphics/CommandBuffer.h"
#include "Nessie/Graphics/DeviceImage.h"
#include "Nessie/Graphics/Pipeline.h"
#include "Nessie/Graphics/PipelineLayout.h"
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
        
    // Create a Graphics Pipeline:
    {
        auto& device = nes::DeviceManager::GetRenderDevice();
        
        // Create the Pipeline Layout:
        nes::PipelineLayoutDesc layoutDesc{};
        nes::EGraphicsResult result = device.CreateResource<nes::PipelineLayout>(m_pPipelineLayout, layoutDesc);
        if (result != nes::EGraphicsResult::Success)
        {
            NES_ERROR("Failed to create Graphics Pipeline Layout!");
            return false;
        }

        // Create the Pipeline
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
        
        nes::GraphicsPipelineDesc pipelineDesc = nes::GraphicsPipelineDesc()
            .SetShaderStages({ vertStage, fragStage });

        NES_ASSERT(m_pPipelineLayout != nullptr);
        result = device.CreateResource<nes::Pipeline>(m_pPipeline, *m_pPipelineLayout, pipelineDesc);
        if (result != nes::EGraphicsResult::Success)
        {
            NES_ERROR("Failed to create Graphics Pipeline!");
            return false;
        }
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
        
        commandBuffer.BindPipeline(*m_pPipeline);
        commandBuffer.BindPipelineLayout(*m_pPipelineLayout);
        
        commandBuffer.SetViewports(viewport);
        commandBuffer.SetScissors(scissor);

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
    auto& device = nes::DeviceManager::GetRenderDevice();

    // Free the pipeline resource.
    if (m_pPipeline)
        device.FreeResource(m_pPipeline);

    if (m_pPipelineLayout)
        device.FreeResource(m_pPipelineLayout);
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

    outRendererDesc.EnableValidationLayer();

    return std::make_unique<SimpleTriangle>(outAppDesc);
}

NES_MAIN()
// Vulkan_Renderer.cpp
#include "Core/Config.h"
#ifdef NES_RENDER_API_VULKAN

#include "Core/Memory/Memory.h"
#include "Graphics/Renderer.h"
#include "Vulkan_ShaderLibrary.hpp"
#include "Debug/CheckedCast.h"

#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_vulkan.h"

namespace nes
{
    static Renderer* g_pInstance = nullptr;
    
    bool Renderer::Init(nes::Window* pWindow, const ApplicationProperties& appProperties)
    {
        NES_ASSERT(g_pInstance == nullptr);
        g_pInstance = this;
        
        NES_ASSERT(m_pRenderContext == nullptr);
        NES_ASSERT(pWindow);
        m_pWindow = pWindow;

        // Initialize the RendererContext    object:
        // [TODO]: This should be loaded from data or handled by a function.
        RendererContext::ConfigOptions options{};
        options.m_debugLogFunc = RendererContext::s_defaultDebugLogFunction;
        options.m_enableDepthStencilBuffer = true;
        options.m_configureDeviceFunc = [](vkb::PhysicalDeviceSelector& selector)
        {
            selector.set_required_features(vk::PhysicalDeviceFeatures()
                .setFillModeNonSolid(true) // Support Wireframe
                .setDualSrcBlend(true)
                );   
        };

        m_pRenderContext = NES_NEW(RendererContext());
        if (!m_pRenderContext->Init(pWindow, appProperties, options))
        {
            NES_ERROR(kRendererLogTag, "Failed to initialize Renderer! Failed to initialize RendererContext!");
            return false;
        }

        NES_LOG(kRendererLogTag, "Selected Device: {}", m_pRenderContext->GetPhysicalDevice().getProperties().deviceName.data());

        // Initialize the Shader Library
        
        GAP311::VulkanShaderLibrary::ConfigOptions shaderOptions{};
#if _DEBUG
        shaderOptions.logMessage = []([[maybe_unused]] const char* msg)
        {
            NES_LOG("ShaderLib: {}", msg);            
        };
#endif
        // [TODO]: Load these from Data:
        shaderOptions.searchDirs =
        {
            NES_SHADER_DIR,
            //std::filesystem::current_path().string(), // working dir
            //std::filesystem::path(argv[0]).parent_path().string(), // exe dir
            //std::filesystem::relative(std::filesystem::path(__FILE__).parent_path()).string(), // source file dir
        };
        
        m_pShaderLibrary = NES_NEW(GAP311::VulkanShaderLibrary());
        if (!m_pShaderLibrary->Initialize(m_pRenderContext->GetDevice(), shaderOptions))
        {
            NES_ERROR(kRendererLogTag, "Failed to initialize Renderer! Failed to initializeShader Library!");
            return false;
        }

        // Initialize ImGui.
        // [TODO]: This should be a setting to include or not.
        InitializeImGui();
        
        return true;
    }

    void Renderer::Shutdown()
    {
        NES_ASSERT(g_pInstance == this);
        
        if (m_pRenderContext == nullptr)
            return;

        m_pRenderContext->GetDevice().waitIdle();

        // ImGui:
        ShutdownImGui();

        // Shader Library:
        if (m_pShaderLibrary != nullptr)
        {
            m_pShaderLibrary->Shutdown();
            NES_DELETE(m_pShaderLibrary);
            m_pShaderLibrary = nullptr;
        }

        // Render Context:
        m_pRenderContext->Shutdown();
        NES_DELETE(m_pRenderContext);
        m_pRenderContext = nullptr;

        // Null out the instance:
        g_pInstance = nullptr;;
    }
    
    void Renderer::WaitUntilIdle() const
    {
        m_pRenderContext->GetDevice().waitIdle();
    }
    
    bool Renderer::BeginFrame()
    {
        if (!m_pRenderContext->BeginFrame(m_commandBuffer, m_frameBuffer))
            return false;

        return true;
    }
    
    void Renderer::EndFrame()
    {
        // NOTE: There must be a Render pass start and end before finishing a Frame with the current
        // architecture.
        m_pRenderContext->EndFrame();
    }

    void Renderer::InitializeImGui()
    {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        (void)io;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
        io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;       // Enable Multi-Viewport / Platform Windows
        
        // Style
        ImGui::StyleColorsDark();
        
        // When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
        ImGuiStyle& style = ImGui::GetStyle();
        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            style.WindowRounding = 0.0f;
            style.Colors[ImGuiCol_WindowBg].w = 1.0f;
        }
        
        // Setup Platform/Renderer backends
        ImGui_ImplGlfw_InitForVulkan(checked_cast<GLFWwindow*>(m_pWindow->GetNativeWindowHandle()), true);
        ImGui_ImplVulkan_InitInfo init_info = {};
        init_info.Instance = m_pRenderContext->GetInstance();
        init_info.PhysicalDevice = m_pRenderContext->GetPhysicalDevice();
        init_info.Device = m_pRenderContext->GetDevice();
        init_info.QueueFamily = m_pRenderContext->GetGraphicsQueueIndex();
        init_info.Queue = m_pRenderContext->GetGraphicsQueue();
        init_info.PipelineCache = nullptr;
        init_info.DescriptorPool = m_pRenderContext->GetDescriptorPool();
        init_info.RenderPass = m_pRenderContext->GetDisplayRenderPass(); 
        init_info.Subpass = 0;
        init_info.MinImageCount = 2;
        init_info.ImageCount = m_pRenderContext->GetImageCount();
        init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
        init_info.Allocator = nullptr;
        
        // [TODO]: 
        //init_info.CheckVkResultFn = check_vk_result;
        ImGui_ImplVulkan_Init(&init_info);
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Close ImGui. 
    //----------------------------------------------------------------------------------------------------
    void Renderer::ShutdownImGui()
    {
        ImGui_ImplVulkan_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Begin a render pass for submitting geometry. Must be paired with a call to EndRenderPass()
    ///             after all draw calls to properly submit.
    ///		@param displayArea : Area of the window to draw on. Offset = {0, 0} and Extent = WindowExtent would be the
    ///                     full screen.
    ///		@param clearValues : The clear value used before drawing to the screen.
    //----------------------------------------------------------------------------------------------------
    void Renderer::BeginRenderPass(const vk::Rect2D& displayArea, const vk::ClearValue clearValues[], const uint32_t clearValueCount)
    {
        vk::RenderPassBeginInfo renderPassInfo{};
        renderPassInfo.setRenderArea(displayArea);
        renderPassInfo.setClearValues(*clearValues);
        renderPassInfo.setClearValueCount(clearValueCount);
        renderPassInfo.setFramebuffer(Instance().m_frameBuffer);
        renderPassInfo.setRenderPass(Instance().m_pRenderContext->GetDisplayRenderPass());
        Instance().m_commandBuffer.beginRenderPass(renderPassInfo, vk::SubpassContents::eInline);
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Ends the current Render Pass. Must be preceded with a call to BeginRenderPass().  
    //----------------------------------------------------------------------------------------------------
    void Renderer::EndRenderPass()
    {
        Instance().m_commandBuffer.endRenderPass();
    }

    //----------------------------------------------------------------------------------------------------
    //		NOTES:
    //		
    ///		@brief : Create an Index Buffer of a given size and initial data.
    ///		@param pIndices : Array of indices to initialize the buffer with.
    ///		@param dataTypeSize : Size of the index type.
    ///		@param count : Number of indices in the array.
    ///		@returns : Handle to the Buffer object.
    //----------------------------------------------------------------------------------------------------
    vk::Buffer Renderer::CreateIndexBuffer(const void* pIndices, const size_t dataTypeSize, const size_t count)
    {
        return Instance().m_pRenderContext->CreateIndexBuffer(dataTypeSize * count, pIndices);
    }

    //----------------------------------------------------------------------------------------------------
    //		NOTES:
    //		
    ///		@brief : Create a Vertex Buffer of a given size and initial Data.  
    ///		@param vertexTypeSize : Size of the Vertex object, in bytes. "sizeof(VertexType)"
    ///		@param count : Number of vertices in the initial buffer.
    ///		@param pVertexData : Data to initialize the buffer with.
    ///		@returns : Handle to the Buffer object.
    //----------------------------------------------------------------------------------------------------
    vk::Buffer Renderer::CreateVertexBuffer(const void* pVertexData, const size_t vertexTypeSize, const size_t count)
    {
        return Instance().m_pRenderContext->CreateVertexBuffer(vertexTypeSize * count, pVertexData);
    }

    vk::Buffer Renderer::CreateIndexBuffer(const std::vector<uint32_t>& bufferData)
    {
        return Instance().m_pRenderContext->CreateIndexBuffer(bufferData.size() * sizeof(uint32_t), bufferData.data());
    }

    void Renderer::DestroyBuffer(vk::Buffer& buffer)
    {
        Instance().m_pRenderContext->DestroyBuffer(buffer);
        buffer = nullptr;
    }

    void Renderer::UpdateBuffer(vk::Buffer buffer, const size_t offset, const size_t typeSize, const void* pData)
    {
        Instance().m_commandBuffer.updateBuffer(buffer, offset, typeSize, pData);
    }
    
    std::shared_ptr<RendererContext::GraphicsPipeline> Renderer::CreatePipeline(const nes::GraphicsPipelineConfig& config)
    {
        return Instance().m_pRenderContext->CreatePipeline(config);
    }

    void Renderer::DestroyPipeline(std::shared_ptr<RendererContext::GraphicsPipeline>& pPipeline)
    {
        Instance().m_pRenderContext->DestroyPipeline(pPipeline);
    }
    
    vk::ShaderModule Renderer::GetShader(const std::string& shaderName)
    {
        return Instance().m_pShaderLibrary->GetModule(shaderName.c_str());
    }

    void Renderer::PushShaderConstant(std::shared_ptr<RendererContext::GraphicsPipeline> pPipeline,
        vk::ShaderStageFlagBits shaderStage, const uint32_t offset, const uint32_t size, const void* pValues)
    {
        Instance().m_commandBuffer.pushConstants(pPipeline->GetLayout(), shaderStage, offset, size, pValues);
    }

    vk::Buffer Renderer::CreateUniformBuffer(const size_t uniformTypeSize, void* pInitialData)
    {
        return Instance().m_pRenderContext->CreateUniformBuffer(uniformTypeSize, pInitialData);
    }

    RendererContext::ShaderUniform Renderer::CreateUniformForBuffer(int binding, vk::Buffer buffer, vk::DeviceSize size,
        vk::DeviceSize offset, vk::ShaderStageFlags stages)
    {
        return Instance().m_pRenderContext->CreateUniformForBuffer(binding, buffer, size, offset, stages);
    }

    void Renderer::DestroyUniform(RendererContext::ShaderUniform uniform)
    {
        Instance().m_pRenderContext->DestroyUniform(uniform);
    }

    void Renderer::BindGraphicsPipeline(std::shared_ptr<RendererContext::GraphicsPipeline>& pPipeline)
    {
        NES_ASSERT(pPipeline != nullptr);
        Instance().m_commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pPipeline->GetPipeline());
    }

    void Renderer::BindDescriptorSets(const std::shared_ptr<RendererContext::GraphicsPipeline>& pPipeline,
        const vk::PipelineBindPoint bindPoint, const vk::ArrayProxy<const vk::DescriptorSet>& descriptorSets)
    {
        Instance().m_commandBuffer.bindDescriptorSets(bindPoint, pPipeline->GetLayout(), 0, descriptorSets, {});
    }
    
    void Renderer::BeginImGui()
    {
        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
    }

    void Renderer::EndImGui()
    {
        auto& drawCommandBuffer = Instance().m_commandBuffer;
        
        ImGui::Render();
        ImDrawData* pDrawData = ImGui::GetDrawData();
        ImGui_ImplVulkan_RenderDrawData(pDrawData, drawCommandBuffer);
        
        ImGuiIO& io = ImGui::GetIO(); (void)io;
        // Update and Render additional Platform Windows
        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
        }
    }
    
    void Renderer::Draw(const vk::Buffer& vertexBuffer, const uint32_t vertexCount)
    {
        // [Consider] I can submit multiple buffers at once...
        Instance().m_commandBuffer.bindVertexBuffers(0, vertexBuffer, {0});
        //m_commandBuffer.bindIndexBuffer()
        Instance().m_commandBuffer.draw(vertexCount, 1, 0, 0);
    }
    
    void Renderer::Draw(const uint32_t vertexCount, const uint32_t instanceCount, const uint32_t firstVertex, const uint32_t firstInstance)
    {
        auto& commandBuffer = Instance().m_commandBuffer;
        commandBuffer.draw(vertexCount, instanceCount, firstVertex, firstInstance);
    }
    
    void Renderer::DrawIndexed(const vk::Buffer& vertexBuffer, const vk::Buffer& indexBuffer, const uint32_t indexCount)
    {
        auto& commandBuffer = Instance().m_commandBuffer;
        commandBuffer.bindVertexBuffers(0, vertexBuffer, {0});
        commandBuffer.bindIndexBuffer(indexBuffer, 0, vk::IndexType::eUint32);
        commandBuffer.drawIndexed(indexCount, 1, 0, 0, 0);
    }

    RendererContext& Renderer::GetContext()
    {
        auto* pContext = Renderer::Instance().m_pRenderContext;
        NES_ASSERT(pContext != nullptr);
        return *pContext;
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Get the static instance of the Renderer class. 
    //----------------------------------------------------------------------------------------------------
    Renderer& Renderer::Instance()
    {
        NES_ASSERT(g_pInstance != nullptr);
        return *g_pInstance;
    }
}

#endif
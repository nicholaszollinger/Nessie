// ImGuiInterface.cpp
#include "ImGuiRenderer.h"

#include <algorithm>
#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_vulkan.h"

#include "Nessie/Asset/AssetManager.h"
#include "Nessie/Graphics/Shader.h"
#include "Nessie/Graphics/CommandBuffer.h"
#include "Nessie/Graphics/Renderer.h"

namespace nes
{
    ImGuiRenderer::ImGuiRenderer(ImGuiRenderer&& other) noexcept
        : m_pDevice(other.m_pDevice)
        , m_descriptorPool(std::move(other.m_descriptorPool))
        , m_iniSettingsPath(std::move(other.m_iniSettingsPath))
    {
        other.m_pDevice = nullptr;
    }

    ImGuiRenderer& ImGuiRenderer::operator=(std::nullptr_t)
    {
        Destroy();
        return *this;
    }

    ImGuiRenderer& ImGuiRenderer::operator=(ImGuiRenderer&& other) noexcept
    {
        if (this != &other)
        {
            m_pDevice = other.m_pDevice;
            m_descriptorPool = std::move(other.m_descriptorPool);
            m_iniSettingsPath = std::move(other.m_iniSettingsPath);
            
            other.m_pDevice = nullptr;
        }

        return *this;
    }
    
    ImGuiRenderer::~ImGuiRenderer()
    {
        if (m_pDevice != nullptr)
            Destroy();
    }

    ImGuiRenderer::ImGuiRenderer(RenderDevice& device, const ImGuiDesc& desc)
        : m_pDevice(&device)
    {
        CreateDescriptorPool(device, desc);
        InitializeImGui(device, desc);
    }

    void ImGuiRenderer::BeginFrame()
    {
        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
    }

    void ImGuiRenderer::EndFrame()
    {
        ImGui::EndFrame();

        // Handle Additional ImGui Windows
        if ((ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable) != 0)
        {
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
        }
    }

    void ImGuiRenderer::CreateRenderData()
    {
        ImGui::Render();
    }

    void ImGuiRenderer::RenderToSwapchain(CommandBuffer& commandBuffer, const RenderFrameContext&)
    {
        ImDrawData* pDrawData = ImGui::GetDrawData();
        ImGui_ImplVulkan_RenderDrawData(pDrawData, *commandBuffer.GetVkCommandBuffer());
    }

    void ImGuiRenderer::CreateDescriptorPool(RenderDevice& device, const ImGuiDesc& desc)
    {
        const uint32 dynamicPoolSize = desc.m_descriptorPoolSize > 0? desc.m_descriptorPoolSize : 128;

        const std::array<vk::DescriptorPoolSize, 1> poolSizes
        {
            vk::DescriptorPoolSize()
                .setType(vk::DescriptorType::eCombinedImageSampler)
                .setDescriptorCount(dynamicPoolSize),
        };
        
        const vk::DescriptorPoolCreateInfo poolInfo = vk::DescriptorPoolCreateInfo()
            .setFlags(vk::DescriptorPoolCreateFlagBits::eUpdateAfterBind | vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet)
            .setMaxSets(dynamicPoolSize)
            .setPoolSizes(poolSizes);

        m_descriptorPool = vk::raii::DescriptorPool(device, poolInfo);
    }

    void ImGuiRenderer::InitializeImGui(RenderDevice& device, const ImGuiDesc& desc)
    {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGui::StyleColorsDark();
        
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
        
        // Must be static for ImGui_ImplVulkan_InitInfo
        static VkFormat imageFormats = VK_FORMAT_B8G8R8A8_UNORM;
        
        if (desc.m_pWindow)
        {
            ImGui_ImplGlfw_InitForVulkan(checked_cast<GLFWwindow*>(desc.m_pWindow->GetNativeWindow().m_glfw), true);
            imageFormats = static_cast<VkFormat>(GetVkFormat(nes::Renderer::GetSwapchainFormat()));
        }
        
        ImGui_ImplVulkan_InitInfo initInfo = {};
        initInfo.ApiVersion = device.GetDesc().m_apiVersion;
        initInfo.Instance = *device.GetVkInstance();
        initInfo.PhysicalDevice = *device.GetVkPhysicalDevice();
        initInfo.Device = *device.GetVkDevice();
        initInfo.Allocator = device.GetVkAllocationCallbacks();
        initInfo.MinAllocationSize = 1024 * 1024;
        initInfo.QueueFamily = desc.m_pRenderQueue->GetFamilyIndex();
        initInfo.Queue = *desc.m_pRenderQueue->GetVkQueue();
        initInfo.UseDynamicRendering = true;
        initInfo.MinImageCount = nes::Renderer::GetMaxFramesInFlight();
        initInfo.ImageCount = nes::Renderer::GetMaxFramesInFlight();
        initInfo.PipelineInfoMain.PipelineRenderingCreateInfo =
        {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
            .colorAttachmentCount = 1,
            .pColorAttachmentFormats = &imageFormats
        };
        
        initInfo.DescriptorPool = *m_descriptorPool;
        ImGui_ImplVulkan_Init(&initInfo);

        // [TODO]: 
        // Fonts:
    }

    void ImGuiRenderer::Destroy()
    {
        // Shutdown ImGui:
        ImGui_ImplVulkan_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
        
        m_pDevice = nullptr;
        m_descriptorPool = nullptr;
    }
}

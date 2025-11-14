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
    ImGuiRenderer::~ImGuiRenderer()
    {
        if (m_pDevice)
            Shutdown();
    }

    void ImGuiRenderer::Init(RenderDevice& device, const ImGuiDesc& desc)
    {
        m_pDevice = &device;
        CreateDescriptorPool(device, desc);
        InitializeImGui(device, desc);
    }

    void ImGuiRenderer::Shutdown()
    {
        if (m_pDevice != nullptr)
        {
            // Shutdown ImGui:
            ImGui_ImplVulkan_Shutdown();
            ImGui_ImplGlfw_Shutdown();
            ImGui::DestroyContext();
        
            m_pDevice = nullptr;
            m_descriptorPool = nullptr;
        }
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
        //ImGuiPlatformIO platformIO = ImGui::GetPlatformIO();
        
        ImGuiIO& io = ImGui::GetIO();
        (void)io;
        
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
        io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;       // Enable Multi-Viewport / Platform Windows
        
        // When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
        ImGuiStyle& style = ImGui::GetStyle();
        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            style.WindowRounding = 0.0f;
            style.Colors[ImGuiCol_WindowBg].w = 1.0f;
            //style.Colors[ImGuiCol_WindowBg] = ImVec4(0.06f, 0.06f, 0.06f, 1.f);
            //style.Colors[ImGuiCol_ChildBg] = ImVec4(0.03f, 0.03f, 0.03f, 1.f);
            //style.Colors[ImGuiCol_Border] = ImVec4(0.02f, 0.02f, 0.02f, 0.50f);
            //style.Colors[ImGuiCol_Separator] = style.Colors[ImGuiCol_Border];
            //style.Colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 1.f);
        }

        // Must be static for ImGui_ImplVulkan_InitInfo
        static VkFormat imguiImageFormats = VK_FORMAT_B8G8R8A8_SRGB;
        
        if (desc.m_pWindow)
        {
            ImGui_ImplGlfw_InitForVulkan(checked_cast<GLFWwindow*>(desc.m_pWindow->GetNativeWindow().m_glfw), true);
            ImGui_ImplGlfw_SetCallbacksChainForAllWindows(true);
            
            imguiImageFormats = static_cast<VkFormat>(GetVkFormat(desc.m_swapchainFormat));
        }
        
        static ImGui_ImplVulkan_InitInfo initInfo = {};
        initInfo.ApiVersion = device.GetDesc().m_apiVersion;
        initInfo.Instance = *device.GetVkInstance();
        initInfo.PhysicalDevice = *device.GetVkPhysicalDevice();
        initInfo.Device = *device.GetVkDevice();
        initInfo.Allocator = device.GetVkAllocationCallbacks();
        initInfo.MinAllocationSize = 1024ull * 1024ull;
        initInfo.QueueFamily = desc.m_pRenderQueue->GetFamilyIndex();
        initInfo.Queue = *desc.m_pRenderQueue->GetVkQueue();
        initInfo.UseDynamicRendering = true;
        initInfo.MinImageCount = desc.m_framesInFlight;
        initInfo.ImageCount = desc.m_framesInFlight;
        initInfo.PipelineInfoMain.PipelineRenderingCreateInfo =
        {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
            .colorAttachmentCount = 1,
            .pColorAttachmentFormats = &imguiImageFormats
        };
        initInfo.PipelineInfoForViewports.PipelineRenderingCreateInfo =
        {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
            .colorAttachmentCount = 1,
            .pColorAttachmentFormats = &imguiImageFormats
        };
        
        initInfo.DescriptorPool = *m_descriptorPool;
        ImGui_ImplVulkan_Init(&initInfo);

        // [TODO]: 
        // Fonts:
    }
}

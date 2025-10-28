// ImGuiInterface.h
#pragma once
#include "Nessie/Asset/AssetBase.h"
#include "Nessie/Graphics/Descriptor.h"
#include "Nessie/Graphics/DescriptorPool.h"
#include "Nessie/Graphics/DeviceBuffer.h"
#include "Nessie/Graphics/DeviceImage.h"
#include "Nessie/Graphics/Pipeline.h"
#include "Nessie/Graphics/PipelineLayout.h"

struct ImDrawData;

namespace nes
{
    class ApplicationWindow;
    class RenderFrameContext;
    
    struct ImGuiDesc
    {
        ApplicationWindow*  m_pWindow = nullptr;
        DeviceQueue*        m_pRenderQueue = nullptr;
        uint32              m_framesInFlight = 2;
        uint32              m_descriptorPoolSize = 0;
        EFormat             m_swapchainFormat = EFormat::BGRA8_SRGB;
        std::filesystem::path m_iniSettingsPath = "imgui.ini";
    };
    
    //----------------------------------------------------------------------------------------------------
    /// @brief : Initializes the ImGui Context on creation, destroys it on destruction or assigning to nullptr.
    /// Use to submit ImGui draw data to the command buffer.
    //----------------------------------------------------------------------------------------------------
    class ImGuiRenderer
    {
    public:
        ImGuiRenderer(std::nullptr_t) {}
        ImGuiRenderer(const ImGuiRenderer&) = delete;
        ImGuiRenderer(ImGuiRenderer&& other) noexcept;
        ImGuiRenderer& operator=(std::nullptr_t);
        ImGuiRenderer& operator=(const ImGuiRenderer&) = delete;
        ImGuiRenderer& operator=(ImGuiRenderer&& other) noexcept;
        ~ImGuiRenderer();

        //----------------------------------------------------------------------------------------------------
        /// @brief : Creates and initializes ImGui. This will also load the shaders and create graphics resources.
        //----------------------------------------------------------------------------------------------------
        ImGuiRenderer(RenderDevice& device, const ImGuiDesc& desc);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Begin a new ImGui frame. Must be called before any ImGui commands are recorded. 
        //----------------------------------------------------------------------------------------------------
        void            BeginFrame();

        //----------------------------------------------------------------------------------------------------
        /// @brief : End recording ImGui draw calls. This creates the data to draw the UI (not on the GPU yet).
        //----------------------------------------------------------------------------------------------------
        void            CreateRenderData();

        //----------------------------------------------------------------------------------------------------
        /// @brief : End the ImGui frame. Must be called after the graphics frame has ended.
        //----------------------------------------------------------------------------------------------------
        void            EndFrame();

        //----------------------------------------------------------------------------------------------------
        /// @brief : Ends the ImGui frame, and submits the draw data to the GPU.
        //----------------------------------------------------------------------------------------------------
        void            RenderToSwapchain(nes::CommandBuffer& commandBuffer, const nes::RenderFrameContext& renderFrameContext);

    private:
        //----------------------------------------------------------------------------------------------------
        /// @brief : Create the Descriptor Pool that ImGui can use for textures. 
        //----------------------------------------------------------------------------------------------------
        void            CreateDescriptorPool(RenderDevice& device, const ImGuiDesc& desc);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Initialize the ImGui context.
        //----------------------------------------------------------------------------------------------------
        void            InitializeImGui(RenderDevice& device, const ImGuiDesc& desc);
        
        //----------------------------------------------------------------------------------------------------
        /// @brief : Destroys the ImGui context and all graphics resources.
        //----------------------------------------------------------------------------------------------------
        void            Destroy();
        
    private:
        RenderDevice*   m_pDevice = nullptr;
        vk::raii::DescriptorPool  m_descriptorPool = nullptr;
        std::string     m_iniSettingsPath = "imgui.ini";
    };
}
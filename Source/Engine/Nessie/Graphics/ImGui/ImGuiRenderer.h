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
        ImGuiRenderer(ImGuiRenderer&& other) noexcept = delete;
        ImGuiRenderer& operator=(const ImGuiRenderer&) = delete;
        ImGuiRenderer& operator=(ImGuiRenderer&& other) noexcept = delete;
        ~ImGuiRenderer();

        //----------------------------------------------------------------------------------------------------
        /// @brief : Creates and initializes the ImGui context. 
        //----------------------------------------------------------------------------------------------------
        void            Init(RenderDevice& device, const ImGuiDesc& desc);
        
        //----------------------------------------------------------------------------------------------------
        /// @brief : Shutdowns and destroys the ImGui context. No ImGui calls can be made past this point!  
        //----------------------------------------------------------------------------------------------------
        void            Shutdown();

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

        //----------------------------------------------------------------------------------------------------
        /// @brief : Advanced use. Get the descriptor pool used for ImGui.
        //----------------------------------------------------------------------------------------------------
        vk::raii::DescriptorPool& GetDescriptorPool() { return m_descriptorPool; }

    private:
        //----------------------------------------------------------------------------------------------------
        /// @brief : Create the Descriptor Pool that ImGui can use for textures. 
        //----------------------------------------------------------------------------------------------------
        void            CreateDescriptorPool(RenderDevice& device, const ImGuiDesc& desc);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Initialize the ImGui context.
        //----------------------------------------------------------------------------------------------------
        void            InitializeImGui(RenderDevice& device, const ImGuiDesc& desc);
        
    private:
        RenderDevice*   m_pDevice = nullptr;
        vk::raii::DescriptorPool  m_descriptorPool = nullptr;
    };
}
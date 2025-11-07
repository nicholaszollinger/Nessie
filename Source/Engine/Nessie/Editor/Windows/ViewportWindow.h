// ViewportWindow.h
#pragma once
#include "Nessie/Core/Memory/StrongPtr.h"
#include "Nessie/Editor/EditorWindow.h"
#include "Nessie/World/WorldCamera.h"
#include "Nessie/Graphics/Descriptor.h"
#include "Nessie/Graphics/DeviceImage.h"

namespace nes
{
    class WorldBase;
    class WorldRenderer;
    class CommandBuffer;
    class RenderFrameContext;
    class RenderTarget;
    class ImGuiRenderer;
    
    //----------------------------------------------------------------------------------------------------
    /// @brief : The Viewport window renders the World from the perspective of an Editor Camera or an
    /// in-world Camera when running the program.
    //----------------------------------------------------------------------------------------------------
    class ViewportWindow : public EditorWindow
    {
        NES_DEFINE_TYPE_INFO(ViewportWindow)
    
    public:
        ViewportWindow();
        virtual ~ViewportWindow() override;
        
        void            Init(const StrongPtr<WorldBase>& pWorld, ImGuiRenderer& imGuiRenderer);
        void            Tick(const float deltaTime);
        virtual void    RenderImGui() override;
        void            RenderWorld(CommandBuffer& commandBuffer, const RenderFrameContext& context);
        virtual void    Deserialize(const YamlNode& in) override;
        virtual void    Serialize(YamlOutStream& out) const override;
        
        //----------------------------------------------------------------------------------------------------
        /// @brief : Returns true if the mouse cursor is over the window.
        //----------------------------------------------------------------------------------------------------
        bool            IsHovered() const                   { return m_isHovered; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Returns true if the window is focused for receiving input.
        //----------------------------------------------------------------------------------------------------
        bool            IsFocused() const                   { return m_isFocused; }
    
    private:
        void            FreeImGuiDescriptorSetAndView();
        void            OnResize(RenderTarget& renderTarget);
        void            FreeImGuiSampler();
        void            GetCameraHeadingAndPitch(float& outHeading, float& outPitch) const;
        void            RenderViewportControlsOverlay(const ImVec2& viewportPos, const ImVec2& viewportSize);

    private:
        ImGuiRenderer* m_pImGui = nullptr;
        StrongPtr<WorldBase> m_pWorld = nullptr;
        StrongPtr<WorldRenderer> m_pRenderer = nullptr;
        
        Descriptor      m_imGuiSampler = nullptr;
        vk::raii::ImageView m_imGuiImageView = nullptr;
        ImTextureID     m_imGuiTexture = 0; // Descriptor Set pointer.
        UVec2           m_viewportSize = UVec2::Zero();
        bool            m_preserveAspectRatio = false;
        bool            m_isHovered = false;
        bool            m_isFocused = false;

        // Editor Camera
        WorldCamera     m_editorCamera{};
        bool            m_rotationEnabled = false;
        float           m_freeCamMoveSpeed = 50.f;
        float           m_freeCamSensitivity = 0.75f;
    };
}

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

    struct AspectRatioPreset
    {
        const char* m_name = nullptr;
        float       m_aspectRatio = 0.f;
    };
    
    //----------------------------------------------------------------------------------------------------
    /// @brief : The Viewport window renders the World from the perspective of an Editor Camera or an
    /// in-world Camera when running the program.
    //----------------------------------------------------------------------------------------------------
    class ViewportWindow : public EditorWindow
    {
        NES_DEFINE_TYPE_INFO(ViewportWindow)

        static constexpr AspectRatioPreset kFillPreset = { "Fill", 0.f };
        static constexpr std::array<AspectRatioPreset, 6> kDefaultAspectRatioPresets =
        {
            kFillPreset,
            { "16:9", 16.f / 9.f },
            { "16:10", 16.f / 10.f },
            { "4:3", 4.f / 3.f },
            { "21:9", 21.f / 9.f },
            { "1:1", 1.f },
        };
    
    public:
        ViewportWindow();
        virtual ~ViewportWindow() override;
        
        void                        Tick(const float deltaTime);
        virtual void                RenderImGui() override;
        void                        RenderWorld(CommandBuffer& commandBuffer, const RenderFrameContext& context);
        virtual void                Deserialize(const YamlNode& in) override;
        virtual void                Serialize(YamlOutStream& out) const override;
        
        //----------------------------------------------------------------------------------------------------
        /// @brief : Returns true if the mouse cursor is over the window.
        //----------------------------------------------------------------------------------------------------
        bool                        IsHovered() const                   { return m_isHovered; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Returns true if the window is focused for receiving input.
        //----------------------------------------------------------------------------------------------------
        bool                        IsFocused() const                   { return m_isFocused; }
    
    private:
        virtual void                OnWorldSet() override;
        void                        FreeImGuiDescriptorSetAndView(const bool forceImGuiDestroy = false);
        void                        OnResize(const UVec2 renderDimensions);
        void                        CreateImGuiSampler();
        void                        FreeImGuiSampler();
        void                        GetCameraHeadingAndPitch(float& outHeading, float& outPitch) const;
        void                        RenderViewportControlsOverlay(const ImVec2& viewportPos, const ImVec2& viewportSize);
        UVec2                       GetRenderDimensions();

    private:
        StrongPtr<WorldRenderer>    m_pRenderer = nullptr;
        Descriptor                  m_imGuiSampler = nullptr;
        vk::raii::ImageView         m_imGuiImageView = nullptr;
        ImTextureID                 m_imGuiTexture = 0; // Descriptor Set pointer.
        UVec2                       m_viewportSize = UVec2::Zero();
        uint32                      m_selectedAspectRatioIndex = 0;
        bool                        m_isHovered = false;
        bool                        m_isFocused = false;

        // Editor Camera
        WorldCamera                 m_editorCamera{};
        bool                        m_rotationEnabled = false;
        float                       m_freeCamMoveSpeed = 50.f;
        float                       m_freeCamSensitivity = 0.75f;
    };
}

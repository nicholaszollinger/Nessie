// ViewportWindow.cpp
#include "ViewportWindow.h"

#include "backends/imgui_impl_vulkan.h"
#include "Nessie/FileIO/YAML/Serializers/YamlMathSerializers.h"
#include "Nessie/FileIO/YAML/Serializers/YamlGraphicsSerializers.h"
#include "Nessie/Graphics/Renderer.h"
#include "Nessie/Graphics/ImGui/ImGuiRenderer.h"
#include "Nessie/Graphics/ImGui/ImGuiUtils.h"
#include "Nessie/Input/InputManager.h"
#include "Nessie/World/WorldBase.h"

namespace nes
{
    ViewportWindow::ViewportWindow()
    {
        m_desc.m_name = "Viewport";
        m_desc.m_flags = ImGuiWindowFlags_NoNav;
        CreateImGuiSampler();
    }

    ViewportWindow::~ViewportWindow()
    {
        FreeImGuiDescriptorSetAndView();
        FreeImGuiSampler();
    }
        
    void ViewportWindow::Tick(const float deltaTime)
    {
        if (!m_isFocused)
            return;
        
        const bool shift = InputManager::IsKeyDown(EKeyCode::LeftShift) || InputManager::IsKeyDown(EKeyCode::RightShift);
        const bool ctrl = InputManager::IsKeyDown(EKeyCode::LeftControl) || InputManager::IsKeyDown(EKeyCode::RightControl);
        
        // Speed:
        float speed = m_freeCamMoveSpeed * deltaTime;
        if (shift)
            speed *= 2.f;

        // Position:
        const Vec3 right = m_editorCamera.m_forward.Cross(m_editorCamera.m_up);
        if (InputManager::IsKeyDown(EKeyCode::A))
            m_editorCamera.m_position += speed * right;
        if (InputManager::IsKeyDown(EKeyCode::D))
            m_editorCamera.m_position -= speed * right;
        if (InputManager::IsKeyDown(EKeyCode::W))
            m_editorCamera.m_position += speed * m_editorCamera.m_forward;
        if (InputManager::IsKeyDown(EKeyCode::S))
            m_editorCamera.m_position -= speed * m_editorCamera.m_forward;
        if (InputManager::IsKeyDown(EKeyCode::Space))
            m_editorCamera.m_position.y += speed;
        if (ctrl)
            m_editorCamera.m_position.y -= speed;
        
        // Forward:
        if (m_rotationEnabled)
        {
            float heading, pitch;
            GetCameraHeadingAndPitch(heading, pitch);
            
            const Vec2 mouseDelta = InputManager::GetCursorDelta();
            heading += math::ToRadians(-mouseDelta.x * m_freeCamSensitivity);
            pitch = math::Clamp(pitch - math::ToRadians(mouseDelta.y  * m_freeCamSensitivity), -0.49f * math::Pi(), 0.49f * math::Pi());
            m_editorCamera.m_forward = Vec3(math::Cos(pitch) * math::Cos(heading), math::Sin(pitch), math::Cos(pitch) * math::Sin(heading));
        }
    }

    void ViewportWindow::RenderImGui()
    {
        NES_UI_SCOPED_STYLE(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
        NES_UI_SCOPED_STYLE(ImGuiStyleVar_WindowBorderSize, 0.f);
        
        ImVec2 viewportSize = ImVec2(0.f, 0.f);
        ImVec2 viewportPosition = ImVec2(0.f, 0.f);
        
        if (ImGui::Begin("Viewport", &m_desc.m_isOpen, ImGuiWindowFlags_NoNav))
        {
            if (!m_pRenderer)
            {
                // [TODO]: I'd like to render this at the center of the viewport. 
                ImGui::Text("No Renderer available");
                ImGui::End();
                return;
            }

            // Get the available viewport size and cache it.
            viewportPosition = ImGui::GetCursorScreenPos();
            viewportSize = ImGui::GetContentRegionAvail();
            m_viewportSize.x = static_cast<uint32>(viewportSize.x);
            m_viewportSize.y = static_cast<uint32>(viewportSize.y);
            const float viewportAspectRatio = viewportSize.x / viewportSize.y;

            // Get the current image size and aspect ratio:
            RenderTarget* colorTarget = m_pRenderer->GetFinalColorTarget();
            const UInt2 imageExtent = colorTarget->GetSize();
            ImVec2 imageSize = {static_cast<float>(imageExtent.x), static_cast<float>(imageExtent.y)};
            const float imageAspectRatio = imageSize.x / imageSize.y;
            
            if (m_preserveAspectRatio)
            {
                ImVec2 scaledSize;
                if (viewportAspectRatio > imageAspectRatio)
                {
                    scaledSize.y = viewportSize.y;
                    scaledSize.x = viewportSize.y * imageAspectRatio;
                }
                else
                {
                    scaledSize.x = viewportSize.x;
                    scaledSize.y = viewportSize.x / imageAspectRatio;
                }

                // Center the image in the viewport
                ImVec2 offset;
                offset.x = (viewportSize.x - scaledSize.x) * 0.5f;
                offset.y = (viewportSize.y - scaledSize.y) * 0.5f;

                // Add padding to center the image
                ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPosX() + offset.x, ImGui::GetCursorPosY() + offset.y));
                ImGui::Image(m_imGuiTexture, scaledSize);
            }

            else
            {
                ImVec2 uv0 = ImVec2(0, 0);
                ImVec2 uv1 = ImVec2(1, 1);

                if (viewportAspectRatio > imageAspectRatio)
                {
                    // Viewport is wider - crop the top/bottom of the image.
                    const float visibleHeight = imageSize.x / viewportAspectRatio;
                    const float crop = (imageSize.y - visibleHeight) / (2.f * imageSize.y);
                    uv0.y = crop;
                    uv1.y = 1.f - crop;
                }
                else
                {
                    // Viewport is taller - crop left/right of the image.
                    const float visibleWidth = imageSize.y * viewportAspectRatio;
                    const float crop = (imageSize.x - visibleWidth) / (2.f * imageSize.x);
                    uv0.x = crop;
                    uv1.x = 1.f - crop;
                }

                ImGui::Image(m_imGuiTexture, viewportSize, uv0, uv1);
            }
        }

        m_isHovered = ImGui::IsItemHovered();
        m_isFocused = ImGui::IsWindowFocused();

        // Handle right-clicking directly on the window, immediately focusing it.
        if (m_isHovered && !m_isFocused)
        {
            if (ImGui::IsMouseClicked(ImGuiMouseButton_Left)
                || ImGui::IsMouseClicked(ImGuiMouseButton_Right)
                || ImGui::IsMouseClicked(ImGuiMouseButton_Middle))
            {
                ImGui::SetWindowFocus();
                m_isFocused = true;

                if (ImGui::IsMouseClicked(ImGuiMouseButton_Right))
                {
                    m_rotationEnabled = true;
                    InputManager::SetCursorMode(nes::ECursorMode::Disabled);
                }
            }
        }
        
        if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Right))
        {
            m_rotationEnabled = true;
    
            // Let ImGui know you're capturing the mouse
            ImGui::SetMouseCursor(ImGuiMouseCursor_None);  // Hide cursor via ImGui
            InputManager::SetCursorMode(nes::ECursorMode::Disabled);
        }

        if (m_rotationEnabled)
        {
            // Keep cursor hidden
            ImGui::SetMouseCursor(ImGuiMouseCursor_None);
    
            if (ImGui::IsMouseReleased(ImGuiMouseButton_Right))
            {
                m_rotationEnabled = false;
                InputManager::SetCursorMode(nes::ECursorMode::Visible);
            }
        }

        // Render the controls overlaid on the Viewport window.
        if (viewportSize.x > 0.f && viewportSize.y > 0.f)
        {
            RenderViewportControlsOverlay(viewportPosition, viewportSize);
        }
        
        ImGui::End();
    }

    void ViewportWindow::RenderWorld(CommandBuffer& commandBuffer, const RenderFrameContext& context)
    {
        if (!m_pRenderer || m_pRenderer->GetFinalColorTarget() == nullptr || m_viewportSize.LengthSqr() <= 0.f)
            return;

        // Render the World using the editor Camera.
        m_pRenderer->RenderWorldWithCamera(m_editorCamera, commandBuffer, context);
        
        RenderTarget* colorTarget = m_pRenderer->GetFinalColorTarget();

        // Transition the color target for ImGui to sample:
        ImageBarrierDesc targetBarrier = nes::ImageBarrierDesc()
            .SetImage(&colorTarget->GetImage())
            .SetLayout(nes::EImageLayout::ColorAttachment, nes::EImageLayout::ShaderResource)
            .SetBarrierStage(EPipelineStageBits::ColorAttachment, nes::EPipelineStageBits::FragmentShader);

        BarrierGroupDesc barrierGroup = nes::BarrierGroupDesc()
            .SetImageBarriers({targetBarrier} );
        
        commandBuffer.SetBarriers(barrierGroup);
    }

    void ViewportWindow::Deserialize(const YamlNode& in)
    {
        EditorWindow::Deserialize(in);
        in["CameraPosition"].Read(m_editorCamera.m_position, nes::Vec3::Zero());
        in["CameraForward"].Read(m_editorCamera.m_forward, nes::Vec3::Forward());
        in["CameraUp"].Read(m_editorCamera.m_up, nes::Vec3::Up());
        in["CameraMovementSpeed"].Read(m_freeCamMoveSpeed, 50.f);
        in["CameraSensitivity"].Read(m_freeCamSensitivity, 0.75f);
        in["PreserveAspectRatio"].Read(m_preserveAspectRatio, false);

        auto camera = in["Camera"];
        CameraSerializer::Deserialize(camera, m_editorCamera.m_camera);
    }

    void ViewportWindow::Serialize(YamlOutStream& out) const
    {
        EditorWindow::Serialize(out);
        
        out.Write("CameraPosition", m_editorCamera.m_position);
        out.Write("CameraForward", m_editorCamera.m_forward);
        out.Write("CameraUp", m_editorCamera.m_up);
        out.Write("CameraMovementSpeed", m_freeCamMoveSpeed);
        out.Write("CameraSensitivity", m_freeCamSensitivity);
        out.Write("PreserveAspectRatio", m_preserveAspectRatio);
        CameraSerializer::Serialize(out, m_editorCamera.m_camera);
    }

    void ViewportWindow::OnWorldSet()
    {
        if (m_pWorld)
        {
            m_pRenderer = m_pWorld->GetRenderer();
            OnResize(*m_pRenderer->GetFinalColorTarget());
        }
        else
        {
            m_pRenderer = nullptr;
        }
    }

    void ViewportWindow::FreeImGuiDescriptorSetAndView()
    {
        nes::Renderer::SubmitResourceFree([view = std::move(m_imGuiImageView)]() mutable
        {
            view = nullptr;
        });
        
        // Free the ImGui Texture
        vk::DescriptorSet descriptorSet = reinterpret_cast<VkDescriptorSet>(m_imGuiTexture);
        if (descriptorSet != nullptr)
            ImGui_ImplVulkan_RemoveTexture(descriptorSet);

        m_imGuiTexture = 0;
    }

    void ViewportWindow::OnResize(RenderTarget& renderTarget)
    {
        FreeImGuiDescriptorSetAndView();
        
        // Create the Image View
        auto& device = Renderer::GetDevice();
        auto& image = renderTarget.GetImage();
        
        vk::ImageViewUsageCreateInfo usageInfo = vk::ImageViewUsageCreateInfo()
            .setUsage(vk::ImageUsageFlagBits::eSampled);
        
        const vk::ImageSubresourceRange subresourceRange = vk::ImageSubresourceRange()
            .setAspectMask(GetVkImageAspectFlags(image.GetDesc().m_format))
            .setBaseMipLevel(0)
            .setBaseArrayLayer(0)
            .setLevelCount(1)
            .setLayerCount(1);
        
        vk::ImageViewCreateInfo viewInfo = vk::ImageViewCreateInfo()
            .setPNext(&usageInfo)
            .setViewType(vk::ImageViewType::e2D)
            .setImage(image.GetVkImage())
            .setSubresourceRange(subresourceRange)
            .setFormat(GetVkFormat(image.GetDesc().m_format));
        
        m_imGuiImageView = vk::raii::ImageView(device, viewInfo, device.GetVkAllocationCallbacks());

        // Add a debug name:
        VkImageView imageView = *m_imGuiImageView;
        NativeVkObject nativeView;
        nativeView.m_pHandle = imageView;
        nativeView.m_type = vk::ObjectType::eImageView;
        device.SetDebugNameVkObject(nativeView, "ImGui Image View");
        
        VkDescriptorSet descriptorSet = ImGui_ImplVulkan_AddTexture(m_imGuiSampler.GetVkSampler(), *m_imGuiImageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
        m_imGuiTexture = reinterpret_cast<ImTextureID>(descriptorSet);
    }

    void ViewportWindow::CreateImGuiSampler()
    {
        auto& device = Renderer::GetDevice();
        
        // These are the same values from the ImGui Vulkan Backend.
        // - Annoyingly, there is no way to get the default sampler that they use.
        SamplerDesc desc{};
        desc.m_filters.m_mag = EFilterType::Linear;
        desc.m_filters.m_min = EFilterType::Linear;
        desc.m_mipMin = -1000.0f;
        desc.m_mipMax = 1000.0f;
        desc.m_anisotropy = static_cast<uint8>(1);
        desc.m_addressModes.u = EAddressMode::ClampToEdge;
        desc.m_addressModes.v = EAddressMode::ClampToEdge;
        desc.m_addressModes.w = EAddressMode::ClampToEdge;
        m_imGuiSampler = nes::Descriptor(device, desc);

    }

    void ViewportWindow::FreeImGuiSampler()
    {
        m_imGuiSampler = nullptr;
    }

    void ViewportWindow::GetCameraHeadingAndPitch(float& outHeading, float& outPitch) const
    {
        outHeading = math::ATan2(m_editorCamera.m_forward.z, m_editorCamera.m_forward.x);
        outPitch = math::ATan2(m_editorCamera.m_forward.y, Vec3(m_editorCamera.m_forward.x, 0.f, m_editorCamera.m_forward.z).Length());
    }

    void ViewportWindow::RenderViewportControlsOverlay(const ImVec2& viewportPos, const ImVec2& viewportSize)
    {
        // Now overlay controls using the window's draw list
        ImDrawList* drawList = ImGui::GetWindowDrawList();

        // Push a clip rect to keep buttons within viewport bounds
        drawList->PushClipRect(viewportPos, 
            ImVec2(viewportPos.x + viewportSize.x, viewportPos.y + viewportSize.y));
    
        static constexpr float kPadding = 10.0f;
        ImVec2 buttonPos = ImVec2
        (
            viewportPos.x + kPadding,
            viewportPos.y + kPadding
        );

        // Set cursor for overlay widgets
        ImGui::SetCursorScreenPos(buttonPos);

        static constexpr float kSliderWidth = 120.f;
        ImGui::BeginGroup();

        // Camera Move Speed
        ImGui::Text("Speed:");
        ImGui::SameLine();
        ImGui::SetNextItemWidth(kSliderWidth);
        ImGui::SliderFloat("##MoveSpeed", &m_freeCamMoveSpeed, 1.f, 100.f, "%.f");
        ImGui::SameLine();

        // Sensitivity
        ImGui::Text("Sensitivity:");
        ImGui::SameLine();
        ImGui::SetNextItemWidth(kSliderWidth);
        ImGui::SliderFloat("##Sensitivity", &m_freeCamSensitivity, 0.001f, 1.f, "%.3f");
        ImGui::SameLine();
        
        // Camera Mode Toggle
        const bool isPerspective = m_editorCamera.m_camera.m_projectionType == Camera::EProjectionType::Perspective;
        if (ImGui::Button(isPerspective ? "Perspective" : "Orthographic"))
        {
            if (isPerspective)
                m_editorCamera.m_camera.m_projectionType = Camera::EProjectionType::Orthographic;
            else
                m_editorCamera.m_camera.m_projectionType = Camera::EProjectionType::Perspective;                
        }

        // FOV slider
        if (isPerspective)
        {
            ImGui::SameLine();
            ImGui::Text("FOV:");
            ImGui::SameLine();
            ImGui::SetNextItemWidth(kSliderWidth);
            ImGui::SliderFloat("##FOV", &m_editorCamera.m_camera.m_perspectiveFOV, 30.0f, 120.0f, "%.0f°");
        }
        // Orthographic Size slider:
        else
        {
            ImGui::SameLine();
            ImGui::Text("OrthoSize:");
            ImGui::SameLine();
            ImGui::SetNextItemWidth(kSliderWidth);
            ImGui::SliderFloat("##OrthoSize", &m_editorCamera.m_camera.m_orthographicSize, 0.1f, 100.f, "%.1f");
        }

        // Preserve Aspect Ratio
        ImGui::SameLine();
        if (ImGui::Button(m_preserveAspectRatio? "Aspect Locked" : "Fill Viewport"))
        {
            m_preserveAspectRatio = !m_preserveAspectRatio;
        }
        
        ImGui::EndGroup();
        drawList->PopClipRect();
    }
}

// ViewportWindow.cpp
#include "ViewportWindow.h"

#include "backends/imgui_impl_vulkan.h"
#include "Nessie/FileIO/YAML/Serializers/YamlMathSerializers.h"
#include "Nessie/FileIO/YAML/Serializers/YamlGraphicsSerializers.h"
#include "Nessie/Graphics/Renderer.h"
#include "Nessie/Graphics/ImGui/ImGuiRenderer.h"
#include "Nessie/Graphics/ImGui/ImGuiUtils.h"
#include "Nessie/Input/InputManager.h"
#include "Nessie/Editor/EditorWorld.h"

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
        FreeImGuiDescriptorSetAndView(true);
        FreeImGuiSampler();
    }

    void ViewportWindow::Tick(const float deltaTime)
    {
        // [TODO]: If the user wants to 'eject' from the in game camera and use the
        // editor camera, use that.
        if (!m_isFocused || m_pWorld->IsSimulating())
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
        const bool isSimulating = m_pRenderer && m_pWorld->IsSimulating();
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
            
            // Get the available space and the current Screen space position.
            viewportPosition = ImGui::GetWindowContentRegionMin();
            viewportSize = ImGui::GetContentRegionAvail();
            
            m_viewportSize.x = static_cast<uint32>(viewportSize.x);
            m_viewportSize.y = static_cast<uint32>(viewportSize.y);

            const auto renderDimensions = GetRenderDimensions();
            ImVec2 imageSize = {static_cast<float>(renderDimensions.x), static_cast<float>(renderDimensions.y)};
            ImVec2 imageDrawPosition = viewportPosition;
            
            if (renderDimensions != m_viewportSize)
            {
                // Center it
                const float offsetX = (viewportSize.x - imageSize.x) * 0.5f;
                const float offsetY = (viewportSize.y - imageSize.y) * 0.5f;
                imageDrawPosition.x += offsetX;
                imageDrawPosition.y += offsetY;
            }
            
            ImGui::SetCursorPos(imageDrawPosition);
            ImGui::Image(m_imGuiTexture, imageSize);
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

                if (ImGui::IsMouseClicked(ImGuiMouseButton_Right) && !isSimulating)
                {
                    m_rotationEnabled = true;
                    InputManager::SetCursorMode(nes::ECursorMode::Disabled);
                }
            }
        }
        
        if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Right) && !isSimulating)
        {
            m_rotationEnabled = true;

            // Tell ImGui you want mouse input
            ImGui::SetWindowFocus();
            ImGuiIO& io = ImGui::GetIO();
            io.WantCaptureMouse = false;  // Release mouse to your application
            
            InputManager::SetCursorMode(nes::ECursorMode::Disabled);
        }

        if (m_rotationEnabled)
        {
            // Block ImGui from seeing mouse movement
            ImGuiIO& io = ImGui::GetIO();
            io.AddMousePosEvent(-FLT_MAX, -FLT_MAX);
            
            // Clear the rotation enabled state if we are now simulating the world.
            if (isSimulating)
            {
                InputManager::SetCursorMode(nes::ECursorMode::Visible);
                m_rotationEnabled = false;
            }

            // Handle releasing right click.
            else
            {
                // Keep cursor hidden
                ImGui::SetMouseCursor(ImGuiMouseCursor_None);
    
                if (ImGui::IsMouseReleased(ImGuiMouseButton_Right))
                {
                    m_rotationEnabled = false;
                    InputManager::SetCursorMode(nes::ECursorMode::Visible);
                }
            }
        }

        // Render the controls overlaid on the Viewport window.
        if (!m_pWorld->IsSimulating() && viewportSize.x > 0.f && viewportSize.y > 0.f)
        {
            RenderViewportControlsOverlay(viewportPosition, viewportSize);
        }
        
        ImGui::End();
    }

    void ViewportWindow::RenderWorld(CommandBuffer& commandBuffer, const RenderFrameContext& context)
    {
        if (!m_pRenderer || m_pRenderer->GetFinalColorTarget() == nullptr || m_viewportSize.LengthSqr() <= 0.f)
            return;
        
        // Check for resize
        static constexpr uint32 kResizeThreshold = 2;
        const auto currentTargetSize = m_pRenderer->GetFinalColorTarget()->GetSize();
        const auto renderDimensions = GetRenderDimensions();

        const bool sizeChanged = math::Abs(static_cast<int>(renderDimensions.x) - static_cast<int>(currentTargetSize.x)) > kResizeThreshold
            || math::Abs(static_cast<int>(renderDimensions.y) - static_cast<int>(currentTargetSize.y)) > kResizeThreshold;
        
        // If the viewport size has significantly changed:
        if (sizeChanged)
        {
            OnResize(renderDimensions);
        }

        if (m_pWorld->IsSimulating())
        {
            // Render the world using an in-game camera.
            m_pRenderer->RenderWorld(commandBuffer, context);
        }
        else
        {
            // Render the World using the editor Camera.
            m_pRenderer->RenderWorldWithCamera(m_editorCamera, commandBuffer, context);    
        }
        
        // Transition the color target for ImGui to sample:
        RenderTarget* colorTarget = m_pRenderer->GetFinalColorTarget();
        NES_ASSERT(colorTarget->GetSampleCount() == 1, "The Final Color Target must not be multisampled! You should have a separate render target for multisampling that is resolved into the final render target.");
        
        ImageBarrierDesc targetBarrier = nes::ImageBarrierDesc()
            .SetImage(&colorTarget->GetImage())
            .SetLayout(nes::EImageLayout::ColorAttachment, nes::EImageLayout::ShaderResource);
        
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
        in["SelectedAspectRatioIndex"].Read(m_selectedAspectRatioIndex, 0u);
        
        CameraSerializer::Deserialize(in, m_editorCamera.m_camera);
    }

    void ViewportWindow::Serialize(YamlOutStream& out) const
    {
        EditorWindow::Serialize(out);
        
        out.Write("CameraPosition", m_editorCamera.m_position);
        out.Write("CameraForward", m_editorCamera.m_forward);
        out.Write("CameraUp", m_editorCamera.m_up);
        out.Write("CameraMovementSpeed", m_freeCamMoveSpeed);
        out.Write("CameraSensitivity", m_freeCamSensitivity);
        out.Write("SelectedAspectRatioIndex", m_selectedAspectRatioIndex);
        CameraSerializer::Serialize(out, m_editorCamera.m_camera);
    }

    void ViewportWindow::OnWorldSet()
    {
        if (m_pWorld)
            m_pRenderer = m_pWorld->GetRenderer();
        else
            m_pRenderer = nullptr;
    }

    void ViewportWindow::FreeImGuiDescriptorSetAndView(const bool forceImGuiDestroy)
    {
        if (forceImGuiDestroy)
        {
            nes::Renderer::SubmitResourceFree([view = std::move(m_imGuiImageView)]() mutable
            {
                // Destroy the View:
                view = nullptr;
            });
        
            // Free the ImGui Descriptor Set:
            VkDescriptorSet descriptorSet = reinterpret_cast<VkDescriptorSet>(m_imGuiTexture);
            if (descriptorSet != nullptr)
                ImGui_ImplVulkan_RemoveTexture(descriptorSet);
        }

        else
        {
            ImTextureID imDescriptorSetID = m_imGuiTexture;
            nes::Renderer::SubmitResourceFree([view = std::move(m_imGuiImageView), imDescriptorSetID]() mutable
            {
                // Destroy the View:
                view = nullptr;
            
                // Free the ImGui Descriptor Set:
                VkDescriptorSet descriptorSet = reinterpret_cast<VkDescriptorSet>(imDescriptorSetID);
                if (descriptorSet != nullptr)
                    ImGui_ImplVulkan_RemoveTexture(descriptorSet);
            });
        }

        m_imGuiTexture = 0;
    }

    void ViewportWindow::OnResize(const UVec2 renderDimensions)
    {
        NES_ASSERT(m_pRenderer != nullptr);
        FreeImGuiDescriptorSetAndView();

        // Resize the Renderer's targets.
        m_pRenderer->OnViewportResize(renderDimensions.x, renderDimensions.y);
        auto& renderTarget = *m_pRenderer->GetFinalColorTarget();
        
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

    void ViewportWindow::RenderViewportControlsOverlay(const ImVec2& viewportPos, const ImVec2& /*viewportSize*/)
    {
        static constexpr float kPadding = 10.0f;
    
        // Position overlay controls at the top-left of the content area (relative to window)
        ImVec2 overlayPos = ImVec2
        (
            viewportPos.x + kPadding,
            viewportPos.y + kPadding
        );
        
        // Set cursor position (relative to window)
        ImGui::SetCursorPos(overlayPos);

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
        ImGui::SetNextItemWidth(100.f);
        
        if (ImGui::BeginCombo("##AspectRatio", kDefaultAspectRatioPresets[m_selectedAspectRatioIndex].m_name))
        {
            for (size_t i = 0; i < kDefaultAspectRatioPresets.size(); ++i)
            {
                const bool isSelected = (m_selectedAspectRatioIndex == i);
                if (ImGui::Selectable(kDefaultAspectRatioPresets[i].m_name, isSelected))
                {
                    m_selectedAspectRatioIndex = static_cast<uint32>(i);
                }

                if (isSelected)
                    ImGui::SetItemDefaultFocus();
            }

            ImGui::EndCombo();
        }
        
        ImGui::EndGroup();
    }

    UVec2 ViewportWindow::GetRenderDimensions()
    {
        if (m_selectedAspectRatioIndex >= kDefaultAspectRatioPresets.size() || m_selectedAspectRatioIndex == 0)
        {
            m_selectedAspectRatioIndex = 0;
            return m_viewportSize;
        }

        const float targetAspect = kDefaultAspectRatioPresets[m_selectedAspectRatioIndex].m_aspectRatio;
        if (targetAspect <= 0.f)
            return m_viewportSize;

        // Calculate dimensions that fit within viewport while maintaining aspect ratio
        const float viewportAspect = static_cast<float>(m_viewportSize.x) / static_cast<float>(m_viewportSize.y);

        UVec2 result;

        if (viewportAspect > targetAspect)
        {
            // Viewport is wider than the target - constrain by height.
            result.y = m_viewportSize.y;
            result.x = static_cast<uint32>(static_cast<float>(result.y) * targetAspect);
        }
        else
        {
            // Viewport is taller than the target - constrain by width.
            result.x = m_viewportSize.x;
            result.y = static_cast<uint32>(static_cast<float>(result.x) / targetAspect);
        }
        
        return result;
    }
}

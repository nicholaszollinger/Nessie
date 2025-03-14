// Camera.cpp
#include "CameraComponent.h"
#include "Application/Application.h"
#include "Math/VectorConversions.h"
#include "Scene/Scene.h"
#include "World/Entity3D.h"

namespace nes
{
    bool CameraComponent::Init()
    {
        const auto windowExtent = Application::Get().GetWindow().GetExtent();
        m_camera.UpdateViewport(windowExtent.m_width, windowExtent.m_height);
        // [TODO]: Subscribe to WindowResize events...
        return true;
    }

    void CameraComponent::SetAsActiveCamera() const
    {
        Scene* pScene = GetOwner()->GetScene();
        NES_ASSERT(pScene);
        pScene->SetActiveCamera(&m_camera);
    }

    void CameraComponent::SetActiveOnEnabled(const bool setActiveOnEnable)
    {
        m_setActiveOnEnable = setActiveOnEnable;
    }

    Camera& CameraComponent::GetCamera()
    {
        return m_camera;
    }

    const Camera& CameraComponent::GetCamera() const
    {
        return m_camera;
    }
    
    bool CameraComponent::IsActiveCamera() const
    {
        Scene* pScene = GetOwner()->GetScene();
        NES_ASSERT(pScene);
        return pScene->GetActiveCamera() == &m_camera;
    }

    void CameraComponent::UpdateCameraViewBasedOnActorTransform()
    {
        // Set the ViewMatrix of the Camera to look toward the position in front of the camera.
        const auto& worldTransformMatrix = GetOwner()->GetWorldTransformMatrix();
        const Mat4 orientationMatrix = math::ExtractMatrixOrientation4x4(worldTransformMatrix);
        
        const auto cameraForward = math::XYZ(orientationMatrix * Vector4(0.f, 0.f, 1.f, 0.f));
        const auto cameraUp = math::XYZ(orientationMatrix * Vector4(0.f, 1.f, 0.f, 0.f));
        const Vector3 cameraPosition = math::XYZ(worldTransformMatrix.GetColumn(3)); 
        m_camera.LookAt(cameraPosition, cameraPosition + cameraForward, cameraUp);
    }

    void CameraComponent::OnEnabled()
    {
        if (m_setActiveOnEnable)
            SetAsActiveCamera();

        // Perform initial view update.
        UpdateCameraViewBasedOnActorTransform();

        // Listen for Actor Transform updates:
        GetOwner()->OnWorldTransformUpdated().AddListener(this, [this]()
        {
            UpdateCameraViewBasedOnActorTransform();
        });
    }

    void CameraComponent::OnDisabled()
    {
        // [Consider] Should this just be an error, and/or assert?
        // If this is currently the Active Camera, we need to disable it.
        if (IsActiveCamera())
        {
            NES_WARN("Disabling active Camera in World!!!");
            Scene* pScene = GetOwner()->GetScene();
            NES_ASSERT(pScene);
            pScene->SetActiveCamera(nullptr);
        }

        // Stop listening to Actor transform updates.
        GetOwner()->OnWorldTransformUpdated().RemoveListener(this);
    }
}

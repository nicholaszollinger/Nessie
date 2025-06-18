// Camera.cpp
#include "CameraComponent.h"
#include "Application/Application.h"
#include "Scene/Scene.h"
#include "World/Entity3D.h"

namespace nes
{
    bool CameraComponent::Init()
    {
        if (!Entity3DComponent::Init())
        {
            return false;
        }

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
        const Mat44 orientationMatrix = worldTransformMatrix.GetRotation();
        
        const auto cameraForward = orientationMatrix.TransformVector(Vec3::Forward());
        const auto cameraUp = orientationMatrix.TransformVector(Vec3::Up());
        const Vec3 cameraPosition = worldTransformMatrix.GetTranslation(); 
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
            if (!GetOwner()->GetLayer()->IsBeingDestroyed())
            {
                NES_WARN("Disabling active Camera in World!!!");
            }

            Scene* pScene = GetOwner()->GetScene();
            NES_ASSERT(pScene);
            pScene->SetActiveCamera(nullptr);
        }

        // Stop listening to Actor transform updates.
        GetOwner()->OnWorldTransformUpdated().RemoveListener(this);
    }
}

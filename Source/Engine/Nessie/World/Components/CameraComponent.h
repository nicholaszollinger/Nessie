// CameraComponent.h
#pragma once
#include "Entity3DComponent.h"
#include "Graphics/Camera.h"

namespace nes
{
    //----------------------------------------------------------------------------------------------------
    ///		@brief : A Camera that exists in 3D space. 
    //----------------------------------------------------------------------------------------------------
    class CameraComponent final : public Entity3DComponent
    {
        NES_DEFINE_COMPONENT_TYPE(CameraComponent)
        
        Camera m_camera;
        bool m_setActiveOnEnable = true;
    
    public:
        virtual bool Init() override;
        void SetAsActiveCamera() const;
        void SetActiveOnEnabled(const bool setActiveOnEnable);
        [[nodiscard]] Camera& GetCamera();
        [[nodiscard]] const Camera& GetCamera() const;
        [[nodiscard]] bool CameraIsSetActiveOnEnable() const { return m_setActiveOnEnable; }
        [[nodiscard]] bool IsActiveCamera() const;
        
    protected:
        void UpdateCameraViewBasedOnActorTransform();
        virtual void OnEnabled() override;
        virtual void OnDisabled() override;
    };
}

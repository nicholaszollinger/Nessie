// CameraComponent.h
#pragma once
#include "WorldComponent.h"
#include "Graphics/Camera.h"

namespace nes
{
    //----------------------------------------------------------------------------------------------------
    ///		@brief : A Camera that exists in 3D space. 
    //----------------------------------------------------------------------------------------------------
    class CameraComponent final : public WorldComponent
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
        virtual void OnWorldTransformUpdated() override;
        virtual void OnEnabled() override;
        virtual void OnDisabled() override;
    };
}

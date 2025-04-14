// ShapeComponent.h
#pragma once
#include "World/Components/Entity3DComponent.h"
#include "Math/AABox.h"

namespace nes
{
    enum class ShapeType : uint8_t
    {
        Empty,
        Sphere,
        Box,
        Plane,
        // ..
    };
    
    //----------------------------------------------------------------------------------------------------
    //		NOTES:
    //		
    ///		@brief : Base class for all Collision based geometry. The Base class contains 
    //----------------------------------------------------------------------------------------------------
    class ShapeComponent : public Entity3DComponent
    {   
        NES_DEFINE_COMPONENT_TYPE(ShapeComponent)

    protected:
        // Local Transform of the Shape.
        Vector3     m_localPosition{};
        Rotation    m_localRotation{};
        Vector3     m_localScale{};
        
        ShapeType m_shapeType = ShapeType::Empty;
        
    public:
        ShapeComponent() = default;
        ShapeComponent(const ShapeComponent&) = delete;
        ShapeComponent& operator=(const ShapeComponent&) = delete;
        ShapeComponent(ShapeComponent&&) noexcept = default;
        ShapeComponent& operator=(ShapeComponent&&) noexcept = default;
        
        virtual Vector3 GetCenterOfMass() const { return Vector3::Zero(); }
        virtual AABox     GetLocalBounds() const = 0;
        virtual AABox     GetWorldBounds() const = 0;
        ShapeType GetShapeType() const { return m_shapeType; }

        // [TODO]: Debug Rendering API

    protected:
        virtual bool Init() override;
        virtual void OnEnabled() override;
        virtual void OnDisabled() override;
    };
}
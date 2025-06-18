// ShapeComponent.h
#pragma once
#include "World/Components/Entity3DComponent.h"
#include "Geometry/AABox.h"
#include "Physics/Collision/Shapes/Shape.h"

namespace nes
{
    //----------------------------------------------------------------------------------------------------
    /// @brief : // [TODO]: Base class for all Collision-based geometry.  
    //----------------------------------------------------------------------------------------------------
    class ShapeComponent : public Entity3DComponent
    {   
        NES_DEFINE_COMPONENT_TYPE(ShapeComponent)
        
    public:
        ShapeComponent() = default;
        ShapeComponent(const ShapeComponent&) = delete;
        ShapeComponent& operator=(const ShapeComponent&) = delete;
        ShapeComponent(ShapeComponent&&) noexcept = default;
        ShapeComponent& operator=(ShapeComponent&&) noexcept = default;
        
        virtual Vec3    GetCenterOfMass() const { return Vec3::Zero(); }
        virtual AABox   GetLocalBounds() const = 0;
        virtual AABox   GetWorldBounds() const = 0;
        EShapeType      GetShapeType() const { return m_shapeType; }

        // [TODO]: Debug Rendering API

    protected:
        virtual bool    Init() override;
        virtual void    OnEnabled() override;
        virtual void    OnDisabled() override;

    protected:
        // Local Transform of the Shape.
        Vec3            m_localPosition{};
        Rotation        m_localRotation{};
        Vec3            m_localScale{};
            
        EShapeType      m_shapeType = EShapeType::Empty;
    };
}
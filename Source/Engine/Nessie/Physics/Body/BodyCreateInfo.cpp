// BodyCreateInfo.cpp
#include "BodyCreateInfo.h"

namespace nes
{
    BodyCreateInfo::BodyCreateInfo(const ShapeSettings* pSettings, const Vector3& position, const Quat& rotation, const EBodyMotionType motionType, const CollisionLayer layer)
        : m_position(position)
        , m_rotation(rotation)
        , m_collisionLayer(layer)
        , m_motionType(motionType)
        , m_shapeSettings(pSettings)
    {
        //
    }

    BodyCreateInfo::BodyCreateInfo(const Shape* pShape, const Vector3& position, const Quat& rotation, const EBodyMotionType motionType, const CollisionLayer layer)
        : m_position(position)
        , m_rotation(rotation)
        , m_collisionLayer(layer)
        , m_motionType(motionType)
        , m_shape(pShape)
    {
        //
    }

    void BodyCreateInfo::SetShapeSettings(const StrongPtr<ShapeSettings>& pSettings)
    {
        m_shapeSettings = pSettings;
        m_shape = nullptr;
    }

    const Shape* BodyCreateInfo::GetShape() const
    {
        // If we already have a shape, return it.
        if (m_shape != nullptr)
            return m_shape;

        // Check if we have shape settings
        if (m_shapeSettings == nullptr)
            return nullptr;

        // Create the shape from current settings:
        Shape::ShapeResult result = m_shapeSettings->Create();
        if (result.IsValid())
            return result.Get();

        NES_ASSERT(false, "Error occurred during Shape Creation! Error: ", result.GetError().c_str());
        return nullptr;
    }

    void BodyCreateInfo::SetShape(const Shape* pShape)
    {
        m_shape = pShape;
        m_shapeSettings = nullptr;
    }

    Shape::ShapeResult BodyCreateInfo::ConvertShapeSettings()
    {
        if (m_shape != nullptr)
        {
            m_shapeSettings = nullptr;

            Shape::ShapeResult result;
            Shape* pNonConst = const_cast<Shape*>(m_shape.Get());
            result.Set(std::move(pNonConst));
            return result;
        }

        // Check if we have shape settings.
        if (m_shapeSettings == nullptr)
        {
            Shape::ShapeResult result;
            result.SetError("No shape settings present!");
            return result;
        }

        Shape::ShapeResult result = m_shapeSettings->Create();
        if (result.IsValid())
            m_shape = result.Get();

        m_shapeSettings = nullptr;
        return result;
    }

    MassProperties BodyCreateInfo::GetMassProperties() const
    {
        MassProperties result;
        switch (m_overrideMassProperties)
        {
            case EOverrideMassProperties::CalculateMassAndInertia:
            {
                result = GetShape()->GetMassProperties();
                result.m_inertia *= m_inertiaMultiplier;
                result.m_inertia[3][3] = 1.f;
                break;
            }
            case EOverrideMassProperties::CalculateInertia:
            {
                result = GetShape()->GetMassProperties();
                result.ScaleToMass(m_massPropertiesOverride.m_mass);
                result.m_inertia *= m_inertiaMultiplier;
                result.m_inertia[3][3] = 1.f;
                break;
            }
            case EOverrideMassProperties::MassAndInertiaProvided:
            {
                result = m_massPropertiesOverride;
                break;
            }
        }
        
        return result;
    }
}

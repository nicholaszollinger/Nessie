// EmptyShape.cpp
#include "EmptyShape.h"

namespace nes
{
    ShapeSettings::ShapeResult EmptyShapeSettings::Create() const
    {
        if (m_cachedResult.IsEmpty())
        {
            // Stores the pointer in the m_cachedResult
            new EmptyShape(*this, m_cachedResult);
        }
        return m_cachedResult;
    }

    EmptyShape::EmptyShape(const EmptyShapeSettings& settings, ShapeResult& outResult)
        : Shape(ShapeType::Empty, ShapeSubType::Empty, settings, outResult)
        , m_centerOfMass(settings.m_centerOfMass)
    {
        outResult.Set(this);
    }

    MassProperties EmptyShape::GetMassProperties() const
    {
        MassProperties massProperties;
        massProperties.m_mass = 1.f;
        massProperties.m_inertia = Mat4::Identity();
        return massProperties;
    }

    void EmptyShape::Register()
    {
        ShapeFunctions& f = ShapeFunctions::Get(ShapeSubType::Empty);
        f.m_construct = []() -> Shape* { return new EmptyShape; };
        f.m_color = Color::Black();
    }
    
}

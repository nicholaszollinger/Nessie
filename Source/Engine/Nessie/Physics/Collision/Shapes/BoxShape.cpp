// BoxShape.cpp
#include "BoxShape.h"

#include "Math/IntersectionQueries3.h"
#include "Math/Geometry/RayAABox.h"
#include "Math/SIMD/VectorRegisterF.h"
#include "Physics/Collision/CastResult.h"
#include "Physics/Collision/CollidePointResult.h"
#include "Physics/Collision/RayCast.h"
#include "Physics/Collision/TransformedShape.h"
#include "Physics/Collision/Shapes/GetTrianglesContext.h"
#include "Physics/Collision/Shapes/ScaleHelpers.h"

namespace nes
{
    static constexpr Vector3 kUnitBoxTriangles[] =
    {
        Vector3(-1, 1, -1),	Vector3(-1, 1, 1),	Vector3(1, 1, 1),
        Vector3(-1, 1, -1),	Vector3(1, 1, 1),		Vector3(1, 1, -1),
        Vector3(-1, -1, -1),	Vector3(1, -1, -1),	Vector3(1, -1, 1),
        Vector3(-1, -1, -1),	Vector3(1, -1, 1),	Vector3(-1, -1, 1),
        Vector3(-1, 1, -1),	Vector3(-1, -1, -1),	Vector3(-1, -1, 1),
        Vector3(-1, 1, -1),	Vector3(-1, -1, 1),	Vector3(-1, 1, 1),
        Vector3(1, 1, 1),		Vector3(1, -1, 1),	Vector3(1, -1, -1),
        Vector3(1, 1, 1),		Vector3(1, -1, -1),	Vector3(1, 1, -1),
        Vector3(-1, 1, 1),	Vector3(-1, -1, 1),	Vector3(1, -1, 1),
        Vector3(-1, 1, 1),	Vector3(1, -1, 1),	Vector3(1, 1, 1),
        Vector3(-1, 1, -1),	Vector3(1, 1, -1),	Vector3(1, -1, -1),
        Vector3(-1, 1, -1),	Vector3(1, -1, -1),	Vector3(-1, -1, -1)
    };

    class BoxShape::Box final : public Support
    {
        AABox m_box;
        float m_convexRadius = 0.f;
        
    public:
        Box(const AABox& box, float convexRadius)
            : m_box(box)
            , m_convexRadius(convexRadius)
        {
            static_assert(sizeof(Box) <= sizeof(SupportBuffer), "Buffer size too small");
            NES_ASSERT(math::IsAligned(this, alignof(Box)));
        }

        virtual Vector3 GetSupport(const Vector3& direction) const override
        {
            return m_box.GetSupport(direction);
        }

        virtual float GetConvexRadius() const override
        {
            return m_convexRadius;
        }
    };

    
    BoxShapeSettings::BoxShapeSettings(const Vector3& halfExtent, float convexRadius)
        : ConvexShapeSettings()
        , m_halfExtent(halfExtent)
        , m_convexRadius(convexRadius)
    {
        //
    }

    ShapeSettings::ShapeResult BoxShapeSettings::Create() const
    {
        if (m_cachedResult.IsEmpty())
        {
            StrongPtr<Shape> pShape = new BoxShape(*this, m_cachedResult);
        }
        return m_cachedResult;
    }

    BoxShape::BoxShape(const BoxShapeSettings& settings, ShapeResult& outResult)
        : ConvexShape(ShapeSubType::Box, settings, outResult)
        , m_halfExtent(settings.m_halfExtent)
        , m_convexRadius(settings.m_convexRadius)
    {
        // Check convex radius
        if (settings.m_convexRadius < 0.f
            || settings.m_halfExtent.ReduceMin() < settings.m_convexRadius)
        {
            outResult.SetError("Invalid Convex Radius");
            return;
        }

        outResult.Set(this);
    }

    AABox BoxShape::GetLocalBounds() const
    {
        return AABox(-m_halfExtent, m_halfExtent);
    }

    float BoxShape::GetInnerRadius() const
    {
        return m_halfExtent.ReduceMin();
    }

    MassProperties BoxShape::GetMassProperties() const
    {
        MassProperties props;
        props.SetMassAndInertiaOfSolidBox(2.f * m_halfExtent, GetDensity());
        return props;
    }

    Vector3 BoxShape::GetSurfaceNormal(const SubShapeID& subShapeID, const Vector3& localSurfacePosition) const
    {
        NES_ASSERTV(subShapeID.IsEmpty(), "Invalid subshape ID");

        // Get the component that is closest to the surface of the box.
        const int index = (localSurfacePosition.Abs() - m_halfExtent).Abs().GetLowestComponentIndex();

        // Calculate the normal
        Vector3 normal = Vector3::Zero();
        normal[index] = localSurfacePosition[index] > 0.f ? 1.f : -1.f;
        return normal;
    }

    bool BoxShape::CastRay(const RayCast& ray, const SubShapeIDCreator& subShapeIDCreator, RayCastResult& hitResult) const
    {
        float fraction = math::Max(RayAABox(ray.m_origin, RayInvDirection(ray.m_direction), -m_halfExtent, m_halfExtent), 0.f);
        if (fraction < hitResult.m_fraction)
        {
            hitResult.m_fraction = fraction;
            hitResult.m_subShapeID2 = subShapeIDCreator.GetID();
            return true;
        }

        return false;
    }

    void BoxShape::CastRay(const RayCast& ray, const RayCastSettings& settings,
        const SubShapeIDCreator& subShapeIDCreator, CastRayCollector& collector, const ShapeFilter& shapeFilter) const
    {
        // Test the shape filter
        if (!shapeFilter.ShouldCollide(this, subShapeIDCreator.GetID()))
            return;

        float minFraction, maxFraction;
        RayAABox(ray.m_origin, RayInvDirection(ray.m_direction), -m_halfExtent, m_halfExtent, minFraction, maxFraction);
        if (minFraction <= maxFraction  // Ray should intersect
            && maxFraction >= 0.f       // End of ray should be inside box
            && minFraction < collector.GetEarlyOutFraction()) // Start of ray should be before the early out fraction
        {
            // Better hit than the current hit
            RayCastResult hit;
            hit.m_bodyID = TransformedShape::GetBodyID(collector.GetContext());
            hit.m_subShapeID2 = subShapeIDCreator.GetID();

            // Check front side
            if (settings.m_treatConvexAsSolid || minFraction > 0.f)
            {
                hit.m_fraction = math::Max(0.f, minFraction);
                collector.AddHit(hit);
            }

            // Check back side hit
            if (settings.m_backfaceModeConvex == BackFaceMode::CollideWithBackFaces
                && maxFraction < collector.GetEarlyOutFraction())
            {
                hit.m_fraction = maxFraction;
                collector.AddHit(hit);
            }
        }
    }

    void BoxShape::CollidePoint(const Vector3& point, const SubShapeIDCreator& subShapeIDCreator,
        CollidePointCollector& collector, const ShapeFilter& shapeFilter) const
    {
        // Test Shape filter
        if (!shapeFilter.ShouldCollide(this, subShapeIDCreator.GetID()))
            return;

        if (VectorRegisterF::LesserOrEqual(point.Abs(), m_halfExtent).TestAllXYZTrue())
        {
            collector.AddHit({ TransformedShape::GetBodyID(collector.GetContext()), subShapeIDCreator.GetID() });
        }
        
    }

    void BoxShape::GetTrianglesStart(GetTrianglesContext& context, [[maybe_unused]] const AABox& box, const Vector3& positionCOM,
        const Quat& rotation, const Vector3& scale) const
    {
        new (&context) GetTrianglesContextVertexList(positionCOM, rotation, scale, math::MakeScaleMatrix(m_halfExtent), kUnitBoxTriangles, std::size(kUnitBoxTriangles));
    }

    int BoxShape::GetTrianglesNext(GetTrianglesContext& context, int maxTrianglesRequested,
        Float3* outTriangleVertices) const
    {
        return (reinterpret_cast<GetTrianglesContextVertexList&>(context).GetTrianglesNext(maxTrianglesRequested, outTriangleVertices));
    }

    float BoxShape::GetVolume() const
    {
        return GetLocalBounds().GetVolume();
    }

    const ConvexShape::Support* BoxShape::GetSupportFunction(SupportMode mode, SupportBuffer& buffer,
        const Vector3& scale) const
    {
        Vector3 scaledHalfExtent = scale.Abs() * m_halfExtent;

        switch (mode)
        {
            case SupportMode::ExcludeConvexRadius:
            case SupportMode::Default:
            {
                // Make a box out of our half extents
                AABox box = AABox(-scaledHalfExtent, scaledHalfExtent);
                NES_ASSERT(box.IsValid());
                return new (&buffer) Box(box, 0.f);
                
            }
            case SupportMode::IncludeConvexRadius:
            {
                // Reduce the box by our convex radius
                float convexRadius = ScaleHelpers::ScaleConvexRadius(m_convexRadius, scale);
                Vector3 convexRadius3 = Vector3::Replicate(convexRadius);
                Vector3 reducedHalfExtent = scaledHalfExtent - convexRadius3;
                AABox box = AABox(-reducedHalfExtent, reducedHalfExtent);
                NES_ASSERT(box.IsValid());
                
                return new (&buffer) Box(box, convexRadius);
            }
        }

        NES_ASSERT(false);
        return nullptr;
    }

    void BoxShape::GetSupportingFace(const SubShapeID& subShapeID, const Vector3& direction, const Vector3& scale,
        const Mat4& centerOfMassTransform, SupportingFace& outVertices) const
    {
        NES_ASSERTV(subShapeID.IsEmpty(), "Invalid subshape ID");

        Vector3 scaledHalfExtent = scale.Abs() * m_halfExtent;
        const AABox box(-scaledHalfExtent, scaledHalfExtent);
        box.GetSupportingFace(direction, outVertices);

        // Transform to world space
        for (Vector3& vertex : outVertices)
        {
            vertex = centerOfMassTransform.TransformPoint(vertex);
        }
    }

    void BoxShape::Register()
    {
        ShapeFunctions& f = ShapeFunctions::Get(ShapeSubType::Box);
        f.m_construct = []() -> Shape* { return new BoxShape; };
        f.m_color = Color::Green();
    }
}

// Shape.cpp
#include "Shape.h"
//#include "ScaledShape.h"
//#include "StaticCompoundShape.h"
#include "ScaleHelpers.h"
#include "Physics/Collision/RayCast.h"
#include "Physics/Collision/CastResult.h"
#include "Physics/Collision/TransformedShape.h"
#include "Physics/Collision/CollidePointResult.h"

namespace nes
{
    ShapeFunctions ShapeFunctions::s_registry[kNumSubShapeTypes];
    
    AABox Shape::GetWorldBounds(const Mat4& centerOfMassTransform, const Vector3& scale) const
    {
        return GetLocalBounds().Scaled(scale).Transformed(centerOfMassTransform);
    }

    const Shape* Shape::GetLeafShape(const SubShapeID& subShapeID, SubShapeID& outRemainder) const
    {
        outRemainder = subShapeID;
        return this;
    }

    TransformedShape Shape::GetSubShapeTransformedShape([[maybe_unused]] const SubShapeID& subShapeID, const Vector3& positionCOM,
        const Quat& rotation, const Vector3& scale, SubShapeID& outRemainder) const
    {
        // We have reached the leaf shape so there is no remainder
        outRemainder = SubShapeID();

        // Just return the transformed shape for this shape
        TransformedShape tShape(Vector3(positionCOM), rotation, this, BodyID());
        tShape.SetShapeScale(scale);
        return tShape;
    }

    void Shape::CollectTransformedShapes([[maybe_unused]] const AABox& box, const Vector3& positionCOM, const Quat& rotation,
        const Vector3& scale, const SubShapeIDCreator& subShapeIDCreator, TransformedShapeCollector& collector,
        const ShapeFilter& shapeFilter) const
    {
        // Test the shape filter
        if (shapeFilter.ShouldCollide(this, subShapeIDCreator.GetID()))
            return;

        TransformedShape tShape(positionCOM, rotation, this, TransformedShape::GetBodyID(collector.GetContext()), subShapeIDCreator);
        tShape.SetShapeScale(scale);
        collector.AddHit(tShape);
    }

    void Shape::TransformShape(const Mat4& centerOfMassTransform, TransformedShapeCollector& collector) const
    {
        Vector3 scale;
        Mat4 transform = centerOfMassTransform.Decompose(scale);
        TransformedShape tShape(transform.GetTranslation(), math::ToQuat(transform), this, BodyID(), SubShapeIDCreator());
        tShape.SetShapeScale(MakeScaleValid(scale));
        collector.AddHit(tShape);
    }

    Shape::ShapeResult Shape::ScaleShape(const Vector3& scale) const
    {
        constexpr Vector3 unitScale = Vector3::Unit();

        if (scale.IsNearZero())
        {
            ShapeResult result;
            result.SetError("Can't use zero scale!");
            return result;
        }

        // First test if we can just wrap this shape in a scaled shape
        if (IsValidScale(scale))
        {
            ShapeResult result;
            if (scale.IsClose(unitScale))
                result.Set(const_cast<Shape*>(this));
            else
            {
                result.SetError("I need to implement the Scaled Shape class!");
                // [TODO]: 
                //result.Set(new ScaledShape(this, scale));
            }
            return result;
        }

        ShapeResult result;
        result.SetError("I need to implement the static compound shape class!");
        return result;

        // [TODO]: 
        // The scale is invalid.
        // Collect the leaf shapes and their transforms:
        // struct Collector : TransformedShapeCollector
        // {
        //     std::vector<TransformedShape> m_shapes;
        //     
        //     virtual void AddHit(const ResultType& result) override
        //     {
        //         m_shapes.push_back(result);
        //     }
        // };
        
        // Collector collector;
        // TransformShape(math::MakeScaleMatrix(scale) * math::MakeTranslationMatrix4(GetCenterOfMass()), collector);
        //
        // // Construct a compound shape
        // StaticCompoundShapeSettings compound;
        // compound.m_subShapes.reserve(collector.m_shapes.size());
        // for (const TransformedShape& tShape : collector.m_shapes)
        // {
        //     const Shape* pShape = tShape.m_pShape;
        //
        //     // Construct a scaled shape if scale is not unit
        //     Vector3 transformedScale = tShape.GetShapeScale();
        //     if (!transformedScale.IsClose(unitScale))
        //         pShape = new ScaledShape(this, transformedScale);
        //
        //     // Add the shape
        //     compound.AddShape(Vector3(tShape.m_shapePositionCOM) - tShape.m_shapeRotation * pShape->GetCenterOfMass(), tShape.m_shapeRotation, pShape);
        // }
        //
        // return compound.Create();
    }

    bool Shape::IsValidScale(const Vector3& scale) const
    {
        return !ScaleHelpers::IsZeroScale(scale);
    }

    Vector3 Shape::MakeScaleValid(const Vector3& scale) const
    {
        return ScaleHelpers::MakeNonZeroScale(scale);
    }

    void Shape::CollidePointUsingRayCast(const Shape& shape, const Vector3& point,
        const SubShapeIDCreator& subShapeIDCreator, CollidePointCollector& collector, const ShapeFilter& shapeFilter)
    {
        // First test if we're inside our bounding box
        AABox bounds = shape.GetLocalBounds();
        if (bounds.Contains(point))
        {
            // A collector that just counts the number of hits:
            struct HitCountCollector : public CastRayCollector
            {
                int         m_hitCount = 0;
                SubShapeID  m_subShapeID;

                virtual void AddHit(const ResultType& result) override
                {
                    // Store the last sub shape ID so that we can provide something to our outer hit collector
                    m_subShapeID = result.m_subShapeID2;
                    ++m_hitCount;
                }
            };

            HitCountCollector hitCollector;

            // Configure the ray cast
            RayCastSettings settings;
            settings.SetBackFaceMode(BackFaceMode::CollideWithBackFaces);

            // Cast a ray that's 10% longer than the height of our bounding box.
            shape.CastRay(RayCast{point, 1.1f * bounds.Size().y * Vector3::Up() }, settings, subShapeIDCreator, hitCollector, shapeFilter);

            if ((hitCollector.m_hitCount & 1) == 1)
            {
                collector.AddHit({ TransformedShape::GetBodyID(collector.GetContext()), hitCollector.m_subShapeID});
            }
        }
    }
}

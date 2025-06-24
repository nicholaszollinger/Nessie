// TransformedShape.cpp
#include "TransformedShape.h"

#include "RayCast.h"
#include "ShapeCast.h"
#include "CastResult.h"
#include "Shapes/SubShapeID.h"
#include "CollisionSolver.h"
#include "Geometry/OrientedBox.h"

namespace nes
{
    bool TransformedShape::CastRay(const RRayCast& ray, RayCastResult& hit) const
    {
        if (m_pShape != nullptr)
        {
            // Transform the ray to local space. Note that this drops precision which is possible because we're in local space now.
            RayCast localRay(ray.Transformed(GetInverseCenterOfMassTransform()));

            // Scale the ray
            Vec3 inverseScale = GetShapeScale().Reciprocal();
            localRay.m_origin *= inverseScale;
            localRay.m_direction *= inverseScale;

            // Cast the ray onto the shape
            SubShapeIDCreator subShapeID(m_subShapeIDCreator);
            if (m_pShape->CastRay(localRay, subShapeID, hit))
            {
                // Set the body ID on the hit result
                hit.m_bodyID = m_bodyID;
                return true;
            }
        }

        return false;
    }

    void TransformedShape::CastRay(const RRayCast& ray, const RayCastSettings& rayCastSettings, CastRayCollector& collector, const ShapeFilter& shapeFilter) const
    {
        if (m_pShape != nullptr)
        {
            // Set the context on the collector and filter
            collector.SetContext(this);
            shapeFilter.m_bodyID2 = m_bodyID;

            // Transform the ray to local space. Note that this drops precision which is possible because we're in local space now.
            RayCast localRay(ray.Transformed(GetInverseCenterOfMassTransform()));

            // Scale the ray
            Vec3 inverseScale = GetShapeScale().Reciprocal();
            localRay.m_origin *= inverseScale;
            localRay.m_direction *= inverseScale;

            // Cast the ray onto the shape
            SubShapeIDCreator subShapeID(m_subShapeIDCreator);
            m_pShape->CastRay(localRay, rayCastSettings, subShapeID, collector, shapeFilter); 
        }
    }

    void TransformedShape::CollidePoint(const RVec3& point, CollidePointCollector& collector, const ShapeFilter& shapeFilter) const
    {
        if (m_pShape != nullptr)
        {
            // Set the context on the collector and filter
            collector.SetContext(this);
            shapeFilter.m_bodyID2 = m_bodyID;

            // Transform and scale the point to local space
            const Vec3 localPoint = Vec3(GetInverseCenterOfMassTransform().TransformPoint(point)) / GetShapeScale();
            
            // Do point collide on the Shape
            SubShapeIDCreator subShapeID(m_subShapeIDCreator);
            m_pShape->CollidePoint(localPoint, subShapeID, collector, shapeFilter);
        }
    }

    void TransformedShape::CollideShape(const Shape* pShape, const Vec3& shapeScale, const Mat44& centerOfMassTransform, const CollideShapeSettings& collideShapeSettings, const RVec3& baseOffset, CollideShapeCollector& collector, const ShapeFilter& shapeFilter) const
    {
        if (m_pShape != nullptr)
        {
            // Set the context on the collector and filter
            collector.SetContext(this);
            shapeFilter.m_bodyID2 = m_bodyID;
            
            // Collide the shapes.
            SubShapeIDCreator subShapeID1(m_subShapeIDCreator);
            SubShapeIDCreator subShapeID2(m_subShapeIDCreator);
            Mat44 transform1 = centerOfMassTransform.PostTranslated(-baseOffset);
            Mat44 transform2 = GetCenterOfMassTransform().PostTranslated(-baseOffset);
            CollisionSolver::CollideShapeVsShape(pShape, m_pShape, shapeScale, GetShapeScale(), transform1, transform2, subShapeID1, subShapeID2, collideShapeSettings, collector, shapeFilter);
        }
    }

    void TransformedShape::CastShape(const RShapeCast& shapeCast, const ShapeCastSettings& settings, const RVec3& baseOffset, CastShapeCollector& collector, const ShapeFilter& shapeFilter) const
    {
        if (m_pShape != nullptr)
        {
            // Set the context on the collector and filter
            collector.SetContext(this);
            shapeFilter.m_bodyID2 = m_bodyID;

            // Get the shape cast relative to the base offset and convert it to floats
            ShapeCast localShapeCast(shapeCast.PostTranslated(-baseOffset));

            // Get center of mass of object we're casting against relative to the base offset and convert it to floats
            Mat44 centerOfMassTransform2 = GetCenterOfMassTransform().PostTranslated(-baseOffset);

            // Cast the shape onto this one.
            SubShapeIDCreator subShapeID1(m_subShapeIDCreator);
            SubShapeIDCreator subShapeID2(m_subShapeIDCreator);
            CollisionSolver::CastShapeVsShapeWorldSpace(localShapeCast, settings, m_pShape, GetShapeScale(), shapeFilter, centerOfMassTransform2, subShapeID1, subShapeID2, collector);
        }
    }

    void TransformedShape::CollectTransformedShapes(const AABox& box, TransformedShapeCollector& collector, const ShapeFilter& shapeFilter) const
    {
        if (m_pShape != nullptr)
        {
            struct MyCollector : public TransformedShapeCollector
            {
                TransformedShapeCollector& m_collector;
                Vec3 m_shapePositionCOM;
                
                MyCollector(TransformedShapeCollector& collector, const Vec3& shapePositionCOM)
                    : TransformedShapeCollector(collector)
                    , m_collector(collector)
                    , m_shapePositionCOM(shapePositionCOM)
                {
                    //
                }

                virtual void AddHit(const TransformedShape& result) override
                {
                    // Apply the center of mass offset
                    TransformedShape tShape = result;
                    tShape.m_shapePositionCOM += m_shapePositionCOM;

                    // Pass hit on to the child collector
                    m_collector.AddHit(tShape);

                    // Update early out fraction based on child collector
                    UpdateEarlyOutFraction(m_collector.GetEarlyOutFraction());
                }
            };

            // Set the context on the collector.
            collector.SetContext(this);

            // Wrap the collector so we can add the center of mass precision, we do this to avoid losing precision because CollectTransformedShapes uses single
            // precision floats.
            MyCollector myCollector(collector, m_shapePositionCOM);

            // Take box to local space for the shape
            AABox localBox = box;
            localBox.Translate(-m_shapePositionCOM);

            m_pShape->CollectTransformedShapes(localBox, Vec3::Zero(), m_shapeRotation, GetShapeScale(), m_subShapeIDCreator, myCollector, shapeFilter);
        }
    }
    
    void TransformedShape::GetTrianglesStart(GetTrianglesContext& context, const AABox& box, const Vec3& baseOffset) const
    {
        if (m_pShape != nullptr)
        {
            // Transform box to local space for the shape
            AABox localBox = box;
            localBox.Translate(-baseOffset);

            m_pShape->GetTrianglesStart(context, localBox, Vec3(m_shapePositionCOM - baseOffset), m_shapeRotation, GetShapeScale());
        }
    }

    int TransformedShape::GetTrianglesNext(GetTrianglesContext& context, int maxTrianglesRequested, Float3* outTriangleVertices) const
    {
        if (m_pShape != nullptr)
            return m_pShape->GetTrianglesNext(context, maxTrianglesRequested, outTriangleVertices/*, outMaterials*/);

        return 0;
    }
}
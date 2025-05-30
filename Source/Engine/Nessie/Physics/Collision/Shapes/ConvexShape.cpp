// ConvexShape.cpp
#include "ConvexShape.h"

#include "Math/IntersectionQueries3.h"
#include "Math/OrientedBox.h"
#include "Math/Geometry/ConvexSupport.h"
#include "Math/Geometry/EPAPenetrationDepth.h"
#include "Math/Geometry/GJKClosestPoint.h"
#include "Physics/Collision/CastResult.h"
#include "Physics/Collision/CollidePointResult.h"
#include "Physics/Collision/CollideShape.h"
#include "Physics/Collision/CollisionSolver.h"
#include "Physics/Collision/RayCast.h"
#include "Physics/Collision/ShapeCast.h"
#include "Physics/Collision/TransformedShape.h"
#include "Physics/Collision/Shapes/GetTrianglesContext.h"
#include "Physics/Collision/Shapes/ScaleHelpers.h"

namespace nes
{
    //----------------------------------------------------------------------------------------------------
    /// @brief : 'ConvexShape GetTrianglesContext'. 
    //----------------------------------------------------------------------------------------------------
    class ConvexShape::CSGetTrianglesContext
    {
    public:
        CSGetTrianglesContext(const ConvexShape* pShape, const Vector3& positionCOM, const Quat& rotation, const Vector3& scale)
            : m_localToWorld(math::MakeRotationTranslationMatrix(positionCOM, rotation) * math::MakeScaleMatrix(scale))
            , m_isInsideOut(ScaleHelpers::IsInsideOut(scale))
        {
            m_pSupport = pShape->GetSupportFunction(ESupportMode::IncludeConvexRadius, m_supportBuffer, Vector3::Unit());
        }

        SupportBuffer		m_supportBuffer;
        const Support*		m_pSupport;
        Mat4				m_localToWorld;
        bool				m_isInsideOut;
        size_t				m_currentVertex = 0;
    };
    
    const StaticArray<Vector3, 384> ConvexShape::s_unitSphereTriangles = []()
    {
        constexpr int level = 2;
        StaticArray<Vector3, 384> result;
        GetTrianglesContextVertexList::CreateHalfUnitSphereTop(result, level);
        GetTrianglesContextVertexList::CreateHalfUnitSphereBottom(result, level);
        return result;
    }();
    
    ConvexShape::ConvexShape(const EShapeSubType subType, const ConvexShapeSettings& settings, ShapeResult& outResult)
        : Shape(EShapeType::Convex, subType, settings, outResult)
        //, m_pMaterial
        , m_density(settings.GetDensity())
    {
        //
    }

    bool ConvexShape::CastRay(const RayCast& ray, const SubShapeIDCreator& subShapeIDCreator, RayCastResult& hitResult) const
    {
        // [Note]: This is a fallback routine, most convex shapes should implement a more performant version!
        // [TODO]: Profile

        // Create support function
        SupportBuffer buffer;
        const Support* pSupport = GetSupportFunction(ESupportMode::IncludeConvexRadius, buffer, Vector3::Unit());

        // Cast Ray
        GJKClosestPoint gjk;
        if (gjk.CastRay(ray.m_origin, ray.m_direction, physics::kDefaultCollisionTolerance, *pSupport, hitResult.m_fraction))
        {
            hitResult.m_subShapeID2 = subShapeIDCreator.GetID();
            return true;
        }

        return false;
    }

    void ConvexShape::CastRay(const RayCast& ray, const RayCastSettings& settings,
        const SubShapeIDCreator& subShapeIDCreator, CastRayCollector& collector, const ShapeFilter& shapeFilter) const
    {
        // [Note]: This is a fallback routine, most convex shapes should implement a more performant version!

        // Test shape filter
        if (!shapeFilter.ShouldCollide(this, subShapeIDCreator.GetID()))
            return;

        // First do a normal raycast, limited to the early out fraction.
        RayCastResult hitResult;
        hitResult.m_fraction = collector.GetEarlyOutFraction();
        if (CastRay(ray, subShapeIDCreator, hitResult))
        {
            // Check front side
            if (settings.m_treatConvexAsSolid || hitResult.m_fraction > 0.f)
            {
                hitResult.m_bodyID = TransformedShape::GetBodyID(collector.GetContext());
                collector.AddHit(hitResult);
            }

            // Check if we want back facing hits and the collector still accepts additional hits 
            if (settings.m_backfaceModeConvex == EBackFaceMode::CollideWithBackFaces && !collector.ShouldEarlyOut())
            {
                // Invert the ray, going from the early out fraction back to the fraction where we found our forward hit.
                const float startFraction = math::Min(1.f, collector.GetEarlyOutFraction());
                const float deltaFraction = hitResult.m_fraction - startFraction;
                if (deltaFraction < 0.f)
                {
                    RayCast invertedRay { ray.m_origin + startFraction * ray.m_direction, deltaFraction * ray.m_direction };

                    // Cast another Ray
                    RayCastResult invertedHit;
                    invertedHit.m_fraction = 1.f;
                    if (CastRay(invertedRay, subShapeIDCreator, invertedHit)
                        && invertedHit.m_fraction > 0.f) // Ignore hits with fraction == 0, this means the ray end inside the object and we don't want to report it as a back-facing hit
                    {
                        // Invert fraction and rescale it to the fraction of the original ray.
                        invertedHit.m_fraction = hitResult.m_fraction + (invertedHit.m_fraction - 1.f) * deltaFraction;
                        invertedHit.m_bodyID = TransformedShape::GetBodyID(collector.GetContext());
                        collector.AddHit(invertedHit);
                    }
                }
            }
        }
    }

    void ConvexShape::CollidePoint(const Vector3& point, const SubShapeIDCreator& subShapeIDCreator, CollidePointCollector& collector, const ShapeFilter& shapeFilter) const
    {
        // Test Shape filter
        if (!shapeFilter.ShouldCollide(this, subShapeIDCreator.GetID()))
            return;

        // First test bounding box
        if (GetLocalBounds().Contains(point))
        {
            // Create the support function
            SupportBuffer buffer;
            const Support* pSupport = GetSupportFunction(ESupportMode::IncludeConvexRadius, buffer, Vector3::Unit());

            // Create the support function for point
            PointConvexSupport convexPoint { point };

            GJKClosestPoint gjk;
            Vector3 vec = point;
            if (gjk.Intersects(*pSupport, convexPoint, physics::kDefaultCollisionTolerance, vec))
            {
                collector.AddHit({ TransformedShape::GetBodyID(collector.GetContext()), subShapeIDCreator.GetID()});
            }
        }
    }

    void ConvexShape::GetTrianglesStart(GetTrianglesContext& context, [[maybe_unused]] const AABox& box, const Vector3& positionCOM,
        const Quat& rotation, const Vector3& scale) const
    {
        static_assert(sizeof(CSGetTrianglesContext) <= sizeof(GetTrianglesContext), "GetTrianglesContext is too small!");
        NES_ASSERT(math::IsAligned(&context, alignof(CSGetTrianglesContext)));

        new (&context) CSGetTrianglesContext(this, positionCOM, rotation, scale);
    }

    int ConvexShape::GetTrianglesNext(GetTrianglesContext& context, int maxTrianglesRequested,
        Float3* outTriangleVertices) const
    {
        NES_ASSERT(maxTrianglesRequested >= kGetTrianglesMinTrianglesRequested);

        CSGetTrianglesContext& csContext = reinterpret_cast<CSGetTrianglesContext&>(context);

        int totalNumVertices = math::Min(maxTrianglesRequested * 3, static_cast<int>(s_unitSphereTriangles.size() - csContext.m_currentVertex));

        if (csContext.m_isInsideOut)
        {
            // Store the triangles flipped
            for (const Vector3* pVert = s_unitSphereTriangles.data() + csContext.m_currentVertex, *pEnd = pVert + totalNumVertices; pVert < pEnd; pVert += 3)
            {
                (csContext.m_localToWorld.TransformPoint(csContext.m_pSupport->GetSupport(pVert[0])).StoreFloat3(outTriangleVertices++));
                (csContext.m_localToWorld.TransformPoint(csContext.m_pSupport->GetSupport(pVert[2])).StoreFloat3(outTriangleVertices++));
                (csContext.m_localToWorld.TransformPoint(csContext.m_pSupport->GetSupport(pVert[1])).StoreFloat3(outTriangleVertices++));
            }
        }
        else
        {
            // Store triangles
            for (const Vector3* pVert = s_unitSphereTriangles.data() + csContext.m_currentVertex, *pEnd = pVert + totalNumVertices; pVert < pEnd; pVert += 3)
            {
                (csContext.m_localToWorld.TransformPoint(csContext.m_pSupport->GetSupport(pVert[0])).StoreFloat3(outTriangleVertices++));
                (csContext.m_localToWorld.TransformPoint(csContext.m_pSupport->GetSupport(pVert[1])).StoreFloat3(outTriangleVertices++));
                (csContext.m_localToWorld.TransformPoint(csContext.m_pSupport->GetSupport(pVert[2])).StoreFloat3(outTriangleVertices++));
            }
        }

        csContext.m_currentVertex += totalNumVertices;
        int totalNumTriangles = totalNumVertices / 3;

        // [TODO]: 
        // Store Materials

        return totalNumTriangles;
    }

    void ConvexShape::Register()
    {
        for (const EShapeSubType subType1 : kConvexSubShapeTypes)
        {
            for (const EShapeSubType subType2 : kConvexSubShapeTypes)
            {
                CollisionSolver::RegisterCollideShape(subType1, subType2, CollideConvexVsConvex);
                CollisionSolver::RegisterCastShape(subType1, subType2, CastConvexVsConvex);
            }
        }
    }

    void ConvexShape::CollideConvexVsConvex(const Shape* pShape1, const Shape* pShape2, const Vector3& scale1,
        const Vector3& scale2, const Mat4& centerOfMassTransform1, const Mat4& centerOfMassTransform2,
        const SubShapeIDCreator& subShapeIDCreator1, const SubShapeIDCreator& subShapeIDCreator2,
        const CollideShapeSettings& collideShapeSettings, CollideShapeCollector& collector,
        [[maybe_unused]] const ShapeFilter& shapeFilter)
    {
        // Get the shapes:
        NES_ASSERT(pShape1->GetType() == EShapeType::Convex);
        NES_ASSERT(pShape2->GetType() == EShapeType::Convex);
        const ConvexShape* pConvex1 = checked_cast<const ConvexShape*>(pShape1);
        const ConvexShape* pConvex2 = checked_cast<const ConvexShape*>(pShape2);

        // Get the transforms
        Mat4 inverseTransform1 = centerOfMassTransform1.InverseRotationTranslation();
        Mat4 transform2To1 = inverseTransform1 * centerOfMassTransform2;

        // Get the bounding boxes
        float maxSeparationDistance = collideShapeSettings.m_maxSeparationDistance;
        AABox shape1Box = pShape1->GetLocalBounds().Scaled(scale1);
        shape1Box.ExpandBy(Vector3::Replicate(maxSeparationDistance));
        AABox shape2Box = pShape2->GetLocalBounds().Scaled(scale2);
        
        // Check if they don't overlap
        if (!OrientedBox(transform2To1, shape2Box).Intersects(shape1Box))
            return;

        // Note: As we don't remember the penetration axis from the last iteration, and it is likely that
        // shape2 is pushed out of collision relative to shape1 by comparing their COM's, we use that as an
        // initial penetration axis: shape2.COM - shape1.COM
        // This has been seen to improve performance by approx. 1% over a fixed axis like (1, 0, 0).
        Vector3 penetrationAxis = transform2To1.GetTranslation();
        
        // Ensure that we do not pass in a near zero penetration axis.
        if (penetrationAxis.IsNearZero())
            penetrationAxis = Vector3::Right();

        Vector3 point1;
        Vector3 point2;
        EPAPenetrationDepth penDepth;
        EPAPenetrationDepth::Status status;

        // Scope to limit lifetime of the Support Buffer
        {
            // Create the Support functions
            SupportBuffer buffer1_ExclConvexRadius;
            SupportBuffer buffer2_ExclConvexRadius;
            const Support* pShape1_ExclConvexRadius = pConvex1->GetSupportFunction(ESupportMode::ExcludeConvexRadius, buffer1_ExclConvexRadius, scale1);
            const Support* pShape2_ExclConvexRadius = pConvex2->GetSupportFunction(ESupportMode::ExcludeConvexRadius, buffer2_ExclConvexRadius, scale2);

            // Transform shape 2 in the space of shape 1
            TransformedConvexObject transformed2_ExclConvexRadius(transform2To1, *pShape2_ExclConvexRadius);

            // Perform GJK step
            status = penDepth.GetPenetrationDepthStepGJK(
                *pShape1_ExclConvexRadius
                , pShape1_ExclConvexRadius->GetConvexRadius() + maxSeparationDistance
                , transformed2_ExclConvexRadius, pShape2_ExclConvexRadius->GetConvexRadius()
                , collideShapeSettings.m_collisionTolerance
                , penetrationAxis
                , point1
                , point2);
        }

        switch (status)
        {
            case EPAPenetrationDepth::Status::Colliding:
                break;
            
            case EPAPenetrationDepth::Status::NotColliding:
                return;
            
            case EPAPenetrationDepth::Status::Indeterminate:
            {
                // Need to run expensive EPA algorithm

                // We know we're overlapping at this point, so we can set the max separation distance to 0.0
                // Numerically it is possible that GJK finds that the shapes are overlapping but EPA finds that
                // they're separated. In order to avoid this, we clamp the max separation distance to 1 so that we
                // don't excessively inflate the shape, but we still inflate it enough to avoid the case where EPA
                // misses the collision.
                maxSeparationDistance = math::Min(maxSeparationDistance, 1.0f);

                // Create the Support functions
                SupportBuffer buffer1_InclConvexRadius;
                SupportBuffer buffer2_InclConvexRadius;
                const Support* pShape1_InclConvexRadius = pConvex1->GetSupportFunction(ESupportMode::IncludeConvexRadius, buffer1_InclConvexRadius, scale1);
                const Support* pShape2_InclConvexRadius = pConvex2->GetSupportFunction(ESupportMode::IncludeConvexRadius, buffer2_InclConvexRadius, scale2);

                // Add separation distance
                AddConvexRadius shape1_AddMaxSeparationDistance(*pShape1_InclConvexRadius, maxSeparationDistance);

                // Transform shape 2 in the space of shape 1
                TransformedConvexObject transformed2_InclConvexRadius(transform2To1, *pShape2_InclConvexRadius);

                // Perform EPA step
                if (!penDepth.GetPenetrationDepthStepEPA(
                    shape1_AddMaxSeparationDistance
                    , transformed2_InclConvexRadius
                    , collideShapeSettings.m_penetrationTolerance
                    , penetrationAxis
                    , point1
                    , point2))
                {
                    return;
                }
                break;
            }
        }

        // Check if the penetration is bigger than the early out function
        float penetrationDepth = (point2 - point1).Magnitude() - maxSeparationDistance;
        if (-penetrationDepth >= collector.GetEarlyOutFraction())
            return;

        // Correct point1 for the added separation distance
        float penetrationAxisLength = penetrationAxis.Magnitude();
        if (penetrationAxisLength > 0.f)
            point1 -= penetrationAxis * (maxSeparationDistance / penetrationAxisLength);

        // Convert to world space
        point1 = centerOfMassTransform1.TransformPoint(point1);
        point2 = centerOfMassTransform2.TransformPoint(point2);
        Vector3 penetrationAxisWorld = centerOfMassTransform1.TransformVector(penetrationAxis);

        // Create collision result
        CollideShapeResult result(point1, point2, penetrationAxisWorld, penetrationDepth, subShapeIDCreator1.GetID(), subShapeIDCreator2.GetID(), TransformedShape::GetBodyID(collector.GetContext()));

        // Gather faces
        if (collideShapeSettings.m_collectFacesMode == ECollectFacesMode::CollectFaces)
        {
            // Set the supporting face of shape 1
            pConvex1->GetSupportingFace(SubShapeID(), -penetrationAxis, scale1, centerOfMassTransform1, result.m_shape1Face);

            // Set the supporting face of shape 2
            pConvex2->GetSupportingFace(SubShapeID(), transform2To1.TransformVectorTranspose(penetrationAxis), scale2, centerOfMassTransform2, result.m_shape2Face);
        }

        // Add the hit to the collector.
        // [TODO]: NarrowPhase Tracking
        collector.AddHit(result);
    }

    void ConvexShape::CastConvexVsConvex(const ShapeCast& shapeCast, const ShapeCastSettings& shapeCastSettings,
        const Shape* pShape, const Vector3& scale, [[maybe_unused]] const ShapeFilter& shapeFilter, const Mat4& centerOfMassTransform2,
        const SubShapeIDCreator& subShapeIDCreator1, const SubShapeIDCreator& subShapeIDCreator2,
        CastShapeCollector& collector)
    {
        NES_ASSERT(shapeCast.m_pShape->GetType() == EShapeType::Convex);
        const ConvexShape* pCastShape = checked_cast<const ConvexShape*>(shapeCast.m_pShape);
        
        NES_ASSERT(pShape->GetType() == EShapeType::Convex);
        const ConvexShape* pTargetShape = checked_cast<const ConvexShape*>(pShape);

        // Determine if we want to use the actual shape or a shrunken shape with convex radius
        ESupportMode supportMode = shapeCastSettings.m_useShrunkenShapeAndConvexRadius? ESupportMode::ExcludeConvexRadius : ESupportMode::Default;
 
        // Create a support function for the cast shape.
        SupportBuffer castBuffer;
        const Support* pCastSupport = pCastShape->GetSupportFunction(supportMode, castBuffer, shapeCast.m_scale);

        // Create a support function for the target shape.
        SupportBuffer targetBuffer;
        const Support* pTargetSupport = pTargetShape->GetSupportFunction(supportMode, targetBuffer, scale);

        // Do a raycast against the result
        EPAPenetrationDepth epa;
        float fraction = collector.GetEarlyOutFraction();
        Vector3 contactPointA;
        Vector3 contactPointB;
        Vector3 contactNormal;

        if (epa.CastShape(
            shapeCast.m_centerOfMassStart
            , shapeCast.m_direction
            , shapeCastSettings.m_collisionTolerance
            , shapeCastSettings.m_penetrationTolerance
            , *pCastSupport, *pTargetSupport
            , pCastSupport->GetConvexRadius(), pTargetSupport->GetConvexRadius()
            , shapeCastSettings.m_returnDeepestPoint
            , fraction
            , contactPointA, contactPointB, contactNormal)
            && (shapeCastSettings.m_backfaceModeConvex == EBackFaceMode::CollideWithBackFaces
                || contactNormal.Dot(shapeCast.m_direction) > 0.f)) // Test if backfacing 
        {
            // Convert to world space
            contactPointA = centerOfMassTransform2.TransformPoint(contactPointA);
            contactPointB = centerOfMassTransform2.TransformPoint(contactPointB);
            const Vector3 contactNormalWorld = centerOfMassTransform2.TransformVector(contactNormal);

            ShapeCastResult result(fraction, contactPointA, contactPointB, contactNormalWorld, false, subShapeIDCreator1.GetID(), subShapeIDCreator2.GetID(), TransformedShape::GetBodyID(collector.GetContext()));

            // Early out if this hit is deeper than the collector's early out value:
            if (fraction == 0.f && -result.m_penetrationDepth >= collector.GetEarlyOutFraction())
                return;

            // Gather Faces
            if (shapeCastSettings.m_collectFacesMode == ECollectFacesMode::CollectFaces)
            {
                // Get the supporting face of shape 1
                Mat4 transform1To2 = shapeCast.m_centerOfMassStart;
                transform1To2.SetTranslation(transform1To2.GetTranslation() + fraction * shapeCast.m_direction);
                pCastShape->GetSupportingFace(SubShapeID(), transform1To2.TransformVectorTranspose(-contactNormal), shapeCast.m_scale, centerOfMassTransform2 * transform1To2, result.m_shape1Face);

                // Get the supporting face of shape 2
                pTargetShape->GetSupportingFace(SubShapeID(), contactNormal, scale, centerOfMassTransform2, result.m_shape2Face);
            }

            // [TODO]: Narrow Phase tracking
            collector.AddHit(result);
        }
    }
}

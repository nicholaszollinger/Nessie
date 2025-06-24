// NarrowPhaseQuery.h
#pragma once

#include "Physics/Body/BodyFilter.h"
#include "Physics/Body/BodyLock.h"
#include "Physics/Body/BodyLockInterface.h"
#include "Physics/Collision/ShapeFilter.h"
#include "Physics/Collision/BroadPhase/BroadPhaseQuery.h"
#include "Physics/Collision/BackFaceMode.h"

namespace nes
{
    class Shape;
    struct CollideShapeSettings;
    struct RayCastResult;

    //----------------------------------------------------------------------------------------------------
    /// @brief : Class that provides an interface for doing precise collision detection against the broad
    ///     and then narrow phase. Unlike a broadphase query, the NarrowPhaseQuery will test against shapes
    ///     and will return collision information against triangles, spheres, etc.
    //----------------------------------------------------------------------------------------------------
    class NarrowPhaseQuery
    {
    public:
        NarrowPhaseQuery() = default;
        NarrowPhaseQuery(const NarrowPhaseQuery&) = delete;
        NarrowPhaseQuery(NarrowPhaseQuery&&) noexcept = delete;
        NarrowPhaseQuery& operator=(const NarrowPhaseQuery&) = delete;
        NarrowPhaseQuery& operator=(NarrowPhaseQuery&&) noexcept = delete;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Cast a ray and find the closest hit. Returns true if it finds a hit.
        /// Hits further than hit.m_fraction will not be considered, and in this case, hit will remain
        /// unmodified (and the function will return false).
        ///
        /// Convex objects will be treated as solid (meaning if the ray starts inside, you'll get a hit fraction of 0),
        /// and the back face hits are returned.
        ///
        /// If you want the surface normal of the hit, use the following on the body with ID == hit.m_bodyID:
        /// <code> Body::GetWorldSpaceSurfaceNormal(hit.m_subShapeID2, ray.GetPointAlongRay(hit.m_fraction)) </code>
        //----------------------------------------------------------------------------------------------------
        bool                CastRay(const RRayCast& ray, RayCastResult& hit, const BroadPhaseLayerFilter& broadPhaseLayerFilter = {}, const CollisionLayerFilter& collisionLayerFilter = {}, const BodyFilter& bodyFilter = {}) const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Cast a ray, allows for collecting multiple hits.
        ///
        /// If you want the surface normal of a particular hit, use the following on the body with ID == hit.m_bodyID:
        /// <code> Body::GetWorldSpaceSurfaceNormal(collectedHit.m_subShapeID2, ray.GetPointAlongRay(collectedHit.m_fraction)) </code>
        ///
        /// @note : This version is more flexible, but also slightly slower than the CastRay function that returns
        /// only a single hit.
        //----------------------------------------------------------------------------------------------------
        void                CastRay(const RRayCast& ray, const RayCastSettings& rayCastSettings, CastRayCollector& inCollector, const BroadPhaseLayerFilter& broadPhaseLayerFilter = {}, const CollisionLayerFilter& collisionLayerFilter = {}, const BodyFilter& bodyFilter = {}, const ShapeFilter& shapeFilter = {}) const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Check if a point is inside any shapes. For this test, all shapes are considered solid.
        /// For a mesh shape, this test will only provide sensible information if the mesh is a closed manifold.
        /// For each shape that collides, 'collector' will receive a hit.
        //----------------------------------------------------------------------------------------------------
        void                CollidePoint(const RVec3 point, CollidePointCollector& inCollector, const BroadPhaseLayerFilter& broadPhaseLayerFilter = {}, const CollisionLayerFilter& collisionLayerFilter = {}, const BodyFilter& bodyFilter = {}, const ShapeFilter& shapeFilter = {}) const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Collide a shape with the Physic Scene.
        ///	@param pShape : Shape to test.
        ///	@param shapeScale : Scale in local space of the shape.
        ///	@param centerOfMassTransform : Center of mass transform for this shape.
        ///	@param collideShapeSettings : Settings.
        ///	@param baseOffset : All hit results will be returned relative to this offset. Can be zero to get results
        ///     in world space, but when you're testing far from the origin, you get better precision by picking a
        ///     position that's closer, e.g., centerOfMassTransform.GetTranslation() since floats are most accurate near the origin. 
        ///	@param inCollector : Collector that receives the hits.
        ///	@param broadPhaseLayerFilter : Filter that filters at the broadphase level.
        ///	@param collisionLayerFilter : Filter that filters at the collision layer level.
        ///	@param bodyFilter : Filter that filters at the body level.
        ///	@param shapeFilter : Filter that filters at the shape level.
        //----------------------------------------------------------------------------------------------------
        void                CollideShape(const Shape* pShape, const Vec3 shapeScale, const Mat44& centerOfMassTransform, const CollideShapeSettings& collideShapeSettings, const RVec3 baseOffset, CollideShapeCollector& inCollector, const BroadPhaseLayerFilter& broadPhaseLayerFilter = {}, const CollisionLayerFilter& collisionLayerFilter = {}, const BodyFilter& bodyFilter = {}, const ShapeFilter& shapeFilter = {}) const;

        // [TODO]: 
        //----------------------------------------------------------------------------------------------------
        /// @brief : Same as CollideShape, but uses InternalEdgeRemovingCollector to remove internal edges from
        ///     the collision results (a.k.a. ghost collisions).
        //----------------------------------------------------------------------------------------------------
        //void                CollideShapeWithInternalEdgeRemoval(const Shape* pShape, const Vec3 shapeScale, const Mat44& centerOfMassTransform, const CollideShapeSettings& collideShapeSettings, const RVec3 baseOffset, CollideShapeCollector& collector, const BroadPhaseLayerFilter& broadPhaseLayerFilter = {}, const CollisionLayerFilter& collisionLayerFilter = {}, const BodyFilter& bodyFilter = {}, const ShapeFilter& shapeFilter = {}) const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Cast a shape into the physics scene and report any hits to inCollector.
        ///	@param shapeCast : The shape cast and its position and direction.
        ///	@param settings : Settings for the shape cast.
        ///	@param baseOffset : All hit results will be returned relative to this offset. Can be zero to get results
        ///     in world space, but when you're testing far from the origin, you get better precision by picking a
        ///     position that's closer, e.g., centerOfMassTransform.GetTranslation() since floats are most accurate near the origin. 
        ///	@param inCollector : Collector that recieves the hits.
        ///	@param broadPhaseLayerFilter : Filter that filters at the broadphase level.
        ///	@param collisionLayerFilter : Filter that filters at the collision layer level.
        ///	@param bodyFilter : Filter that filters at the body level.
        ///	@param shapeFilter : Filter that filters at the shape level.
        //----------------------------------------------------------------------------------------------------
        void                CastShape(const RShapeCast& shapeCast, const ShapeCastSettings& settings, const RVec3 baseOffset, CastShapeCollector& inCollector, const BroadPhaseLayerFilter& broadPhaseLayerFilter = {}, const CollisionLayerFilter& collisionLayerFilter = {}, const BodyFilter& bodyFilter = {}, const ShapeFilter& shapeFilter = {}) const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Collect all leaf transformed shapes that fall inside the world space 'box'.
        //----------------------------------------------------------------------------------------------------
        void                CollectTransformedShapes(const AABox& box, TransformedShapeCollector& inCollector, const BroadPhaseLayerFilter& broadPhaseLayerFilter = {}, const CollisionLayerFilter& collisionLayerFilter = {}, const BodyFilter& bodyFilter = {}, const ShapeFilter& shapeFilter = {}) const;
        
        //----------------------------------------------------------------------------------------------------
        /// @brief : Initialize the interface (should only be called by the PhysicsScene).
        //----------------------------------------------------------------------------------------------------
        void                Internal_Init(BodyLockInterface& bodyLockInterface, BroadPhaseQuery& broadPhaseQuery) { m_pBodyLockInterface = &bodyLockInterface; m_pBroadPhaseQuery = &broadPhaseQuery; }
    
    private:
        BodyLockInterface*  m_pBodyLockInterface = nullptr;
        BroadPhaseQuery*    m_pBroadPhaseQuery = nullptr;
    };
}
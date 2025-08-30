// BroadPhaseQuery.h
#pragma once

#include "BroadPhaseLayer.h"
#include "Nessie/Geometry/AABox.h"
#include "Nessie/Physics/Body/BodyID.h"
#include "Nessie/Physics/Collision/CollisionCollector.h"

namespace nes
{
    struct BroadPhaseCastResult;
    struct RayCast;
    struct OrientedBox;
    struct AABoxCast;
    
    using RayCastBodyCollector = CollisionCollector<BroadPhaseCastResult, CollisionCollectorTraitsCastRay>;
    using CastShapeBodyCollector = CollisionCollector<BroadPhaseCastResult, CollisionCollectorTraitsCastShape>;
    using CollideShapeBodyCollector = CollisionCollector<BodyID, CollisionCollectorTraitsCollideShape>;

    //----------------------------------------------------------------------------------------------------
    /// @brief : Interface to the broadphase that can perform collision queries. These queries will only
    ///     test the bounding box of the Body to quickly determine a potential set of colliding Bodies.
    ///     The shapes of the Bodies are not tested - if you want this then you should use the NarrowPhaseQuery
    ///     interface.
    //----------------------------------------------------------------------------------------------------
    class BroadPhaseQuery
    {
    public:
        virtual ~BroadPhaseQuery() = default;
        BroadPhaseQuery() = default;
        BroadPhaseQuery(const BroadPhaseQuery&) = delete;
        BroadPhaseQuery& operator=(const BroadPhaseQuery&) = delete;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Cast a Ray and add any hits to the collector. 
        ///	@param ray : Ray information.
        ///	@param collector : Collector that will contain the hit information, if any.
        ///	@param broadPhaseLayerFilter : Filter to test which BroadPhaseLayers should interact with the Ray.
        ///	@param collisionLayerFilter : Filter to test which Collision layers are valid for the Ray.
        //----------------------------------------------------------------------------------------------------
        virtual void CastRay(const RayCast& ray, RayCastBodyCollector& collector, const BroadPhaseLayerFilter& broadPhaseLayerFilter = { }, const CollisionLayerFilter& collisionLayerFilter = { }) const = 0;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Cast a AABox and add any hits to the collector.
        ///	@param box : Box to Cast.
        ///	@param collector : Collector that will contain the hit information, if any.
        ///	@param broadPhaseLayerFilter : Filter to test which BroadPhaseLayers should interact with the AABox.
        ///	@param collisionLayerFilter : Filter to test which Collision layers are valid for the AABox.
        //----------------------------------------------------------------------------------------------------
        virtual void CastAABox(const AABoxCast& box, CastShapeBodyCollector& collector, const BroadPhaseLayerFilter& broadPhaseLayerFilter = { }, const CollisionLayerFilter& collisionLayerFilter = { }) const = 0;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the Bodies that intersect with the given AABox and add any hits to the Collector.
        ///	@param box : Box that we are testing.
        ///	@param collector : Collector that will contain the BodyIDs of all Bodies that collide with the box.
        ///	@param broadPhaseLayerFilter : Filter to test which BroadPhaseLayers should interact with the AABox.
        ///	@param collisionLayerFilter : Filter to test which Collision layers are valid for the AABox.
        //----------------------------------------------------------------------------------------------------
        virtual void CollideAABox(const AABox& box, CollideShapeBodyCollector& collector, const BroadPhaseLayerFilter& broadPhaseLayerFilter = { }, const CollisionLayerFilter& collisionLayerFilter = { }) const = 0;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the Bodies that intersect with the given Sphere and add any hits to the Collector.
        ///	@param center : Center of the Sphere.
        ///	@param radius : Radius of the Sphere.
        ///	@param collector : Collector that will contain the BodyIDs of all Bodies that collide with the Sphere.
        ///	@param broadPhaseLayerFilter : Filter to test which BroadPhaseLayers should interact with the Sphere.
        ///	@param collisionLayerFilter : Filter to test which Collision layers are valid for the Sphere.
        //----------------------------------------------------------------------------------------------------
        virtual void CollideSphere(const Vec3& center, const float radius, CollideShapeBodyCollector& collector, const BroadPhaseLayerFilter& broadPhaseLayerFilter = { }, const CollisionLayerFilter& collisionLayerFilter = { }) const = 0;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the Bodies that intersect with the given Point and add any hits to the Collector.
        ///	@param point : Point to test against.
        ///	@param collector : Collector that will contain the BodyIDs of all Bodies that collide with the Point.
        ///	@param broadPhaseLayerFilter : Filter to test which BroadPhaseLayers should interact with the Point.
        ///	@param collisionLayerFilter : Filter to test which Collision layers are valid for the Point.
        //----------------------------------------------------------------------------------------------------
        virtual void CollidePoint(const Vec3& point, CollideShapeBodyCollector& collector, const BroadPhaseLayerFilter& broadPhaseLayerFilter = { }, const CollisionLayerFilter& collisionLayerFilter = { }) const = 0;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the Bodies that intersect with the given Point and add any hits to the Collector. 
        ///	@param box : Oriented Box that we are testing.
        ///	@param collector : Collector that will contain the BodyIDs of all Bodies that collide with the Point.
        ///	@param broadPhaseLayerFilter : Filter to test which BroadPhaseLayers should interact with the Box.
        ///	@param collisionLayerFilter : Filter to test which Collision layers are valid for the Box.
        //----------------------------------------------------------------------------------------------------
        virtual void CollideOrientedBox(const OrientedBox& box, CollideShapeBodyCollector& collector, const BroadPhaseLayerFilter& broadPhaseLayerFilter = { }, const CollisionLayerFilter& collisionLayerFilter = { }) const = 0;
    };
}
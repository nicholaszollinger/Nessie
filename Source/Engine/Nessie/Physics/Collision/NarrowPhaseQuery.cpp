// NarrowPhaseQuery.cpp
#include "NarrowPhaseQuery.h"
#include "Nessie/Physics/Collision/CollisionSolver.h"
#include "Nessie/Physics/Collision/AABoxCast.h"
#include "Nessie/Physics/Collision/RayCast.h"
#include "Nessie/Physics/Collision/ShapeCast.h"
#include "Nessie/Physics/Collision/CollideShape.h"
#include "Nessie/Physics/Collision/CollisionCollector.h"
#include "Nessie/Physics/Collision/CastResult.h"
#include "Nessie/Physics/Collision/InternalEdgeRemovingCollector.h"

namespace nes
{
    bool NarrowPhaseQuery::CastRay(const RRayCast& ray, RayCastResult& hit, const BroadPhaseLayerFilter& broadPhaseLayerFilter, const CollisionLayerFilter& collisionLayerFilter, const BodyFilter& bodyFilter) const
    {
        struct MyCollector : public RayCastBodyCollector
        {
            RRayCast                    m_ray;
            RayCastResult&              m_hit;
            const BodyLockInterface&    m_bodyLockInterface;
            const BodyFilter&           m_bodyFilter;
            
            MyCollector(const RRayCast& ray, RayCastResult& hit, const BodyLockInterface& bodyLockInterface, const BodyFilter& bodyFilter)
                : m_ray(ray)
                , m_hit(hit)
                , m_bodyLockInterface(bodyLockInterface)
                , m_bodyFilter(bodyFilter)
            {
                ResetEarlyOutFraction(hit.m_fraction);
            }

            virtual void AddHit(const ResultType& result) override
            {
                NES_ASSERT(result.m_fraction < m_hit.m_fraction, "This hit should not have been passed on to the collector.");

                // Only test if it passes the body filter.
                if (m_bodyFilter.ShouldCollide(result.m_bodyID))
                {
                    // Lock the Body
                    BodyLockRead lock(m_bodyLockInterface, result.m_bodyID);
                    if (lock.SucceededAndIsInBroadPhase()) // Race condition: Body could have been removed since it has been found in the broadphase, ensures that the body is in the broadphase while we call the callbacks.
                    {
                        const Body& body = lock.GetBody();

                        // Check body filter again now that we've locked the body
                        if (m_bodyFilter.ShouldCollideLocked(body))
                        {
                            // Collect the transformed shape
                            const TransformedShape ts = body.GetTransformedShape();

                            // Release the lock now, we have all the info that we need in the transformed shape.
                            lock.ReleaseLock();

                            // Do the narrow phase collision check.
                            if (ts.CastRay(m_ray, m_hit))
                            {
                                // Test that we didn't find a further hit by accident.
                                NES_ASSERT(m_hit.m_fraction >= 0.f && m_hit.m_fraction < GetEarlyOutFraction());

                                // Update the early out fraction based on the narrow phase collector.
                                UpdateEarlyOutFraction(m_hit.m_fraction);
                            }
                        }
                    }
                }
            }
        };

        // Do the broadphase test; note that the broadphase uses floats so we drop precision here.
        MyCollector collector(ray, hit, *m_pBodyLockInterface, bodyFilter);
        m_pBroadPhaseQuery->CastRay(RayCast(ray), collector, broadPhaseLayerFilter, collisionLayerFilter);
        return hit.m_fraction <= 1.0f;
    }

    void NarrowPhaseQuery::CastRay(const RRayCast& ray, const RayCastSettings& rayCastSettings, CastRayCollector& inCollector, const BroadPhaseLayerFilter& broadPhaseLayerFilter, const CollisionLayerFilter& collisionLayerFilter, const BodyFilter& bodyFilter, const ShapeFilter& shapeFilter) const
    {
        struct MyCollector : public RayCastBodyCollector
        {
            RRayCast                    m_ray;
            RayCastSettings             m_rayCastSettings;
            CastRayCollector&           m_collector;
            const BodyLockInterface&    m_bodyLockInterface;
            const BodyFilter&           m_bodyFilter;
            const ShapeFilter&          m_shapeFilter;
            
            MyCollector(const RRayCast& ray, const RayCastSettings& settings, CastRayCollector& collector, const BodyLockInterface& bodyLockInterface, const BodyFilter& bodyFilter, const ShapeFilter& shapeFilter)
                : m_ray(ray)
                , m_rayCastSettings(settings)
                , m_collector(collector)
                , m_bodyLockInterface(bodyLockInterface)
                , m_bodyFilter(bodyFilter)
                , m_shapeFilter(shapeFilter)
            {
                //
            }

            virtual void AddHit(const ResultType& result) override
            {
                NES_ASSERT(result.m_fraction < m_collector.GetEarlyOutFraction(), "This hit should not have been passed on to the collector.");

                // Only test if it passes the body filter.
                if (m_bodyFilter.ShouldCollide(result.m_bodyID))
                {
                    // Lock the Body
                    BodyLockRead lock(m_bodyLockInterface, result.m_bodyID);
                    if (lock.SucceededAndIsInBroadPhase()) // Race condition: Body could have been removed since it has been found in the broadphase, ensures that the body is in the broadphase while we call the callbacks.
                    {
                        const Body& body = lock.GetBody();

                        // Check body filter again now that we've locked the body
                        if (m_bodyFilter.ShouldCollideLocked(body))
                        {
                            // Collect the transformed shape
                            const TransformedShape ts = body.GetTransformedShape();

                            // Notify collector of new body.
                            m_collector.OnBody(body);
                            
                            // Release the lock now, we have all the info that we need in the transformed shape.
                            lock.ReleaseLock();

                            // Do the narrow phase collision check.
                            ts.CastRay(m_ray, m_rayCastSettings, m_collector, m_shapeFilter);
                            
                            // Notify the collector of the end of this body.
                            // We do this before updating the early out fraction so that the collector can still modify it.
                            m_collector.OnBodyEnd();

                            // Update the early out fraction based on the narrow phase collector.
                            UpdateEarlyOutFraction(m_collector.GetEarlyOutFraction());
                        }
                    }
                }
            }
        };

        // Do the broadphase test; note that the broadphase uses floats, so we drop precision here.
        MyCollector collector(ray, rayCastSettings, inCollector, *m_pBodyLockInterface, bodyFilter, shapeFilter);
        m_pBroadPhaseQuery->CastRay(RayCast(ray), collector, broadPhaseLayerFilter, collisionLayerFilter);
    }

    void NarrowPhaseQuery::CollidePoint(const RVec3 point, CollidePointCollector& inCollector, const BroadPhaseLayerFilter& broadPhaseLayerFilter, const CollisionLayerFilter& collisionLayerFilter, const BodyFilter& bodyFilter, const ShapeFilter& shapeFilter) const
    {
        struct MyCollector : public CollideShapeBodyCollector
        {
            RVec3                       m_point;
            CollidePointCollector&      m_collector;
            const BodyLockInterface&    m_bodyLockInterface;
            const BodyFilter&           m_bodyFilter;
            const ShapeFilter&          m_shapeFilter;
            
            MyCollector(const RVec3 point, CollidePointCollector& collector, const BodyLockInterface& bodyLockInterface, const BodyFilter& bodyFilter, const ShapeFilter& shapeFilter)
                : CollideShapeBodyCollector(collector)
                , m_point(point)
                , m_collector(collector)
                , m_bodyLockInterface(bodyLockInterface)
                , m_bodyFilter(bodyFilter)
                , m_shapeFilter(shapeFilter)
            {
                //
            }

            virtual void AddHit(const ResultType& result) override
            {
                // Only test if it passes the body filter.
                if (m_bodyFilter.ShouldCollide(result))
                {
                    // Lock the Body
                    BodyLockRead lock(m_bodyLockInterface, result);
                    if (lock.SucceededAndIsInBroadPhase()) // Race condition: Body could have been removed since it has been found in the broadphase, ensures that the body is in the broadphase while we call the callbacks.
                    {
                        const Body& body = lock.GetBody();

                        // Check body filter again now that we've locked the body
                        if (m_bodyFilter.ShouldCollideLocked(body))
                        {
                            // Collect the transformed shape
                            const TransformedShape ts = body.GetTransformedShape();

                            // Notify collector of new body.
                            m_collector.OnBody(body);
                            
                            // Release the lock now, we have all the info that we need in the transformed shape.
                            lock.ReleaseLock();

                            // Do the narrow phase collision check.
                            ts.CollidePoint(m_point, m_collector, m_shapeFilter);
                            
                            // Notify the collector of the end of this body.
                            // We do this before updating the early out fraction so that the collector can still modify it.
                            m_collector.OnBodyEnd();

                            // Update the early out fraction based on the narrow phase collector.
                            UpdateEarlyOutFraction(m_collector.GetEarlyOutFraction());
                        }
                    }
                }
            }
        };

        // Do the broadphase test; note that the broadphase uses floats, so we drop precision here.
        MyCollector collector(point, inCollector, *m_pBodyLockInterface, bodyFilter, shapeFilter);
        m_pBroadPhaseQuery->CollidePoint(Vec3(point), collector, broadPhaseLayerFilter, collisionLayerFilter);
    }

    void NarrowPhaseQuery::CollideShape(const Shape* pShape, const Vec3 shapeScale, const Mat44& centerOfMassTransform, const CollideShapeSettings& collideShapeSettings, const RVec3 baseOffset, CollideShapeCollector& inCollector, const BroadPhaseLayerFilter& broadPhaseLayerFilter, const CollisionLayerFilter& collisionLayerFilter, const BodyFilter& bodyFilter, const ShapeFilter& shapeFilter) const
    {
        struct MyCollector : public CollideShapeBodyCollector
        {
            const Shape*                m_pShape;
            Vec3                        m_shapeScale;
            Mat44                       m_centerOfMassTransform; // RMat44
            const CollideShapeSettings& m_collideShapeSettings;
            RVec3                       m_baseOffset;
            CollideShapeCollector&      m_collector;
            const BodyLockInterface&    m_bodyLockInterface;
            const BodyFilter&           m_bodyFilter;
            const ShapeFilter&          m_shapeFilter;

            MyCollector(const Shape* pShape, const Vec3 shapeScale, const Mat44& centerOfMassTransform, const CollideShapeSettings& settings, const RVec3 baseOffset, CollideShapeCollector& collector, const BodyLockInterface& bodyLockInterface, const BodyFilter& bodyFilter, const ShapeFilter& shapeFilter)
                : CollideShapeBodyCollector(collector)
                , m_pShape(pShape)
                , m_shapeScale(shapeScale)
                , m_centerOfMassTransform(centerOfMassTransform)
                , m_collideShapeSettings(settings)
                , m_baseOffset(baseOffset)
                , m_collector(collector)
                , m_bodyLockInterface(bodyLockInterface)
                , m_bodyFilter(bodyFilter)
                , m_shapeFilter(shapeFilter)
            {
                //
            }

            virtual void AddHit(const ResultType& result) override
            {
                // Only test if it passes the body filter.
                if (m_bodyFilter.ShouldCollide(result))
                {
                    // Lock the Body
                    BodyLockRead lock(m_bodyLockInterface, result);
                    if (lock.SucceededAndIsInBroadPhase()) // Race condition: Body could have been removed since it has been found in the broadphase, ensures that the body is in the broadphase while we call the callbacks.
                    {
                        const Body& body = lock.GetBody();

                        // Check body filter again now that we've locked the body
                        if (m_bodyFilter.ShouldCollideLocked(body))
                        {
                            // Collect the transformed shape
                            const TransformedShape ts = body.GetTransformedShape();

                            // Notify collector of new body.
                            m_collector.OnBody(body);
                            
                            // Release the lock now, we have all the info that we need in the transformed shape.
                            lock.ReleaseLock();

                            // Do the narrow phase collision check.
                            ts.CollideShape(m_pShape, m_shapeScale, m_centerOfMassTransform, m_collideShapeSettings, m_baseOffset, m_collector, m_shapeFilter);
                            
                            // Notify the collector of the end of this body.
                            // We do this before updating the early out fraction so that the collector can still modify it.
                            m_collector.OnBodyEnd();

                            // Update the early out fraction based on the narrow phase collector.
                            UpdateEarlyOutFraction(m_collector.GetEarlyOutFraction());
                        }
                    }
                }
            }
        };

        // Calculate bounds for the shape and expand by the max separation distance
        AABox bounds = pShape->GetWorldBounds(centerOfMassTransform, shapeScale);
        bounds.ExpandBy(Vec3::Replicate(collideShapeSettings.m_maxSeparationDistance));

        // Do broadphase test
        MyCollector collector(pShape, shapeScale, centerOfMassTransform, collideShapeSettings, baseOffset, inCollector, *m_pBodyLockInterface, bodyFilter, shapeFilter);
        m_pBroadPhaseQuery->CollideAABox(bounds, collector, broadPhaseLayerFilter, collisionLayerFilter);
    }
    
    void NarrowPhaseQuery::CollideShapeWithInternalEdgeRemoval(const Shape* pShape, const Vec3 shapeScale, const Mat44& centerOfMassTransform, const CollideShapeSettings& collideShapeSettings, const RVec3 baseOffset, CollideShapeCollector& collector, const BroadPhaseLayerFilter& broadPhaseLayerFilter, const CollisionLayerFilter& collisionLayerFilter, const BodyFilter& bodyFilter, const ShapeFilter& shapeFilter) const
    {
        // We require these settings to internal-edge removal to work.
        CollideShapeSettings settings = collideShapeSettings;
        settings.m_activeEdgeMode = EActiveEdgeMode::CollideWithAll;
        settings.m_collectFacesMode = ECollectFacesMode::CollectFaces;

        InternalEdgeRemovingCollector wrapper(collector);
        CollideShape(pShape, shapeScale, centerOfMassTransform, settings, baseOffset, wrapper, broadPhaseLayerFilter, collisionLayerFilter, bodyFilter, shapeFilter);
    }

    void NarrowPhaseQuery::CastShape(const RShapeCast& shapeCast, const ShapeCastSettings& settings, const RVec3 baseOffset, CastShapeCollector& inCollector, const BroadPhaseLayerFilter& broadPhaseLayerFilter, const CollisionLayerFilter& collisionLayerFilter, const BodyFilter& bodyFilter, const ShapeFilter& shapeFilter) const
    {
        struct MyCollector : public CastShapeBodyCollector
        {
            RShapeCast                  m_shapeCast;
            const ShapeCastSettings&    m_shapeCastSettings;
            RVec3                       m_baseOffset;
            CastShapeCollector&         m_collector;
            const BodyLockInterface&    m_bodyLockInterface;
            const BodyFilter&           m_bodyFilter;
            const ShapeFilter&          m_shapeFilter;

            MyCollector(const RShapeCast& shapeCast, const ShapeCastSettings& settings, const RVec3 baseOffset, CastShapeCollector& collector, const BodyLockInterface& bodyLockInterface, const BodyFilter& bodyFilter, const ShapeFilter& shapeFilter)
                : CastShapeBodyCollector(collector)
                , m_shapeCast(shapeCast)
                , m_shapeCastSettings(settings)
                , m_baseOffset(baseOffset)
                , m_collector(collector)
                , m_bodyLockInterface(bodyLockInterface)
                , m_bodyFilter(bodyFilter)
                , m_shapeFilter(shapeFilter)
            {
                //
            }

            virtual void AddHit(const ResultType& result) override
            {
                NES_ASSERT(result.m_fraction <= math::Max(0.f, m_collector.GetEarlyOutFraction()), "This hit should not have been passed on to the collector.");

                // Only test if it passes the body filter.
                if (m_bodyFilter.ShouldCollide(result.m_bodyID))
                {
                    // Lock the Body
                    BodyLockRead lock(m_bodyLockInterface, result.m_bodyID);
                    if (lock.SucceededAndIsInBroadPhase()) // Race condition: Body could have been removed since it has been found in the broadphase, ensures that the body is in the broadphase while we call the callbacks.
                    {
                        const Body& body = lock.GetBody();

                        // Check body filter again now that we've locked the body
                        if (m_bodyFilter.ShouldCollideLocked(body))
                        {
                            // Collect the transformed shape
                            const TransformedShape ts = body.GetTransformedShape();

                            // Notify collector of new body.
                            m_collector.OnBody(body);
                            
                            // Release the lock now, we have all the info that we need in the transformed shape.
                            lock.ReleaseLock();

                            // Do the narrow phase collision check.
                            ts.CastShape(m_shapeCast, m_shapeCastSettings, m_baseOffset, m_collector, m_shapeFilter);
                            
                            // Notify the collector of the end of this body.
                            // We do this before updating the early out fraction so that the collector can still modify it.
                            m_collector.OnBodyEnd();

                            // Update the early out fraction based on the narrow phase collector.
                            UpdateEarlyOutFraction(m_collector.GetEarlyOutFraction());
                        }
                    }
                }
            }
        };

        // Do the broadphase test
        MyCollector collector(shapeCast, settings, baseOffset, inCollector, *m_pBodyLockInterface, bodyFilter, shapeFilter);
        m_pBroadPhaseQuery->CastAABox({ shapeCast.m_shapeWorldBounds, shapeCast.m_direction}, collector, broadPhaseLayerFilter, collisionLayerFilter);
    }

    void NarrowPhaseQuery::CollectTransformedShapes(const AABox& box, TransformedShapeCollector& inCollector, const BroadPhaseLayerFilter& broadPhaseLayerFilter, const CollisionLayerFilter& collisionLayerFilter, const BodyFilter& bodyFilter, const ShapeFilter& shapeFilter) const
    {
        struct MyCollector : public CollideShapeBodyCollector
        {
            const AABox&                m_box;
            TransformedShapeCollector&  m_collector;
            const BodyLockInterface&    m_bodyLockInterface;
            const BodyFilter&           m_bodyFilter;
            const ShapeFilter&          m_shapeFilter;

            MyCollector(const AABox& box, TransformedShapeCollector& collector, const BodyLockInterface& bodyLockInterface, const BodyFilter& bodyFilter, const ShapeFilter& shapeFilter)
                : CollideShapeBodyCollector(collector)
                , m_box(box)
                , m_collector(collector)
                , m_bodyLockInterface(bodyLockInterface)
                , m_bodyFilter(bodyFilter)
                , m_shapeFilter(shapeFilter)
            {
                //
            }

            virtual void AddHit(const ResultType& result) override
            {
                // Only test if it passes the body filter.
                if (m_bodyFilter.ShouldCollide(result))
                {
                    // Lock the Body
                    BodyLockRead lock(m_bodyLockInterface, result);
                    if (lock.SucceededAndIsInBroadPhase()) // Race condition: Body could have been removed since it has been found in the broadphase, ensures that the body is in the broadphase while we call the callbacks.
                    {
                        const Body& body = lock.GetBody();

                        // Check body filter again now that we've locked the body
                        if (m_bodyFilter.ShouldCollideLocked(body))
                        {
                            // Collect the transformed shape
                            const TransformedShape ts = body.GetTransformedShape();

                            // Notify collector of new body.
                            m_collector.OnBody(body);
                            
                            // Release the lock now, we have all the info that we need in the transformed shape.
                            lock.ReleaseLock();

                            // Do the narrow phase collision check.
                            ts.CollectTransformedShapes(m_box, m_collector, m_shapeFilter);
                            
                            // Notify the collector of the end of this body.
                            // We do this before updating the early out fraction so that the collector can still modify it.
                            m_collector.OnBodyEnd();

                            // Update the early out fraction based on the narrow phase collector.
                            UpdateEarlyOutFraction(m_collector.GetEarlyOutFraction());
                        }
                    }
                }
            }
        };

        // Do the broadphase test
        MyCollector collector(box, inCollector, *m_pBodyLockInterface, bodyFilter, shapeFilter);
        m_pBroadPhaseQuery->CollideAABox(box, collector, broadPhaseLayerFilter, collisionLayerFilter);
    }
}

// CollisionSolver.cpp
#include "CollisionSolver.h"

namespace nes
{
    CollisionSolver::CollideShape CollisionSolver::s_collideShapeFunctions[kNumSubShapeTypes][kNumSubShapeTypes];
    CollisionSolver::CastShape CollisionSolver::s_castShapeFunctions[kNumSubShapeTypes][kNumSubShapeTypes];
    
    void CollisionSolver::Internal_Init()
    {
        // Set each non-set shape pair with an invalid collision/cast function.
        for (size_t i = 0; i < kNumSubShapeTypes; ++i)
        {
            for (size_t j = 0; j < kNumSubShapeTypes; ++j)
            {
                if (s_collideShapeFunctions[i][j] == nullptr)
                {
                    s_collideShapeFunctions[i][j] = [](const Shape*, const Shape*, const Vector3&, const Vector3&, const Mat4&, const Mat4&, const SubShapeIDCreator&, const SubShapeIDCreator&, const CollideShapeSettings&, CollideShapeCollector&, const ShapeFilter&)
                    {
                        NES_ASSERTV(false, "Attempted to collide unsupported shape pair!");  
                    };
                }

                if (s_castShapeFunctions[i][j] == nullptr)
                {
                    s_castShapeFunctions[i][j] = [](const ShapeCast&, const ShapeCastSettings&, const Shape*, const Vector3&, const ShapeFilter&, const Mat4&, const SubShapeIDCreator&, const SubShapeIDCreator&, CastShapeCollector&)
                    {
                        NES_ASSERTV(false, "Attempted to cast unsupported shape pair!");  
                    };
                }
            }
        }
    }
    
    void CollisionSolver::RegisterCollideShape(const ShapeSubType type1, const ShapeSubType type2, const CollideShape& function)
    {
        s_collideShapeFunctions[static_cast<int>(type1)][static_cast<int>(type2)] = function; 
    }

    void CollisionSolver::RegisterCastShape(const ShapeSubType type1, const ShapeSubType type2, const CastShape& function)
    {
        s_castShapeFunctions[static_cast<int>(type1)][static_cast<int>(type2)] = function;
    }

    void CollisionSolver::ReversedCollideShape(const Shape* pShape1, const Shape* pShape2, const Vector3& scale1,
        const Vector3& scale2, const Mat4& centerOfMassTransform1, const Mat4& centerOfMassTransform2,
        const SubShapeIDCreator& subShapeIDCreator1, const SubShapeIDCreator& subShapeIDCreator2,
        const CollideShapeSettings& collideShapeSettings, CollideShapeCollector& collector,
        const ShapeFilter& shapeFilter)
    {
        class ReversedCollector : public CollideShapeCollector
        {
            CollideShapeCollector& m_collector;
        public:
            explicit ReversedCollector(CollideShapeCollector& collector)
                : CollideShapeCollector(collector)
                , m_collector(collector)
            {
                //
            }

            virtual void AddHit(const CollideShapeResult& result) override
            {
                // Add the reversed hit
                m_collector.AddHit(result.Reversed());

                // If our chained collector updated its early out fraction, we need to follow
                UpdateEarlyOutFraction(m_collector.GetEarlyOutFraction());
            }
        };

        ReverseShapeFilter reversedShapeFilter(shapeFilter);
        ReversedCollector reversedCollector(collector);
        CollideShapeVsShape(pShape2, pShape1, scale2, scale1, centerOfMassTransform2, centerOfMassTransform1, subShapeIDCreator2, subShapeIDCreator1, collideShapeSettings, reversedCollector, reversedShapeFilter);
    }

    void CollisionSolver::ReversedCastShape(const ShapeCast& shapeCast, const ShapeCastSettings& shapeCastSettings,
        const Shape* pShape, const Vector3& scale, const ShapeFilter& shapeFilter, const Mat4& centerOfMassTransform2,
        const SubShapeIDCreator& subShapeIDCreator1, const SubShapeIDCreator& subShapeIDCreator2,
        CastShapeCollector& collector)
    {
        class ReversedCollector : public CastShapeCollector
        {
            CastShapeCollector& m_collector;
            Vector3             m_worldDirection;
        public:
            explicit ReversedCollector(CastShapeCollector& collector, const Vector3& worldDirection)
                : CastShapeCollector(collector)
                , m_collector(collector)
                , m_worldDirection(worldDirection)
            {
                //
            }

            virtual void AddHit(const ShapeCastResult& result) override
            {
                // Add the reversed hit
                m_collector.AddHit(result.Reversed(m_worldDirection));

                // If our chained collector updated its early out fraction, we need to follow
                UpdateEarlyOutFraction(m_collector.GetEarlyOutFraction());
            }
        };

        // Reverse the Shape Cast (shape cast is in local space to shape 2).
        Mat4 comStartInverse = shapeCast.m_centerOfMassStart.InverseRotationTranslation();
        ShapeCast localShapeCast(pShape, scale, comStartInverse, -comStartInverse.TransformVector(shapeCast.m_direction));

        // Calculate the center of mass of Shape 1 at the start of the sweep
        const Mat4 shape1COM = centerOfMassTransform2 * shapeCast.m_centerOfMassStart;

        // Calculate the world space direction vector of the shape cast
        const Vector3 worldDirection = -centerOfMassTransform2.TransformVector(shapeCast.m_direction);

        // Forward the Cast
        ReverseShapeFilter reversedShapeFilter(shapeFilter);
        ReversedCollector reversedCollector(collector, worldDirection);
        CastShapeVsShapeLocalSpace(localShapeCast, shapeCastSettings, shapeCast.m_pShape, shapeCast.m_scale, reversedShapeFilter, shape1COM, subShapeIDCreator2, subShapeIDCreator1, reversedCollector);
    }
}

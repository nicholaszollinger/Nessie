// CollisionSolver.h
#pragma once

#include "ShapeCast.h"
#include "ShapeFilter.h"
#include "Shapes/Shape.h"
#include "Shapes/SubShapeID.h"

namespace nes
{
    struct CollideShapeSettings;

    //----------------------------------------------------------------------------------------------------
    /// @brief : The CollisionSolver class acts as the central hub for solving collisions between different
    ///     shape subtypes.
    //----------------------------------------------------------------------------------------------------
    class CollisionSolver
    {
    public:
        /// Function type that collides 2 shapes (see CollideShapeVsShape)
        using CollideShape = void (*)(const Shape* pShape1, const Shape* pShape2, const Vector3& scale1, const Vector3& scale2, const Mat4& centerOfMassTransform1, const Mat4& centerOfMassTransform2, const SubShapeIDCreator& subShapeIDCreator1, const SubShapeIDCreator& subShapeIDCreator2, const CollideShapeSettings& collideShapeSettings, CollideShapeCollector& collector, const ShapeFilter& shapeFilter);

        /// Function type that casts a shape vs another shape (see CastShapeVsShapeLocalSpace)
        using CastShape = void (*)(const ShapeCast& shapeCast, const ShapeCastSettings& shapeCastSettings, const Shape* pShape, const Vector3& scale, const ShapeFilter& shapeFilter, const Mat4& centerOfMassTransform2, const SubShapeIDCreator& subShapeIDCreator1, const SubShapeIDCreator& subShapeIDCreator2, CastShapeCollector& collector);

    private:
        static CollideShape s_collideShapeFunctions[kNumSubShapeTypes][kNumSubShapeTypes];
        static CastShape    s_castShapeFunctions[kNumSubShapeTypes][kNumSubShapeTypes];

    public:
        //----------------------------------------------------------------------------------------------------
        /// @brief : Collide two shapes and pass any collisions to the 'collector'.
        ///	@param pShape1 : The first shape.
        ///	@param pShape2 : The second shape.
        ///	@param scale1 : Local space scale of shape 1 (scales relative to its center of mass).
        ///	@param scale2 : Local space scale of shape 2 (scales relative to its center of mass).
        ///	@param centerOfMassTransform1 : Transform to transform center of mass of shape 1 into world space.
        ///	@param centerOfMassTransform2 : Transform to transform center of mass of shape 2 into world space.
        ///	@param subShapeIDCreator1 : Class that tracks the current sub shape ID for shape 1.
        ///	@param subShapeIDCreator2 : Class that tracks the current sub shape ID for shape 2.
        ///	@param collideShapeSettings : Options for the CollideShape test.
        ///	@param collector : The collector that receives the results.
        ///	@param shapeFilter : Allows selectively disabling collisions between pairs of (sub) shapes.
        //----------------------------------------------------------------------------------------------------
        static inline void CollideShapeVsShape(const Shape* pShape1, const Shape* pShape2, const Vector3& scale1, const Vector3& scale2, const Mat4& centerOfMassTransform1, const Mat4& centerOfMassTransform2, const SubShapeIDCreator& subShapeIDCreator1, const SubShapeIDCreator& subShapeIDCreator2, const CollideShapeSettings& collideShapeSettings, CollideShapeCollector& collector, const ShapeFilter& shapeFilter = { })
        {
            // [TODO]: Track Narrow phase stats
            if (shapeFilter.ShouldCollide(pShape1, subShapeIDCreator1.GetID(), pShape2, subShapeIDCreator2.GetID()))
            {
                const auto& function = s_collideShapeFunctions[static_cast<int>(pShape1->GetSubType())][static_cast<int>(pShape2->GetSubType())];
                function(pShape1, pShape2, scale1, scale2, centerOfMassTransform1, centerOfMassTransform2, subShapeIDCreator1, subShapeIDCreator2, collideShapeSettings, collector, shapeFilter);
            }
        }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Cast a shape against this shape, passes any hits found to ioCollector.
        /// @note : This version takes the shape cast in local space relative to the center of mass of pShape,
        ///     take a look at CastShapeVsShapeWorldSpace() if you have a shape cast in world space.
        ///	@param shapeCastLocal : The shape to cast against the other shape and its start and direction.
        ///	@param shapeCastSettings : Settings for performing the cast.
        ///	@param pShape : The shape to cast against.
        ///	@param scale : Local space scale for the shape to cast against (scales relative to its center of mass).
        ///	@param shapeFilter : Allows selectively disabling collisions between pairs of (sub) shapes.
        ///	@param centerOfMassTransform2 : Is the center of mass transform of shape 2 (excluding scale), this
        ///     is used to provide a transform to the shape cast result so that local hit result quantities can
        ///     be transformed into world space.
        ///	@param subShapeIDCreator1 : Class that tracks the current sub shape ID for the casting shape.
        ///	@param subShapeIDCreator2 : Class that tracks the current sub shape ID for the shape we're casting against.
        ///	@param collector : The collector that receives the results.
        //----------------------------------------------------------------------------------------------------
        static inline void CastShapeVsShapeLocalSpace(const ShapeCast& shapeCastLocal, const ShapeCastSettings& shapeCastSettings, const Shape* pShape, const Vector3& scale, const ShapeFilter& shapeFilter, const Mat4& centerOfMassTransform2, const SubShapeIDCreator& subShapeIDCreator1, const SubShapeIDCreator& subShapeIDCreator2, CastShapeCollector& collector)
        {
            // [TODO]: Track Narrow phase stats
            if (shapeFilter.ShouldCollide(shapeCastLocal.m_pShape, subShapeIDCreator1.GetID(), pShape, subShapeIDCreator2.GetID()))
            {
                const auto& function = s_castShapeFunctions[static_cast<int>(shapeCastLocal.m_pShape->GetSubType())][static_cast<int>(pShape->GetSubType())];
                function(shapeCastLocal, shapeCastSettings, pShape, scale, shapeFilter, centerOfMassTransform2, subShapeIDCreator1, subShapeIDCreator2, collector);
            }
        }

        //----------------------------------------------------------------------------------------------------
        /// @brief : See CastShapeVsShapeLocalSpace() for details. The only difference is that the shape cast
        ///     (shapeCastWorld) is provided in world space.
        /// @note: A shape cast contains the center of mass start of the shape, if you have the world transform
        ///     of the shape you probably want to construct it using ShapeCast::FromWorldTransform(). 
        //----------------------------------------------------------------------------------------------------
        static inline void CastShapeVsShapeWorldSpace(const ShapeCast& shapeCastWorld, const ShapeCastSettings& shapeCastSettings, const Shape* pShape, const Vector3& scale, const ShapeFilter& shapeFilter, const Mat4& centerOfMassTransform2, const SubShapeIDCreator& subShapeIDCreator1, const SubShapeIDCreator& subShapeIDCreator2, CastShapeCollector& collector)
        {
            const ShapeCast localShapeCast = shapeCastWorld.PostTransformed(centerOfMassTransform2.InverseRotationTranslation());
            CastShapeVsShapeLocalSpace(localShapeCast, shapeCastSettings, pShape, scale, shapeFilter, centerOfMassTransform2, subShapeIDCreator1, subShapeIDCreator2, collector);
        }
        
        //----------------------------------------------------------------------------------------------------
        /// @brief : Register a collide shape function in the collision table.
        //----------------------------------------------------------------------------------------------------
        static void RegisterCollideShape(const ShapeSubType type1, const ShapeSubType type2, const CollideShape& function);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Register a cast shape function in the collision table.
        //----------------------------------------------------------------------------------------------------
        static void RegisterCastShape(const ShapeSubType type1, const ShapeSubType type2, const CastShape& function);

        //----------------------------------------------------------------------------------------------------
        /// @brief : An implementation of CollideShape that swaps pShape1 and pShape 2 and swaps the results
        ///     back, can be registered if the collision function only exists the other way around.
        //----------------------------------------------------------------------------------------------------
        static void ReversedCollideShape(const Shape* pShape1, const Shape* pShape2, const Vector3& scale1, const Vector3& scale2, const Mat4& centerOfMassTransform1, const Mat4& centerOfMassTransform2, const SubShapeIDCreator& subShapeIDCreator1, const SubShapeIDCreator& subShapeIDCreator2, const CollideShapeSettings& collideShapeSettings, CollideShapeCollector& collector, const ShapeFilter& shapeFilter);

        //----------------------------------------------------------------------------------------------------
        /// @brief : An implementation of CastShape that swaps pShape1 and pShape 2 and swaps the results
        ///     back, can be registered if the collision function only exists the other way around.
        //----------------------------------------------------------------------------------------------------
        static void ReversedCastShape(const ShapeCast& shapeCast, const ShapeCastSettings& shapeCastSettings, const Shape* pShape, const Vector3& scale, const ShapeFilter& shapeFilter, const Mat4& centerOfMassTransform2, const SubShapeIDCreator& subShapeIDCreator1, const SubShapeIDCreator& subShapeIDCreator2, CastShapeCollector& collector);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Initialize all collision functions with a function that asserts and returns no collision. 
        //----------------------------------------------------------------------------------------------------
        static void Internal_Init();
    };
}
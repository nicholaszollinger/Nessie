// ShapeFilter.h
#pragma once
#include "Physics/Body/BodyID.h"

namespace nes
{
    class Shape;
    class SubShapeID;

    class ShapeFilter
    {
    public:
        /// Used during NarrowPhase queries and TransformedShape queries. Set to the body ID of shape2 before calling
        /// ShouldCollide(). Provides context to the filter to indicate which body is colliding.
        mutable BodyID m_bodyID2;
        
    public:
        ShapeFilter() = default;
        ShapeFilter(const ShapeFilter&) = delete;
        ShapeFilter& operator=(const ShapeFilter&) = delete;
        virtual ~ShapeFilter() = default;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Filter function to determine if we should collide with a shape. Returns true if the filter
        ///     passes. This overload is called when the query doesn't have a source shape (e.g. ray cast / collide point).
        ///	@param pShape2 : Shape we're colliding against.
        ///	@param subShapeIDOfShape2 : The sub shape ID that will lead from the root shape to pShape2 (i.e. the shape of
        ///     m_bodyID2).
        //----------------------------------------------------------------------------------------------------
        virtual bool ShouldCollide([[maybe_unused]] const Shape* pShape2, [[maybe_unused]] const SubShapeID& subShapeIDOfShape2) const { return true; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Filter function to determine if two shapes should collide. Returns true if the filter passes.
        ///     This overload is called when querying a shape vs a shape (e.g. collide object / cast object).
        ///     It is called at each level of the shape hierarchy, so if you have a compound shape with a box, this function
        ///     will be called twice.
        ///     It will not be called on triangles that are part of another shape, i.e. a mesh shape will not trigger a
        ///     callback per triangle. You can filter out individual triangles in the Collision Collector::AddHit() function
        ///     by their sub shape ID.
        ///	@param pShape1 : 1st shape that is colliding.
        ///	@param subShapeIDOfShape1 : The sub shape ID that will lead from the root shape to pShape1 (i.e. the shape that is used to collide or cast against shape 2)
        ///	@param pShape2 : 2nd shape that is colliding.
        ///	@param subShapeIDOfShape2 : The sub shape ID that will lead from the root shape to pShape2 (i.e. the shape of m_bodyID2).
        //----------------------------------------------------------------------------------------------------
        virtual bool ShouldCollide([[maybe_unused]] const Shape* pShape1, [[maybe_unused]] const SubShapeID& subShapeIDOfShape1, [[maybe_unused]] const Shape* pShape2, [[maybe_unused]] const SubShapeID& subShapeIDOfShape2) const { return true; }
    };

    //----------------------------------------------------------------------------------------------------
    /// @brief : Helper class to reverse the order of the shapes in the ShouldCollide function 
    //----------------------------------------------------------------------------------------------------
    class ReverseShapeFilter : public ShapeFilter
    {
        const ShapeFilter& m_filter; 
        
    public:
        explicit ReverseShapeFilter(const ShapeFilter& filter) : m_filter(filter) { m_bodyID2 = filter.m_bodyID2; }

        virtual bool ShouldCollide(const Shape* pShape2, const SubShapeID& subShapeIDOfShape2) const override
        {
            return m_filter.ShouldCollide(pShape2, subShapeIDOfShape2);
        }

        virtual bool ShouldCollide(const Shape* pShape1, const SubShapeID& subShapeIDOfShape1, const Shape* pShape2, const SubShapeID& subShapeIDOfShape2) const override
        {
            // Reverse the argument order.
            return m_filter.ShouldCollide(pShape2, subShapeIDOfShape2, pShape1, subShapeIDOfShape1);
        }
    };
}

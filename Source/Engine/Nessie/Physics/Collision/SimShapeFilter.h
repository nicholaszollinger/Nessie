// SimShapeFilter.h
#pragma once

namespace nes
{
    class Body;
    class Shape;
    class SubShapeID;

    //----------------------------------------------------------------------------------------------------
    /// @brief : Filter class used during the simulation (PhysicsScene::Update) to filter out collisions
    ///     at the shape level.
    //----------------------------------------------------------------------------------------------------
    class SimShapeFilter
    {
    public:
        SimShapeFilter() = default;
        SimShapeFilter(const SimShapeFilter&) = delete;
        SimShapeFilter(SimShapeFilter&&) noexcept = delete;
        SimShapeFilter& operator=(const SimShapeFilter&) = delete;
        SimShapeFilter& operator=(SimShapeFilter&&) noexcept = delete;
        virtual ~SimShapeFilter() = default;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Filter function to determine if two shapes should collide. Returns true if the filter passes.
        ///     This overload is called during the simulation (PhysicsScene::Update) and must be registered with
        ///     PhysicsScene::SetSimShapeFilter(). It is called at each level of the shape hierarchy, so if you
        ///     have a compound shape with a box, this function will be called twice.
        ///     It will not be called on triangles that are part of another shape, i.e., a mesh shape will not
        ///     trigger a callback per triangle.
        /// @note : This function is called from multiple threads and must be thread safe. All properties are read-only.
        ///	@param body1 : First body that is colliding.
        ///	@param pShape1 : First shape that is colliding.
        ///	@param subShapeIDOfShape1 : The sub shape ID that will lead from body1.GetShape() to pShape1.
        ///	@param body2 : Second body that is colliding.
        ///	@param pShape2 : Second shape that is colliding.
        ///	@param subShapeIDOfShape2 : The sub shape ID that will lead from body2.GetShape() to pShape2.
        //----------------------------------------------------------------------------------------------------
        virtual bool ShouldCollide([[maybe_unused]] const Body& body1, [[maybe_unused]] const Shape* pShape1, [[maybe_unused]] const SubShapeID& subShapeIDOfShape1,
                                   [[maybe_unused]] const Body& body2, [[maybe_unused]] const Shape* pShape2, [[maybe_unused]] const SubShapeID& subShapeIDOfShape2) const
        {
            return true;
        }
    };
}
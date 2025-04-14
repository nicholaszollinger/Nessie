// BroadPhaseLayer.h
#pragma once
#include <compare>
#include <cstdint>

#include "Physics/Collision/CollisionLayer.h"

namespace nes
{
    //----------------------------------------------------------------------------------------------------
    //	NOTES:
    //  This was made as a wrapper over a value in order to not get confused with the CollisionLayer type.
    //		
    /// @brief : A Collision Layer can be mapped to a broadphase layer. Bodies with the same Broadphase layer
    ///     will end up in the same sub structure (a tree) of the broadphase. When there are many layers, this
    ///     reduces the total amount of substructures the broadphase needs to manage. Usually you want to put
    ///     Bodies that don't collide with each other in different broadphase layers, but there could be
    ///     exceptions if collision layers only contain a minor amount of Bodies so it is not beneficial
    ///     to give each layer its own substructure in the broadphase.
    //----------------------------------------------------------------------------------------------------
    class BroadPhaseLayer
    {
    public:
        using Type = uint8_t;
        
    private:
        Type m_value{};
        
    public:
        constexpr BroadPhaseLayer() = default;
        explicit constexpr BroadPhaseLayer(const uint8_t value) : m_value(value) {}
        constexpr BroadPhaseLayer(const BroadPhaseLayer&) = default;
        BroadPhaseLayer& operator=(const BroadPhaseLayer&) = default;

        constexpr auto operator<=>(const BroadPhaseLayer& other) const = default;
        explicit constexpr operator Type() const { return m_value; }

        [[nodiscard]] Type GetValue() const { return m_value; }
    };

    static constexpr BroadPhaseLayer kInvalidBroadPhaseLayer(0xff);

    // class BroadPhaseLayerManager
    // {
    //       
    // };

    //----------------------------------------------------------------------------------------------------
    /// @brief : Base class used to test if a Body can collide with a Broadphase Layer. Use when finding
    ///     collision pairs of Bodies.
    //----------------------------------------------------------------------------------------------------
    class CollisionVsBroadPhaseLayerFilter
    {
    public:
        virtual ~CollisionVsBroadPhaseLayerFilter() = default;

        virtual bool ShouldCollide([[maybe_unused]] const CollisionLayer collisionLayer, [[maybe_unused]] const BroadPhaseLayer broadPhaseLayer) { return true; }
    };

    //----------------------------------------------------------------------------------------------------
    //	NOTES:
    //		
    /// @brief : Filter class for determining if two Broadphase layers should 
    //----------------------------------------------------------------------------------------------------
    class BroadPhaseLayerFilter
    {
    public:
        BroadPhaseLayerFilter(const BroadPhaseLayerFilter&) = delete;
        BroadPhaseLayerFilter& operator=(const BroadPhaseLayerFilter&) = delete;
        virtual ~BroadPhaseLayerFilter() = default;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Function to filter out Broadphase Layers when doing collision query test. Return tru to allow
        ///     testing against Bodies with this Layer.
        //----------------------------------------------------------------------------------------------------
        virtual bool ShouldCollide([[maybe_unused]] const BroadPhaseLayer& inLayer) { return true; }
    };
}

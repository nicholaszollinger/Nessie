// BroadPhaseLayer.h
#pragma once
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

        constexpr bool operator==(const BroadPhaseLayer& other) const { return m_value == other.m_value; }
        constexpr bool operator!=(const BroadPhaseLayer& other) const { return m_value != other.m_value; }
        constexpr bool operator< (const BroadPhaseLayer& other) const { return m_value <  other.m_value; }
        constexpr bool operator> (const BroadPhaseLayer& other) const { return m_value >  other.m_value; }
        constexpr bool operator<=(const BroadPhaseLayer& other) const { return m_value <= other.m_value; }
        constexpr bool operator>=(const BroadPhaseLayer& other) const { return m_value >= other.m_value; }
        
        explicit constexpr operator Type() const { return m_value; }

        [[nodiscard]] Type GetValue() const { return m_value; }
    };

    static constexpr BroadPhaseLayer kInvalidBroadPhaseLayer(0xff);

    class BroadPhaseLayerInterface
    {
    public:
        BroadPhaseLayerInterface() = default;
        BroadPhaseLayerInterface(const BroadPhaseLayerInterface&) = delete;
        BroadPhaseLayerInterface& operator=(const BroadPhaseLayerInterface&) = delete;
        virtual ~BroadPhaseLayerInterface() = default;

        /// Return the number of BroadPhase layer that there are.
        virtual unsigned int GetNumBroadPhaseLayers() const = 0;

        /// Convert a Collision Layer to the corresponding BroadPhase Layer
        virtual BroadPhaseLayer GetBroadPhaseLayer(const CollisionLayer layer) const = 0;

        // [TODO]: In Debug Mode only:
        // const char* GetBroadPhaseLayerName(const BroadPhaseLayer layer) const = 0;
    };

    //----------------------------------------------------------------------------------------------------
    /// @brief : Base class used to test if a Body can collide with a Broadphase Layer. Use when finding
    ///     collision pairs of Bodies.
    //----------------------------------------------------------------------------------------------------
    class CollisionVsBroadPhaseLayerFilter
    {
    public:
        virtual ~CollisionVsBroadPhaseLayerFilter() = default;

        virtual bool ShouldCollide([[maybe_unused]] const CollisionLayer collisionLayer, [[maybe_unused]] const BroadPhaseLayer broadPhaseLayer) const { return true; }
    };

    //----------------------------------------------------------------------------------------------------
    //	NOTES:
    //		
    /// @brief : Filter class for determining if two Broadphase layers should 
    //----------------------------------------------------------------------------------------------------
    class BroadPhaseLayerFilter
    {
    public:
        BroadPhaseLayerFilter() = default;
        BroadPhaseLayerFilter(const BroadPhaseLayerFilter&) = delete;
        BroadPhaseLayerFilter& operator=(const BroadPhaseLayerFilter&) = delete;
        virtual ~BroadPhaseLayerFilter() = default;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Function to filter out Broadphase Layers when doing collision query test. Return tru to allow
        ///     testing against Bodies with this Layer.
        //----------------------------------------------------------------------------------------------------
        virtual bool ShouldCollide([[maybe_unused]] const BroadPhaseLayer& inLayer) const { return true; }
    };
}

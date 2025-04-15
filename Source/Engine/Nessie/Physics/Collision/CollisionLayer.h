// CollisionLayer.h
#pragma once
#include <cstdint>

namespace nes
{
    //----------------------------------------------------------------------------------------------------
    ///		@brief : Layer that Bodies can be in; determines which other Bodies that can collide with.  
    //----------------------------------------------------------------------------------------------------
    using CollisionLayer = uint16_t;
    static constexpr CollisionLayer kInvalidCollisionLayer = static_cast<CollisionLayer>(~static_cast<CollisionLayer>(0));

    //----------------------------------------------------------------------------------------------------
    /// @brief : Class used to test if two Collision Layers should interact with each other (allow collisions
    ///     between layers).
    //----------------------------------------------------------------------------------------------------
    class CollisionLayerFilter
    {
    public:
        CollisionLayerFilter() = default;
        CollisionLayerFilter(const CollisionLayerFilter&) = delete;
        CollisionLayerFilter& operator=(const CollisionLayerFilter&) = delete;
        virtual ~CollisionLayerFilter() = default;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Function to filter out collision layers when doing a collision query test. Return
        ///     true to allow testing objects against this layer.
        //----------------------------------------------------------------------------------------------------
        virtual bool ShouldCollide([[maybe_unused]] const CollisionLayer layer) const
        {
            return true;
        }
    };

    //----------------------------------------------------------------------------------------------------
    /// @brief : Collision Layer Filter that tests two Collision Layers for filtering. 
    //----------------------------------------------------------------------------------------------------
    class CollisionLayerPairFilter
    {
    public:
        CollisionLayerPairFilter() = default;
        CollisionLayerPairFilter(const CollisionLayerPairFilter&) = default;
        CollisionLayerPairFilter& operator=(const CollisionLayerPairFilter&) = delete;
        virtual ~CollisionLayerPairFilter() = default;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Return true if the two layers should collide. 
        //----------------------------------------------------------------------------------------------------
        [[nodiscard]] virtual bool ShouldCollide([[maybe_unused]] const CollisionLayer layer1, [[maybe_unused]] const CollisionLayer layer2) const
        {
            return true;
        }
    };

    //----------------------------------------------------------------------------------------------------
    /// @brief : Collision Layer Filter that uses a CollisionLayerPairFilter and a Collision Layer to filter
    ///     out Layers.
    //----------------------------------------------------------------------------------------------------
    class DefaultCollisionLayerFilter final : public CollisionLayerFilter
    {
        CollisionLayerPairFilter m_layerPairFilter;
        CollisionLayer m_layer;
        
    public:
        DefaultCollisionLayerFilter(const CollisionLayerPairFilter& pairFilter, const CollisionLayer layer)
            : m_layerPairFilter(pairFilter)
            , m_layer(layer)
        {
            //
        }

        DefaultCollisionLayerFilter(const DefaultCollisionLayerFilter& other)
            : m_layerPairFilter(other.m_layerPairFilter)
            , m_layer(other.m_layer)
        {
            //
        }

        [[nodiscard]] virtual bool ShouldCollide(const CollisionLayer layer) const override
        {
            return m_layerPairFilter.ShouldCollide(layer, m_layer);
        }
    };

    //----------------------------------------------------------------------------------------------------
    /// @brief : Filter class used to test other Collision Layers against a specified Layer class. 
    //----------------------------------------------------------------------------------------------------
    class SpecifiedCollisionLayerFilter : public CollisionLayerFilter
    {
        CollisionLayer m_layer;

    public:
        explicit SpecifiedCollisionLayerFilter(const CollisionLayer& layer) : m_layer(layer) {}

        [[nodiscard]] virtual bool ShouldCollide(const CollisionLayer layer) const override
        {
            return m_layer == layer;
        }
    };
}

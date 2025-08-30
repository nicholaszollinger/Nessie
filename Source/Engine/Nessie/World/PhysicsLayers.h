// PhysicsLayers.h
#pragma once
#include "Nessie/Debug/Assert.h"
#include "Nessie/Physics/Collision/CollisionLayer.h"
#include "Nessie/Physics/Collision/BroadPhase/BroadPhaseLayer.h"

// This is here mostly for testing purposes. I want to refactor how the layers are set up, with
// a more data driven approach when I have time. These values were taken from Jolt's Test Project.

namespace nes
{
    namespace PhysicsLayers
    {
        static constexpr CollisionLayer kUnused1    = 0;
        static constexpr CollisionLayer kUnused2    = 1;
        static constexpr CollisionLayer kUnused3    = 2;
        static constexpr CollisionLayer kUnused4    = 3;
        static constexpr CollisionLayer kNonMoving  = 4;
        static constexpr CollisionLayer kMoving     = 5; 
        static constexpr CollisionLayer kDebris     = 6; // Example: Debris collides with kNonMoving.
        static constexpr CollisionLayer kSensor     = 7; // Sensors only collide with Moving objects.
        static constexpr CollisionLayer kNumLayers  = 8;
    }

    namespace BroadPhaseLayers
    {
        static constexpr BroadPhaseLayer    kNonMoving(0);
        static constexpr BroadPhaseLayer    kMoving(1);
        static constexpr BroadPhaseLayer    kDebris(2);
        static constexpr BroadPhaseLayer    kSensor(3);
        static constexpr BroadPhaseLayer    kUnused(4);
        static constexpr unsigned           kNumLayers(5);
    }
    
    //----------------------------------------------------------------------------------------------------
    /// @brief : Test class for Collision Layer Pair Filter implementation. 
    //----------------------------------------------------------------------------------------------------
    class CollisionLayerPairFilterTest final : public nes::CollisionLayerPairFilter
    {
    public:
        virtual bool ShouldCollide(const CollisionLayer layer1, const CollisionLayer layer2) const override
        {
            switch (layer1)
            {
                case PhysicsLayers::kUnused1:
                case PhysicsLayers::kUnused2:
                case PhysicsLayers::kUnused3:
                case PhysicsLayers::kUnused4:
                    return false;

                case PhysicsLayers::kNonMoving:
                    return layer2 == PhysicsLayers::kMoving || layer2 == PhysicsLayers::kDebris;

                case PhysicsLayers::kMoving:
                    return layer2 == PhysicsLayers::kMoving || layer2 == PhysicsLayers::kNonMoving || layer2 == PhysicsLayers::kSensor;

                case PhysicsLayers::kDebris:
                    return layer2 == PhysicsLayers::kNonMoving;

                case PhysicsLayers::kSensor:
                    return layer2 == PhysicsLayers::kMoving;

                default:
                    NES_ASSERT(false);
                    return false;
            }
        }
    };

    //----------------------------------------------------------------------------------------------------
    /// @brief : Test class for Broadphase Layer Interface implementation. 
    //----------------------------------------------------------------------------------------------------
    class BroadPhaseLayerInterfaceTest final : public BroadPhaseLayerInterface
    {
        BroadPhaseLayer m_layerToBroadPhase[PhysicsLayers::kNumLayers];
        
    public:
        BroadPhaseLayerInterfaceTest()
        {
            m_layerToBroadPhase[PhysicsLayers::kUnused1] = BroadPhaseLayers::kUnused;            
            m_layerToBroadPhase[PhysicsLayers::kUnused2] = BroadPhaseLayers::kUnused;            
            m_layerToBroadPhase[PhysicsLayers::kUnused3] = BroadPhaseLayers::kUnused;            
            m_layerToBroadPhase[PhysicsLayers::kUnused4] = BroadPhaseLayers::kUnused;           
            m_layerToBroadPhase[PhysicsLayers::kNonMoving] = BroadPhaseLayers::kNonMoving;           
            m_layerToBroadPhase[PhysicsLayers::kMoving] = BroadPhaseLayers::kMoving;           
            m_layerToBroadPhase[PhysicsLayers::kDebris] = BroadPhaseLayers::kDebris;
            m_layerToBroadPhase[PhysicsLayers::kSensor] = BroadPhaseLayers::kSensor;
        }

        virtual unsigned GetNumBroadPhaseLayers() const override
        {
            return BroadPhaseLayers::kNumLayers;
        }

        virtual BroadPhaseLayer GetBroadPhaseLayer(const CollisionLayer layer) const override
        {
            NES_ASSERT(layer < PhysicsLayers::kNumLayers);
            return m_layerToBroadPhase[layer];
        }
    };

    class CollisionVsBroadPhaseLayerFilterTest final : public CollisionVsBroadPhaseLayerFilter
    {
    public:
        virtual bool ShouldCollide(const CollisionLayer collisionLayer, const BroadPhaseLayer broadPhaseLayer) const override
        {
            switch (collisionLayer)
            {
                case PhysicsLayers::kUnused1:
                case PhysicsLayers::kUnused2:
                case PhysicsLayers::kUnused3:
                case PhysicsLayers::kUnused4:
                    return false;

                case PhysicsLayers::kNonMoving:
                    return broadPhaseLayer == BroadPhaseLayers::kMoving || broadPhaseLayer == BroadPhaseLayers::kDebris;

                case PhysicsLayers::kMoving:
                    return broadPhaseLayer == BroadPhaseLayers::kMoving || broadPhaseLayer == BroadPhaseLayers::kNonMoving || broadPhaseLayer == BroadPhaseLayers::kSensor;

                case PhysicsLayers::kDebris:
                    return broadPhaseLayer == BroadPhaseLayers::kNonMoving;

                case PhysicsLayers::kSensor:
                    return broadPhaseLayer == BroadPhaseLayers::kMoving;

                default:
                    NES_ASSERT(false);
                    return false;
            }
        }
    };
}




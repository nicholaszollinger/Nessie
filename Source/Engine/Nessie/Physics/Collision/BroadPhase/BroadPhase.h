// BroadPhase.h
#pragma once
#include "Math/AABox.h"

namespace nes
{
    class BodyManager;
    
    //----------------------------------------------------------------------------------------------------
    /// @brief : Used to do coarse collision detection operations to quickly prune out Bodies that will not
    ///     collide.
    //----------------------------------------------------------------------------------------------------
    class BroadPhase
    {

    public:
        virtual ~BroadPhase() = default;

        //----------------------------------------------------------------------------------------------------
        // [TODO]: BroadPhaseLayerInterface object, for the Broadphase to understand how different layers interact.
        //		
        /// @brief : Initialize the Broadphase. 
        ///	@param pBodyManager : Reference to the Body Manager.
        //----------------------------------------------------------------------------------------------------
        virtual void Init(BodyManager* pBodyManager /*, [[maybe_unused]] const BroadPhaseLayerInterface& interface*/) { m_pBodyManager = pBodyManager; }
        virtual void Optimize()  {}
        virtual void FrameSync() {}

        //virtual void CastRay() const = 0;
        //virtual void CollideBox() const = 0;
        //virtual void CollideSphere() const = 0;
        //virtual void CollidePoint() const = 0;
        //virtual void CastBox() const = 0;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the Bounding Box of all Bodies in this Broadphase. 
        //----------------------------------------------------------------------------------------------------
        virtual AABox GetBounds() const = 0;

    protected:
        BodyManager* m_pBodyManager = nullptr;
    };
}

// BroadPhase.h
#pragma once
#include "BroadPhaseLayer.h"
#include "BroadPhaseQuery.h"

namespace nes
{
    class BodyManager;
    struct BodyPair;
    
    using BodyPairCollector = CollisionCollector<BodyPair, CollisionCollectorTraitsCollideShape>;
    
    //----------------------------------------------------------------------------------------------------
    /// @brief : Used to do coarse collision detection operations to quickly prune out Bodies that will not
    ///     collide.
    //----------------------------------------------------------------------------------------------------
    class BroadPhase : public BroadPhaseQuery
    {
    public:
        /// Context used during the Broadphase update.
        struct UpdateState
        {
            void* m_pData[4];
        };

        /// Handle used during adding Bodies to the Broadphase.
        using AddState = void*;

    public:
        //----------------------------------------------------------------------------------------------------
        // [TODO]: BroadPhaseLayerInterface object, for the Broadphase to understand how different layers interact.
        //		
        /// @brief : Initialize the Broadphase. 
        ///	@param pBodyManager : Reference to the Body Manager.
        /// @param layerInterface : The Mappings between BroadPhaseLayers and CollisionLayers.
        //----------------------------------------------------------------------------------------------------
        virtual void        Init(BodyManager* pBodyManager, [[maybe_unused]] const BroadPhaseLayerInterface& layerInterface) { m_pBodyManager = pBodyManager; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Should be called after many objects have been inserted to make the broadphase more efficient,
        ///     usually done on startup only.
        //----------------------------------------------------------------------------------------------------
        virtual void        Optimize()  {}

        //----------------------------------------------------------------------------------------------------
        /// @brief : Must be called just before updating the broadphase when none of the body mutexes are locked.
        //----------------------------------------------------------------------------------------------------
        virtual void        FrameSync() {}

        //----------------------------------------------------------------------------------------------------
        /// @brief : Must be called before UpdatePrepare() to prevent modifications being made to the tree.
        //----------------------------------------------------------------------------------------------------
        virtual void        LockModifications() {}

        //----------------------------------------------------------------------------------------------------
        /// @brief : Must be called after UpdateFinalize() to allow modifications to the broadphase.
        //----------------------------------------------------------------------------------------------------
        virtual void        UnlockModifications() {}

        //----------------------------------------------------------------------------------------------------
        /// @brief : Update the broadphase, needs to be called frequently to update the internal state when bodies
        ///     have been modified. This UpdatePrepare() function can be run in a background thread without
        ///     influencing the broadphase.
        //----------------------------------------------------------------------------------------------------
        virtual UpdateState UpdatePrepare() { return UpdateState(); }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Finalizing the update will quickly apply the changes made during UpdatePrepare().
        //----------------------------------------------------------------------------------------------------
        virtual void        UpdateFinalize([[maybe_unused]] const UpdateState&) { /* Optionally overridden by implementation */}

        //----------------------------------------------------------------------------------------------------
        /// @brief : Prepare adding number of Bodies in pBodies to the broadphase. Returns a handle that should
        ///     be used in AddBodiesFinalize()/Abort().
        /// @note : pBodies may be shuffled around by this function and should be kept until AddBodiesFinalize()/Abort()
        ///     is called. 
        //----------------------------------------------------------------------------------------------------
        virtual AddState    AddBodiesPrepare([[maybe_unused]] BodyID* pBodies, [[maybe_unused]] int number) { return nullptr; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Finalize adding bodies to the broadphase. Supply the return value of AddBodiesPrepare() to
        ///     the addState. Ensure that the pBodies passed into AddBodiesPrepare() is unmodified and passed again
        ///     to this function.
        ///	@param pBodies : Array of Bodies to add. This should be the same, unmodified array passed into AddBodiesPrepare().
        ///	@param number : Number of Bodies in pBodies to add.
        ///	@param addState : The AddState value returned from AddBodiesPrepare().
        //----------------------------------------------------------------------------------------------------
        virtual void        AddBodiesFinalize(BodyID* pBodies, int number, AddState addState) = 0;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Abort adding bodies to the broadphase. Supply the return value of AddBodiesPrepare() to
        ///     the addState. Ensure that the pBodies passed into AddBodiesPrepare() is unmodified and passed again
        ///     to this function.
        ///	@param pBodies : Array of Bodies to add. This should be the same, unmodified array passed into AddBodiesPrepare().
        ///	@param number : Number of Bodies in pBodies to add.
        ///	@param addState : The AddState value returned from AddBodiesPrepare().
        //----------------------------------------------------------------------------------------------------
        virtual void        AddBodiesAbort([[maybe_unused]] BodyID* pBodies, [[maybe_unused]] int number, [[maybe_unused]] AddState addState) { /*Does nothing by default. */ }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Remove number of Bodies in pBodies from the broadphase.
        /// @note : pBodies may be shuffled around by this function.
        //----------------------------------------------------------------------------------------------------
        virtual void        RemoveBodies(BodyID* pBodies, int number) = 0;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Call whenever the AABB of a Body changes. 
        /// @note : pBodies may be shuffled around by this function.
        /// @note : takeLock should be false if we're between calls of LockModifications()/UnlockModifications(),
        ///     in which case care needs to be taken so that this is not called between UpdatePrepare() & UpdateFinalize/Abort().
        //----------------------------------------------------------------------------------------------------
        virtual void        NotifyBodiesAABBChanged(BodyID* pBodies, int number, bool takeLock = true) = 0;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Called whenever the layer (and optionally the AABB as well) of a body changes.
        /// @note : pBodies may be shuffled around by this function.
        //----------------------------------------------------------------------------------------------------
        virtual void        NotifyBodiesLayerChanged(BodyID* pBodies, int number) = 0;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Find all colliding pairs between dynamic bodies.
        ///	@param pActiveBodies : A list of Bodies for which we need to find colliding pairs. This function can
        ///     change the order of this array.
        ///	@param numActiveBodies : Size of the pActiveBodies array.
        ///	@param speculativeContactDistance : Distance at which speculative contact points will be created.
        ///	@param collisionVsBroadPhaseLayerFilter : The filter that determines if an object can collide with a
        ///     broadphase layer.
        ///	@param collisionLayerPairFilter : The filter that determines two objects can collide.
        ///	@param pairCollector : Receives callbacks for every body pair found.
        //----------------------------------------------------------------------------------------------------
        virtual void        FindCollidingPairs(BodyID* pActiveBodies, int numActiveBodies, float speculativeContactDistance, const CollisionVsBroadPhaseLayerFilter& collisionVsBroadPhaseLayerFilter, const CollisionLayerPairFilter& collisionLayerPairFilter, BodyPairCollector& pairCollector) const = 0;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the Bounding Box of all Bodies in this Broadphase. 
        //----------------------------------------------------------------------------------------------------
        virtual AABox       GetBounds() const = 0;

    protected:
        BodyManager*        m_pBodyManager = nullptr;
    };
}

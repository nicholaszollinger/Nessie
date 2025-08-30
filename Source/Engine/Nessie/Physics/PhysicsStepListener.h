// PhysicsStepListener.h
#pragma once

namespace nes
{
    class PhysicsScene;

    //----------------------------------------------------------------------------------------------------
    /// @brief : Context information for the step listener. 
    //----------------------------------------------------------------------------------------------------
    struct PhysicsStepListenerContext
    {
        float           m_deltaTime;        /// Delta time of the current step.
        bool            m_isFirstStep;      /// True if this is the first step.
        bool            m_isLastStep;       /// True if this is the last step.
        PhysicsScene*   m_pPhysicsScene;    /// The physics scene being stepped.
    };

    //----------------------------------------------------------------------------------------------------
    /// @brief : A listener class that receives a callback before every physics simulation step. 
    //----------------------------------------------------------------------------------------------------
    class PhysicsStepListener
    {
    public:
        virtual ~PhysicsStepListener() = default;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Called before every simulation step (received collisionStep times for every PhysicsScene::Update(...)
        ///     call). This is called while all body and constraint mutexes are locked. You can read/write bodies
        ///     and constraints but *not* add/remove them. Multiple listeners can be executed in parallel, and
        ///     it is the responsibility of the listener to avoid race conditions.
        ///     The best way to do this is to have each step listener operate on a subset of the bodies and
        ///     constraints and to make sure that these bodies and constraints are not touched by another
        ///     step listener.
        /// @note : This function is not called if there aren't any active bodies or when the physics system is
        ///     updated with 0 delta time.
        //----------------------------------------------------------------------------------------------------
        virtual void OnStep(const PhysicsStepListenerContext& context) = 0;
    };
}
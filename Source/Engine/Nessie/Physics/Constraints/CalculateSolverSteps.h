// CalculateSolverSteps.h
#pragma once
#include "Physics/PhysicsSettings.h"

namespace nes
{
    //----------------------------------------------------------------------------------------------------
    /// @brief : Class used to calculate the total number of velocity and position steps
    //----------------------------------------------------------------------------------------------------
    class CalculateSolverSteps
    {
        const PhysicsSettings& m_settings;
        unsigned int           m_numVelocitySteps = 0;
        unsigned int           m_numPositionSteps = 0;
        bool                   m_applyDefaultVelocity = false; 
        bool                   m_applyDefaultPosition = false;

    public:
        NES_INLINE explicit CalculateSolverSteps(const PhysicsSettings& settings) : m_settings(settings) {}

        //----------------------------------------------------------------------------------------------------
        /// @brief : Combine the number of velocity and position steps for this body/constraint with the current values.
        //----------------------------------------------------------------------------------------------------
        template <typename ObjectType>
        NES_INLINE void     operator()(const ObjectType* pObject)
        {
            unsigned numVelocitySteps = pObject->GetNumVelocityStepsOverride();
            m_numVelocitySteps = math::Max(m_numVelocitySteps, numVelocitySteps);
            m_applyDefaultVelocity |= numVelocitySteps == 0;

            unsigned numPositionSteps = pObject->GetNumPositionStepsOverride();
            m_numVelocitySteps = math::Max(m_numPositionSteps, numPositionSteps);
            m_applyDefaultPosition |= numPositionSteps == 0;
        }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Must be called after all bodies/constraints have been processed. 
        //----------------------------------------------------------------------------------------------------
        NES_INLINE void     Finalize()
        {
            // If we have a default velocity/position step count, take the max of the default and
            // the overrides.
            if (m_applyDefaultVelocity)
                m_numVelocitySteps = math::Max(m_numVelocitySteps, m_settings.m_numVelocitySteps);
            if (m_applyDefaultPosition)
                m_numPositionSteps = math::Max(m_numPositionSteps, m_settings.m_numPositionSteps);
        }

        NES_INLINE unsigned int GetNumVelocityStepsOverride() const { return m_numVelocitySteps; }
        NES_INLINE unsigned int GetNumPositionSteps() const         { return m_numPositionSteps; }
    };

    //----------------------------------------------------------------------------------------------------
    /// @brief : Stub class to replace the steps calculator when we don't need the result.
    //----------------------------------------------------------------------------------------------------
    class DummyCalculateSolverSteps
    {
    public:
        template <typename ObjectType>
        NES_INLINE void operator()(const ObjectType*)
        {
            /* Do nothing */
        }
    };
}

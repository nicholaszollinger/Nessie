// TickFunction.cpp
#include "TickFunction.h"
#include "TickManager.h"
#include "Debug/Assert.h"

namespace nes
{
    TickFunction::~TickFunction()
    {
        UnregisterTick();
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Register this function to a Tick Group.
    //----------------------------------------------------------------------------------------------------
    void TickFunction::RegisterTick(TickGroup* pGroup)
    {
        // A Tick should be fully unregistered before registering again.
        // This will probably come up in the future, but for now I will just assert that this behavior
        // is not desired.
        NES_ASSERT(m_pTickGroup == nullptr);
        
        if (!m_isRegistered)
        {
            pGroup->AddTickFunction(this);
            m_pTickGroup = pGroup;
            m_isRegistered = true;
        }

        else
        {
            NES_ASSERT(pGroup->HasTickFunction(this));
        }
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Removes this Tick from its current Tick Group. By next frame, this Tick will not
    ///         run until registered again.
    //----------------------------------------------------------------------------------------------------
    void TickFunction::UnregisterTick()
    {
        if (m_isRegistered)
        {
            m_pTickGroup->RemoveTickFunction(this);
            Reset();
        }
    }

    //----------------------------------------------------------------------------------------------------
    //		NOTES:
    //		
    ///		@brief : Set the Enabled state of this Tick. If the Tick is enabled, it will  
    ///		@param enabled : 
    //----------------------------------------------------------------------------------------------------
    void TickFunction::SetTickEnabled(const bool enabled)
    {
        if (m_isRegistered)
        {
            // If there was a change in state:
            if (enabled == (m_tickState == TickState::Disabled))
            {
                NES_ASSERT(m_pTickGroup != nullptr);

                auto* pGroup = m_pTickGroup;
                m_pTickGroup->RemoveTickFunction(this);
                // Temp remove the tick group, to satisfy AddTickFunction's !HasTickFunction() check.
                // HasTickFunction may change to be the slower version, as it is only used in Debug scenarios.
                m_pTickGroup = nullptr;
                
                m_tickState = enabled? TickState::Enabled : TickState::Disabled;
                
                pGroup->AddTickFunction(this);
                m_pTickGroup = pGroup;
            }

            // Clear our lastTimeTicked if disabled.
            if (m_tickState == TickState::Disabled)
                m_lastTimeTicked = -1.f;
        }

        else
        {
            m_tickState = enabled? TickState::Enabled : TickState::Disabled;
        }
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Set the interval, in seconds, that this Tick should execute on. An interval less than
    ///         or equal to zero will execute every frame.
    //----------------------------------------------------------------------------------------------------
    void TickFunction::SetTickInterval(const float interval)
    {
        const bool wasUpdatedOnInterval = m_tickInterval > 0.f;
        m_tickInterval = interval;
        
        if (m_isRegistered && m_tickState != TickState::Disabled && wasUpdatedOnInterval)
        {
            NES_ASSERT(m_pTickGroup != nullptr);
            m_pTickGroup->OnTickFunctionIntervalUpdated(this, m_tickInterval);
        }
    }

    //----------------------------------------------------------------------------------------------------
    //		NOTES:
    //		
    ///		@brief : Calculate the raw time since the last execution of this function. If executing every frame,
    ///         this will just return the deltaTime.
    ///		@param deltaTime : Delta time between frames.
    ///		@param currentTime : The current time that has passed.
    //----------------------------------------------------------------------------------------------------
    float TickFunction::CalculateDeltaTime(float deltaTime, const float currentTime)
    {
        const bool wasUpdatedOnInterval = m_tickInterval > 0.f;

        if (!wasUpdatedOnInterval)
        {
            m_lastTimeTicked = -1.f;
        }

        else
        {
            if (m_lastTimeTicked >= 0.f)
            {
                deltaTime = currentTime - m_lastTimeTicked;
            }
            
            m_lastTimeTicked = currentTime;
        }

        return deltaTime;
    }

    //----------------------------------------------------------------------------------------------------
    //		NOTES:
    //		
    ///		@brief : Resets the Tick Function's managed internal state to defaults. 
    //----------------------------------------------------------------------------------------------------
    void TickFunction::Reset()
    {
        m_isRegistered = false;
        m_pTickGroup = nullptr;
        m_pNextTick = nullptr;
        m_relativeTickCooldown = 0.f;
    }
}

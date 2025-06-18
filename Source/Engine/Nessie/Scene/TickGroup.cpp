// TickGroup.cpp
#include "TickGroup.h"

#include <algorithm>
#include "TickManager.h"
#include "Debug/Assert.h"

namespace nes
{
    //----------------------------------------------------------------------------------------------------
    ///		@brief : Attempts to Remove a TickFunction from the list. O(N). Returns true if the TickFunction
    ///             was successfully removed.
    //----------------------------------------------------------------------------------------------------
    bool TickGroup::CooldownTickList::TryRemove(TickFunction* pTickFunction)
    {
        TickFunction* pPrevious = nullptr;
        TickFunction* pCurrent = m_pHead;
        bool wasRemoved = false;
        
        while (pCurrent != nullptr && !wasRemoved)
        {
            if (pCurrent == pTickFunction)
            {
                if (pPrevious)
                {
                    pPrevious->m_pNextTick = pTickFunction->m_pNextTick;
                }

                else
                {
                    NES_ASSERT(m_pHead == pCurrent);
                    m_pHead = pTickFunction->m_pNextTick;
                }
                
                pTickFunction->m_pNextTick = nullptr;
                wasRemoved = true;
            }
            
            else
            {
                pPrevious = pCurrent;
                pCurrent = pCurrent->m_pNextTick;
            }
        }

        return wasRemoved;
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Returns true if the TickFunction is in the List. O(N).
    //----------------------------------------------------------------------------------------------------
    bool TickGroup::CooldownTickList::Contains(const TickFunction* pTickFunction) const
    {
        const TickFunction* pCurrent = m_pHead;
        while (pCurrent)
        {
            if (pCurrent == pTickFunction)
                return true;

            pCurrent = pCurrent->m_pNextTick;
        }

        return false;
    }

    TickGroup::TickGroup(const ETickStage stage)
        : m_stage(stage)
    {
        //
    }

    TickGroup::~TickGroup()
    {
        UnregisterAllTickFunctions();
        UnregisterGroup();
    }

    //----------------------------------------------------------------------------------------------------
    //		NOTES:
    //		
    ///		@brief : Register this group to the Tick Manager. 
    //----------------------------------------------------------------------------------------------------
    void TickGroup::RegisterGroup()
    {
        if (!m_isRegistered)
        {
            TickManager::Get().RegisterTickGroup(this);
            m_isRegistered = true;
        }
    }

    //----------------------------------------------------------------------------------------------------
    //		NOTES:
    //		
    ///		@brief : Unregister this group from the TickManager. 
    //----------------------------------------------------------------------------------------------------
    void TickGroup::UnregisterGroup()
    {
        // [TODO]: Need to handle during 
        if (m_isRegistered)
        {
            TickManager::Get().UnregisterTickGroup(this);
            m_isRegistered = false;
        }
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Register a Tick Function to this group. 
    //----------------------------------------------------------------------------------------------------
    void TickGroup::AddTickFunction(TickFunction* pTickFunction)
    {
        NES_ASSERT(!HasTickFunction(pTickFunction));

        if (pTickFunction->m_tickState == TickFunction::TickState::Enabled)
        {
            m_allEnabledTicks.emplace(pTickFunction);
        }

        else
        {
            NES_ASSERT(pTickFunction->m_tickState == TickFunction::TickState::Disabled);
            m_allDisabledTicks.emplace(pTickFunction);
        }
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Remove a Tick Function from this group. This will leave the tick in an unregistered
    ///             state.
    //----------------------------------------------------------------------------------------------------
    void TickGroup::RemoveTickFunction(TickFunction* pTickFunction)
    {
        NES_ASSERT(HasTickFunction(pTickFunction));

        switch (pTickFunction->m_tickState)
        {
            case TickFunction::TickState::Enabled:
            {
                // If this Tick is not in the Enabled Set, then it is being rescheduled or in the cooldown list.
                if (m_allEnabledTicks.erase(pTickFunction) == 0)
                {
                    RemoveFromRescheduleOrCooldownList(pTickFunction);
                }
                break;
            }
            
            case TickFunction::TickState::Disabled:
            {
                [[maybe_unused]] const auto removed = m_allDisabledTicks.erase(pTickFunction);
                NES_ASSERT(removed == 1);
                break;
            }
            
            case TickFunction::TickState::OnCooldown:
            {
                RemoveFromRescheduleOrCooldownList(pTickFunction);
                break;
            }
        }
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Set the scale factor of deltaTime for this Tick Group. The delta Time that is passed
    ///         to the Tick Function will be scaled by this value. A Time Scale of 1 will keep the deltaTime
    ///         unchanged. 2 will double time, and 0.5 will halve it.
    //----------------------------------------------------------------------------------------------------
    void TickGroup::SetTimeScale(const float scale)
    {
        m_timeScale = scale;
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Set the enabled status of this Tick Group. All Tick Functions will not be run if the
    ///         Group is disabled.
    //----------------------------------------------------------------------------------------------------
    void TickGroup::SetGroupEnabled(bool enable)
    {
        m_isEnabled = enable;
    }

    void TickGroup::OnTickFunctionIntervalUpdated(TickFunction* pTickFunction, const float newInterval)
    {
        const auto findFunctionPredicate = [pTickFunction](const TickRescheduleInfo& info)
        {
            return info.m_pFunction == pTickFunction;  
        };

        auto it = std::ranges::find_if(m_ticksToReschedule.begin(), m_ticksToReschedule.end(), findFunctionPredicate);
        if (it != m_ticksToReschedule.end())
        {
            it->m_requiredCooldown = newInterval;
            return;
        }

        // If it was not in the rescheduling array, then we need to remove from the Cooldown list and reschedule.
        [[maybe_unused]] const bool success = m_cooldownList.TryRemove(pTickFunction);
        NES_ASSERT(success);
        RescheduleTickFunction(pTickFunction, newInterval);
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Returns whether this TickFunction is a part of this tick group.
    //----------------------------------------------------------------------------------------------------
    bool TickGroup::HasTickFunction(TickFunction* pTickFunction) const
    {
        NES_ASSERT(pTickFunction);
        if (pTickFunction->m_pTickGroup != this)
            return false;

        // Slow debug checks:
#if NES_LOGGING_ENABLED
        const auto findFunctionPredicate = [pTickFunction](const TickRescheduleInfo& info)
        {
            return info.m_pFunction == pTickFunction;  
        };
        
        NES_ASSERT(m_allEnabledTicks.contains(pTickFunction)
            || m_allDisabledTicks.contains(pTickFunction)
            || (std::ranges::find_if(m_ticksToReschedule.begin(), m_ticksToReschedule.end(), findFunctionPredicate) != m_ticksToReschedule.end())
            || m_cooldownList.Contains(pTickFunction));
#endif
        
        return true;
    }

    //----------------------------------------------------------------------------------------------------
    //		NOTES:
    //		
    ///		@brief : Begin a Tick Frame, which builds the array of Ticks that are ready to be processed during
    ///         ExecuteReadyTicks()
    ///		@param deltaTime : The delta time for this frame.
    ///     @param currentTime : The current time that has elapsed.
    //----------------------------------------------------------------------------------------------------
    void TickGroup::BeginFrame(const float deltaTime, const float currentTime)
    {
        // Update this frame's TickContext.
        m_frameContext.m_deltaTime = deltaTime;
        m_frameContext.m_currentTime = currentTime;

        // Schedule ticks that are set to be on cooldown, before building the array of ticks to execute.
        ScheduleCooldowns();

        // Build the array of Ready Ticks:

        // Add all enabled Ticks.
        for (auto it = m_allEnabledTicks.begin(); it != m_allEnabledTicks.end();)
        {
            auto* pFunction = *it;
            m_readyTicks.emplace_back(pFunction);

            if (pFunction->m_tickInterval > 0.f)
            {
                it = m_allEnabledTicks.erase(it);
                RescheduleTickFunction(pFunction, pFunction->m_tickInterval);
                continue;
            }

            ++it;
        }

        // Process Cooldowns, and add those who are ready to go this frame.
        float cumulativeCooldownTime = 0.f; // Amount of time accumulated from relative time in cooldown list.
        while (TickFunction* pTickFunction = m_cooldownList.m_pHead)
        {
            // If the current tick's cooldown is greater than the frame time, then any Ticks past this point in the
            // cooldown list also need to wait.
            if (cumulativeCooldownTime + pTickFunction->m_relativeTickCooldown > m_frameContext.m_deltaTime)
            {
                // Update the relative time of the Head Tick for the next frame. We don't have to update
                // any other ticks, because they store a relative cooldown time. We get to just update
                // this and break.
                pTickFunction->m_relativeTickCooldown -= (m_frameContext.m_deltaTime - cumulativeCooldownTime);
                break;
            }

            // Otherwise, this Tick is ready to run this frame.
            cumulativeCooldownTime += pTickFunction->m_relativeTickCooldown;
            pTickFunction->m_tickState = TickFunction::TickState::Enabled;
            m_readyTicks.emplace_back(pTickFunction);

            // Queue reschedule for next frame, accounting for relative wait time.
            RescheduleTickFunction(pTickFunction, pTickFunction->m_tickInterval - (m_frameContext.m_deltaTime - cumulativeCooldownTime));

            // Move the head to the next tick.
            m_cooldownList.m_pHead = pTickFunction->m_pNextTick;
        }
    }

    //----------------------------------------------------------------------------------------------------
    //		NOTES:
    //      This runs synchronously for now, but a future refactor could dispatch these on a separate thread.
    //		
    ///		@brief : Executes all Tick Functions that are ready to go. 
    //----------------------------------------------------------------------------------------------------
    void TickGroup::ExecuteReadyTicks()
    {
        m_isRunning = true;
        
        for (auto* pFunction : m_readyTicks)
        {
            NES_ASSERT(pFunction);
            NES_ASSERT(pFunction->m_tickState == TickFunction::TickState::Enabled);
            
            // Calculate the DeltaTime for this specific Tick:
            TickDeltaTime deltaTimeInfo;
            const float tickDelta = pFunction->CalculateDeltaTime(m_frameContext.m_deltaTime, m_frameContext.m_currentTime);
            deltaTimeInfo.m_rawDeltaTime = tickDelta;
            deltaTimeInfo.m_deltaTime    = tickDelta * m_timeScale;
            deltaTimeInfo.m_isPaused     = m_frameContext.m_isPaused;
            pFunction->ExecuteTick(deltaTimeInfo);
        }
        
        m_readyTicks.clear(); // Purge the array for the next Frame.
        m_isRunning = false;
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Unregisters all Ticks from this group, and clears all tick function containers.
    //----------------------------------------------------------------------------------------------------
    void TickGroup::UnregisterAllTickFunctions()
    {
        // Unregister all ticks from each possible bucket.
        for (auto it = m_allEnabledTicks.begin(); it != m_allEnabledTicks.end(); ++it)
        {
            (*it)->m_isRegistered = false;
        }

        for (auto it = m_allDisabledTicks.begin(); it != m_allDisabledTicks.end(); ++it)
        {
            (*it)->m_isRegistered = false;
        }

        auto* pFunction = m_cooldownList.m_pHead;
        while (pFunction)
        {
            pFunction->m_isRegistered = false;
            pFunction = pFunction->m_pNextTick;
        }

        for (const auto& info : m_ticksToReschedule)
        {
            info.m_pFunction->m_isRegistered = false;
        }
        
        m_cooldownList.m_pHead = nullptr;
        m_ticksToReschedule.clear();
        m_allEnabledTicks.clear();
        m_allDisabledTicks.clear();
        m_readyTicks.clear();
    }

    //----------------------------------------------------------------------------------------------------
    //		NOTES:
    //		
    ///		@brief : Adds the Ticks that need to be Rescheduled into the Cooldown List, in
    ///       order of cooldown. At the end of this function, the Reschedule array will be empty, and the
    ///        Cooldown List will have all Ticks on Cooldown in order of shortest to longest relative cooldown time. 
    //----------------------------------------------------------------------------------------------------
    void TickGroup::ScheduleCooldowns()
    {
        if (m_ticksToReschedule.empty())
            return;

        // Sort the array so that the ticks with the shortest cooldown is at the front.
        std::ranges::sort(m_ticksToReschedule, [](const TickRescheduleInfo& a, const TickRescheduleInfo& b)
        {
            return a.m_requiredCooldown < b.m_requiredCooldown; 
        });

        size_t rescheduleIndex = 0;
        float cumulativeCooldown = 0.f;
        TickFunction* pCurrent = m_cooldownList.m_pHead;
        TickFunction* pPrevious = nullptr;

        while (pCurrent && rescheduleIndex < m_ticksToReschedule.size())
        {
            const float cooldownTime = m_ticksToReschedule[rescheduleIndex].m_requiredCooldown;

            // If the cooldown time is less than the cumulative time of the current tick then this
            // tick needs to be set before the current tick.
            if (cumulativeCooldown + pCurrent->m_relativeTickCooldown > cooldownTime)
            {
                TickFunction* pFunction = m_ticksToReschedule[rescheduleIndex].m_pFunction;
                // Catch the case where a Function was disabled, ignore rescheduling it.
                if (pFunction->m_tickState == TickFunction::TickState::Disabled)
                {
                    ++rescheduleIndex;
                    continue;
                }
                
                pFunction->m_tickState = TickFunction::TickState::OnCooldown;
                pFunction->m_relativeTickCooldown = cooldownTime - cumulativeCooldown;

                // Set the Previous to point at this Tick
                if (pPrevious)
                {
                    pPrevious->m_pNextTick = pFunction;
                }

                // Or this is the new head.
                else
                {
                    NES_ASSERT(m_cooldownList.m_pHead == pCurrent);
                    m_cooldownList.m_pHead = pFunction;
                }

                pFunction->m_pNextTick = pCurrent;
                pPrevious = pFunction;
                pCurrent->m_relativeTickCooldown -= pFunction->m_relativeTickCooldown;
                cumulativeCooldown += pFunction->m_relativeTickCooldown;

                // Move to the next Tick to reschedule.
                ++rescheduleIndex;
            }

            // Otherwise, move forward in the Cooldown List until we find a tick with a longer cooldown.
            else
            {
                cumulativeCooldown += pCurrent->m_relativeTickCooldown;
                pPrevious = pCurrent;
                pCurrent = pCurrent->m_pNextTick;
            }
        }

        // All remaining ticks need to wait longer than all the cooldown ticks currently in the list.
        // Append each remaining tick to the end.
        for (; rescheduleIndex < m_ticksToReschedule.size(); ++rescheduleIndex)
        {
            TickFunction* pFunction = m_ticksToReschedule[rescheduleIndex].m_pFunction;
            // Catch the case where the Function is now disabled. Ignore rescheduling it.
            if (pFunction->m_tickState == TickFunction::TickState::Disabled)
                continue;

            pFunction->m_tickState = TickFunction::TickState::OnCooldown;
            
            const float cooldownTime = m_ticksToReschedule[rescheduleIndex].m_requiredCooldown;
            pFunction->m_relativeTickCooldown = cooldownTime - cumulativeCooldown;
            pFunction->m_pNextTick = nullptr;
            
            // Set the Previous to point at this Tick
            if (pPrevious)
            {
                pPrevious->m_pNextTick = pFunction;
            }

            // Or this is the new head.
            else
            {
                NES_ASSERT(m_cooldownList.m_pHead == pCurrent);
                m_cooldownList.m_pHead = pFunction;
            }

            pPrevious = pFunction;
            cumulativeCooldown += pFunction->m_relativeTickCooldown;
        }

        m_ticksToReschedule.clear();
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Remove a TickFunction that was either in the process of being rescheduled or is in the
    ///         cooldown list.
    ///         This is O(N + M) where N is the number of elements in the rescheduling array, and M is the
    ///         number of elements in the cooldown list. If the function is found in the array, the cooldown list
    ///         check is skipped.
    //----------------------------------------------------------------------------------------------------
    void TickGroup::RemoveFromRescheduleOrCooldownList(TickFunction* pTickFunction)
    {
        const auto findFunctionPredicate = [pTickFunction](const TickRescheduleInfo& info)
        {
            return info.m_pFunction == pTickFunction;  
        };
        
        const auto it = std::ranges::find_if(m_ticksToReschedule.begin(), m_ticksToReschedule.end(), findFunctionPredicate);
        bool foundTick = it != m_ticksToReschedule.end();
        if (foundTick)
        {
            std::swap(*it, m_ticksToReschedule.back());
            m_ticksToReschedule.pop_back();
        }

        // If not found, attempt to remove from the Cooldown List.
        if (!foundTick)
        {
            foundTick = m_cooldownList.TryRemove(pTickFunction);
        }

        NES_ASSERT(foundTick);
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Queues the Tick Function to be rescheduled at the start of the next Frame.
    ///		@param pFunction : Tick function that needs to be rescheduled.
    ///		@param cooldownTime : Time that this Tick needs to cooldown before being called again. This may
    ///         not be the same as the Tick Interval, in the case that the Tick is just being registered or
    ///         re-enabled.
    //----------------------------------------------------------------------------------------------------
    void TickGroup::RescheduleTickFunction(TickFunction* pFunction, const float cooldownTime)
    {
        m_ticksToReschedule.emplace_back(pFunction, cooldownTime, false);
    }
}

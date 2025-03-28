// TickGroup.h
#pragma once
#include <unordered_set>
#include "TickFunction.h"
#include "Core/String/StringID.h"

namespace nes
{
    class TickManager;

    enum class TickStage : uint8_t
    {
        PrePhysics = 0,
        Physics,
        PostPhysics,
        Late,
        
        NumStages,
    };
    
    //----------------------------------------------------------------------------------------------------
    //		NOTES:
    //		
    ///		@brief : A Tick Group maintains a collection of Tick Functions that are executed together.
    //----------------------------------------------------------------------------------------------------
    class TickGroup
    {
        friend class TickManager;

        struct TickRescheduleInfo
        {
            TickFunction* m_pFunction = nullptr;
            float m_requiredCooldown = 0.f;
            bool m_needsToRemove = false;
        };

        // Information on the current Frame's Tick.
        struct TickFrameContext
        {
            float m_deltaTime = 0.f;    // Delta Time this Frame.
            float m_currentTime = 0.f;  // Current global Time, in seconds.
            bool m_isPaused = false;
        };

        //----------------------------------------------------------------------------------------------------
        ///		@brief : Linked List of Ticks that are currently on Cooldown. I need to be able to quickly
        ///             insert and remove when rescheduling Ticks, and be able to iterate from front to back when
        ///             updating cooldowns.
        //----------------------------------------------------------------------------------------------------
        struct CooldownTickList
        {
            TickFunction* m_pHead = nullptr;

            bool TryRemove(TickFunction* pTickFunction);
            bool Contains(const TickFunction* pTickFunction) const;
        };

        std::unordered_set<TickFunction*> m_allEnabledTicks{};
        std::unordered_set<TickFunction*> m_allDisabledTicks{};
        std::vector<TickRescheduleInfo> m_ticksToReschedule{};
        std::vector<TickFunction*> m_readyTicks{};
        CooldownTickList m_cooldownList{};
        
        TickFrameContext m_frameContext{};
        size_t m_numReadyTicks = 0;
        StringID m_debugName{};
        float m_timeScale = 1.f;                    // Current scale factor applied to the delta time of all Tick Functions in the Group.
        TickStage m_stage = TickStage::PrePhysics;  // The Stage of the overall update loop that this TickGroup will be run.
        bool m_isRegistered = false;
        bool m_isEnabled = true;
        bool m_isRunning = false;                   // Whether this Group is currently being executed or not.
        
    public:
        explicit TickGroup(const TickStage stage);
        ~TickGroup();

        TickGroup(const TickGroup&) = delete;
        TickGroup& operator=(const TickGroup&) = delete;
        TickGroup(TickGroup&&) noexcept = default;
        TickGroup& operator=(TickGroup&&) noexcept = default;

        void RegisterGroup();
        void UnregisterGroup();
        void AddTickFunction(TickFunction* pTickFunction);
        void RemoveTickFunction(TickFunction* pTickFunction);
        void SetTimeScale(const float scale);
        void SetDebugName(const StringID& debugName) { m_debugName = debugName; }
        void SetGroupEnabled(bool enable);
        void OnTickFunctionIntervalUpdated(TickFunction* pTickFunction, const float newInterval);

        [[nodiscard]] float     GetTimeScale() const        { return m_timeScale; }
        [[nodiscard]] StringID  GetDebugName() const        { return m_debugName; }
        [[nodiscard]] TickStage GetStage() const            { return m_stage; }
        [[nodiscard]] bool      IsEnabled() const           { return m_isEnabled; }
        [[nodiscard]] bool      HasTickFunction(TickFunction* pTickFunction) const;

    private:
        void BeginFrame(const float deltaTime, const float currentTime);
        void ExecuteReadyTicks();
        void UnregisterAllTickFunctions();
        
        void ScheduleCooldowns();
        void RemoveFromRescheduleOrCooldownList(TickFunction* pTickFunction);
        void RescheduleTickFunction(TickFunction* pFunction, const float cooldownTime);
    };
}

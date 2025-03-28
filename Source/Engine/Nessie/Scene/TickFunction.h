// TickFunction.h
#pragma once
#include <cstdint>
#include <limits>

namespace nes
{
    struct TickDeltaTime
    {
        float m_deltaTime       = 0.f;   // The amount of time, in seconds, since the last Tick, with Time Scaling applied.
        float m_rawDeltaTime    = 0.f;   // The amount of time, in seconds, since the last Tick, without Time Scaling applied.
        bool m_isPaused         = false; // Whether the Tick Function's group is paused or not.
    };
    
    class TickFunction
    {
        friend class TickManager;
        friend class TickGroup;

    public:
        enum class TickState : uint8_t
        {
            Enabled,
            Disabled,
            OnCooldown,
        };

    private:
        /// The Tick Group that this Tick Function was registered to.
        TickGroup* m_pTickGroup = nullptr;
        
        //----------------------------------------------------------------------------------------------------
        ///		@brief : When in Cooldown, this represents the Tick that comes after this one in the TickGroup's
        ///         Cooldown List.
        //----------------------------------------------------------------------------------------------------
        TickFunction* m_pNextTick = nullptr;
        
        //----------------------------------------------------------------------------------------------------
        ///		@brief :  The interval, in seconds, that this Function should be scheduled to run. If less than
        ///         or equal to zero, then it will run every frame.
        //----------------------------------------------------------------------------------------------------
        float m_tickInterval = 0.f;

        //----------------------------------------------------------------------------------------------------
        ///		@brief : The interval, in seconds, that this Function has left before being called again, relative
        ///         to the cooldown of the tick before it.
        //----------------------------------------------------------------------------------------------------
        float m_relativeTickCooldown = 0.f;

        //----------------------------------------------------------------------------------------------------
        ///		@brief : The last time that this Tick Function was executed to calculate the deltaTime between
        ///         executions. If executing every frame, this is set to -1.f.
        //----------------------------------------------------------------------------------------------------
        float m_lastTimeTicked = -1.f;
        TickState m_tickState   = TickState::Enabled;
        bool m_isRegistered     = false;
        
    public:
        virtual ~TickFunction();
        
        void RegisterTick(TickGroup* pGroup);
        void UnregisterTick();
        void SetTickEnabled(const bool enabled);
        void SetTickInterval(const float interval);

        virtual void ExecuteTick(const TickDeltaTime& deltaTime) = 0;

        [[nodiscard]] float GetTickInterval() const     { return m_tickInterval; }
        [[nodiscard]] bool IsRegistered() const         { return m_isRegistered; }
        [[nodiscard]] bool IsEnabled() const            { return m_tickState != TickState::Disabled; }

    private:
        float CalculateDeltaTime(float deltaTime, const float currentTime);
        void Reset();
    };
}

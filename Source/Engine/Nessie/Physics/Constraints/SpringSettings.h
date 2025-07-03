// SpringSettings.h
#pragma once
#include "Nessie/Core/Config.h"

namespace nes
{
    //----------------------------------------------------------------------------------------------------
    /// @brief : Enum used by constraints to specify how a spring is defined. 
    //----------------------------------------------------------------------------------------------------
    enum class ESpringMode : uint8
    {
        FrequencyAndDamping,    /// Frequency and Damping are specified.
        StiffnessAndDamping,    /// Stiffness and Damping are specified.
    };

    //----------------------------------------------------------------------------------------------------
    /// @brief : Settings for a linear or angular spring. 
    //----------------------------------------------------------------------------------------------------
    struct SpringSettings
    {
        SpringSettings() = default;
        SpringSettings(const SpringSettings&) = default;
        SpringSettings(SpringSettings&&) noexcept = default;
        SpringSettings& operator=(const SpringSettings&) = default;
        SpringSettings& operator=(SpringSettings&&) noexcept = default;
        SpringSettings(ESpringMode mode, float frequencyOrStiffness, float damping);
        
        /// Selects the way in which the spring is defined
        /// If the mode is StiffnessAndDamping, then m_frequency becomes the stiffness (k) and m_damping becomes the damping ratio (c) in the spring equation F = -k * x - c * v. Otherwise the properties are as documented.
        ESpringMode m_springMode = ESpringMode::FrequencyAndDamping;
        
        union
        {
            /// Valid when m_springMode = ESpringMode::FrequencyAndDamping.
            /// If m_frequency > 0 the constraint will be soft and m_frequency specifies the oscillation frequency in Hz.
            /// If m_frequency <= 0, m_damping is ignored and the constraint will have hard limits (as hard as the time step / the number of velocity / position solver steps allows).
            float   m_frequency = 0.f;

            /// Valid when m_springMode = ESpringMode::StiffnessAndDamping.
            /// If m_stiffness > 0 the constraint will be soft and m_stiffness specifies the stiffness (k) in the spring equation F = -k * x - c * v for a linear or T = -k * theta - c * w for an angular spring.
            /// If m_stiffness <= 0, m_damping is ignored and the constraint will have hard limits (as hard as the time step / the number of velocity / position solver steps allows).
            ///
            /// Note that stiffness values are large numbers. To calculate a ballpark value for the needed stiffness you can use:
            /// force = stiffness * delta_spring_length = mass * gravity <=> stiffness = mass * gravity / delta_spring_length.
            /// So if your object weighs 1500 kg and the spring compresses by 2 meters, you need a stiffness in the order of 1500 * 9.81 / 2 ~ 7500 N/m.
            float   m_stiffness;
        };

        /// When m_springMode = ESpringMode::FrequencyAndDamping m_damping is the damping ratio (0 = no damping, 1 = critical damping).
        /// When m_springMode = ESpringMode::StiffnessAndDamping m_damping is the damping (c) in the spring equation F = -k * x - c * v for a linear or T = -k * theta - c * w for an angular spring.
        /// Note that if you set m_damping = 0, you will not get an infinite oscillation. Because we integrate physics using an explicit Euler scheme, there is always energy loss.
        /// This is done to keep the simulation from exploding, because with a damping of 0 and even the slightest rounding error, the oscillation could become bigger and bigger until the simulation explodes.
        float       m_damping = 0.f;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Check if the spring has a valid frequency or stiffness. If not, the spring will be hard. 
        //----------------------------------------------------------------------------------------------------
        inline bool HasStiffness() const { return m_frequency > 0.f; }
    };
}
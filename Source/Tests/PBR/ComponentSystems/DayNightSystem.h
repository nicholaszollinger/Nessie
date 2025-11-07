// DayNightSystem.h
#pragma once
#include "Nessie/World.h"

namespace pbr
{
    struct DayNightSimComponent
    {
        nes::Vec3   m_dayColor = nes::Vec3(1.f, 1.f, 0.95f);          // Color of the Sun at the peak of the afternoon.  
        nes::Vec3   m_nightColor = nes::Vec3(1.f, 0.6f, 0.3f);        // Color of the Sun in the middle of the night.
        float       m_dayDuration = 20.f;                                   // Duration of both Day and Night, in seconds.
        float       m_sunMaxLux = 120'000.f;                                // The Lux at the peak of the afternoon.

        static void Serialize(nes::YamlOutStream& out, const DayNightSimComponent& component);
        static void Deserialize(const nes::YamlNode& in, DayNightSimComponent& component);
    };

    class DayNightSystem final : public nes::ComponentSystem
    {
    public:
        DayNightSystem(nes::WorldBase& world) : nes::ComponentSystem(world) {}

        virtual void    RegisterComponentTypes() override;
        void            Tick(const float deltaTime);
        void            OnEvent(nes::Event& e);

    private:
        float           m_accumulatedTime = 0.f;
        bool            m_shouldSimulate = true;           
    };
}
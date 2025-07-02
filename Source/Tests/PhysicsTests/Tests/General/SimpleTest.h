// SimpleTest.h
#pragma once
#include "../Test.h"
#include "Nessie/Physics/Body/BodyActivationListener.h"

class SimpleTest final : public Test
{
public:
    virtual         ~SimpleTest() override;
    virtual void    Init() override;

private:
    // Demo of the activation listener.
    class Listener final : public nes::BodyActivationListener
    {
    public:
        virtual void OnBodyActivated(const nes::BodyID& bodyID, [[maybe_unused]] const uint64_t bodyUserData) override
        {
            NES_LOG("Body {} activated.", bodyID.GetIndex());
        }

        virtual void OnBodyDeactivated(const nes::BodyID& bodyID, [[maybe_unused]] const uint64_t bodyUserData) override
        {
            NES_LOG("Body {} deactivated.", bodyID.GetIndex());
        }
    };

    Listener        m_bodyActivationListener;
};

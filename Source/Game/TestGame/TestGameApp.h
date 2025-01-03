// TestGameApp.h
#pragma once
#include "Application/Application.h"

class TestGameApp final : public nes::Application
{
public:
    TestGameApp(const nes::CommandLineArgs& args);
    
protected:
    virtual void Update(double deltaTime) override;
};

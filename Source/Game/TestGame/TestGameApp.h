// TestGameApp.h
#pragma once
#include "Demo.h"
#include "Application/Application.h"

class TestGameApp final : public nes::Application
{
    std::vector<Demo*> m_demos{};
    int m_currentDemo = -1;
    
public:
    TestGameApp(const nes::CommandLineArgs& args);
    
protected:
    virtual bool PostInit() override;
    virtual void Update(double deltaTime) override;

private:
    void RenderMenuBar();
    void RenderCurrentDemo(const nes::Renderer& renderer, const nes::Rectf& worldViewport) const;
};

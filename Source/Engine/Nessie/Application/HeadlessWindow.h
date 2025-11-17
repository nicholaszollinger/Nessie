// HeadlessWindow.h
#pragma once
#include "ApplicationWindow.h"

namespace nes
{
    //----------------------------------------------------------------------------------------------------
    /// @brief : A headless window is not visible and does not receive input. You can think of it as a
    ///     null window. It is used as a stand in if the application does not need window support.
    //----------------------------------------------------------------------------------------------------
    class HeadlessWindow final : public ApplicationWindow
    {
    public:
        virtual bool Internal_Init(Application& app, WindowDesc&& windowDesc) override;
        virtual void Internal_ProcessEvents() override;
    
        virtual void SetCursorMode(const ECursorMode) override {}
        virtual void SetIsMinimized(const bool) override {}
        virtual void SetVsync(const bool) override {}
    };
}
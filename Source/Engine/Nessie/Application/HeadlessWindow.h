// HeadlessWindow.h
#pragma once
#include "ApplicationWindow.h"

namespace nes
{
    //----------------------------------------------------------------------------------------------------
    /// @brief : A window that is invisible, and does not receive any input. This window is used when the
    ///     Application Desc is set to headless.
    //----------------------------------------------------------------------------------------------------
    class HeadlessWindow final : public ApplicationWindow
    {
    public:
        virtual bool Internal_Init(Platform& platform, const WindowDesc& desc) override;
        virtual void Internal_ProcessEvents() override;
    
        virtual void SetCursorMode(const ECursorMode) override {}
        virtual void SetIsMinimized(const bool) override {}
        virtual void SetVsync(const bool) override {}
    };
}
// WindowsPlatform.cpp
#include "Nessie/Platform/Platform.h"
#ifdef NES_PLATFORM_WINDOWS

#include "WindowsInclude.h"
#include "WinConsoleTarget.h"
#include "Nessie/Debug/Logger/LogTargets/MSVCTarget.h"

namespace nes
{
    LogTargetPtr Platform::CreateDefaultLogTarget()
    {
#ifdef NES_FORCE_SINGLE_THREADED
        return std::make_shared<WinConsoleStdCoutTargetST>();
#else
        return std::make_shared<WinConsoleStdCoutTargetMT>();
#endif
    }

    void Platform::HandleFatalError(const std::string& reason, const std::string& message)
    {
        if (IsDebuggerPresent())
        {
            // Retry to go into the debugger, cancel exits.
            if (MessageBoxA(nullptr, message.c_str(), reason.c_str(), MB_RETRYCANCEL | MB_ICONERROR) == IDRETRY)
                return;
        }

        else
        {
            // Ok prompt, exits.
            MessageBoxA(nullptr, message.c_str(), reason.c_str(), MB_OK | MB_ICONERROR);
        }

        exit(1);
    }
}

#endif

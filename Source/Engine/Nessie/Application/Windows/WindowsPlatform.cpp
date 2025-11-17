// WindowsPlatform.cpp
#include "Nessie/Application/Application.h"

#ifdef NES_PLATFORM_WINDOWS
#include "Nessie/Core/PlatformConstants.h"
#include "WindowsInclude.h"
#include "WinConsoleTarget.h"
#include "Nessie/Debug/Logger/LoggerRegistry.h"
#include "Nessie/Debug/Logger/LogTargets/MSVCTarget.h"

namespace nes
{
    void internal::HandleFatalError(const std::string& reason, const std::string& message)
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

    LogTargetPtr LoggerRegistry::CreateDefaultLogTarget()
    {
#ifdef NES_FORCE_SINGLE_THREADED
        if (IsDebuggerPresent())
        {
            return std::make_shared<MSVCTargetST>();
        }
        return std::make_shared<WinConsoleStdCoutTargetST>();
        
#else
        if (IsDebuggerPresent())
        {
            return std::make_shared<MSVCTargetMT>();
        }
        return std::make_shared<WinConsoleStdCoutTargetMT>();
#endif
    }

}

#endif
// DefaultLogTarget_Windows.cpp
// - Provides a Default log target implementation for the Windows console.

#include "Core/Log/Logger.h"

#if NES_USE_DEFAULT_LOG_TARGET && defined(NES_PLATFORM_WINDOWS)

#include <conio.h>
#include <iostream>
#pragma warning (push)
#pragma warning (disable : 5105)
#include <Windows.h>
#pragma warning (pop)

namespace nes
{
    bool LogTarget::Init()
    {
        AllocConsole();
        return true;
    }

    void LogTarget::Close()
    {
        FreeConsole();
    }

    void LogTarget::PrePost(const LogSeverity type)
    {
        const HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);

        switch (type)
        {
            case LogSeverity::kLog:
                // Grey Color
                SetConsoleTextAttribute(h, FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_GREEN);
                break;
            
            case LogSeverity::kWarning:
                // Yellow Color
                SetConsoleTextAttribute(h, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY);
                break;
        
            case LogSeverity::kError:
            case LogSeverity::kCritical:
                // Red Color
                SetConsoleTextAttribute(h, FOREGROUND_RED);
                break;
        }
    }

    void LogTarget::Post(const std::string& msg)
    {
        std::cout << msg;
    }
}

#endif
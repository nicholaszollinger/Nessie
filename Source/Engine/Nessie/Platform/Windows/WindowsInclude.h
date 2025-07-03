// WindowsInclude.h
#pragma once
#include "Nessie/Core/Config.h"

#ifdef NES_PLATFORM_WINDOWS
    // Prevent Windows from redefining min and max macros
    #ifndef NOMINMAX
        #define NOMINMAX
    #endif

    #ifndef WIN32_LEAN_AND_MEAN
        #define WIN32_LEAN_AND_MEAN
    #endif
    #pragma warning (push)
    #pragma warning (disable : 5039)
    #include <windows.h>
    #pragma warning (pop)
#endif

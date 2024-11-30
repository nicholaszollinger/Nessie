#pragma once
// EntryPoint.h
#include "Platform.h"

#if defined(NES_PLATFORM_WINDOWS)
#define NES_MAIN()                                                                  \
int main(int argc, char** argv)                                                     \
{                                                                                   \
    nes::CommandLineArgs args = {static_cast<size_t>(argc), argv };                 \
                                                                                    \
    nes::Platform platform(args);                                                   \
    auto code = platform.Initialize();                                              \
                                                                                    \
    if (code == nes::Platform::ExitCode::Success)                                   \
    {                                                                               \
        code = platform.MainLoop();                                                 \
    }                                                                               \
                                                                                    \
    platform.Terminate(code);                                                       \
                                                                                    \
    return 0;                                                                       \
}                                                                                   
#endif
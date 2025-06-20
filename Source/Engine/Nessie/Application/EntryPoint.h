#pragma once
// EntryPoint.h
#include "Application.h"

#if defined(NES_PLATFORM_WINDOWS)
#define NES_MAIN()                                                                  \
int main(int argc, char** argv)                                                     \
{                                                                                   \
    nes::CommandLineArgs args = {static_cast<size_t>(argc), argv };                 \
                                                                                    \
    nes::Application app(args);                                                     \
    auto code = app.Internal_Init();                                                \
                                                                                    \
    if (code == nes::Application::EExitCode::Success)                               \
    {                                                                               \
        code = app.Internal_RunMainLoop();                                          \
    }                                                                               \
                                                                                    \
    app.Internal_Close(code);                                                       \
                                                                                    \
    return static_cast<int>(code);                                                  \
}                                                                                   
#endif
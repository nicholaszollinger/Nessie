#pragma once
// EntryPoint.h
#include "Application.h"

#if defined(NES_PLATFORM_WINDOWS)
#define NES_MAIN()                                                                  \
int main(int argc, char** argv)                                                     \
{                                                                                   \
    nes::CommandLineArgs args = {static_cast<size_t>(argc), argv };                 \
                                                                                    \
    nes::Application* pApp = CreateApplication(args);                               \
    auto code = pApp->Init();                                                       \
                                                                                    \
    if (code == nes::Application::ExitCode::Success)                                \
    {                                                                               \
        code = pApp->RunMainLoop();                                                 \
    }                                                                               \
                                                                                    \
    pApp->Close(code);                                                              \
                                                                                    \
    delete (pApp);                                                                  \
    return static_cast<int>(code);                                                  \
}                                                                                   
#endif
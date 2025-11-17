// EntryPoint.h
#pragma once
#include "Nessie/Application/Application.h"

#if defined(NES_PLATFORM_WINDOWS)
#define NES_MAIN()                                                          \
int main(int argc, char** argv)                                             \
{                                                                           \
    NES_INIT_LEAK_DETECTOR();                                               \
    nes::LoggerRegistry::Instance().Internal_Init();                        \
    nes::CommandLineArgs args = {static_cast<size_t>(argc), argv };         \
                                                                            \
    std::unique_ptr<nes::Application> pApp = nes::CreateApplication(args);  \
    if (pApp->Internal_Init())                                              \
    {                                                                       \
        pApp->Internal_RunMainLoop();                                       \
    }                                                                       \
    pApp->Internal_Shutdown();                                              \
    pApp.reset();                                                           \
                                                                            \
    nes::LoggerRegistry::Instance().Internal_Shutdown();                    \
    NES_DUMP_AND_DESTROY_LEAK_DETECTOR();                                   \
                                                                            \
    return 0;                                                               \
}                                                                                   
#endif
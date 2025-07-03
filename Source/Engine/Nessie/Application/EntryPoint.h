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
    nes::Application* app = nes::CreateApplication(args);                   \
    NES_ASSERT(app != nullptr);                                             \
                                                                            \
    if (app->Internal_Init())                                               \
    {                                                                       \
        app->Internal_RunMainLoop();                                        \
    }                                                                       \
                                                                            \
    NES_DELETE(app);                                                        \
    nes::LoggerRegistry::Instance().Internal_Shutdown();                    \
    NES_DUMP_AND_DESTROY_LEAK_DETECTOR();                                   \
                                                                            \
    return 0;                                                               \
}                                                                                   
#endif
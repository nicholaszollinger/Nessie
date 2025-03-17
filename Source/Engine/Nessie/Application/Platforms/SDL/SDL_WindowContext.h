// SDL_WindowContext.h
#pragma once
#include "Core/Config.h"
#ifdef NES_WINDOW_API_SDL
struct SDL_Window;
struct SDL_Renderer;


namespace nes
{
    class Application;
    
    struct SDL_WindowContext
    {
        Application& m_application;
        SDL_Window* m_pNativeWindow = nullptr;
        SDL_Renderer* m_pNativeRenderer = nullptr;
        bool m_shouldClose = false;

        SDL_WindowContext(Application& app, SDL_Window* window, SDL_Renderer* renderer);
        ~SDL_WindowContext();
    };
}
#endif
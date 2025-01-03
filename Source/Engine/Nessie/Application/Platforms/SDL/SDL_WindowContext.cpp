// SDL_WindowContext.cpp

#include "SDL_WindowContext.h"
#include <SDL_render.h>

namespace nes
{
    SDL_WindowContext::SDL_WindowContext(Application& app, SDL_Window* window, SDL_Renderer* renderer)
        : m_application(app)
        , m_pNativeWindow(window)
        , m_pNativeRenderer(renderer)
    {
        //
    }
    
    SDL_WindowContext::~SDL_WindowContext()
    {
        // Destroy the Renderer for the Window
        if (m_pNativeRenderer)
        {
            SDL_DestroyRenderer(m_pNativeRenderer);
            m_pNativeRenderer = nullptr;
        }

        // Destroy the Window
        if (m_pNativeWindow)
        {
            SDL_DestroyWindow(m_pNativeWindow);
            m_pNativeWindow = nullptr;
        }
    }
}

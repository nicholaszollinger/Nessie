// SDL_Window.cpp
// SDL Implementation of the Window class.
#include "Core/Config.h"
#ifdef NES_WINDOW_API_SDL
#include <SDL.h>
#include "SDL_Input.h"
#include "SDL_WindowContext.h"
#include "BleachNew.h"
#include "Application/Application.h"
#include "Application/Window.h"
#include "Debug/CheckedCast.h"
#include "backends/imgui_impl_sdl2.h"

namespace nes
{
    bool Window::Init(Application& app, const WindowProperties& props)
    {
        m_properties = props;

        // Setup SDL
        // There is only one window for the app...so this should be fine.
        // You should consider moving framework initialization & cleanup like this
        // into the Platform.
        if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_GAMECONTROLLER) != 0)
        {
            NES_ERRORV("SDL", "Failed to initialize SDL!");
            return false;
        }

        // From 2.0.18: Enable native IME.
#ifdef SDL_HINT_IME_SHOW_UI
        SDL_SetHint(SDL_HINT_IME_SHOW_UI, "1");
#endif
        
        uint32_t flags{};
        if (m_properties.m_isResizable)
            flags |= SDL_WINDOW_RESIZABLE;

        if (m_properties.m_extent.m_height == 0 && m_properties.m_extent.m_width == 0)
        {
            flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
        }

        SDL_Window* pWindow = SDL_CreateWindow(m_properties.m_label.c_str()
            , SDL_WINDOWPOS_CENTERED
            , SDL_WINDOWPOS_CENTERED
            ,static_cast<int>(m_properties.m_extent.m_width)
            ,static_cast<int>(m_properties.m_extent.m_height)
            , flags);

        if (!pWindow)
        {
            NES_ERRORV("SDL", "Failed to create window!");
            return false;
        }


        uint32_t renderFlags = SDL_RENDERER_ACCELERATED;
        if (m_properties.m_vsyncEnabled)
        {
            renderFlags |= SDL_RENDERER_PRESENTVSYNC;
        }
        
        SDL_Renderer* pRenderer = SDL_CreateRenderer(pWindow, -1, renderFlags);
        if (!pRenderer)
        {
            NES_ERRORV("SDL", "Failed to create SDL_Renderer!");
            return false;
        }

        m_pWindowContext = BLEACH_NEW(SDL_WindowContext(app, pWindow, pRenderer));

        SDL_RendererInfo info;
        SDL_GetRendererInfo(pRenderer, &info);
        NES_LOGV("SDL", "Current Renderer: ", info.name);

        // Set the Initial Cursor Position
        int mouseX, mouseY;
        SDL_GetMouseState(&mouseX, &mouseY);
        m_cursorPosition.x = static_cast<float>(mouseX);
        m_cursorPosition.y = static_cast<float>(mouseY);
        
        return true;
    }

    void Window::ProcessEvents()
    {
        // Grab the Context:
        SDL_WindowContext* pContext = checked_cast<SDL_WindowContext*>(m_pWindowContext);
        NES_ASSERT(pContext);
        NES_ASSERT(pContext->m_pNativeWindow);

        Application& app = pContext->m_application;
        const Modifiers mods = SDL::CurrentModifiers();
        
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            // ImGui Process
            ImGui_ImplSDL2_ProcessEvent(&event);
            
            // Quit Event
            if (event.type == SDL_QUIT)
            {
                pContext->m_shouldClose = true;
                app.Quit();
            }

            // Window Events
            else if (event.type == SDL_WINDOWEVENT && event.window.windowID == SDL_GetWindowID(pContext->m_pNativeWindow))
            {
                switch (event.window.event)
                {
                    case SDL_WINDOWEVENT_RESIZED:
                    {
                        Resize(event.window.data1, event.window.data2);
                        break;
                    }

                    case SDL_WINDOWEVENT_MINIMIZED:
                    {
                        SetIsMinimized(true);
                        break;
                    }

                    case SDL_WINDOWEVENT_RESTORED:
                    {
                        SetIsMinimized(false);
                        break;
                    }

                    case SDL_WINDOWEVENT_CLOSE:
                    {
                        pContext->m_shouldClose = true;
                        app.Quit();
                        break;
                    }
                       
                    default: break;
                }
            }
            
            // Key Down
            else if (event.type == SDL_KEYDOWN)
            {
                const KeyAction action = event.key.repeat > 0? KeyAction::Repeat : KeyAction::Pressed;
                const KeyCode key = SDL::ToKeyCode(event.key.keysym.scancode);
                KeyEvent keyEvent(key, action, mods);
                app.PushEvent(keyEvent);
            }

            // Key Up
            else if (event.type == SDL_KEYUP)
            {
                const KeyCode key = SDL::ToKeyCode(event.key.keysym.scancode);
                KeyEvent keyEvent(key, KeyAction::Released, mods);
                app.PushEvent(keyEvent);
            }

            // Mouse Button Down
            else if (event.type == SDL_MOUSEBUTTONDOWN)
            {
                const SDL_MouseButtonEvent& mouseButton = event.button;
                const MouseButton button = SDL::ToMouseButton(mouseButton.button);
                MouseButtonEvent mouseEvent(button, MouseAction::Pressed, mods, static_cast<float>(mouseButton.x), static_cast<float>(mouseButton.y));
                app.PushEvent(mouseEvent);
            }

            // Mouse Button Released
            else if (event.type == SDL_MOUSEBUTTONUP)
            {
                const SDL_MouseButtonEvent& mouseButton = event.button;
                const MouseButton button = SDL::ToMouseButton(mouseButton.button);
                MouseButtonEvent mouseEvent(button, MouseAction::Released, mods, static_cast<float>(mouseButton.x), static_cast<float>(mouseButton.y));
                app.PushEvent(mouseEvent);
            }

            // Mouse Motion
            else if (event.type == SDL_MOUSEMOTION)
            {
                MouseMoveEvent moveEvent
                (
                    static_cast<float>(event.motion.x)
                    ,static_cast<float>(event.motion.y)
                    ,static_cast<float>( event.motion.xrel)
                    ,static_cast<float>( event.motion.yrel)
                );
                app.PushEvent(moveEvent);
            }
            
            // Mouse Scroll
            else if (event.type == SDL_MOUSEWHEEL)
            {
                MouseScrollEvent scrollEvent(static_cast<float>(event.wheel.x), static_cast<float>(event.wheel.y));
                app.PushEvent(scrollEvent);
            }
        }
    }

    bool Window::ShouldClose()
    {
        SDL_WindowContext* pContext = checked_cast<SDL_WindowContext*>(m_pWindowContext);
        NES_ASSERT(pContext);
        return pContext->m_shouldClose;
    }

    void Window::Close()
    {
        SDL_WindowContext* pContext = checked_cast<SDL_WindowContext*>(m_pWindowContext);
        
        if (pContext)
        {
            // Deleting the Window Context will destroy the SDL Window and Renderer
            BLEACH_DELETE(pContext);
            m_pWindowContext = nullptr;
        }

        // There is only one window for the app...so this should be fine.
        // You should consider moving framework initialization & cleanup like this
        // into the Platform.
        SDL_Quit();
    }

    WindowExtent Window::Resize(const WindowExtent& extent)
    {
        return Resize(extent.m_width, extent.m_height);
    }

    WindowExtent Window::Resize(const uint32_t width, const uint32_t height)
    {
        SDL_WindowContext* pContext = checked_cast<SDL_WindowContext*>(m_pWindowContext);
        NES_ASSERT(pContext);
        SDL_SetWindowSize(pContext->m_pNativeWindow, static_cast<int>(width), static_cast<int>(height));
        
        m_properties.m_extent.m_width = width;
        m_properties.m_extent.m_height = height;
        return m_properties.m_extent;
    }

    void Window::SetVsync([[maybe_unused]] const bool enabled)
    {
        // [TODO]: The fix here is to use SDL_GL_SetSwapInterval(enabled? 1 : 0);
        // That requires OpenGL though.
    }

    void Window::SetIsMinimized(const bool minimized)
    {
        if (minimized == m_properties.m_isMinimized)
            return;

        SDL_WindowContext* pContext = checked_cast<SDL_WindowContext*>(m_pWindowContext);
        NES_ASSERT(pContext);
        NES_ASSERT(pContext->m_pNativeWindow);

        m_properties.m_isMinimized = minimized;
        if (minimized)
            SDL_MinimizeWindow(pContext->m_pNativeWindow);
        else
            SDL_RestoreWindow(pContext->m_pNativeWindow);
    }

    void* Window::GetNativeWindowHandle() const
    {
        SDL_WindowContext* pContext = checked_cast<SDL_WindowContext*>(m_pWindowContext);
        NES_ASSERT(pContext);
        return pContext->m_pNativeWindow;
    }
}
#endif
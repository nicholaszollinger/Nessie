// ApplicationWindow.cpp
#include "Nessie/Core/Config.h"

#include "GLFW/GLFWInputConversions.h"
#include "Application.h"
#include "Nessie/Debug/CheckedCast.h"

#include "GLFW/glfw3.h"
#include "Nessie/Math/Vec2.h"

// Native Window conversion: 
#ifdef NES_PLATFORM_WINDOWS
    #define GLFW_EXPOSE_NATIVE_WIN32
#endif
#include "GLFW/glfw3native.h"

namespace nes
{
    static void SetGLFWCursorMode(GLFWwindow* pWindow, const ECursorMode oldMode, const ECursorMode newMode);
    
    bool ApplicationWindow::Internal_Init(Application&, const WindowDesc& desc)
    {
        m_label = desc.m_label;
        m_vsyncEnabled = desc.m_vsyncEnabled;
        m_isResizable = desc.m_isResizable;
        m_style = desc.m_styleFlags;
        
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

        // Hide until explicitly shown.
        glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
        
        // Set the styling:
        if (desc.m_styleFlags == EWindowStyleFlags::NoDecoration)
        {
            glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);
        }
        
        // Set whether the Window is resizable or not.
        glfwWindowHint(GLFW_RESIZABLE, desc.m_isResizable);
        
        GLFWwindow* pWindow = nullptr;
        switch (desc.m_windowMode)
        {
            case EWindowMode::Fullscreen:
            {
                auto* pMonitor = glfwGetPrimaryMonitor();
                const auto* pMode = glfwGetVideoMode(pMonitor);
                pWindow = glfwCreateWindow(pMode->width, pMode->height, desc.m_label.c_str(), pMonitor, nullptr);
                break;
            }

            case EWindowMode::FullscreenBorderless:
            {
                auto* pMonitor = glfwGetPrimaryMonitor();
                const auto* pMode = glfwGetVideoMode(pMonitor);
                
                glfwWindowHint(GLFW_RED_BITS, pMode->redBits);
			    glfwWindowHint(GLFW_GREEN_BITS, pMode->greenBits);
			    glfwWindowHint(GLFW_BLUE_BITS, pMode->blueBits);
			    glfwWindowHint(GLFW_REFRESH_RATE, pMode->refreshRate);

                pWindow = glfwCreateWindow(pMode->width, pMode->height, desc.m_label.c_str(), pMonitor, nullptr);
                break;
            }
            
            case EWindowMode::Windowed:
            {
                const int width = static_cast<int>(desc.m_windowResolution.x);
                const int height = static_cast<int>(desc.m_windowResolution.y);
                pWindow = glfwCreateWindow(width, height, desc.m_label.c_str(), nullptr, nullptr);
                break;
            }
        }

        if (!pWindow)
        {
            return false;
        }
        
        // Set the Native Window handles:
        m_nativeWindow.m_glfw = pWindow;
#if NES_PLATFORM_WINDOWS
        m_nativeWindow.m_windows.m_hwnd = glfwGetWin32Window(pWindow);
#endif

        m_subWindowWithFocus = pWindow;
        m_subWindowLastUnderCursor = pWindow;

        // Set a minimum width and height for the window:
        // Settings based on Jetbrains Rider Application:
        static constexpr int kMinWidth = 326;
        static constexpr int kMinHeight = 40;
        glfwSetWindowSizeLimits(pWindow, kMinWidth, kMinHeight, GLFW_DONT_CARE, GLFW_DONT_CARE);

        // Set the GLFW Callbacks:
        // Window Resize Callback.
        glfwSetWindowSizeCallback(pWindow, []([[maybe_unused]] GLFWwindow* pWindow, const int /*width*/, const int /*height*/)
        {
            auto& window = Application::Get().GetWindow();
            if (window.GetNativeWindow().m_glfw != pWindow)
                return;
            
            window.m_swapChainNeedsRebuild = true;
        });

        glfwSetFramebufferSizeCallback(pWindow, []([[maybe_unused]] GLFWwindow* pWindow, const int /*width*/, const int /*height*/)
        {
            auto& window = Application::Get().GetWindow();
            if (window.GetNativeWindow().m_glfw != pWindow)
                return;
            
            window.m_swapChainNeedsRebuild = true;
        });

        // Window Close Callback.
        glfwSetWindowCloseCallback(pWindow, [](GLFWwindow* pWindow)
        {
            glfwSetWindowShouldClose(pWindow, GLFW_TRUE);
        });

        glfwSetWindowFocusCallback(pWindow, [](GLFWwindow* pWindow, const int focused)
        {
            if (focused == GLFW_TRUE)
            {
                auto& window = Application::Get().GetWindow();
                window.m_subWindowWithFocus = pWindow;
            }
        });

        glfwSetCursorEnterCallback(pWindow, [](GLFWwindow* pWindow, const int entered)
        {
            if (entered == GLFW_TRUE)
            {
                auto& window = Application::Get().GetWindow();
                window.m_subWindowLastUnderCursor = pWindow;
            }
        });

        // Window Key Callback.
        glfwSetKeyCallback(pWindow, []([[maybe_unused]] GLFWwindow* pWindow, const int key, [[maybe_unused]] const int scanCode, const int action, const int modifiers)
        {
            auto& app = Application::Get();
            
            const Modifiers mods = glfw::ConvertToModifiers(modifiers);
            const EKeyCode keyCode = glfw::ConvertToKeyCode(key);
            const EKeyAction keyAction = glfw::ConvertToKeyAction(action);

            KeyEvent event(keyCode, keyAction, mods);
            app.Internal_OnInputEvent(event);
        });

        // Mouse Button Callback.
        glfwSetMouseButtonCallback(pWindow, [](GLFWwindow* pWindow, const int button, const int action, const int modifiers)
        {
            auto& app = Application::Get();
            
            // Get the mouse position at the time of the event.
            double xPos, yPos;
            glfwGetCursorPos(pWindow, &xPos, &yPos);
            const Float2 mousePos(static_cast<float>(xPos), static_cast<float>(yPos));
            const EMouseButton mouseButton = glfw::ConvertToMouseButton(button);
            const Modifiers mods = glfw::ConvertToModifiers(modifiers);
            const EMouseAction mouseAction = glfw::ConvertToMouseAction(action);

            MouseButtonEvent event(mouseButton, mouseAction, mods, mousePos.x, mousePos.y);
            app.Internal_OnInputEvent(event);
        });

        // Mouse Scroll Callback.
        glfwSetScrollCallback(pWindow, []([[maybe_unused]] GLFWwindow* pWindow, const double deltaX, const double deltaY)
        {
            auto& app = Application::Get();
            
            MouseScrollEvent event(static_cast<float>(deltaX), static_cast<float>(deltaY));
            app.Internal_OnInputEvent(event);
        });

        // Mouse Move Callback.
        glfwSetCursorPosCallback(pWindow, []([[maybe_unused]] GLFWwindow* pWindow, const double xPos, const double yPos)
        {
            auto& app = Application::Get();
            
            MouseMoveEvent event(static_cast<float>(xPos), static_cast<float>(yPos));
            app.Internal_OnInputEvent(event);
        });

        glfwSetTitlebarHitTestCallback(pWindow, [](GLFWwindow*, const int xPos, const int yPos, int* hit)
        {
            auto& app = Application::Get();
            WindowTitlebarHitTestEvent event(xPos, yPos, *hit);
            app.PushEvent(event);
        });

    #ifdef NES_PLATFORM_WINDOWS
        if (desc.m_styleFlags == EWindowStyleFlags::NoTitlebar)
        {
            glfwSetWindowAttrib(pWindow, GLFW_TITLEBAR, GLFW_FALSE);
        }
    #endif
        
        // Save the initial state.
        glfwGetWindowSize(pWindow, &m_resolution.x, &m_resolution.y);
        glfwGetWindowPos(pWindow, &m_position.x, &m_position.y);
        
        // Ensure that the first call to process events will rebuild the swap chain.
        m_swapChainNeedsRebuild = true;

        return true;
    }

    void ApplicationWindow::Internal_ProcessEvents()
    {
        // This function is called when all threads are synced.
        glfwPollEvents();
        ApplyPendingStateChanges();
        
        // Check if the swapchain needs to be rebuilt. This is set any time
        // the window is resized or if the vsync setting is changed. 
        if (m_swapChainNeedsRebuild)
        {
            auto& app = Application::Get();
            app.Internal_OnWindowResize(static_cast<uint32>(m_resolution.x), static_cast<uint32>(m_resolution.y));

            // Clear the flag.
            m_swapChainNeedsRebuild = false;
        }
    }

    void ApplicationWindow::Internal_Shutdown()
    {
        // Destroy the window.
        if (m_nativeWindow.m_glfw != nullptr)
        {
            glfwDestroyWindow(checked_cast<GLFWwindow*>(m_nativeWindow.m_glfw));
            m_nativeWindow = {};
        }
    }

    void ApplicationWindow::ApplyPendingStateChanges()
    {
        auto* pWindow = checked_cast<GLFWwindow*>(m_nativeWindow.m_glfw);
        
        if (!RequestedStateChange())
        {
            // Update our current state, then return.
            glfwGetWindowSize(pWindow, &m_resolution.x, &m_resolution.y);
            glfwGetWindowPos(pWindow, &m_position.x, &m_position.y);
            return;
        }

        if ((m_requestedState & EUpdateState::Minimize))
        {
            glfwIconifyWindow(pWindow);
        }
        else if (m_requestedState & EUpdateState::Maximize)
        {
            glfwMaximizeWindow(pWindow);
        }
        else if (m_requestedState & EUpdateState::Restore)
        {
            glfwRestoreWindow(pWindow);
        }
        if (m_requestedState & EUpdateState::Reposition)
        {
            glfwSetWindowPos(pWindow, m_position.x, m_position.y);
        }
        if (m_requestedState & EUpdateState::Resize)
        {
            glfwSetWindowSize(pWindow, m_resolution.x, m_resolution.y);
        }

        // Update our current state.
        glfwGetWindowSize(pWindow, &m_resolution.x, &m_resolution.y);
        glfwGetWindowPos(pWindow, &m_position.x, &m_position.y);
        
        m_swapChainNeedsRebuild = true;
        m_requestedState = EUpdateState::None;
    }

    void ApplicationWindow::Close()
    {
        glfwSetWindowShouldClose(checked_cast<GLFWwindow*>(m_nativeWindow.m_glfw),true);
    }

    bool ApplicationWindow::IsMainApplicationWindow() const
    {
        auto& appWindow = Application::Get().GetWindow();
        return appWindow.GetNativeWindow().m_glfw == m_nativeWindow.m_glfw;
    }

    bool ApplicationWindow::ShouldClose() const
    {
        return glfwWindowShouldClose(checked_cast<GLFWwindow*>(m_nativeWindow.m_glfw));
    }

    void ApplicationWindow::SetMinimized(const bool minimized)
    {
        const bool isMinimized = IsMinimized();
        if (isMinimized == minimized)
            return;
        
        if (minimized)
            m_requestedState |= EUpdateState::Minimize;
        else
            m_requestedState |= EUpdateState::Restore;
    }

    bool ApplicationWindow::IsMinimized() const
    {
        return glfwGetWindowAttrib(checked_cast<GLFWwindow*>(m_nativeWindow.m_glfw), GLFW_ICONIFIED);
    }

    void ApplicationWindow::SetMaximized(const bool enabled)
    {
        if (enabled)
            m_requestedState |= EUpdateState::Maximize;
        else if (glfwGetWindowAttrib(checked_cast<GLFWwindow*>(m_nativeWindow.m_glfw), GLFW_VISIBLE))
            m_requestedState |= EUpdateState::Restore;
    }
    
    bool ApplicationWindow::IsMaximized() const
    {
        return glfwGetWindowAttrib(checked_cast<GLFWwindow*>(m_nativeWindow.m_glfw), GLFW_MAXIMIZED);
    }

    void ApplicationWindow::SetWindowStyle(const EWindowStyleFlags style)
    {
        if (style == m_style)
            return;

        auto* pWindow = checked_cast<GLFWwindow*>(m_nativeWindow.m_glfw);
        
        if (style == EWindowStyleFlags::None)
        {
            glfwSetWindowAttrib(pWindow, GLFW_DECORATED, GLFW_TRUE);
            glfwSetWindowAttrib(pWindow, GLFW_TITLEBAR, GLFW_TRUE);
        }
        else if (style == EWindowStyleFlags::NoDecoration)
        {
            glfwSetWindowAttrib(pWindow, GLFW_DECORATED, GLFW_FALSE);
        }
        else if (style == EWindowStyleFlags::NoTitlebar)
        {
            glfwSetWindowAttrib(pWindow, GLFW_DECORATED, GLFW_TRUE);
            glfwSetWindowAttrib(pWindow, GLFW_TITLEBAR, GLFW_FALSE);
        }
        
        m_style = style;
    }

    void ApplicationWindow::SetWindowRestoreStateCentered(const int width, const int height)
    {
        Resize(width, height);
        
        // Calculate the center position in the primary monitor.
        GLFWmonitor* pMonitor = glfwGetPrimaryMonitor();
        const GLFWvidmode* mode = glfwGetVideoMode(pMonitor);
        int monitorX, monitorY;
        glfwGetMonitorPos(pMonitor, &monitorX, &monitorY);
        SetPosition(monitorX + (mode->width - width) / 2, monitorY + (mode->height - height) / 2);
    }

    Vec2 ApplicationWindow::GetCursorPosition() const
    {
        double posX, posY;
        glfwGetCursorPos(checked_cast<GLFWwindow*>(m_subWindowLastUnderCursor), &posX, &posY);
        return Vec2(static_cast<float>(posX), static_cast<float>(posY));
    }

    void ApplicationWindow::SetVsync(const bool enabled)
    {
        if (enabled != m_vsyncEnabled)
        {
            m_vsyncEnabled = enabled;
            m_swapChainNeedsRebuild = true;
        }
    }

    void ApplicationWindow::SetCursorMode(const ECursorMode mode)
    {
        GLFWwindow* pWindow = checked_cast<GLFWwindow*>(m_subWindowWithFocus);
        SetGLFWCursorMode(pWindow, m_cursorMode, mode);
        m_cursorMode = mode;
    }

    void ApplicationWindow::ShowWindow()
    {
        GLFWwindow* pWindow = checked_cast<GLFWwindow*>(m_nativeWindow.m_glfw);
        glfwShowWindow(pWindow);
    }

    void ApplicationWindow::Resize(const UVec2& extent)
    {
        Resize(extent.x, extent.y);
    }

    void ApplicationWindow::Resize(const uint32_t width, const uint32_t height)
    {
        m_resolution = IVec2(width, height);
        m_requestedState |= EUpdateState::Resize;
    }

    IVec2 ApplicationWindow::GetResolution() const
    {
        return m_resolution;
    }

    void ApplicationWindow::SetPosition(const int x, const int y)
    {
        m_position = IVec2(x, y);
        m_requestedState |= EUpdateState::Reposition;
    }

    IVec2 ApplicationWindow::GetPosition() const
    {
        return m_position;
    }

    void ApplicationWindow::CenterWindow()
    {
        GLFWmonitor* pMonitor = glfwGetPrimaryMonitor();
        const GLFWvidmode* mode = glfwGetVideoMode(pMonitor);
        
        int monitorX, monitorY;
        glfwGetMonitorPos(pMonitor, &monitorX, &monitorY);

        const auto resolution= GetResolution();
        SetPosition(monitorX + (mode->width - resolution.x) / 2, monitorY + (mode->height - resolution.y) / 2);
    }

    void SetGLFWCursorMode(GLFWwindow* pWindow, const ECursorMode oldMode, const ECursorMode newMode)
    {
        glfwSetInputMode(pWindow, GLFW_CURSOR, glfw::ConvertToGLFWCursorMode(newMode));

        // If we are setting the mouse cursor to disabled,
        if (glfwRawMouseMotionSupported())
        {
            if (newMode == ECursorMode::Disabled)
            {
                glfwSetInputMode(pWindow, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
            }

            // If changing from disabled, change the raw mouse motion back. 
            else if (oldMode == ECursorMode::Disabled)
            {
                glfwSetInputMode(pWindow, GLFW_RAW_MOUSE_MOTION, GLFW_FALSE);
            }
        }
    }

}
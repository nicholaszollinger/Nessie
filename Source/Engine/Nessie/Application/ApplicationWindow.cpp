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
    
    bool ApplicationWindow::Internal_Init(Application&, WindowDesc&& desc)
    {
        m_desc = std::move(desc);
        
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

        // Set whether the Window is resizable or not.
        glfwWindowHint(GLFW_RESIZABLE, desc.m_isResizable);
        
        GLFWwindow* pWindow = nullptr;
        switch (m_desc.m_windowMode)
        {
            case EWindowMode::Fullscreen:
            {
                auto* pMonitor = glfwGetPrimaryMonitor();
                const auto* pMode = glfwGetVideoMode(pMonitor);
                m_desc.m_windowResolution.x = pMode->width;
                m_desc.m_windowResolution.y = pMode->height;
                pWindow = glfwCreateWindow(pMode->width, pMode->height, m_desc.m_label.c_str(), pMonitor, nullptr);
                break;
            }

            case EWindowMode::FullscreenBorderless:
            {
                auto* pMonitor = glfwGetPrimaryMonitor();
                const auto* pMode = glfwGetVideoMode(pMonitor);

                m_desc.m_windowResolution.x = pMode->width;
                m_desc.m_windowResolution.y = pMode->height;

                glfwWindowHint(GLFW_RED_BITS, pMode->redBits);
			    glfwWindowHint(GLFW_GREEN_BITS, pMode->greenBits);
			    glfwWindowHint(GLFW_BLUE_BITS, pMode->blueBits);
			    glfwWindowHint(GLFW_REFRESH_RATE, pMode->refreshRate);

                pWindow = glfwCreateWindow(pMode->width, pMode->height, m_desc.m_label.c_str(), pMonitor, nullptr);
                break;
            }

            case EWindowMode::Windowed:
            {
                const int width = static_cast<int>(m_desc.m_windowResolution.x);
                const int height = static_cast<int>(m_desc.m_windowResolution.y);
                pWindow = glfwCreateWindow(width, height, m_desc.m_label.c_str(), nullptr, nullptr);
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

        // Set the GLFW Callbacks:
        // Window Resize Callback.
        glfwSetWindowSizeCallback(pWindow, []([[maybe_unused]] GLFWwindow* pWindow, const int width, const int height)
        {
            auto& window = Application::Get().GetWindow();
            if (window.GetNativeWindow().m_glfw != pWindow)
                return;
            
            window.m_swapChainNeedsRebuild = true;
            window.m_desc.m_windowResolution.x = width;
            window.m_desc.m_windowResolution.y = height;

            // Set minimized state:
            if (width == 0 && height == 0)
                window.m_desc.m_isMinimized = true;
            else
                window.m_desc.m_isMinimized = false;
        });

        glfwSetFramebufferSizeCallback(pWindow, []([[maybe_unused]] GLFWwindow* pWindow, const int width, const int height)
        {
            auto& window = Application::Get().GetWindow();
            if (window.GetNativeWindow().m_glfw != pWindow)
                return;
            
            window.m_swapChainNeedsRebuild = true;
            window.m_desc.m_windowResolution.x = width;
            window.m_desc.m_windowResolution.y = height;

            // Set minimized state:
            if (width == 0 && height == 0)
                window.m_desc.m_isMinimized = true;
            else
                window.m_desc.m_isMinimized = false;
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
        
        // Ensure that the first call to process events will rebuild the swap chain.
        m_swapChainNeedsRebuild = true;

        return true;
    }

    void ApplicationWindow::Internal_ProcessEvents()
    {
        // This function is called when all threads are synced.
        glfwPollEvents();
        
        // Check if the swapchain needs to be rebuilt. This is set any time
        // the window is resized or if the vsync setting is changed. 
        if (m_swapChainNeedsRebuild)
        {
            auto& app = Application::Get();
            app.Internal_OnWindowResize(m_desc.m_windowResolution.x, m_desc.m_windowResolution.y);

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

    void ApplicationWindow::SetIsMinimized(const bool minimized)
    {
        m_desc.m_isMinimized = minimized;
    }

    Vec2 ApplicationWindow::GetCursorPosition() const
    {
        double posX, posY;
        glfwGetCursorPos(checked_cast<GLFWwindow*>(m_subWindowLastUnderCursor), &posX, &posY);
        return Vec2(static_cast<float>(posX), static_cast<float>(posY));
    }

    void ApplicationWindow::SetVsync(const bool enabled)
    {
        if (enabled != m_desc.m_vsyncEnabled)
        {
            m_desc.m_vsyncEnabled = enabled;
            m_swapChainNeedsRebuild = true;
        }
    }

    void ApplicationWindow::SetCursorMode(const ECursorMode mode)
    {
        GLFWwindow* pWindow = checked_cast<GLFWwindow*>(m_subWindowWithFocus);
        SetGLFWCursorMode(pWindow, m_desc.m_cursorMode, mode);
        
        m_desc.m_cursorMode = mode;
    }
    
    void ApplicationWindow::Resize(const UVec2& extent)
    {
        Resize(extent.x, extent.y);
    }

    void ApplicationWindow::Resize(const uint32_t width, const uint32_t height)
    {
        GLFWwindow* pWindow = checked_cast<GLFWwindow*>(m_nativeWindow.m_glfw);
        glfwSetWindowSize(pWindow, static_cast<int>(width), static_cast<int>(height));
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
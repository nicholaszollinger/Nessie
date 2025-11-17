// HeadlessWindow.cpp
#include "HeadlessWindow.h"
#include "Nessie/Core/Config.h"

#include "Application.h"
#include "GLFW/GLFWInputConversions.h"
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
    bool HeadlessWindow::Internal_Init(Application& app, WindowDesc&& desc)
    {
        m_desc = std::move(desc);
        m_desc.m_windowResolution = { 0, 0 };
        m_desc.m_isResizable = false;
        m_desc.m_cursorMode = ECursorMode::Visible;

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

        // Hide the window.
        glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
        
        // macOS: The first time a window is created, the menu bar is created.
        // This is not desirable when writing a command line only application.
        // Menu bar creation can be disabled with the GLFW_COCOA_MENUBAR init hint.
        //glfwWindowHint(GLFW_COCOA_MENUBAR, GLFW_FALSE);

        GLFWwindow* pWindow = glfwCreateWindow(640, 480, "", nullptr, nullptr);
        m_nativeWindow.m_glfw = pWindow;
        
        #if NES_PLATFORM_WINDOWS
        m_nativeWindow.m_windows.m_hwnd = glfwGetWin32Window(pWindow);
        #endif

        glfwSetWindowUserPointer(pWindow, &app);

        // Window Close Callback.
        glfwSetWindowCloseCallback(pWindow, [](GLFWwindow* pWindow)
        {
            glfwSetWindowShouldClose(pWindow, GLFW_TRUE);
        });

        // Window Key Callback.
        glfwSetKeyCallback(pWindow, [](GLFWwindow* pWindow, const int key, [[maybe_unused]] const int scanCode, const int action, const int modifiers)
        {
            auto* pApp = checked_cast<Application*>(glfwGetWindowUserPointer(pWindow));

            const Modifiers mods = glfw::ConvertToModifiers(modifiers);
            const EKeyCode keyCode = glfw::ConvertToKeyCode(key);
            const EKeyAction keyAction = glfw::ConvertToKeyAction(action);

            KeyEvent event(keyCode, keyAction, mods);
            pApp->Internal_OnInputEvent(event);
        });

        // Mouse Button Callback.
        glfwSetMouseButtonCallback(pWindow, [](GLFWwindow* pWindow, const int button, const int action, const int modifiers)
        {
            auto* pApp = checked_cast<Application*>(glfwGetWindowUserPointer(pWindow));

            // Get the mouse position at the time of the event.
            double xPos, yPos;
            glfwGetCursorPos(pWindow, &xPos, &yPos);
            const Float2 mousePos(static_cast<float>(xPos), static_cast<float>(yPos));
            const EMouseButton mouseButton = glfw::ConvertToMouseButton(button);
            const Modifiers mods = glfw::ConvertToModifiers(modifiers);
            const EMouseAction mouseAction = glfw::ConvertToMouseAction(action);

            MouseButtonEvent event(mouseButton, mouseAction, mods, mousePos.x, mousePos.y);
            pApp->Internal_OnInputEvent(event);
        });

        // Mouse Scroll Callback.
        glfwSetScrollCallback(pWindow, [](GLFWwindow* pWindow, const double deltaX, const double deltaY)
        {
            auto* pApp = checked_cast<Application*>(glfwGetWindowUserPointer(pWindow));
            MouseScrollEvent event(static_cast<float>(deltaX), static_cast<float>(deltaY));
            pApp->Internal_OnInputEvent(event);
        });

        // Mouse Move Callback.
        glfwSetCursorPosCallback(pWindow, [](GLFWwindow* pWindow, const double xPos, const double yPos)
        {
            auto* pApp = checked_cast<Application*>(glfwGetWindowUserPointer(pWindow));
            MouseMoveEvent event(static_cast<float>(xPos), static_cast<float>(yPos));
            pApp->Internal_OnInputEvent(event);
        });

        return true;
    }

    void HeadlessWindow::Internal_ProcessEvents()
    {
        glfwPollEvents();
    }
}
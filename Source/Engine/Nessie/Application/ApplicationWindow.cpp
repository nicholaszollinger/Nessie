// ApplicationWindow.cpp
#include "Nessie/Core/Config.h"

#include "GLFW/GLFWInputConversions.h"
#include "Nessie/Application/Platform.h"
#include "Nessie/Debug/CheckedCast.h"
#include "GLFW/glfw3.h"
#include "Nessie/Math/Vec2.h"

namespace nes
{
    bool ApplicationWindow::Internal_Init(Platform& platform, const WindowDesc& desc)
    {
        m_description = desc;
        
    #ifdef NES_RENDER_API_VULKAN
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    #endif
        // Set whether the Window is resizable or not.
        glfwWindowHint(GLFW_RESIZABLE, desc.m_isResizable);
        
        GLFWwindow* pWindow = nullptr;
        switch (m_description.m_windowMode)
        {
            case EWindowMode::Fullscreen:
            {
                auto* pMonitor = glfwGetPrimaryMonitor();
                const auto* pMode = glfwGetVideoMode(pMonitor);
                m_description.m_windowResolution.x = pMode->width;
                m_description.m_windowResolution.y = pMode->height;
                pWindow = glfwCreateWindow(pMode->width, pMode->height, m_description.m_label.c_str(), pMonitor, nullptr);
                break;
            }

            case EWindowMode::FullscreenBorderless:
            {
                auto* pMonitor = glfwGetPrimaryMonitor();
                const auto* pMode = glfwGetVideoMode(pMonitor);

                m_description.m_windowResolution.x = pMode->width;
                m_description.m_windowResolution.y = pMode->height;

                glfwWindowHint(GLFW_RED_BITS, pMode->redBits);
			    glfwWindowHint(GLFW_GREEN_BITS, pMode->greenBits);
			    glfwWindowHint(GLFW_BLUE_BITS, pMode->blueBits);
			    glfwWindowHint(GLFW_REFRESH_RATE, pMode->refreshRate);

                pWindow = glfwCreateWindow(pMode->width, pMode->height, m_description.m_label.c_str(), pMonitor, nullptr);
                break;
            }

            case EWindowMode::Windowed:
            {
                const int width = static_cast<int>(m_description.m_windowResolution.x);
                const int height = static_cast<int>(m_description.m_windowResolution.y);
                pWindow = glfwCreateWindow(width, height, m_description.m_label.c_str(), nullptr, nullptr);
                break;
            }
        }

        if (!pWindow)
        {
            return false;
        }

        glfwSetWindowUserPointer(pWindow, &platform);
        m_pNativeWindowHandle = pWindow;

        // Set the GLFW Callbacks:
        // Window Resize Callback.
        glfwSetWindowSizeCallback(pWindow, []([[maybe_unused]] GLFWwindow* pWindow, const int width, const int height)
        {
            ApplicationWindow& window = Platform::GetWindow();
            window.m_description.m_windowResolution.x = width;
            window.m_description.m_windowResolution.y = height;
        });

        // Window Close Callback.
        glfwSetWindowCloseCallback(pWindow, [](GLFWwindow* pWindow)
        {
            glfwSetWindowShouldClose(pWindow, GLFW_TRUE);
        });

        // Window Key Callback.
        glfwSetKeyCallback(pWindow, [](GLFWwindow* pWindow, const int key, [[maybe_unused]] const int scanCode, const int action, const int modifiers)
        {
            auto* pPlatform = checked_cast<Platform*>(glfwGetWindowUserPointer(pWindow));

            const Modifiers mods = glfw::ConvertToModifiers(modifiers);
            const EKeyCode keyCode = glfw::ConvertToKeyCode(key);
            const EKeyAction keyAction = glfw::ConvertToKeyAction(action);

            KeyEvent event(keyCode, keyAction, mods);
            pPlatform->OnInputEvent(event);
        });

        // Mouse Button Callback.
        glfwSetMouseButtonCallback(pWindow, [](GLFWwindow* pWindow, const int button, const int action, const int modifiers)
        {
            auto* pPlatform = checked_cast<Platform*>(glfwGetWindowUserPointer(pWindow));

            // Get the mouse position at the time of the event.
            double xPos, yPos;
            glfwGetCursorPos(pWindow, &xPos, &yPos);
            const Float2 mousePos(static_cast<float>(xPos), static_cast<float>(yPos));
            const EMouseButton mouseButton = glfw::ConvertToMouseButton(button);
            const Modifiers mods = glfw::ConvertToModifiers(modifiers);
            const EMouseAction mouseAction = glfw::ConvertToMouseAction(action);

            MouseButtonEvent event(mouseButton, mouseAction, mods, mousePos.x, mousePos.y);
            pPlatform->OnInputEvent(event);
        });

        // Mouse Scroll Callback.
        glfwSetScrollCallback(pWindow, [](GLFWwindow* pWindow, const double deltaX, const double deltaY)
        {
            auto* pPlatform = checked_cast<Platform*>(glfwGetWindowUserPointer(pWindow));
            MouseScrollEvent event(static_cast<float>(deltaX), static_cast<float>(deltaY));
            pPlatform->OnInputEvent(event);
        });

        // Mouse Move Callback.
        glfwSetCursorPosCallback(pWindow, [](GLFWwindow* pWindow, const double xPos, const double yPos)
        {
            auto* pPlatform = checked_cast<Platform*>(glfwGetWindowUserPointer(pWindow));
            
            MouseMoveEvent event(static_cast<float>(xPos), static_cast<float>(yPos));
            pPlatform->OnInputEvent(event);
        });
        
        // Ensure that the first call to process events will rebuild the swap chain.
        m_swapChainNeedsRebuild = true;

        return true;
    }

    void ApplicationWindow::Internal_ProcessEvents()
    {
        // This function is called when all threads are synced.
        glfwPollEvents();
        
        // Check if the swapchain needs to be rebuilt: this 
        if (m_swapChainNeedsRebuild)
        {
            auto* pGlfwWindow = checked_cast<GLFWwindow*>(m_pNativeWindowHandle);
            auto* pPlatform = checked_cast<Platform*>(glfwGetWindowUserPointer(pGlfwWindow));
            pPlatform->OnWindowResize(m_description.m_windowResolution.x, m_description.m_windowResolution.y, m_description.m_vsyncEnabled);

            // Clear the flag.
            m_swapChainNeedsRebuild = false;
        }
    }

    void ApplicationWindow::Internal_Shutdown()
    {
        // Destroy the window.
        if (m_pNativeWindowHandle != nullptr)
        {
            glfwDestroyWindow(checked_cast<GLFWwindow*>(m_pNativeWindowHandle));
            m_pNativeWindowHandle = nullptr;
        }
    }

    void ApplicationWindow::Close()
    {
        glfwSetWindowShouldClose(checked_cast<GLFWwindow*>(m_pNativeWindowHandle),true);
    }

    bool ApplicationWindow::ShouldClose() const
    {
        return glfwWindowShouldClose(checked_cast<GLFWwindow*>(m_pNativeWindowHandle));
    }

    void ApplicationWindow::SetIsMinimized(const bool minimized)
    {
        m_description.m_isMinimized = minimized;
    }

    Vec2 ApplicationWindow::GetCursorPosition() const
    {
        double posX, posY;
        glfwGetCursorPos(checked_cast<GLFWwindow*>(m_pNativeWindowHandle), &posX, &posY);
        return Vec2(static_cast<float>(posX), static_cast<float>(posY));
    }

    void ApplicationWindow::SetVsync(const bool enabled)
    {
        if (enabled != m_description.m_vsyncEnabled)
        {
            m_description.m_vsyncEnabled = enabled;
            m_swapChainNeedsRebuild = true;
        }
    }

    void ApplicationWindow::SetCursorMode(const ECursorMode mode)
    {
        // [TODO]: This should probably be handled by the input manager.
        
        if (m_description.m_cursorMode == mode)
            return;
        
        GLFWwindow* pWindow = checked_cast<GLFWwindow*>(m_pNativeWindowHandle);
        
        glfwSetInputMode(pWindow, GLFW_CURSOR, glfw::ConvertToGLFWCursorMode(mode));

        // If we are setting the mouse cursor to disabled,
        if (glfwRawMouseMotionSupported())
        {
            if (mode == ECursorMode::Disabled)
            {
                glfwSetInputMode(pWindow, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
            }

            // If changing from disabled, change the raw mouse motion back. 
            else if (m_description.m_cursorMode == ECursorMode::Disabled)
            {
                glfwSetInputMode(pWindow, GLFW_RAW_MOUSE_MOTION, GLFW_FALSE);
            }
        }
        
        m_description.m_cursorMode = mode;
    }


    void ApplicationWindow::Resize(const UVec2& extent)
    {
        Resize(extent.x, extent.y);
    }

    void ApplicationWindow::Resize(const uint32_t width, const uint32_t height)
    {
        m_description.m_windowResolution.x = width;
        m_description.m_windowResolution.y = height;
        m_swapChainNeedsRebuild = true;
    }
}

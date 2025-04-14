// GLFW_Window.cpp

#include "Core/Config.h"
#ifdef NES_WINDOW_API_GLFW
#include "Core/Memory/Memory.h"
#include "GLFW_InputConversions.h"
#include "Application/Application.h"
#include "Debug/CheckedCast.h"
#include "GLFW/glfw3.h"

static void GLFW_ErrorCallback([[maybe_unused]] int error, [[maybe_unused]] const char* description);

namespace nes
{
    //----------------------------------------------------------------------------------------------------
    //		NOTES:
    //		
    ///		@brief : Creates the Window and sets up Window Callbacks.
    ///		@param app : Application creating this Window.
    ///		@param props : Properties for the Window.
    ///		@returns : False if there was an error setting up the Window.
    //----------------------------------------------------------------------------------------------------
    bool Window::Init(Application& app, const WindowProperties& props)
    {
        m_properties = props;

        // [Consider] Right now, I am only supporting a single window.
        // If I want to support more, I need to have GLFW initialization and cleanup happen
        // once. I also have to make sure that single resources in the RendererContext are
        // only created once.
        if (!glfwInit())
        {
            NES_ERRORV("GLFW", "GLFW could not be initialized!");
            return false;
        }

        glfwSetErrorCallback(GLFW_ErrorCallback);
        
#ifdef NES_RENDER_API_VULKAN
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
#endif
        // Set whether the Window is resizable or not.
        glfwWindowHint(GLFW_RESIZABLE, props.m_isResizable);
        
        GLFWwindow* pWindow = nullptr;
        switch (m_properties.m_windowMode)
        {
            case WindowMode::Fullscreen:
            {
                auto* pMonitor = glfwGetPrimaryMonitor();
                const auto* pMode = glfwGetVideoMode(pMonitor);
                m_properties.m_extent.m_width = pMode->width;
                m_properties.m_extent.m_height = pMode->height;
                pWindow = glfwCreateWindow(pMode->width, pMode->height, m_properties.m_label.c_str(), pMonitor, nullptr);
                break;
            }

            case WindowMode::FullscreenBorderless:
            {
                auto* pMonitor = glfwGetPrimaryMonitor();
                const auto* pMode = glfwGetVideoMode(pMonitor);

                m_properties.m_extent.m_width = pMode->width;
                m_properties.m_extent.m_height = pMode->height;

                glfwWindowHint(GLFW_RED_BITS, pMode->redBits);
			    glfwWindowHint(GLFW_GREEN_BITS, pMode->greenBits);
			    glfwWindowHint(GLFW_BLUE_BITS, pMode->blueBits);
			    glfwWindowHint(GLFW_REFRESH_RATE, pMode->refreshRate);

                pWindow = glfwCreateWindow(pMode->width, pMode->height, m_properties.m_label.c_str(), pMonitor, nullptr);
                break;
            }

            case WindowMode::Windowed:
            {
                const int width = static_cast<int>(m_properties.m_extent.m_width);
                const int height = static_cast<int>(m_properties.m_extent.m_height);
                pWindow = glfwCreateWindow(width, height, m_properties.m_label.c_str(), nullptr, nullptr);
                break;
            }
        }

        if (!pWindow)
        {
            glfwTerminate();
            return false;
        }

        glfwSetWindowUserPointer(pWindow, &app);
        m_pNativeWindowHandle = pWindow;

        // Set the GLFW Callbacks:
        // Window Resize Callback.
        glfwSetWindowSizeCallback(pWindow, [](GLFWwindow* pWindow, const int width, const int height)
        {
            Application* pApp = checked_cast<Application*>(glfwGetWindowUserPointer(pWindow));
            pApp->GetWindow().Resize(width, height);
        });

        // Window Close Callback.
        glfwSetWindowCloseCallback(pWindow, [](GLFWwindow* pWindow)
        {
            Application* pApp = checked_cast<Application*>(glfwGetWindowUserPointer(pWindow));
            pApp->Quit();
        });

        // Window Key Callback.
        glfwSetKeyCallback(pWindow, [](GLFWwindow* pWindow, const int key, [[maybe_unused]] const int scanCode, const int action, const int modifiers)
        {
            Application* pApp = checked_cast<Application*>(glfwGetWindowUserPointer(pWindow));

            const Modifiers mods = glfw::ConvertToModifiers(modifiers);
            const KeyCode keyCode = glfw::ConvertToKeyCode(key);
            const KeyAction keyAction = glfw::ConvertToKeyAction(action);

            KeyEvent event(keyCode, keyAction, mods);
            pApp->PushEvent(event);
        });

        // Mouse Button Callback.
        glfwSetMouseButtonCallback(pWindow, [](GLFWwindow* pWindow, const int button, const int action, const int modifiers)
        {
            Application* pApp = checked_cast<Application*>(glfwGetWindowUserPointer(pWindow));

            // Get the mouse position at the time of the event.
            double xPos, yPos;
            glfwGetCursorPos(pWindow, &xPos, &yPos);
            const Vector2 mousePos(static_cast<float>(xPos), static_cast<float>(yPos));
            const MouseButton mouseButton = glfw::ConvertToMouseButton(button);
            const Modifiers mods = glfw::ConvertToModifiers(modifiers);
            const MouseAction mouseAction = glfw::ConvertToMouseAction(action);

            MouseButtonEvent event(mouseButton, mouseAction, mods, mousePos.x, mousePos.y);
            pApp->PushEvent(event);
        });

        // Mouse Scroll Callback.
        glfwSetScrollCallback(pWindow, [](GLFWwindow* pWindow, const double deltaX, const double deltaY)
        {
            Application* pApp = checked_cast<Application*>(glfwGetWindowUserPointer(pWindow));

            MouseScrollEvent event(static_cast<float>(deltaX), static_cast<float>(deltaY));
            pApp->PushEvent(event);
        });

        // Mouse Move Callback.
        glfwSetCursorPosCallback(pWindow, [](GLFWwindow* pWindow, const double xPos, const double yPos)
        {
            Application* pApp = checked_cast<Application*>(glfwGetWindowUserPointer(pWindow));
            Window& window = pApp->GetWindow();

            // New Mouse Position.
            Vector2 position{static_cast<float>(xPos), static_cast<float>(yPos)};

            // Calculate the relative motion from the last cursor position of the mouse.
            Vector2 deltaPosition = position - window.m_cursorPosition;

            // Update the last cursor position.
            window.m_cursorPosition = position;

            MouseMoveEvent event(position.x, position.y, deltaPosition.x, deltaPosition.y);
            pApp->PushEvent(event);
        });

        // Set Window Size.
        glfwSetWindowSizeCallback(pWindow, [](GLFWwindow* pWindow, const int width, const int height)
        {
            Application* pApp = checked_cast<Application*>(glfwGetWindowUserPointer(pWindow));

            pApp->GetWindow().m_properties.m_extent = WindowExtent{static_cast<uint32_t>(width), static_cast<uint32_t>(height)};

            WindowResizeEvent event(width, height);
            pApp->PushEvent(event);
        });

        // [TODO]: 
        // FrameBuffer Resize Callback.
        glfwSetFramebufferSizeCallback(pWindow, [](GLFWwindow* pWindow, const int width, const int height)
        {
            Application* pApp = checked_cast<Application*>(glfwGetWindowUserPointer(pWindow));

            auto& window = pApp->GetWindow();

            // Window Minimized:
            if ((width == 0 || height == 0) && !window.IsMinimized())
            {
                pApp->GetWindow().SetIsMinimized(true);

                WindowMinimizeEvent event(true);
                pApp->PushEvent(event);
            }

            // If the window is un-minimized
            else if (window.IsMinimized())
            {
                pApp->GetWindow().SetIsMinimized(false);

                WindowMinimizeEvent event(false);
                pApp->PushEvent(event);
            }

            // Normal Resize:
            else
            {
                WindowResizeEvent event(width, height);
                pApp->GetWindow().m_properties.m_extent = WindowExtent{static_cast<uint32_t>(width), static_cast<uint32_t>(height)};

                pApp->PushEvent(event);
            }
        });

        // Set the initial cursor position.
        double xPos, yPos;
        glfwGetCursorPos(pWindow, &xPos, &yPos);
        m_cursorPosition.x = static_cast<float>(xPos);
        m_cursorPosition.y = static_cast<float>(yPos);

        return true;
    }

    void Window::ProcessEvents()
    {
        glfwPollEvents();
    }

    void Window::Close()
    {
        glfwDestroyWindow(checked_cast<GLFWwindow*>(m_pNativeWindowHandle));
        m_pNativeWindowHandle = nullptr;

        // [Consider]: This will destroy all Windows, so if you wanted multiple you
        // need to address it.
        glfwTerminate();
    }

    bool Window::ShouldClose()
    {
        return glfwWindowShouldClose(checked_cast<GLFWwindow*>(m_pNativeWindowHandle));
    }

    void Window::SetIsMinimized(const bool minimized)
    {
        m_properties.m_isMinimized = minimized;
    }

    void Window::SetVsync(const bool enabled)
    {
        m_properties.m_vsyncEnabled = enabled;
        glfwSwapInterval(enabled ? 1 : 0);
    }

    WindowExtent Window::Resize(const WindowExtent& extent)
    {
        return Resize(extent.m_width, extent.m_height);
    }

    WindowExtent Window::Resize(const uint32_t width, const uint32_t height)
    {
        GLFWwindow* pWindow = checked_cast<GLFWwindow*>(m_pNativeWindowHandle);
        glfwSetWindowSize(pWindow, static_cast<int>(width), static_cast<int>(height));
        m_properties.m_extent.m_width = width;
        m_properties.m_extent.m_height = height;
        return m_properties.m_extent;
    }
    
}

void GLFW_ErrorCallback([[maybe_unused]] int error, [[maybe_unused]] const char* description)
{
    NES_ERRORV("GLFW", "Error: ", error, " - ", description);
}

#endif
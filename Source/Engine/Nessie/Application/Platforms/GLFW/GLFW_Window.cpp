// GLFW_Window.cpp

#if NES_PLATFORM_WINDOWS
#include <GLFW/glfw3.h>
#include "Application/Application.h"
#include "Application/Platform.h"
#include "Application/Window.h"
#include "Debug/CheckedCast.h"
#include "Input/InputEvents.h"
#include "Math/Vector2.h"

static void GLFW_ErrorCallback([[maybe_unused]] int error, [[maybe_unused]] const char* description);

namespace nes
{
    static KeyCode GLFW_ConvertToKeyCode(const int key);
    static MouseButton GLFW_ConvertToMouseButton(const int button);
    static Modifiers GLFW_ConvertToModifiers(const int modifiers);
    static KeyAction GLFW_ConvertToKeyAction(const int action);
    static MouseAction GLFW_ConvertToMouseAction(const int action);

    //----------------------------------------------------------------------------------------------------
    //		NOTES:
    //		
    ///		@brief : Creates the Window and sets up Window Callbacks.
    ///		@param platform : Platform creating this Window.
    ///		@param props : Properties for the Window.
    ///		@returns : False if there was an error setting up the Window.
    //----------------------------------------------------------------------------------------------------
    bool Window::Init(Platform& platform, const WindowProperties& props)
    {
        m_properties = props;

        if (!glfwInit())
        {
            NES_ERRORV("GLFW", "GLFW could not be initialized!");
            return false;
        }

        glfwSetErrorCallback(&GLFW_ErrorCallback);

        // [TODO]: 
        // Set the Hint for Vulkan
        //glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

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

        glfwSetWindowUserPointer(pWindow, &platform);
        m_pNativeWindowHandle = pWindow;

        // Set the GLFW Callbacks:
        // Window Resize Callback.
        glfwSetWindowSizeCallback(pWindow, [](GLFWwindow* pWindow, const int width, const int height)
        {
            Platform* pPlatform = checked_cast<Platform*>(glfwGetWindowUserPointer(pWindow));
            pPlatform->GetWindow().Resize(width, height);
        });

        // Window Close Callback.
        glfwSetWindowCloseCallback(pWindow, [](GLFWwindow* pWindow)
        {
            Platform* pPlatform = checked_cast<Platform*>(glfwGetWindowUserPointer(pWindow));
            pPlatform->GetApplication().Quit();
        });

        // Window Key Callback.
        glfwSetKeyCallback(pWindow, [](GLFWwindow* pWindow, const int key, [[maybe_unused]] const int scanCode, const int action, const int modifiers)
        {
            Platform* pPlatform = checked_cast<Platform*>(glfwGetWindowUserPointer(pWindow));

            const Modifiers mods = GLFW_ConvertToModifiers(modifiers);
            const KeyCode keyCode = GLFW_ConvertToKeyCode(key);
            const KeyAction keyAction = GLFW_ConvertToKeyAction(action);

            KeyEvent event(keyCode, keyAction, mods);
            pPlatform->GetApplication().PushEvent(event);
        });

        // Mouse Button Callback.
        glfwSetMouseButtonCallback(pWindow, [](GLFWwindow* pWindow, const int button, const int action, const int modifiers)
        {
            Platform* pPlatform = checked_cast<Platform*>(glfwGetWindowUserPointer(pWindow));

            // Get the mouse position at the time of the event.
            double xPos, yPos;
            glfwGetCursorPos(pWindow, &xPos, &yPos);
            const Vec2 mousePos(static_cast<float>(xPos), static_cast<float>(yPos));
            const MouseButton mouseButton = GLFW_ConvertToMouseButton(button);
            const Modifiers mods = GLFW_ConvertToModifiers(modifiers);
            const MouseAction mouseAction = GLFW_ConvertToMouseAction(action);

            MouseButtonEvent event(mouseButton, mouseAction, mods, mousePos.x, mousePos.y);
            pPlatform->GetApplication().PushEvent(event);
        });

        // Mouse Scroll Callback.
        glfwSetScrollCallback(pWindow, [](GLFWwindow* pWindow, const double deltaX, const double deltaY)
        {
            Platform* pPlatform = checked_cast<Platform*>(glfwGetWindowUserPointer(pWindow));

            MouseScrollEvent event(static_cast<float>(deltaX), static_cast<float>(deltaY));
            pPlatform->GetApplication().PushEvent(event);
        });

        // Mouse Move Callback.
        glfwSetCursorPosCallback(pWindow, [](GLFWwindow* pWindow, const double xPos, const double yPos)
        {
            Platform* pPlatform = checked_cast<Platform*>(glfwGetWindowUserPointer(pWindow));
            Window& window = pPlatform->GetWindow();

            // New Mouse Position.
            Vec2 position{static_cast<float>(xPos), static_cast<float>(yPos)};

            // Calculate the relative motion from the last cursor position of the mouse.
            Vec2 deltaPosition = position - window.m_cursorPosition;

            // Update the last cursor position.
            window.m_cursorPosition = position;

            MouseMoveEvent event(position.x, position.y, deltaPosition.x, deltaPosition.y);
            pPlatform->GetApplication().PushEvent(event);
        });

        // Set Window Size.
        glfwSetWindowSizeCallback(pWindow, [](GLFWwindow* pWindow, const int width, const int height)
        {
            Platform* pPlatform = checked_cast<Platform*>(glfwGetWindowUserPointer(pWindow));

            pPlatform->GetWindow().m_properties.m_extent = WindowExtent{static_cast<uint32_t>(width), static_cast<uint32_t>(height)};

            WindowResizeEvent event(width, height);
            pPlatform->GetApplication().PushEvent(event);
        });

        // [TODO]: 
        // FrameBuffer Resize Callback.
        glfwSetFramebufferSizeCallback(pWindow, [](GLFWwindow* pWindow, const int width, const int height)
        {
            Platform* pPlatform = checked_cast<Platform*>(glfwGetWindowUserPointer(pWindow));

            auto& window = pPlatform->GetWindow();

            // Window Minimized:
            if ((width == 0 || height == 0) && !window.IsMinimized())
            {
                pPlatform->GetWindow().SetIsMinimized(true);

                WindowMinimizeEvent event(true);
                pPlatform->GetApplication().PushEvent(event);
            }

            // If the window is un-minimized
            else if (window.IsMinimized())
            {
                pPlatform->GetWindow().SetIsMinimized(false);

                WindowMinimizeEvent event(false);
                pPlatform->GetApplication().PushEvent(event);
            }

            // Normal Resize:
            else
            {
                WindowResizeEvent event(width, height);
                pPlatform->GetWindow().m_properties.m_extent = WindowExtent{static_cast<uint32_t>(width), static_cast<uint32_t>(height)};

                pPlatform->GetApplication().PushEvent(event);
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
        glfwTerminate();
        m_pNativeWindowHandle = nullptr;
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

namespace nes
{
    KeyCode GLFW_ConvertToKeyCode(const int key)
    {
        switch (key)
        {
            // Letters
            case GLFW_KEY_A: return nes::KeyCode::A;
            case GLFW_KEY_B: return nes::KeyCode::B;
            case GLFW_KEY_C: return nes::KeyCode::C;
            case GLFW_KEY_D: return nes::KeyCode::D;
            case GLFW_KEY_E: return nes::KeyCode::E;
            case GLFW_KEY_F: return nes::KeyCode::F;
            case GLFW_KEY_G: return nes::KeyCode::G;
            case GLFW_KEY_H: return nes::KeyCode::H;
            case GLFW_KEY_I: return nes::KeyCode::I;
            case GLFW_KEY_J: return nes::KeyCode::J;
            case GLFW_KEY_K: return nes::KeyCode::K;
            case GLFW_KEY_L: return nes::KeyCode::L;
            case GLFW_KEY_M: return nes::KeyCode::M;
            case GLFW_KEY_N: return nes::KeyCode::N;
            case GLFW_KEY_O: return nes::KeyCode::O;
            case GLFW_KEY_P: return nes::KeyCode::P;
            case GLFW_KEY_Q: return nes::KeyCode::Q;
            case GLFW_KEY_R: return nes::KeyCode::R;
            case GLFW_KEY_S: return nes::KeyCode::S;
            case GLFW_KEY_T: return nes::KeyCode::T;
            case GLFW_KEY_U: return nes::KeyCode::U;
            case GLFW_KEY_V: return nes::KeyCode::V;
            case GLFW_KEY_W: return nes::KeyCode::W;
            case GLFW_KEY_X: return nes::KeyCode::X;
            case GLFW_KEY_Y: return nes::KeyCode::Y;
            case GLFW_KEY_Z: return nes::KeyCode::Z;

            // Numbers
            case GLFW_KEY_0: return nes::KeyCode::Num0;
            case GLFW_KEY_1: return nes::KeyCode::Num1;
            case GLFW_KEY_2: return nes::KeyCode::Num2;
            case GLFW_KEY_3: return nes::KeyCode::Num3;
            case GLFW_KEY_4: return nes::KeyCode::Num4;
            case GLFW_KEY_5: return nes::KeyCode::Num5;
            case GLFW_KEY_6: return nes::KeyCode::Num6;
            case GLFW_KEY_7: return nes::KeyCode::Num7;
            case GLFW_KEY_8: return nes::KeyCode::Num8;
            case GLFW_KEY_9: return nes::KeyCode::Num9;

            // Navigation
            case GLFW_KEY_UP: return nes::KeyCode::Up;
            case GLFW_KEY_DOWN: return nes::KeyCode::Down;
            case GLFW_KEY_LEFT: return nes::KeyCode::Left;
            case GLFW_KEY_RIGHT: return nes::KeyCode::Right;
            case GLFW_KEY_PAGE_UP: return nes::KeyCode::PageUp;
            case GLFW_KEY_PAGE_DOWN: return nes::KeyCode::PageDown;
            case GLFW_KEY_HOME: return nes::KeyCode::Home;
            case GLFW_KEY_END: return nes::KeyCode::End;

            case GLFW_KEY_COMMA: return nes::KeyCode::Comma;
            case GLFW_KEY_PERIOD: return nes::KeyCode::Period;
            case GLFW_KEY_ESCAPE: return nes::KeyCode::Escape;
            case GLFW_KEY_SPACE: return nes::KeyCode::Space;
            case GLFW_KEY_ENTER: return nes::KeyCode::Enter;
            case GLFW_KEY_BACKSPACE: return nes::KeyCode::Backspace;
            case GLFW_KEY_DELETE: return nes::KeyCode::Delete;
            case GLFW_KEY_TAB: return nes::KeyCode::Tab;
            case GLFW_KEY_INSERT: return nes::KeyCode::Insert;
            case GLFW_KEY_CAPS_LOCK: return nes::KeyCode::Capslock;
            case GLFW_KEY_NUM_LOCK: return nes::KeyCode::NumLock;
            case GLFW_KEY_PRINT_SCREEN: return nes::KeyCode::PrintScreen;
            case GLFW_KEY_PAUSE: return nes::KeyCode::Pause;

            // Modifiers
            case GLFW_KEY_LEFT_CONTROL: return nes::KeyCode::LeftControl;
            case GLFW_KEY_RIGHT_CONTROL: return nes::KeyCode::RightControl;
            case GLFW_KEY_LEFT_SHIFT: return nes::KeyCode::LeftShift;
            case GLFW_KEY_RIGHT_SHIFT: return nes::KeyCode::RightShift;
            case GLFW_KEY_LEFT_ALT: return nes::KeyCode::LeftAlt;
            case GLFW_KEY_RIGHT_ALT: return nes::KeyCode::RightAlt;
            case GLFW_KEY_LEFT_SUPER: return nes::KeyCode::LeftSuper;
            case GLFW_KEY_RIGHT_SUPER: return nes::KeyCode::RightSuper;

            default: return nes::KeyCode::Unknown;
        }
    }

    MouseButton GLFW_ConvertToMouseButton(const int button)
    {
        if (button < GLFW_MOUSE_BUTTON_6)
	    {
		    return static_cast<MouseButton>(button);
	    }

	    return MouseButton::Unknown;
    }

    KeyAction GLFW_ConvertToKeyAction(const int action)
    {
        switch (action)
        {
            case GLFW_PRESS: return KeyAction::Pressed;
            case GLFW_RELEASE: return KeyAction::Released;
            case GLFW_REPEAT: return KeyAction::Repeat;
            default: return KeyAction::Unknown;
        }
    }

    MouseAction GLFW_ConvertToMouseAction(const int action)
    {
        switch (action)
        {
            case GLFW_PRESS: return MouseAction::Pressed;
            case GLFW_RELEASE: return MouseAction::Released;
            default: return MouseAction::Unknown;
        }
    }

    Modifiers GLFW_ConvertToModifiers(const int modifiers)
    {
        Modifiers result{};
        result.m_control    = modifiers & GLFW_MOD_CONTROL;
        result.m_alt        = modifiers & GLFW_MOD_ALT;
        result.m_shift      = modifiers & GLFW_MOD_SHIFT;
        result.m_super      = modifiers & GLFW_MOD_SUPER;
        result.m_capsLock   = modifiers & GLFW_MOD_CAPS_LOCK;
        result.m_numLock    = modifiers & GLFW_MOD_NUM_LOCK;
        return result;
    }
}

#endif

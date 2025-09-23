// InputManager.cpp
#include "InputManager.h"
#include "Nessie/Application/ApplicationWindow.h"
#include "Nessie/Debug/Assert.h"
#include "Nessie/Math/Vec2.h"

namespace nes
{
    static InputManager* g_pInstance = nullptr;
    
    bool InputManager::Init(ApplicationWindow* pWindow)
    {
        NES_ASSERT(g_pInstance == nullptr);
        NES_ASSERT(pWindow != nullptr);
        
        g_pInstance = this;
        m_pWindow = pWindow;

        // Initialize KeyStates
        for (auto& state : m_keyStates)
        {
            state = EKeyAction::Released;
        }

        // Initialize Mouse States
        for (auto& state : m_mouseButtonStates)
        {
            state = EMouseAction::Released;
        }
        
        m_cursorPosition = m_pWindow->GetCursorPosition();
        m_cursorDelta = {};
        return true;
    }

    void InputManager::Update([[maybe_unused]] const double deltaTime)
    {
        // Update cursor delta:
        const auto newPosition = m_pWindow->GetCursorPosition();
        m_cursorDelta = newPosition - m_cursorPosition;
        m_cursorPosition = newPosition;
        
        // [TODO Later]: Transition pressed keys to repeat if the delta is enough. 
        
        // [TODO Later]: Update key actions information.

        // [TODO Later]: Update controller states. 
    }

    void InputManager::OnInputEvent(Event& event)
    {
        // Update Key States.
        if (auto* pKeyEvent = event.Cast<KeyEvent>())
        {
            m_keyStates[static_cast<uint32>(pKeyEvent->GetKeyCode())] = pKeyEvent->GetAction();
        }

        // Mouse Button
        else if (auto* pMouseEvent = event.Cast<MouseButtonEvent>())
        {
            m_mouseButtonStates[static_cast<uint32>(pMouseEvent->GetButton())] = pMouseEvent->GetAction();
        }
    }

    void InputManager::Shutdown()
    {
        g_pInstance = nullptr;
        m_pWindow = nullptr;
    }
    
    bool InputManager::IsKeyDown(const EKeyCode key)
    {
        NES_ASSERT(g_pInstance != nullptr);
        
        const auto keyState = g_pInstance->m_keyStates[static_cast<uint32>(key)];
        return keyState == EKeyAction::Pressed || keyState == EKeyAction::Repeat; 
    }

    bool InputManager::IsKeyUp(const EKeyCode key)
    {
        NES_ASSERT(g_pInstance != nullptr);
        return g_pInstance->m_keyStates[static_cast<uint32>(key)] == EKeyAction::Released;
    }

    bool InputManager::IsMouseButtonDown(const EMouseButton button)
    {
        NES_ASSERT(g_pInstance != nullptr);
        return g_pInstance->m_mouseButtonStates[static_cast<uint32>(button)] == EMouseAction::Pressed;
    }

    bool InputManager::IsMouseButtonUp(const EMouseButton button)
    {
        NES_ASSERT(g_pInstance != nullptr);
        return g_pInstance->m_mouseButtonStates[static_cast<uint32>(button)] == EMouseAction::Released;
    }

    void InputManager::SetCursorMode(const ECursorMode mode)
    {
        NES_ASSERT(g_pInstance != nullptr);
        g_pInstance->m_pWindow->SetCursorMode(mode);
    }

    ECursorMode InputManager::GetCursorMode()
    {
        NES_ASSERT(g_pInstance != nullptr);
        return g_pInstance->m_pWindow->GetCursorMode();
    }

    Vec2 InputManager::GetCursorPosition()
    {
        NES_ASSERT(g_pInstance != nullptr);
        return g_pInstance->m_cursorPosition;
    }

    Vec2 InputManager::GetCursorDelta()
    {
        NES_ASSERT(g_pInstance != nullptr);
        return g_pInstance->m_cursorDelta;
    }
}

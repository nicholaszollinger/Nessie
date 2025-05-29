// InputManager.cpp

#include "InputManager.h"

#include "Application/Window.h"
#include "Debug/Assert.h"

namespace nes
{
    static InputManager* g_pInstance = nullptr;
    
    bool InputManager::Init(Window* pWindow)
    {
        NES_ASSERT(g_pInstance == nullptr);
        NES_ASSERT(pWindow != nullptr);
        
        g_pInstance = this;
        m_pWindow = pWindow;

        Vector2d pos = GetCursorPosition_Impl(m_pWindow->GetNativeWindowHandle());
        m_cursorPosition = pos.CastTo<float>();
        m_cursorDelta = {};
        return true;
    }

    void InputManager::Update([[maybe_unused]] const double deltaTime)
    {
        // Update cursor position:
        Vector2 currentCursorPos = GetCursorPosition_Impl(m_pWindow->GetNativeWindowHandle()).CastTo<float>();
        m_cursorDelta = currentCursorPos - m_cursorPosition;
        m_cursorPosition = currentCursorPos;

        // [TODO Later]: Update key state information.
    }

    void InputManager::Shutdown()
    {
        g_pInstance = nullptr;
        m_pWindow = nullptr;
    }
    
    bool InputManager::IsKeyDown(const EKeyCode key)
    {
        NES_ASSERT(g_pInstance != nullptr);
        return g_pInstance->IsKeyDown_Impl(g_pInstance->m_pWindow->GetNativeWindowHandle(), key);
    }

    bool InputManager::IsKeyUp(const EKeyCode key)
    {
        NES_ASSERT(g_pInstance != nullptr);
        return g_pInstance->IsKeyUp_Impl(g_pInstance->m_pWindow->GetNativeWindowHandle(), key);
    }

    bool InputManager::IsMouseButtonDown(const EMouseButton button)
    {
        NES_ASSERT(g_pInstance != nullptr);
        return g_pInstance->IsMouseButtonDown_Impl(g_pInstance->m_pWindow->GetNativeWindowHandle(), button);
    }

    bool InputManager::IsMouseButtonUp(const EMouseButton button)
    {
        NES_ASSERT(g_pInstance != nullptr);
        return g_pInstance->IsMouseButtonUp_Impl(g_pInstance->m_pWindow->GetNativeWindowHandle(), button);
    }

    void InputManager::SetCursorMode(const ECursorMode mode)
    {
        NES_ASSERT(g_pInstance != nullptr);
        g_pInstance->SetCursorMode_Impl(g_pInstance->m_pWindow->GetNativeWindowHandle(), mode);
    }

    ECursorMode InputManager::GetCursorMode()
    {
        NES_ASSERT(g_pInstance != nullptr);
        return g_pInstance->m_cursorMode;
    }

    Vector2 InputManager::GetCursorPosition()
    {
        NES_ASSERT(g_pInstance != nullptr);
        return g_pInstance->m_cursorPosition;
    }

    Vector2 InputManager::GetCursorDelta()
    {
        NES_ASSERT(g_pInstance != nullptr);
        return g_pInstance->m_cursorDelta;
    }
}
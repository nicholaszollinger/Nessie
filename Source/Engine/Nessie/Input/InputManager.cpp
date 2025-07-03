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

        const Double2 pos = GetCursorPosition_Impl(m_pWindow->GetNativeWindowHandle());
        m_cursorPosition = Vec2(pos.CastTo<float>());
        m_cursorDelta = {};
        return true;
    }

    void InputManager::Update([[maybe_unused]] const double deltaTime)
    {
        // Update cursor position:
        const Vec2 currentCursorPos = Vec2(GetCursorPosition_Impl(m_pWindow->GetNativeWindowHandle()).CastTo<float>());
        m_cursorDelta = Vec2(currentCursorPos) - Vec2(m_cursorPosition);
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

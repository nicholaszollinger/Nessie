// Window.h
#pragma once
#include <string>
#include "WindowEvents.h"
#include "Math/Vector2.h"

namespace nes
{
    class Platform;

    enum class EWindowMode
    {
        Windowed,
        Fullscreen,
        FullscreenBorderless,
    };

    struct WindowExtent
    {
        uint32_t m_width{};
        uint32_t m_height{};
    };
    
    struct WindowProperties
    {
        std::string     m_label         = "App Window";
        WindowExtent    m_extent        = { 1600, 900 };
        EWindowMode     m_windowMode    = EWindowMode::Windowed;
        bool            m_isResizable   = true;
        bool            m_vsyncEnabled  = false;
        bool            m_isMinimized   = false;
    };

    //----------------------------------------------------------------------------------------------------
    /// @brief : Base Window class created by the Application. 
    //----------------------------------------------------------------------------------------------------
    class Window
    {
        friend class Application;
    
    private:
        Window() = default;

    public:
        ~Window() = default;

        /// No Move or Copy
        Window(const Window&) = delete;
        Window& operator=(const Window&) = delete;
        Window(Window&&) noexcept = delete;
        Window& operator=(Window&&) noexcept = delete;

    public:
        //----------------------------------------------------------------------------------------------------
        /// @brief : Resize the window. 
        //----------------------------------------------------------------------------------------------------
        WindowExtent        Resize(const WindowExtent& extent);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Resize the window.
        //----------------------------------------------------------------------------------------------------
        WindowExtent        Resize(const uint32_t width, const uint32_t height);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the current extent, or size, of the window. The size will be in pixel dimensions. 
        //----------------------------------------------------------------------------------------------------
        const WindowExtent& GetExtent() const               { return m_properties.m_extent; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get whether the Window is in Fullscreen, Windowed, etc. 
        //----------------------------------------------------------------------------------------------------
        EWindowMode         GetWindowMode() const           { return m_properties.m_windowMode; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the current position of the cursor in the window. 
        //----------------------------------------------------------------------------------------------------
        const Vector2f&     GetCursorPosition() const       { return m_cursorPosition; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Check if the Window can be resized. 
        //----------------------------------------------------------------------------------------------------
        bool                IsResizable() const             { return m_properties.m_isResizable; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Set whether the Window should sync its framerate with the monitor. Only if supported. 
        //----------------------------------------------------------------------------------------------------
        void                SetVsync(const bool enabled);
        
        //----------------------------------------------------------------------------------------------------
        /// @brief : Check if the Vsync is enabled on the Window. 
        //----------------------------------------------------------------------------------------------------
        bool                IsVsyncEnabled() const          { return m_properties.m_vsyncEnabled; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Set whether the window is minimized or not. 
        //----------------------------------------------------------------------------------------------------
        void                SetIsMinimized(const bool minimized);
        
        //----------------------------------------------------------------------------------------------------
        /// @brief : Check if the Window is minimized. 
        //----------------------------------------------------------------------------------------------------
        bool                IsMinimized() const             { return m_properties.m_isMinimized; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the platform-specific raw window pointer. Only use if you know what you are doing.   
        //----------------------------------------------------------------------------------------------------
        [[nodiscard]] void* GetNativeWindowHandle() const   { return m_pNativeWindowHandle; }

    private:
        //----------------------------------------------------------------------------------------------------
        /// @brief : Initialize the Window. Returns false on failure.
        ///	@param app : Owning Application.
        ///	@param props : Properites requested by the Application.
        //----------------------------------------------------------------------------------------------------
        bool                Init(Application& app, const WindowProperties& props);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Process window events. Must be called every frame. 
        //----------------------------------------------------------------------------------------------------
        void                ProcessEvents();

        //----------------------------------------------------------------------------------------------------
        /// @brief : Check whether the Window needs to close. 
        //----------------------------------------------------------------------------------------------------
        bool                ShouldClose();

        //----------------------------------------------------------------------------------------------------
        /// @brief : Close the window.
        //----------------------------------------------------------------------------------------------------
        void                Close();
        
    private:
        WindowProperties    m_properties{};
        void*               m_pNativeWindowHandle = nullptr;
        Vector2f            m_cursorPosition{};
    };
}
// Window.h
#pragma once
#include <functional>
#include <string>
#include "Nessie/Application/WindowEvents.h"
#include "Nessie/Core/Memory/StrongPtr.h"
#include "Nessie/Math/Vec2.h"
#include "Nessie/Math/IVec2.h"
#include "Nessie/Input/Cursor.h"

namespace nes
{
    class Application;
    class Swapchain;
    class RendererContext;
    struct RendererDesc;
    struct ApplicationDesc;

    enum class EWindowMode
    {
        Windowed,
        Fullscreen,
        FullscreenBorderless,
    };
    
    //----------------------------------------------------------------------------------------------------
    /// @brief : Expects NES_PLATFORM_WINDOWS macro. 
    //----------------------------------------------------------------------------------------------------
    struct WindowsWindow
    {
        void*           m_hwnd = nullptr;
    };

    //----------------------------------------------------------------------------------------------------
    // [TODO]: Add other platforms as necessary. 
    /// @brief : Group of native window handles for different platforms. Only one will be valid, depending
    ///     on the current platform.
    //----------------------------------------------------------------------------------------------------
    struct NativeWindow
    {
        void*           m_glfw = nullptr; /// GLFW Window* 
        WindowsWindow   m_windows{};
    };

    struct WindowState
    {
        IVec2           m_position{};
        IVec2           m_resolution{};
    };

    //----------------------------------------------------------------------------------------------------
    /// @brief : Various properties about the window. Used in Window creation as well.
    //----------------------------------------------------------------------------------------------------
    struct WindowDesc
    {
        std::string     m_label         = "App Window";

        /// Window resolution
        UVec2           m_windowResolution { 1600, 900 };
        UVec2           m_windowPosition   { 0, 0 };
        EWindowMode     m_windowMode    = EWindowMode::Windowed;
        ECursorMode     m_cursorMode    = ECursorMode::Visible;
        bool            m_isResizable   = true;
        bool            m_vsyncEnabled  = false;
        bool            m_isDecorated   = true;

        WindowDesc&     SetLabel(const char* label)             { m_label = label; return *this; }
        WindowDesc&     SetResolution(const uint32 width, const uint32 height) { m_windowResolution = {width, height}; return *this; }
        WindowDesc&     SetWindowMode(const EWindowMode mode)   { m_windowMode = mode; return *this; }
        WindowDesc&     EnableVsync(const bool enabled)         { m_vsyncEnabled = enabled; return *this; }
        WindowDesc&     EnableResize(const bool enabled)        { m_isResizable = enabled; return *this; }
        WindowDesc&     RemoveDefaultDecoration()               { m_isDecorated = false; return *this; }
    };

    //----------------------------------------------------------------------------------------------------
    /// @brief : Base Window class created by the Application. 
    //----------------------------------------------------------------------------------------------------
    class ApplicationWindow
    {
    public:
        enum class ERequestedModeChange : uint8
        {
            None = 0,
            Minimize = NES_BIT(0),
            Maximize = NES_BIT(1),
            Restore = NES_BIT(2),
        };
        
    public:
        ApplicationWindow() = default;
        virtual ~ApplicationWindow() = default;

        /// No Move or Copy
        ApplicationWindow(const ApplicationWindow&) = delete;
        ApplicationWindow& operator=(const ApplicationWindow&) = delete;
        ApplicationWindow(ApplicationWindow&&) noexcept = delete;
        ApplicationWindow& operator=(ApplicationWindow&&) noexcept = delete;

    public:
        //----------------------------------------------------------------------------------------------------
        /// @brief : The Application window starts off hidden until explicitly shown. This must be called to
        ///     reveal the window.
        //----------------------------------------------------------------------------------------------------
        void                    ShowWindow();
        
        //----------------------------------------------------------------------------------------------------
        /// @brief : Resize the window. 
        //----------------------------------------------------------------------------------------------------
        void                    Resize(const UVec2& extent);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Resize the window.
        //----------------------------------------------------------------------------------------------------
        virtual void            Resize(const uint32 width, const uint32 height);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the current resolution of the window. The size will be in pixel dimensions. 
        //----------------------------------------------------------------------------------------------------
        IVec2                   GetResolution() const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Set the position of the window. 
        //----------------------------------------------------------------------------------------------------
        void                    SetPosition(const int x, const int y);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Center the window on the current monitor.
        //----------------------------------------------------------------------------------------------------
        void                    CenterWindow();

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the size and position that the window will be restored to when un-maximizing.
        //----------------------------------------------------------------------------------------------------
        WindowState             GetWindowRestoreState() const                   { return m_restoreState; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Advanced use. Set the size and position of the window when un-maximizing the window.
        //----------------------------------------------------------------------------------------------------
        void                    SetWindowRestoreState(const WindowState& state) { m_restoreState = state; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Advanced use. Set the size of the window when un-maximizing the window. This will set the
        /// restore position to center the window.
        //----------------------------------------------------------------------------------------------------
        void                    SetWindowRestoreStateCentered(const int width, const int height);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get whether the Window is in Fullscreen, Windowed, etc. 
        //----------------------------------------------------------------------------------------------------
        EWindowMode             GetWindowMode() const                           { return m_windowMode; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the current relative position of the cursor in the window. The window origin is
        ///     the top left of the window's "content area" (not including title bar). X-axis: left->right,
        ///     Y-axis: top->bottom. Position range will be [0, 1].
        //----------------------------------------------------------------------------------------------------
        Vec2                    GetCursorPosition() const;       

        //----------------------------------------------------------------------------------------------------
        /// @brief : Check if the Window can be resized. 
        //----------------------------------------------------------------------------------------------------
        bool                    IsResizable() const                             { return m_isResizable; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Set whether the Window should sync its framerate with the monitor. Only if supported. 
        //----------------------------------------------------------------------------------------------------
        virtual void            SetVsync(const bool enabled);
        
        //----------------------------------------------------------------------------------------------------
        /// @brief : Check if the Vsync is enabled on the Window. 
        //----------------------------------------------------------------------------------------------------
        bool                    IsVsyncEnabled() const                          { return m_vsyncEnabled; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Set whether the window is minimized or not. 
        //----------------------------------------------------------------------------------------------------
        virtual void            SetMinimized(const bool minimized);
        
        //----------------------------------------------------------------------------------------------------
        /// @brief : Check if the Window is minimized. 
        //----------------------------------------------------------------------------------------------------
        bool                    IsMinimized() const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Set whether the window is in fullscreen mode.
        //----------------------------------------------------------------------------------------------------
        void                    SetMaximized(const bool enabled);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Check if the window is fullscreen.
        //----------------------------------------------------------------------------------------------------
        bool                    IsMaximized() const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Set how the cursor interacts with the window. 
        //----------------------------------------------------------------------------------------------------
        virtual void            SetCursorMode(const ECursorMode mode);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the current cursor mode.
        //----------------------------------------------------------------------------------------------------
        ECursorMode             GetCursorMode() const           { return m_cursorMode; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Determine if this is the main application window - if the main application window is closed,
        ///     then the Application will close.
        //----------------------------------------------------------------------------------------------------
        bool                    IsMainApplicationWindow() const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Check whether the Window needs to close. 
        //----------------------------------------------------------------------------------------------------
        bool                    ShouldClose() const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Close the window.
        //----------------------------------------------------------------------------------------------------
        void                    Close();
        
        //----------------------------------------------------------------------------------------------------
        /// @brief : Advanced use. Get the native window handle for the platform.
        //----------------------------------------------------------------------------------------------------
        const NativeWindow&     GetNativeWindow() const         { return m_nativeWindow; }
        
        //----------------------------------------------------------------------------------------------------
        /// @brief : Initialize the Window. Returns false on failure.
        ///	@param app : The Application that has created this window.
        ///	@param desc : Window properties requested by the Application.
        //----------------------------------------------------------------------------------------------------
        virtual bool            Internal_Init(Application& app, const WindowDesc& desc);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Process window events. Must be called every frame, when all threads are synced. 
        //----------------------------------------------------------------------------------------------------
        virtual void            Internal_ProcessEvents();

        //----------------------------------------------------------------------------------------------------
        /// @brief : Destroy the window instance.
        //----------------------------------------------------------------------------------------------------
        void                    Internal_Shutdown();
    
    protected:
        std::string             m_label{};
        NativeWindow            m_nativeWindow{};                   // Platform specific window handles, and the GLFW Window*.
        WindowState             m_restoreState{};                   // Saved Window Properties to restore to when maximizing.
        void*                   m_subWindowWithFocus = nullptr;
        void*                   m_subWindowLastUnderCursor = nullptr;
        ECursorMode             m_cursorMode = ECursorMode::Visible;
        EWindowMode             m_windowMode = EWindowMode::Windowed;
        bool                    m_swapChainNeedsRebuild = false;    // Flag to determine if the Renderer needs to update the swap chain.
        bool                    m_isResizable = true;
        bool                    m_vsyncEnabled = false;
        
    private:
        // [TODO]: I am handling deferred minimize/maximize and restore with these flags for now.
        // Should probably be changed later:
        void                    ApplyPendingStateChanges();
        bool                    RequestedStateChange() const { return m_requestedState != ERequestedModeChange::None; }
        ERequestedModeChange    m_requestedState = ERequestedModeChange::None;
    };

    NES_DEFINE_BIT_OPERATIONS_FOR_ENUM(ApplicationWindow::ERequestedModeChange)
}
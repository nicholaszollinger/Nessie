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
    class Platform;
    struct ApplicationDesc;

    enum class EWindowMode
    {
        Windowed,
        Fullscreen,
        FullscreenBorderless,
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
        bool            m_isMinimized   = false;

        WindowDesc&     SetLabel(const char* label)             { m_label = label; return *this; }
        WindowDesc&     SetResolution(const uint32 width, const uint32 height) { m_windowResolution = {width, height}; return *this; }
        WindowDesc&     SetWindowMode(const EWindowMode mode)   { m_windowMode = mode; return *this; }
        WindowDesc&     EnableVsync(const bool enabled)         { m_vsyncEnabled = enabled; return *this; }
        WindowDesc&     EnableResize(const bool enabled)        { m_isResizable = enabled; return *this; }
    };

    //----------------------------------------------------------------------------------------------------
    /// @brief : Base Window class created by the Application. 
    //----------------------------------------------------------------------------------------------------
    class ApplicationWindow
    {
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
        UVec2                   GetResolution() const           { return m_description.m_windowResolution; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get whether the Window is in Fullscreen, Windowed, etc. 
        //----------------------------------------------------------------------------------------------------
        EWindowMode             GetWindowMode() const           { return m_description.m_windowMode; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the current relative position of the cursor in the window. The window origin is
        ///     the top left of the window's "content area" (not including title bar). X-axis: left->right,
        ///     Y-axis: top->bottom. Position range will be [0, 1].
        //----------------------------------------------------------------------------------------------------
        Vec2                    GetCursorPosition() const;       

        //----------------------------------------------------------------------------------------------------
        /// @brief : Check if the Window can be resized. 
        //----------------------------------------------------------------------------------------------------
        bool                    IsResizable() const             { return m_description.m_isResizable; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Set whether the Window should sync its framerate with the monitor. Only if supported. 
        //----------------------------------------------------------------------------------------------------
        virtual void            SetVsync(const bool enabled);
        
        //----------------------------------------------------------------------------------------------------
        /// @brief : Check if the Vsync is enabled on the Window. 
        //----------------------------------------------------------------------------------------------------
        bool                    IsVsyncEnabled() const          { return m_description.m_vsyncEnabled; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Set whether the window is minimized or not. 
        //----------------------------------------------------------------------------------------------------
        virtual void            SetIsMinimized(const bool minimized);
        
        //----------------------------------------------------------------------------------------------------
        /// @brief : Check if the Window is minimized. 
        //----------------------------------------------------------------------------------------------------
        bool                    IsMinimized() const             { return m_description.m_isMinimized; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Set how the cursor interacts with the window. 
        //----------------------------------------------------------------------------------------------------
        virtual void            SetCursorMode(const ECursorMode mode);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the current cursor mode.
        //----------------------------------------------------------------------------------------------------
        ECursorMode             GetCursorMode() const           { return m_description.m_cursorMode; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Check whether the Window needs to close. 
        //----------------------------------------------------------------------------------------------------
        bool                    ShouldClose() const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Close the window.
        //----------------------------------------------------------------------------------------------------
        void                    Close();

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the current properties of the application window. 
        //----------------------------------------------------------------------------------------------------
        const WindowDesc&       GetDesc() const                 { return m_description; }
        
        //----------------------------------------------------------------------------------------------------
        /// @brief : Advanced use. Get the platform-specific raw window pointer. Only use if you know what you are doing.   
        //----------------------------------------------------------------------------------------------------
        [[nodiscard]] void*     GetNativeWindowHandle() const   { return m_pNativeWindowHandle; }
        
        //----------------------------------------------------------------------------------------------------
        /// @brief : Initialize the Window. Returns false on failure.
        ///	@param platform : The platform that has created this window.
        ///	@param desc : Window properties requested by the Application.
        //----------------------------------------------------------------------------------------------------
        virtual bool            Internal_Init(Platform& platform, const WindowDesc& desc);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Process window events. Must be called every frame, when all threads are synced. 
        //----------------------------------------------------------------------------------------------------
        virtual void            Internal_ProcessEvents();

        //----------------------------------------------------------------------------------------------------
        /// @brief : Destroy the window instance.
        //----------------------------------------------------------------------------------------------------
        void                    Internal_Shutdown();
    
    protected:
        WindowDesc              m_description;                      /// Current window properties.
        void*                   m_pNativeWindowHandle = nullptr;    /// Raw pointer to the window native window implementation.
        bool                    m_swapChainNeedsRebuild = false;    /// Flag to determine if the Renderer needs to update the swap chain.
    };
}
// InputCodes.h
#pragma once

namespace nes
{
    enum class EKeyCode : int
    {
        Unknown,
        A,
        B,
        C,
        D,
        E,
        F,
        G,
        H,
        I,
        J,
        K,
        L,
        M,
        N,
        O,
        P,
        Q,
        R,
        S,
        T,
        U,
        V,
        W,
        X,
        Y,
        Z,

        Num0,
        Num1,
        Num2,
        Num3,
        Num4,
        Num5,
        Num6,
        Num7,
        Num8,
        Num9,

        Space,
        Enter,
        Escape,
        Backspace,
        Delete,
        Tab,
        Insert,
        Capslock,
        NumLock,
        PrintScreen,
        Pause,
        Comma,
        Period,
        Exclamation,
        At,         /* @ */
        Pound,
        Dollar,
        Percent,
        CaretUp,
        And,
        Star,
        LeftParen,
        RightParen,
        LeftBracket,  /* [ */
	    RightBracket, /* \ */
        Backslash,    /* ] */
        GraveAccent,  /* ` */

        // Navigation
        Up,
        Down,
        Left,
        Right,
        PageUp,
        PageDown,
        Home,
        End,

        // Modifier Keys
        LeftControl,
        LeftShift,
        LeftAlt,
        LeftSuper,
        RightControl,
        RightShift,
        RightAlt,
        RightSuper,
    };

    enum class EKeyAction : int
    {
        Pressed,
        Released,
        Repeat,
        Unknown,
    };

    struct Modifiers
    {
        bool m_shift    = false;
        bool m_control  = false;
        bool m_alt      = false;
        bool m_super    = false;
        bool m_capsLock = false;
        bool m_numLock  = false;
    };

    enum class EMouseButton : int
    {
        Left,
        Right,
        Middle,
        Back,
        Forward,
        Unknown,
    };

    enum class EMouseAction : int
    {
        Pressed,
        Released,
        Move,
        Unknown,
    };
}
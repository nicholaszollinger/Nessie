// WarningSuppression.h
#pragma once

/// MSVC Warnings
#ifdef NES_COMPILER_MSVC

#define NES_PRAGMA(x) __pragma(x) 
#define NES_SUPPRESS_WARNINGS_BEGIN     NES_PRAGMA(warning (push))
#define NES_SUPPRESS_WARNINGS_END       NES_PRAGMA(warning (pop))
#define NES_MSVC_SUPPRESS_WARNING(w)    NES_PRAGMA(warning (disable : w))

//----------------------------------------------------------------------------------------------------
/// @brief : Suppresses Warning: "'X': 'Y' bytes padding added after data member 'Z'"
//----------------------------------------------------------------------------------------------------
#define NES_SUPPRESS_WARNING_MEMBER_PADDING_ADDED NES_MSVC_SUPPRESS_WARNING(4820)

//----------------------------------------------------------------------------------------------------
/// @brief : Suppresses Warning: "'X': structure was padded due to alignment specifier"
//----------------------------------------------------------------------------------------------------
#define NES_SUPPRESS_WARNING_STRUCTURE_PADDING_ADDED NES_MSVC_SUPPRESS_WARNING(4342)

//----------------------------------------------------------------------------------------------------
/// @brief : Suppresses Warning: "nonstandard extension used: nameless struct/union"
//----------------------------------------------------------------------------------------------------
#define NES_SUPPRESS_WARNING_NAMELESS_STRUCT NES_MSVC_SUPPRESS_WARNING(4201)

#else
#define NES_MSVC_SUPPRESS_WARNING(w)
#endif

/// GCC Warnings
#if defined(NES_COMPILER_GCC)
    #define NES_PRAGMA(x) _Pragma(#x)
    #define NES_SUPPRESS_WARNINGS_BEGIN     NES_PRAGMA(GCC diagnostic push)
    #define NES_SUPPRESS_WARNINGS_END       NES_PRAGMA(GCC diagnostic pop)
    #define NES_GCC_SUPPRESS_WARNING(w)     NES_PRAGMA(GCC diagnostic ignored w)

    //----------------------------------------------------------------------------------------------------
    /// @brief : Suppresses Warning: "'X': 'Y' bytes padding added after data member 'Z'"
    //----------------------------------------------------------------------------------------------------
    #define NES_SUPPRESS_WARNING_MEMBER_PADDING_ADDED

    //----------------------------------------------------------------------------------------------------
    /// @brief : Suppresses Warning: "'X': structure was padded due to alignment specifier"
    //----------------------------------------------------------------------------------------------------
    #define NES_SUPPRESS_WARNING_STRUCTURE_PADDING_ADDED

    //----------------------------------------------------------------------------------------------------
    /// @brief : Suppresses Warning: "nonstandard extension used: nameless struct/union"
    //----------------------------------------------------------------------------------------------------
    #define NES_SUPPRESS_WARNING_NAMELESS_STRUCT NES_GCC_SUPPRESS_WARNING("-Wpedantic")

#else
    #define NES_GCC_SUPPRESS_WARNING(w)
#endif

/// CLANG Warnings
#if defined(NES_COMPILER_CLANG)
    #define NES_PRAGMA(x) _Pragma(#x)
    #define NES_SUPPRESS_WARNINGS_BEGIN     NES_PRAGMA(clang diagnostic push)
    #define NES_SUPPRESS_WARNINGS_END       NES_PRAGMA(clang diagnostic pop)
    #define NES_CLANG_SUPPRESS_WARNING(w)   NES_PRAGMA(clang diagnostic ignored w)

    //----------------------------------------------------------------------------------------------------
    /// @brief : Suppresses Warning: "'X': 'Y' bytes padding added after data member 'Z'"
    //----------------------------------------------------------------------------------------------------
    #define NES_SUPPRESS_WARNING_MEMBER_PADDING_ADDED

    //----------------------------------------------------------------------------------------------------
    /// @brief : Suppresses Warning: "'X': structure was padded due to alignment specifier"
    //----------------------------------------------------------------------------------------------------
    #define NES_SUPPRESS_WARNING_STRUCTURE_PADDING_ADDED

    //----------------------------------------------------------------------------------------------------
    /// @brief : Suppresses Warning: "nonstandard extension used: nameless struct/union"
    //----------------------------------------------------------------------------------------------------
    #define NES_SUPPRESS_WARNING_NAMELESS_STRUCT \
        NES_CLANG_SUPPRESS_WARNING("-Wgnu-anonymous-struct")\
        NES_CLANG_SUPPRESS_WARNING("-Wnested-anon-types")

#else
    #define NES_CLANG_SUPPRESS_WARNING(w)

#endif





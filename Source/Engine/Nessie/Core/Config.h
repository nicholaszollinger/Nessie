#pragma once
// Config.h
// This file will contain core configuration macros for Nessie.

// Determine compiler
#if defined(__clang__)
    #define NES_COMPILER_CLANG
#elif defined(__GNUC__)
    #define NES_COMPILER_GCC
#elif defined(_MSC_VER)
    #define NES_COMPILER_MSVC
#endif

// CPU Architecture
#if defined(__x86_64__) || defined(_M_X64)
    #define NES_CPU_X86
    #if defined(_M_X64)
        #define NES_CPU_ADDRESS_BITS 64
    #else
        #define NES_CPU_ADDRESS_BITS 32
    #endif
    //#define NES_USE_SSE
    #define NES_VECTOR_ALIGNMENT 16
    #define NES_DVECTOR_ALIGNMENT 32
#else
#error "Nessie not setup for current Architecture!!!"
#endif

// Use "Trailing Zero Count" instruction.
//#define NES_USE_TZCNT
// Use "Leading Zero Count" instruction.
//#define NES_USE_LZCNT

// Macro to get the current function name.
#if defined(NES_COMPILER_MSVC)
    #define NES_FUNCTION_NAME __FUNCTION__
#endif

// Macro for a BreakPoint
#if defined(NES_DEBUG)
    #if defined(NES_COMPILER_MSVC)
        #define NES_BREAKPOINT __debugbreak()
    #else
        #error "No Breakpoint macro setup for current compiler."
    #endif

#else
    #define NES_BREAKPOINT void(0)
#endif

// Define Macro for inline
#if defined(NES_NO_FORCE_INLINE)
    #define NES_INLINE inline
#elif defined(NES_COMPILER_MSVC)
    #define NES_INLINE __forceinline
#else
#error "Undefined Inline macro for current compiler."
#endif

#ifndef NES_CACHE_LINE_SIZE
    #define NES_CACHE_LINE_SIZE 64
#endif

#define NOMINMAX
#ifdef NES_USE_SSE
    #include <immintrin.h>
#endif

// Shorthand for #ifdef NES_DEBUG ... #endif
// Should use only for single line operations. 
#ifdef NES_DEBUG
    #define NES_IF_DEBUG(...) __VA_ARGS__
    #define NES_IF_NOT_DEBUG(...)
#else
    #define NES_IF_DEBUG(...)
    #define NES_IF_NOT_DEBUG(...) __VA_ARGS__ 
#endif

// Multithreading Configuration 
// Define to force single threaded applications.
//#define NES_FORCE_SINGLE_THREADED

#ifdef NES_FORCE_SINGLE_THREADED
    #define NES_IF_SINGLE_THREADED(...) __VA_ARGS__
    #define NES_IF_MULTITHREADED(...)
#else
    #define NES_IF_SINGLE_THREADED(...)
    #define NES_IF_MULTITHREADED(...) __VA_ARGS__
#endif

/// SDL
#if defined(_RENDER_API_SDL)
#define NES_RENDER_API_SDL
#define NES_WINDOW_API_SDL

/// Vulkan
#elif defined(_RENDER_API_VULKAN)
#define NES_RENDER_API_VULKAN
#define NES_WINDOW_API_GLFW

#else
#error "No Valid Window & RenderAPI found! 
#endif

/// Enabling Asserts
#ifndef NES_RELEASE
    #define NES_ASSERTS_ENABLED 1
#else
    #define NES_ASSERTS_DISABLED 0
#endif

#if NES_ASSERTS_ENABLED
    #define NES_IF_ASSERTS_ENABLED(...) __VA_ARGS__
    #define NES_IF_ASSERTS_DISABLED(...)
#else
    #define NES_IF_ASSERTS_ENABLED(...)
    #define NES_IF_ASSERTS_DISABLED(...) __VA_ARGS__
#endif

// Standard Types
#include <cstdint>
using uint = unsigned int;
using uint8 = std::uint8_t;
using uint16 = std::uint16_t;
using uint32 = std::uint32_t;
using uint64 = std::uint64_t;

#include "WarningSuppression.h"

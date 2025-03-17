#pragma once
// Config.h
// This file will contain core configuration macros for Nessie.

// SDL
#if defined(_RENDER_API_SDL)
#define NES_RENDER_API_SDL
#define NES_WINDOW_API_SDL

// Vulkan
#elif defined(_RENDER_API_VULKAN)
#define NES_RENDER_API_VULKAN
#define NES_WINDOW_API_GLFW

#else
#error "No Valid Window & RenderAPI found! 
#endif

// Swapchain.cpp
#include "Swapchain.h"

// #ifdef NES_RENDER_API_VULKAN
// #include "Nessie/Graphics/Vulkan/Vulkan_Swapchain.h"
// #endif
//
// namespace nes
// {
//     Swapchain* Swapchain::Create()
//     {
// #ifdef NES_RENDER_API_VULKAN
//         return NES_NEW(Vulkan_Swapchain());
// #else
//         NES_ASSERT(false, "No swapchain implementation for the current Render API!");
//         return nullptr;
// #endif
//     }
//
// }
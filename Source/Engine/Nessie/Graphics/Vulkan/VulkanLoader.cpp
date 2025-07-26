// VulkanLoader.cpp
#include "VulkanLoader.h"

namespace nes
{
    // Initialize both to nullptr.
    PFN_vkGetInstanceProcAddr VulkanLoader::s_getInstanceProcAddr = nullptr;
    void* VulkanLoader::s_library = nullptr;
    
    PFN_vkGetInstanceProcAddr VulkanLoader::LoadVulkanLibrary()
    {
        // Exit early if already loaded.
        if (s_getInstanceProcAddr != nullptr)
            return s_getInstanceProcAddr;

#if NES_PLATFORM_WINDOWS
        s_library = LoadLibrary(TEXT("vulkan-1.dll"));
        
#else
        NES_ASSERT(false, "No support for Vulkan Library on current Platform!");
#endif

        if (!s_library)
            return nullptr;

        s_getInstanceProcAddr = reinterpret_cast<PFN_vkGetInstanceProcAddr>(GetProcAddress(static_cast<HMODULE>(s_library), "vkGetInstanceProcAddr"));
        return s_getInstanceProcAddr;
    }

    void VulkanLoader::UnloadLibrary()
    {
        if (s_library != nullptr)
        {
            FreeLibrary(static_cast<HMODULE>(s_library));
            s_library = nullptr;
            s_getInstanceProcAddr = nullptr;
        }
    }
}

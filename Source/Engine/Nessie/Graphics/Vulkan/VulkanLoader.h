// VulkanLoader.h
#pragma once
#include "VulkanCore.h"

namespace nes
{
    //----------------------------------------------------------------------------------------------------
    /// @brief : Class that handles loading/unloading the Vulkan Library and getting function pointers
    ///     from it.
    //----------------------------------------------------------------------------------------------------
    class VulkanLoader
    {
    public:
        //----------------------------------------------------------------------------------------------------
        /// @brief : Load the Vulkan Library. Returns the function to get instance functions. The result will
        ///     be nullptr if there was an error.
        //----------------------------------------------------------------------------------------------------
        static PFN_vkGetInstanceProcAddr   LoadVulkanLibrary();

        //----------------------------------------------------------------------------------------------------
        /// @brief : Unload the Vulkan Library. 
        //----------------------------------------------------------------------------------------------------
        static void                        UnloadLibrary();
        
    private:
        static PFN_vkGetInstanceProcAddr   s_getInstanceProcAddr;
        static void*                       s_library;
    };
}

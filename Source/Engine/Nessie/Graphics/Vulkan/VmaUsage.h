// VmaUsage.h
#pragma once

//------------------------------------------------------------------------------------
// Include this file rather than using "vk_mem_alloc.h" directly. 
//------------------------------------------------------------------------------------

// Set to zero because we use the 'volk' library.
#define VMA_STATIC_VULKAN_FUNCTIONS 0
#define VMA_DYNAMIC_VULKAN_FUNCTIONS 0

// If not defined, define an empty version of it, as VMA enables Volk functions based on
// whether this is defined or not.
//#ifndef VOLK_HEADER_VERSION
//#define VOLK_HEADER_VERSION
//#endif

#include "vk_mem_alloc.h"

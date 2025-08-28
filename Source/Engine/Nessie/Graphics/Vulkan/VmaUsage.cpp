// VmaUsage.cpp

//------------------------------------------------------------------------------------
// Contains the implementation macro that needs to be in a single cpp file for VMA. 
//------------------------------------------------------------------------------------
#include "Nessie/Core/Config.h"

NES_SUPPRESS_WARNINGS_BEGIN
NES_MSVC_SUPPRESS_WARNING(4100 4189) // Unreferenced parameters, unused variable. 

#define VMA_IMPLEMENTATION
#include "vk_mem_alloc.h"

NES_SUPPRESS_WARNINGS_END
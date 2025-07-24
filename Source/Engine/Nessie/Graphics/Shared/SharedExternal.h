//// SharedExternal.h
//#pragma once
//#include "Nessie/Math/Generic.h"
//#include "Nessie/Graphics/RenderDevice.h"
//#include "Nessie/Graphics/Helpers.h"
//#include "Nessie/Core/String/StringConversions.h"
//
////-------------------------------------------------------------------------------------------------
//// Under development. This code is largely from NRI. I will be making changes to this as I simplify.
////-------------------------------------------------------------------------------------------------
//
////----------------------------------------------------------------------------------------------------
///// @brief : Score a potential graphics queue based on features available. 
////----------------------------------------------------------------------------------------------------
//#define NES_GRAPHICS_QUEUE_SCORE ((graphics ? 100 : 0) + (compute ? 10 : 0) + (copy ? 10 : 0) + (sparse ? 5 : 0) + (videoDecode ? 2 : 0) + (videoEncode ? 2 : 0) + (protect ? 1 : 0) + (opticalFlow ? 1 : 0))
//
////----------------------------------------------------------------------------------------------------
///// @brief : Score a potential compute queue based on features available. 
////----------------------------------------------------------------------------------------------------
//#define NES_COMPUTE_QUEUE_SCORE  ((!graphics ? 10 : 0) + (compute ? 100 : 0) + (!copy ? 10 : 0) + (sparse ? 5 : 0) + (!videoDecode ? 2 : 0) + (!videoEncode ? 2 : 0) + (protect ? 1 : 0) + (!opticalFlow ? 1 : 0))
//
////----------------------------------------------------------------------------------------------------
///// @brief : Score a potential copy queue based on features available. 
////----------------------------------------------------------------------------------------------------
//#define NES_COPY_QUEUE_SCORE     ((!graphics ? 10 : 0) + (!compute ? 10 : 0) + (copy ? 100 * familyProps.queueCount : 0) + (sparse ? 5 : 0) + (!videoDecode ? 2 : 0) + (!videoEncode ? 2 : 0) + (protect ? 1 : 0) + (!opticalFlow ? 1 : 0))
//
//NES_BEGIN_GRAPHICS_NAMESPACE
//
////----------------------------------------------------------------------------------------------------
///// @brief : Convert from milliseconds to microseconds.
////----------------------------------------------------------------------------------------------------
//constexpr uint64    MsToUs(const uint32 val) { return val * 1000000ull; }
//
////----------------------------------------------------------------------------------------------------
///// @brief : Convert to EVendor from a raw vendor id value.
////----------------------------------------------------------------------------------------------------
//inline EVendor      GetVendorFromID(const uint32 vendorID)
//{
//    switch (vendorID)
//    {
//        case 0x10DE:
//            return EVendor::NVIDIA;
//        case 0x1002:
//            return EVendor::AMD;
//        case 0x8086:
//            return EVendor::INTEL;
//            
//        default: break;
//    }
//    return EVendor::Unknown;
//}
//
//NES_END_GRAPHICS_NAMESPACE

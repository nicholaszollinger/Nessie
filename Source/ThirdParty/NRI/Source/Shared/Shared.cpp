// © 2021 NVIDIA Corporation

#ifndef _WIN32
#    include <csignal> // raise
#    include <cstdarg> // va_start, va_end
#endif

#include "SharedExternal.h"

#include "HelperInterface.h"
#include "ImguiInterface.h"
#include "StreamerInterface.h"
#include "UpscalerInterface.h"

using namespace nri;

#if (_WIN32) && (NRI_ENABLE_D3D11_SUPPORT == 0) && (NRI_ENABLE_D3D12_SUPPORT == 0)
    #include <csignal> // raise
    #include <cstdarg> // va_start, va_end 
#endif

#include "HelperInterface.hpp"
#include "ImguiInterface.hpp"
#include "StreamerInterface.hpp"
#include "UpscalerInterface.hpp"

#include "SharedExternal.hpp"
#include "SharedLibrary.hpp"

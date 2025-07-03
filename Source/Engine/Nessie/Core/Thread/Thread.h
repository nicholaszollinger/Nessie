// Thread.h
#pragma once
#include <thread>
#include "Nessie/Debug/Assert.h"

namespace nes
{
    NES_DEFINE_LOG_TAG(kLogTagThread, "Thread", Warn);
    
    //----------------------------------------------------------------------------------------------------
    /// @brief : Set the name of a Thread. 
    //----------------------------------------------------------------------------------------------------
    void SetThreadName(const char* threadName);
}

// Thread.h
#pragma once
#include <thread>
#include "Nessie/Debug/Assert.h"

namespace nes
{
    NES_DEFINE_LOG_TAG(kLogTagThread, "Thread", Warn);

    //----------------------------------------------------------------------------------------------------
    /// @brief : Enum to determine single vs. multithreaded state of systems.
    //----------------------------------------------------------------------------------------------------
    enum class EThreadPolicy : uint8
    {
        SingleThreaded = 0,
        Multithreaded
    };

    namespace thread
    {
        //----------------------------------------------------------------------------------------------------
        /// @brief : Set the name of a Thread. 
        //----------------------------------------------------------------------------------------------------
        void SetThreadName(const char* threadName);
    }
}

// CommandLineArgs.h
#pragma once

namespace nes
{
    //-----------------------------------------------------------------------------------------------------------------------------
    /// @brief : Container for the arguments passed into the executable. The first argument will always be the executable's path.
    //-----------------------------------------------------------------------------------------------------------------------------
    struct CommandLineArgs
    {
        size_t m_count  = 0;
        char** m_args   = nullptr;

        const char* operator[](const size_t index) const;
        const char* operator[](const int index) const;
    };
}

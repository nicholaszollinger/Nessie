// LogSource.h
#pragma once

namespace nes::internal
{
    //----------------------------------------------------------------------------------------------------
    /// @brief : Struct that contains the information about the source of the log message.
    //----------------------------------------------------------------------------------------------------
    struct LogSource
    {
        constexpr LogSource() = default;
        constexpr LogSource(const char* fileName, int line, const char* functionName)
            : m_fileName(fileName)
            , m_functionName(functionName)
            , m_line(line)
        {
            //
        }

        const char*     m_fileName      = nullptr;
        const char*     m_functionName  = nullptr;
        int             m_line          = 0;

        constexpr bool  IsValid() const { return m_line > 0; }
    };
}
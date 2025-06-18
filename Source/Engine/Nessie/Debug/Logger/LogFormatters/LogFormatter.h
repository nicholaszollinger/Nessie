// LogFormatter.h
#pragma once
#include "Debug/Logger/Details/LogMessage.h"

namespace nes
{
    //----------------------------------------------------------------------------------------------------
    /// @brief : Base class to format Log Messages.
    //----------------------------------------------------------------------------------------------------
    class LogFormatter
    {
    public:
        virtual ~LogFormatter() = default;
        virtual void Format(const internal::LogMessage& msg, LogMemoryBuffer& memoryBuffer) = 0;
        virtual std::unique_ptr<LogFormatter> Clone() const = 0;
    };
}
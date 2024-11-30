#pragma once
// LogCategory.h
#include "Core/String/StringID.h"

namespace nes
{
    enum class LogOutputLevel : int
    {
        kNone,
        kDisplay,
        kFile,
        kAll,
    };

    class LogCategory
    {
        StringID m_name;
        LogOutputLevel m_output = LogOutputLevel::kNone;

    public:
        LogCategory() = default;
        LogCategory(const StringID name, const LogOutputLevel outputLevel) : m_name(name), m_output(outputLevel){}
        LogCategory(const LogCategory&) = default;
        LogCategory(LogCategory&&) noexcept = default;
        LogCategory& operator=(const LogCategory&) = default;
        LogCategory& operator=(LogCategory&&) noexcept = default;
        ~LogCategory() = default;

        [[nodiscard]] const std::string& GetName() const { return m_name.GetStringRef(); }
        [[nodiscard]] LogOutputLevel GetOutputLevel() const { return m_output; }
    };
} 


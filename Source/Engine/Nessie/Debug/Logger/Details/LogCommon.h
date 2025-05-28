// LogCommon.h
#pragma once
#include <chrono>
#include "fmt/format.h"

namespace nes
{
    using LogMemoryBuffer = fmt::basic_memory_buffer<char, 250>;
    using LogClock = std::chrono::system_clock;
    using LogTimePoint = LogClock::time_point;

    template <typename...Args>
    using FormatString = fmt::format_string<Args...>;

    template <typename Type, typename Char = char>
    concept IsConvertibleToBaseFormatString =
        std::is_convertible_v<Type, fmt::basic_string_view<Char>>
        || std::is_same_v<typename std::remove_cvref_t<Type>, fmt::basic_string_view<Char>>;

    template <typename Type>
    concept IsNotConvertibleToAnyFormatString =
           !IsConvertibleToBaseFormatString<Type, char>
        && !IsConvertibleToBaseFormatString<Type, wchar_t>;
}
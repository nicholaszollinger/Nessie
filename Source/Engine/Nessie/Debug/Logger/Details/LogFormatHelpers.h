// LogFormatHelpers.h
#pragma once
#include <algorithm>
#include "LogCommon.h"

namespace nes::FormatHelpers
{
    //----------------------------------------------------------------------------------------------------
    /// @brief : Append a string_view to a formatted LogMemoryBuffer.
    //----------------------------------------------------------------------------------------------------
    inline void AppendStringView(std::string_view view, LogMemoryBuffer& dest)
    {
        auto* pBufferPtr = view.data();
        dest.append(pBufferPtr, pBufferPtr + view.size());   
    }

    //----------------------------------------------------------------------------------------------------
    /// @brief : Append an integer value to a formatted LogMemoryBuffer.
    //----------------------------------------------------------------------------------------------------
    template <std::integral Type>
    inline void AppendInt(Type val, LogMemoryBuffer& dest)
    {
        const fmt::format_int i(val);
        dest.append(i.data(), i.data() + i.size());
    }

    //----------------------------------------------------------------------------------------------------
    /// @brief : Count the number of decimal digits in a value.
    //----------------------------------------------------------------------------------------------------
    template <std::integral Type>
    constexpr unsigned int CountDigits(Type val)
    {
        using CountType = std::conditional_t<(sizeof(Type) > sizeof(uint32_t)), uint64_t, uint32_t>;
        return static_cast<unsigned int>(fmt::detail::count_digits(static_cast<CountType>(val)));
    }

    inline void Pad2(int n, LogMemoryBuffer& dest)
    {
        if (n >= 0 && n < 100)
        {
            dest.push_back(static_cast<char>('0' + n / 10));
            dest.push_back(static_cast<char>('0' + n % 10));
        }
        else
        {
            fmt::format_to(std::back_inserter(dest), FMT_STRING("{:02}"), n);
        }
    }

    template <std::integral Type>
    inline void PadUint(Type n, unsigned int width, LogMemoryBuffer& dest)
    {
        for (auto digits = CountDigits(n); digits < width; ++digits)
        {
            dest.push_back('0');
        }
        AppendInt(n, dest);
    }

    template <std::integral Type>
    inline void Pad3(Type n, LogMemoryBuffer& dest)
    {
        if (n < 1000)
        {
            dest.push_back(static_cast<char>((n / 100) + '0'));
            n = n % 100;
            dest.push_back(static_cast<char>((n / 10) + '0'));
            dest.push_back(static_cast<char>((n % 10) + '0'));
        }
        else
        {
            AppendInt(n, dest);
        }
    }

    template <std::integral Type>
    inline void Pad6(Type n, LogMemoryBuffer& dest)
    {
        PadUint(n, 6, dest);   
    }

    template <std::integral Type>
    inline void Pad9(Type n, LogMemoryBuffer& dest)
    {
        PadUint(n, 9, dest);   
    }

    //----------------------------------------------------------------------------------------------------
    /// @brief : Return a fraction of a second of a time point. E.g., TimeFraction<std::milliseconds>(timePoint) will
    ///     return the millisecond part of the second.
    //----------------------------------------------------------------------------------------------------
    template <typename ToDuration>
    inline ToDuration TimeFraction(LogTimePoint timePoint)
    {
        using std::chrono::duration_cast;
        using std::chrono::seconds;
        const auto duration = timePoint.time_since_epoch();
        auto secs = duration_cast<seconds>(duration);
        return duration_cast<ToDuration>(duration) - duration_cast<ToDuration>(secs);   
    }

    //----------------------------------------------------------------------------------------------------
    /// @brief : Get the base filename, Ex: Get "Main.cpp" from "Source/Main.cpp".
    //----------------------------------------------------------------------------------------------------
    static inline const char* GetFileBasename(const char* filename)
    {
        // [TODO]: This is Platform-specific. Move to Platform object.
        static constexpr char kFolderSeparators[] { "\\/"};
            
        if constexpr (sizeof(kFolderSeparators) == 2)
        {
            const char* finalName = std::strrchr(filename, kFolderSeparators[0]);
            return finalName != nullptr ? finalName + 1 : filename;
        }
        else
        {
            const std::reverse_iterator<const char*> begin(filename + std::strlen(filename));
            const std::reverse_iterator<const char*> end(filename);

            const auto it = std::find_first_of(begin, end, std::begin(kFolderSeparators), std::end(kFolderSeparators) - 1);
            return it != end ? it.base() : filename;
        }
    }
}

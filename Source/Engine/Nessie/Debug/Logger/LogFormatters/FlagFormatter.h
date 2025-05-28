// FlagFormatter.h
#pragma once
#include "Debug/Logger/Details/LogFormatHelpers.h"
#include "LogFormatter.h"

namespace nes::internal
{
    struct PaddingInfo
    {
        enum class EPaddingSide
        {
            Left,
            Right,
            Center,
        };

        PaddingInfo() = default;
        PaddingInfo(EPaddingSide side, size_t width, bool truncate)
            : m_width(width)
            , m_side(side)
            , m_truncate(truncate)
            , m_isEnabled(true)
        {
            //
        }
        
        size_t          m_width = 0;
        EPaddingSide    m_side = EPaddingSide::Left;
        bool            m_truncate = false;
        bool            m_isEnabled = false;
    };

    class ScopedPadder
    {
    public:
        ScopedPadder(const size_t wrappedSize, const PaddingInfo& paddingInfo, LogMemoryBuffer& dest)
            : m_paddingInfo(paddingInfo)
            , m_dest(dest)
        {
            m_remainingPadding = static_cast<long>(paddingInfo.m_width) - static_cast<long>(wrappedSize);
            if (m_remainingPadding <= 0)
                return;

            if (m_paddingInfo.m_side == PaddingInfo::EPaddingSide::Left)
            {
                Pad(m_remainingPadding);
                m_remainingPadding = 0;
            }
            else if (m_paddingInfo.m_side == PaddingInfo::EPaddingSide::Center)
            {
                const auto halfPad = m_remainingPadding / 2;
                const auto remainder = m_remainingPadding % 2;
                Pad(halfPad);
                m_remainingPadding = halfPad + remainder;
            }
        }

        ~ScopedPadder()
        {
            if (m_remainingPadding >= 0)
            {
                Pad(m_remainingPadding);
            }
            else if (m_paddingInfo.m_truncate)
            {
                long newSize = static_cast<long>(m_dest.size()) + m_remainingPadding;
                newSize = std::max<long>(newSize, 0);
                m_dest.resize(static_cast<size_t>(newSize));
            }
        }

        template <std::integral Type>
        static unsigned int CountDigits(Type value) { return FormatHelpers::CountDigits(value); }
    
    private:
        void Pad(const long count) const
        {
            FormatHelpers::AppendStringView(std::string_view(m_spaces.data(), static_cast<size_t>(count)), m_dest);
        }
        
        const PaddingInfo& m_paddingInfo;
        LogMemoryBuffer&   m_dest;
        long               m_remainingPadding;
        std::string_view   m_spaces{"                                                                ", 64};
    };

    //----------------------------------------------------------------------------------------------------
    /// @brief : Version of ScopedPadder that does no padding. 
    //----------------------------------------------------------------------------------------------------
    struct NullScopedPadder
    {
        NullScopedPadder(size_t, const PaddingInfo&, LogMemoryBuffer&) {}
        
        template <std::integral Type>
        static unsigned int CountDigits(Type) { return 0; }
    };

    //----------------------------------------------------------------------------------------------------
    /// @brief : Base Formatter for replacing a char symbol with a string in a log message.
    /// @note : If you want to make your own formatter, use nes::CustomFlagFormatter.
    //----------------------------------------------------------------------------------------------------
    class FlagFormatter
    {
    public:
        FlagFormatter() = default;
        explicit FlagFormatter(PaddingInfo paddingInfo) : m_paddingInfo(paddingInfo) {}
        virtual ~FlagFormatter() = default;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Format a log message for a particular flag.
        //----------------------------------------------------------------------------------------------------
        virtual void    Format(const internal::LogMessage& msg, const std::tm& tmTime, LogMemoryBuffer& dest) = 0;

    protected:
        PaddingInfo     m_paddingInfo;
    };
}


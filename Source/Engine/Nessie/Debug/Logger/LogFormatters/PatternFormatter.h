// PatternFormatter.h
#pragma once

#include <unordered_map>
#include <vector>
#include "CustomFlagFormatter.h"
#include "Platform/Platform.h"

namespace nes
{
    //----------------------------------------------------------------------------------------------------
    /// @brief : Formats a log message based on a set of Flags. 
    //----------------------------------------------------------------------------------------------------
    class PatternFormatter final : public LogFormatter
    {
    public:
        using CustomFlags = std::unordered_map<char, std::unique_ptr<CustomFlagFormatter>>;

    public:
        explicit PatternFormatter(std::string pattern = "%+", std::string eol = nes::platform::kEOL, CustomFlags customFlags = {});
        
        PatternFormatter(const PatternFormatter&) = delete;
        PatternFormatter(PatternFormatter&&) noexcept = delete;
        PatternFormatter& operator=(const PatternFormatter&) = delete;
        PatternFormatter& operator=(PatternFormatter&&) noexcept = delete;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Make a clone of this Formatter. 
        //----------------------------------------------------------------------------------------------------
        virtual std::unique_ptr<LogFormatter> Clone() const override;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Format a log message, storing the result in the given memory buffer. 
        //----------------------------------------------------------------------------------------------------
        virtual void Format(const internal::LogMessage& msg, LogMemoryBuffer& dest) override;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Set if this formatter needs to update its internal cached time point.
        //----------------------------------------------------------------------------------------------------
        void SetNeedUpdateCachedTime(const bool shouldUpdate = true) { m_needUpdateCachedTime = shouldUpdate; }
        
    private:
        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the time based on the log message.
        //----------------------------------------------------------------------------------------------------
        std::tm GetTime(const internal::LogMessage& msg);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Handles updating the formatted message based on the passed in flag.
        //----------------------------------------------------------------------------------------------------
        template <typename Padder>
        void HandleFlag(const char flag, internal::PaddingInfo& padding);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Create the formatter objects for the given pattern.
        //----------------------------------------------------------------------------------------------------
        void CompilePattern(const std::string& pattern);

        //----------------------------------------------------------------------------------------------------
        /// @brief : 
        //----------------------------------------------------------------------------------------------------
        static internal::PaddingInfo HandlePadSpec(std::string::const_iterator& it, std::string::const_iterator end);

    private:
        std::vector<std::unique_ptr<internal::FlagFormatter>> m_flagFormatters;
        CustomFlags                 m_customFlags;
        std::string                 m_pattern;
        std::string                 m_eol;
        std::tm                     m_cachedTmTime;
        std::chrono::seconds        m_lastLogSeconds;
        bool                        m_needUpdateCachedTime;
    };
}

#include "PatternFormatter.inl"

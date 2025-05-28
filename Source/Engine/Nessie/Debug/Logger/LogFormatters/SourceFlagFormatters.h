// SourceFlagFormatters.h
#pragma once
#include "FlagFormatter.h"

namespace nes::internal
{
    //----------------------------------------------------------------------------------------------------
    /// @brief : Prints the location of the log message in the format "filename(lineNum)". Ex: Source/Main.cpp(5) 
    //----------------------------------------------------------------------------------------------------
    template <typename ScopedPadder>
    class SourceLocationFormatter final : public FlagFormatter
    {
    public:
        explicit SourceLocationFormatter(const PaddingInfo paddingInfo) : FlagFormatter(paddingInfo) {}

        virtual void Format(const internal::LogMessage& msg, const std::tm&, LogMemoryBuffer& dest) override
        {
            if (!msg.m_source.IsValid())
            {
                ScopedPadder padder(0, m_paddingInfo, dest);
                return;
            }

            size_t textSize;
            if (m_paddingInfo.m_isEnabled)
            {
                textSize = std::char_traits<char>::length(msg.m_source.m_fileName)
                    + ScopedPadder::CountDigits(msg.m_source.m_line) + 2;
            }
            else
            {
                textSize = 0;
            }

            ScopedPadder padder(textSize, m_paddingInfo, dest);
            FormatHelpers::AppendStringView(msg.m_source.m_fileName, dest);
            dest.push_back('(');
            FormatHelpers::AppendStringView(std::to_string(msg.m_source.m_line), dest);
            dest.push_back(')');       
        }
    };

    //----------------------------------------------------------------------------------------------------
    /// @brief : Adds the filename location of the log message. Ex: Source/Main.cpp 
    //----------------------------------------------------------------------------------------------------
    template <typename ScopedPadder>
    class SourceFilenameFormatter final : public FlagFormatter
    {
    public:
        explicit SourceFilenameFormatter(const PaddingInfo paddingInfo) : FlagFormatter(paddingInfo) {}
        virtual void Format(const internal::LogMessage& msg, const std::tm&, LogMemoryBuffer& dest) override
        {
            if (!msg.m_source.IsValid())
            {
                ScopedPadder padder(0, m_paddingInfo, dest);
                return;
            }

            size_t textSize = m_paddingInfo.m_isEnabled ? std::char_traits<char>::length(msg.m_source.m_fileName) : 0;
            ScopedPadder padder(textSize, m_paddingInfo, dest);
            FormatHelpers::AppendStringView(msg.m_source.m_fileName, dest);       
        }
    };

    //----------------------------------------------------------------------------------------------------
    /// @brief : Adds the shortened filename location of the log message. Ex: "Main.cpp". 
    //----------------------------------------------------------------------------------------------------
    template <typename ScopedPadder>
    class ShortFilenameFormatter final : public FlagFormatter
    {
    public:
        explicit ShortFilenameFormatter(const PaddingInfo paddingInfo) : FlagFormatter(paddingInfo) {}
        
        virtual void Format(const internal::LogMessage& msg, const std::tm&, LogMemoryBuffer& dest) override
        {
            if (!msg.m_source.IsValid())
            {
                ScopedPadder padder(0, m_paddingInfo, dest);
                return;
            }

            auto shortFilename = FormatHelpers::GetFileBasename(msg.m_source.m_fileName);
            size_t textSize = m_paddingInfo.m_isEnabled ? std::char_traits<char>::length(shortFilename) : 0;
            ScopedPadder padder(textSize, m_paddingInfo, dest);
            FormatHelpers::AppendStringView(shortFilename, dest);
        }
    };

    //----------------------------------------------------------------------------------------------------
    /// @brief : Adds the file line that the log message was created on. 
    //----------------------------------------------------------------------------------------------------
    template <typename ScopedPadder>
    class SourceLineNumberFormatter final : public FlagFormatter
    {
    public:
        explicit SourceLineNumberFormatter(const PaddingInfo paddingInfo) : FlagFormatter(paddingInfo) {}

        virtual void Format(const internal::LogMessage& msg, const std::tm&, LogMemoryBuffer& dest) override
        {
            if (!msg.m_source.IsValid())
            {
                ScopedPadder padder(0, m_paddingInfo, dest);
                return;
            }

            const size_t fieldSize = ScopedPadder::CountDigits(msg.m_source.m_line);
            ScopedPadder padder(fieldSize, m_paddingInfo, dest);
            FormatHelpers::AppendInt(msg.m_source.m_line, dest);       
        }
    };

    //----------------------------------------------------------------------------------------------------
    /// @brief : Adds the name of the function where the log message was created. 
    //----------------------------------------------------------------------------------------------------
    template <typename ScopedPadder>
    class SourceFunctionNameFormatter final : public FlagFormatter
    {
    public:
        explicit SourceFunctionNameFormatter(const PaddingInfo paddingInfo) : FlagFormatter(paddingInfo) {}

        virtual void Format(const internal::LogMessage& msg, const std::tm&, LogMemoryBuffer& dest) override
        {
            if (!msg.m_source.IsValid())
            {
                ScopedPadder padder(0, m_paddingInfo, dest);
                return;
            }

            const size_t textSize = m_paddingInfo.m_isEnabled ? std::char_traits<char>::length(msg.m_source.m_functionName) : 0;
            ScopedPadder padder(textSize, m_paddingInfo, dest);
            FormatHelpers::AppendStringView(msg.m_source.m_functionName, dest);
        }
    };
}

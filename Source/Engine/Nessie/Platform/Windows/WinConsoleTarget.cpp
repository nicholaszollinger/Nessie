// WinConsoleTarget.cpp
#include "WinConsoleTarget.h"

#include "WindowsInclude.h"
#include <wincon.h>

namespace nes
{
    template <MutexType Mutex>
    const std::uint16_t WinConsoleTarget<Mutex>::kWhite        = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE;

    template <MutexType Mutex>
    const std::uint16_t WinConsoleTarget<Mutex>::kRed          = FOREGROUND_RED;

    template <MutexType Mutex>
    const std::uint16_t WinConsoleTarget<Mutex>::kCyan         = FOREGROUND_GREEN | FOREGROUND_BLUE;

    template <MutexType Mutex>
    const std::uint16_t WinConsoleTarget<Mutex>::kYellow       = FOREGROUND_RED | FOREGROUND_GREEN;

    template <MutexType Mutex>
    const std::uint16_t WinConsoleTarget<Mutex>::kGreen        = FOREGROUND_GREEN;

    template <MutexType Mutex>
    const std::uint16_t WinConsoleTarget<Mutex>::kBlue         = FOREGROUND_BLUE;

    template <MutexType Mutex>
    const std::uint16_t WinConsoleTarget<Mutex>::kIntensityVal = FOREGROUND_INTENSITY;
    
    template <>
    WinConsoleTarget<std::mutex>::WinConsoleTarget(void* pOutHandle)
        //: m_mutex()
        : m_pOutHandle(pOutHandle)
    {
        m_colors[static_cast<uint8_t>(ELogLevel::Trace)] = kWhite;
        m_colors[static_cast<uint8_t>(ELogLevel::Debug)] = kCyan;
        m_colors[static_cast<uint8_t>(ELogLevel::Info)] = kGreen;
        m_colors[static_cast<uint8_t>(ELogLevel::Warn)] = kYellow | kIntensityVal;
        m_colors[static_cast<uint8_t>(ELogLevel::Error)] = kRed | kIntensityVal;
        m_colors[static_cast<uint8_t>(ELogLevel::Fatal)] = kWhite | kIntensityVal | BACKGROUND_RED;
        m_colors[static_cast<uint8_t>(ELogLevel::Off)] = 0;
    }

    template <>
    WinConsoleTarget<NullMutex>::WinConsoleTarget(void* pOutHandle)
        //: m_mutex()
        : m_pOutHandle(pOutHandle)
    {
        m_colors[static_cast<uint8_t>(ELogLevel::Trace)] = kWhite;
        m_colors[static_cast<uint8_t>(ELogLevel::Debug)] = kCyan;
        m_colors[static_cast<uint8_t>(ELogLevel::Info)] = kGreen;
        m_colors[static_cast<uint8_t>(ELogLevel::Warn)] = kYellow | kIntensityVal;
        m_colors[static_cast<uint8_t>(ELogLevel::Error)] = kRed | kIntensityVal;
        m_colors[static_cast<uint8_t>(ELogLevel::Fatal)] = kWhite | kIntensityVal | BACKGROUND_RED;
        m_colors[static_cast<uint8_t>(ELogLevel::Off)] = 0;
    }

    template <>
    void WinConsoleTarget<NullMutex>::SetColor(const ELogLevel level, const std::uint16_t color)
    {
        m_colors[static_cast<uint8_t>(level)] = color;
    }

    template <>
    void WinConsoleTarget<std::mutex>::SetColor(const ELogLevel level, const std::uint16_t color)
    {
        std::lock_guard lock(m_mutex);
        m_colors[static_cast<uint8_t>(level)] = color;
    }

    template <>
    std::uint16_t WinConsoleTarget<NullMutex>::SetForegroundColor(std::uint16_t attribs) const
    {
        CONSOLE_SCREEN_BUFFER_INFO originalBufferInfo;
        if (!::GetConsoleScreenBufferInfo(static_cast<HANDLE>(m_pOutHandle), &originalBufferInfo))
        {
            return kWhite;
        }

        // Change only the foreground color bits (lowest 4 bits).
        const WORD newAttribs = static_cast<WORD>(attribs) | (originalBufferInfo.wAttributes & 0xfff0);
        auto ignored = ::SetConsoleTextAttribute(static_cast<HANDLE>(m_pOutHandle), newAttribs);
        (void)ignored;
        return static_cast<std::uint16_t>(originalBufferInfo.wAttributes);
    }

    template <>
    std::uint16_t WinConsoleTarget<std::mutex>::SetForegroundColor(std::uint16_t attribs) const
    {
        CONSOLE_SCREEN_BUFFER_INFO originalBufferInfo;
        if (!::GetConsoleScreenBufferInfo(static_cast<HANDLE>(m_pOutHandle), &originalBufferInfo))
        {
            return kWhite;
        }

        // Change only the foreground color bits (lowest 4 bits).
        const WORD newAttribs = static_cast<WORD>(attribs) | (originalBufferInfo.wAttributes & 0xfff0);
        auto ignored = ::SetConsoleTextAttribute(static_cast<HANDLE>(m_pOutHandle), newAttribs);
        (void)ignored;
        return static_cast<std::uint16_t>(originalBufferInfo.wAttributes);
    }

    template <>
    void WinConsoleTarget<NullMutex>::PrintRange(const LogMemoryBuffer& formattedMsg, const size_t start, const size_t end)
    {
        if (end > start)
        {
            const auto size = static_cast<DWORD>(end - start);
            [[maybe_unused]] auto ignored = ::WriteConsoleA(static_cast<HANDLE>(m_pOutHandle), formattedMsg.data() + start, size, nullptr, nullptr);
        }
    }

    template <>
    void WinConsoleTarget<std::mutex>::PrintRange(const LogMemoryBuffer& formattedMsg, const size_t start, const size_t end)
    {
        if (end > start)
        {
            const auto size = static_cast<DWORD>(end - start);
            auto ignored = ::WriteConsoleA(static_cast<HANDLE>(m_pOutHandle), formattedMsg.data() + start, size, nullptr, nullptr);
            (void)(ignored);
        }
    }

    template <>
    void WinConsoleTarget<NullMutex>::WriteToFile(const LogMemoryBuffer& formattedMsg)
    {
        const auto size = static_cast<DWORD>(formattedMsg.size());
        DWORD bytesWritten = 0;
        [[maybe_unused]] auto ignored = ::WriteFile(static_cast<HANDLE>(m_pOutHandle), formattedMsg.data(), size, &bytesWritten, nullptr);
    }

    template <>
    void WinConsoleTarget<std::mutex>::WriteToFile(const LogMemoryBuffer& formattedMsg)
    {
        const auto size = static_cast<DWORD>(formattedMsg.size());
        DWORD bytesWritten = 0;
        [[maybe_unused]] auto ignored = ::WriteFile(static_cast<HANDLE>(m_pOutHandle), formattedMsg.data(), size, &bytesWritten, nullptr);
    }

    template <>
    void WinConsoleTarget<NullMutex>::LogImpl(const internal::LogMessage& message)
    {
        message.m_colorRangeStart = 0;
        message.m_colorRangeEnd = 0;
        nes::LogMemoryBuffer formattedMsg{};
        
        m_pFormatter->Format(message, formattedMsg);
        if (m_shouldUseColors || message.m_colorRangeEnd > message.m_colorRangeStart)
        {
            // Before Color range:
            PrintRange(formattedMsg, 0, message.m_colorRangeStart);

            // In Color Range:
            const auto originalColor = SetForegroundColor(m_colors[static_cast<uint8_t>(message.m_level)]);
            PrintRange(formattedMsg, message.m_colorRangeStart, message.m_colorRangeEnd);

            // Reset To Original Color:
            ::SetConsoleTextAttribute(static_cast<HANDLE>(m_pOutHandle), originalColor);

            // Finish any remaining characters:
            PrintRange(formattedMsg, message.m_colorRangeEnd, formattedMsg.size());
        }
        else
        {
            // Print without colors if the color range is invalid (or color is disabled).
            WriteToFile(formattedMsg);
        }
    }

    template <>
    void WinConsoleTarget<std::mutex>::LogImpl(const internal::LogMessage& message)
    {
        message.m_colorRangeStart = 0;
        message.m_colorRangeEnd = 0;
        nes::LogMemoryBuffer formattedMsg{};
        
        m_pFormatter->Format(message, formattedMsg);
        if (m_shouldUseColors || message.m_colorRangeEnd > message.m_colorRangeStart)
        {
            // Before Color range:
            PrintRange(formattedMsg, 0, message.m_colorRangeStart);

            // In Color Range:
            const auto originalColor = SetForegroundColor(m_colors[static_cast<uint8_t>(message.m_level)]);
            PrintRange(formattedMsg, message.m_colorRangeStart, message.m_colorRangeEnd);

            // Reset To Original Color:
            ::SetConsoleTextAttribute(static_cast<HANDLE>(m_pOutHandle), originalColor);

            // Finish any remaining characters:
            PrintRange(formattedMsg, message.m_colorRangeEnd, formattedMsg.size());
        }
        else
        {
            // Print without colors if the color range is invalid (or color is disabled).
            WriteToFile(formattedMsg);
        }
    }

    template <>
    WinConsoleStdCoutTarget<NullMutex>::WinConsoleStdCoutTarget()
        : WinConsoleTarget<NullMutex>(::GetStdHandle(STD_OUTPUT_HANDLE))
    {
        //
    }

    template <>
    WinConsoleStdCoutTarget<std::mutex>::WinConsoleStdCoutTarget()
        : WinConsoleTarget<std::mutex>(::GetStdHandle(STD_OUTPUT_HANDLE))
    {
        //
    }

    template <>
    WinConsoleStdErrTarget<NullMutex>::WinConsoleStdErrTarget()
        : WinConsoleTarget<NullMutex>(::GetStdHandle(STD_ERROR_HANDLE))
    {
        //
    }

    template <>
    WinConsoleStdErrTarget<std::mutex>::WinConsoleStdErrTarget()
        : WinConsoleTarget<std::mutex>(::GetStdHandle(STD_ERROR_HANDLE))
    {
        //
    }
}

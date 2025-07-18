// WinConsoleTarget.h
#pragma once
#include <array>
#include "Nessie/Debug/Logger/LogTargets/LogTargetBase.h"

namespace nes
{
    //----------------------------------------------------------------------------------------------------
    /// @brief : Log Target for posting log messages to the Windows Console. 
    //----------------------------------------------------------------------------------------------------
    template <MutexType Mutex>
    class WinConsoleTarget : public LogTargetBase<Mutex>
    {
    protected:
        using ColorArray = std::array<std::uint16_t, static_cast<size_t>(ELogLevel::NumLevels)>;

    public:
        static const std::uint16_t kWhite;          /// White                   - Default for Trace messages
        static const std::uint16_t kRed;            /// Red                     - Default for Error messages, with added Intensity.
        static const std::uint16_t kCyan;           /// Cyan                    - Default for Debug messages
        static const std::uint16_t kGreen;          /// Green                   - Default for Info messages
        static const std::uint16_t kBlue;           /// Blue                    - Default for Info messages
        static const std::uint16_t kYellow;         /// Yellow                  - Default for Warnings, with added Intensity.
        static const std::uint16_t kIntensityVal;   /// Flag to make a foreground color intense. 'Or' with a color value.
        
    public:
        WinConsoleTarget(void* pOutHandle);
        virtual ~WinConsoleTarget() override { this->FlushImpl(); }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Set the Color for a particular log level.
        //----------------------------------------------------------------------------------------------------
        void            SetColor(const ELogLevel level, const std::uint16_t color);
        
    protected:
        virtual void    SetPatternImpl(const std::string& pattern) override final { LogTargetBase<Mutex>::SetPatternImpl(pattern); }
        virtual void    LogImpl(const internal::LogMessage& message) override final;
        virtual void    FlushImpl() override final { /* Windows console is always flushed automatically.*/ }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Set foreground-color and return the original console attributes to restore later.  
        //----------------------------------------------------------------------------------------------------
        std::uint16_t   SetForegroundColor(std::uint16_t attribs) const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Print out a section of the formatted message to the console. 
        //----------------------------------------------------------------------------------------------------
        void            PrintRange(const LogMemoryBuffer& formattedMsg, const size_t start, const size_t end);

        //----------------------------------------------------------------------------------------------------
        /// @brief : In case we are redirected to a file - no console was available. 
        //----------------------------------------------------------------------------------------------------
        void            WriteToFile(const LogMemoryBuffer& formattedMsg);
        
    protected:
        ColorArray      m_colors;
        void*           m_pOutHandle = nullptr;
        bool            m_shouldUseColors = true;
    };

    template <MutexType Mutex>
    class WinConsoleStdCoutTarget final : public WinConsoleTarget<Mutex>
    {
    public:
        WinConsoleStdCoutTarget();
    };

    template <MutexType Mutex>
    class WinConsoleStdErrTarget final : public WinConsoleTarget<Mutex>
    {
    public:
        WinConsoleStdErrTarget();
    };

    //----------------------------------------------------------------------------------------------------
    /// @brief : Single-threaded version of the std::cout target. 
    //----------------------------------------------------------------------------------------------------
    using WinConsoleStdCoutTargetST = WinConsoleStdCoutTarget<NullMutex>;

    //----------------------------------------------------------------------------------------------------
    /// @brief : Single-threaded version of the std::cout target. 
    //----------------------------------------------------------------------------------------------------
    using WinConsoleStdCoutTargetMT = WinConsoleStdCoutTarget<std::mutex>;

    //----------------------------------------------------------------------------------------------------
    /// @brief : Single-threaded version of the std::err target. 
    //----------------------------------------------------------------------------------------------------
    using WinConsoleStdErrTargetST = WinConsoleStdErrTarget<NullMutex>;

    //----------------------------------------------------------------------------------------------------
    /// @brief : Single-threaded version of the std::err target. 
    //----------------------------------------------------------------------------------------------------
    using WinConsoleStdErrTargetMT = WinConsoleStdErrTarget<std::mutex>;
}
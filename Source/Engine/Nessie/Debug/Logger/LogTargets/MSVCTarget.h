// MSVCTarget.h
#pragma once
#include "Nessie/Core/Config.h"
#ifdef NES_PLATFORM_WINDOWS
#include "LogTargetBase.h"

extern "C" __declspec(dllimport) void __stdcall OutputDebugStringA(const char* lpOutputString);
extern "C" __declspec(dllimport) int __stdcall  IsDebuggerPresent();

namespace nes
{
    //----------------------------------------------------------------------------------------------------
    /// @brief : Log Target for posting log messages to the Windows Debug Console in MSVC. 
    //----------------------------------------------------------------------------------------------------
    template <MutexType Mutex>
    class MSVCTarget final : public LogTargetBase<Mutex>
    {
    public:
        MSVCTarget() = default;
        MSVCTarget(const bool checkDebuggerPresent) : m_checkDebuggerPresent(checkDebuggerPresent) {}
    
    protected:
        virtual void    LogImpl(const internal::LogMessage& message) override
        {
            if (m_checkDebuggerPresent && !IsDebuggerPresent())
                return;

            LogMemoryBuffer formattedMsg;
            LogTargetBase<Mutex>::m_pFormatter->Format(message, formattedMsg);
            formattedMsg.push_back('\0');
            OutputDebugStringA(formattedMsg.data());
        }
        
        virtual void    FlushImpl() override {}

    private:
        bool            m_checkDebuggerPresent = true;
    };

    using MSVCTargetMT = MSVCTarget<std::mutex>;
    using MSVCTargetST = MSVCTarget<NullMutex>;
}
#endif
// ConsoleWindowLogTarget.h
#pragma once
#include "Nessie/Debug/Logger/LogTargets/LogTargetBase.h"

namespace nes
{
    class EditorConsole;
    
    template <MutexType Mutex>
    class EditorConsoleLogTarget final : public LogTargetBase<Mutex>
    {
    public:
        void            SetConsoleWindow(EditorConsole* pConsole) { m_pConsole = pConsole; }
        
    protected:
        virtual void    LogImpl(const internal::LogMessage& message) override;
        virtual void    FlushImpl() override {}
        static void     PostToConsole(EditorConsole* pConsole, const LogMemoryBuffer& formattedMsg);

        EditorConsole*  m_pConsole = nullptr;
    };

    template <MutexType Mutex>
    void EditorConsoleLogTarget<Mutex>::LogImpl(const internal::LogMessage& message)
    {
        if (!m_pConsole)
            return;
        
        LogMemoryBuffer formattedMsg;
        LogTargetBase<Mutex>::m_pFormatter->Format(message, formattedMsg);
        //formattedMsg.push_back('\0');
        PostToConsole(m_pConsole, formattedMsg);
    }

    using EditorConsoleLogTargetMT = EditorConsoleLogTarget<std::mutex>;
    using EditorConsoleLogTargetST = EditorConsoleLogTarget<NullMutex>;
}

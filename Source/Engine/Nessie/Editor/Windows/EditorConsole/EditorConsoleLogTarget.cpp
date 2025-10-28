// EditorConsoleLogTarget.cpp
#include "EditorConsoleLogTarget.h"
#include "Nessie/Editor/Windows/EditorConsole.h"

namespace nes
{
    template <>
    void EditorConsoleLogTarget<std::mutex>::PostToConsole(EditorConsole* pConsole, const LogMemoryBuffer& formattedMsg)
    {
        pConsole->PostToConsole(formattedMsg);
    }

    template <>
    void EditorConsoleLogTarget<NullMutex>::PostToConsole(EditorConsole* pConsole, const LogMemoryBuffer& formattedMsg)
    {
        pConsole->PostToConsole(formattedMsg);
    }
}

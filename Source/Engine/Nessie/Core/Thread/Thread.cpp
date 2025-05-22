// Thread.cpp
#include "Thread.h"
#include "Core/Config.h"

#ifdef NES_PLATFORM_WINDOWS
#pragma warning(push)
#pragma warning(disable:5039)
    #ifndef WIN32_LEAN_AND_MEAN
        #define WIN32_LEAN_AND_MEAN
    #endif
#include <Windows.h>
#pragma warning(pop)
#endif

namespace nes
{
#ifdef NES_PLATFORM_WINDOWS
    //----------------------------------------------------------------------------------------------------
    /// @brief : Sets the current Thread Name in the MSVC debugger.
    //----------------------------------------------------------------------------------------------------
    static void RaiseThreadNameException(const char* threadName)
    {
    #pragma pack(push, 8)
        struct THREADNAME_INFO
        {
            DWORD   dwType;         // Must be 0x1000.
            LPCSTR  szThreadName;   // Pointer to name (user address space).
            DWORD   dwThreadID;     // Thread ID (-1 = caller thread)
            DWORD   dwFlags;        // Reserved for future use, must be zero.
        };
    #pragma pack(pop)

        THREADNAME_INFO info;
        info.dwType = 0x1000;
        info.szThreadName = threadName;
        info.dwThreadID = static_cast<DWORD>(-1);
        info.dwFlags = 0;

        __try
        {
            RaiseException(0x406D1388, 0, sizeof(info)/ sizeof(ULONG_PTR), reinterpret_cast<ULONG_PTR*>(&info));            
        }
        __except (EXCEPTION_EXECUTE_HANDLER)
        {
        }
    }

    
    void SetThreadName(const char* threadName)
    {
#pragma warning(push)
#pragma warning(disable:4191)
        using SetThreadDescriptionFunc = HRESULT(WINAPI*)(HANDLE hThread, PCWSTR lpThreadDescription);
        static SetThreadDescriptionFunc setThreadDescription = reinterpret_cast<SetThreadDescriptionFunc>(GetProcAddress(GetModuleHandleW(L"Kernel32.dll"), "SetThreadDescription"));
#pragma warning(pop)

        if (setThreadDescription)
        {
            wchar_t nameBuffer[64] = { 0 };
            if (MultiByteToWideChar(CP_UTF8, 0, threadName, -1, nameBuffer, sizeof(nameBuffer) / sizeof(wchar_t) - 1) == 0)
            {
                return;   
            }
            setThreadDescription(GetCurrentThread(), nameBuffer);
        }
        
        else if (IsDebuggerPresent())
        {
            RaiseThreadNameException(threadName);
        }
    }
#else
    void SetThreadName([[maybe_unused]] const char* threadName)
    {
        //
    }
#endif


}
// Memory.cpp
#include "Memory.h"
#include "Debug/Assert.h"

#if defined(NES_PLATFORM_WINDOWS)
#include <crtdbg.h>
#endif

//---------------------------------------------------------------------------------------------------------------------
// The following is from Rez Graham's Bleach Leak Detector. It will save a string record for each memory allocation so that
// the output message when there are memory leaks will tell you exactly where the allocation occurred.
//---------------------------------------------------------------------------------------------------------------------

//---------------------------------------------------------------------------------------------------------------------
// Memory debugging.  We maintain two hash maps, one for all the records keyed by address and one to keep track of 
// the incremental ids, keyed by source hash.  The source hash is the hash of the filename and line number.  When an 
// allocation happens, we generate a new record and insert it into the records hash and then increment the count for 
// it in the counts hash.
// 
// This process gives us two important things:
//      1) Allocations are categorized by source.
//      2) All allocations of a particular source will have a consistent id.
// 
// So if you have a loop allocating 10 objects, you will have 10 different records keyed by the addresses which 
// contain the order in which that allocation was made.  You can then use BLEACH_NEW_BREAK() to break on that specific 
// allocation.
//---------------------------------------------------------------------------------------------------------------------
#if NES_ENABLE_ALLOCATION_TRACKING
    #include <mutex>
    #include <atomic>
    #include <unordered_map>
    #include <string>

    //-----------------------------------------------------------------------------------------------------------------
    // Windows is required.
    //-----------------------------------------------------------------------------------------------------------------
    #ifdef NES_PLATFORM_WINDOWS
    #include "Platform/Windows/WindowsInclude.h"
    #else
        #error "Allocation Tracking requires a Windows platform."
    #endif

    //-----------------------------------------------------------------------------------------------------------------
    // Macro to break into the debugger.
    //-----------------------------------------------------------------------------------------------------------------
    #ifndef BREAK_INTO_DEBUGGER
        #if defined(_MSC_VER)
            extern void __cdecl __debugbreak(void);
            #define BREAK_INTO_DEBUGGER() __debugbreak()
        #else
            #error "Couldn't generate BREAK_INTO_DEBUGGER() macro."
        #endif
    #endif

    
    namespace nes::memory::internal
    {
        using Filename = std::string;
        using StringHasher = std::hash<const char*>;
        
        //---------------------------------------------------------------------------------------------------------------------
        // Memory debugger class, used for storing memory allocation records.
        //---------------------------------------------------------------------------------------------------------------------
        class MemoryDebugger
        {
            struct MemoryRecord
            {
                uint64_t id;  // unique ID per allocation which is incrementally updated
                uint32_t allocLocationHash;  // hash of location and line number to serve as a unique allocation point
                void* pAddress;  // the address of the returned allocation

                MemoryRecord(uint32_t _allocLocationHash, void* _pAddress, uint64_t _id)
                    : allocLocationHash(_allocLocationHash)
                    , pAddress(_pAddress)
                    , id(_id)
                {
                    //
                }
            };

            struct CountRecord
            {
                Filename filename;
                int line;
                uint64_t count;

                CountRecord(const char* _filename, int _line, uint64_t _count)
                    : filename(_filename)
                    , line(_line)
                    , count(_count)
                {
                    //
                }
            };

            using Counts = std::unordered_map<uint32_t, CountRecord>;
            using Records = std::unordered_map<size_t, MemoryRecord>;

            Counts m_counts;  // memory hash => CountRecord
            Records m_records;  // pointer => MemoryRecord
            std::recursive_mutex m_mutex;
            std::atomic_bool m_destroying;

        public:
            MemoryDebugger()
                : m_destroying(false)
            {
                // find() crashes if the bucket array has a zero length, so this gets around that
                m_counts.reserve(4);
                m_records.reserve(4);
            }

            ~MemoryDebugger()
            {
                m_destroying = true;  // *sigh*
            }

            void AddRecord(void* pPtr, const char* filename, int lineNum, uint64_t breakPoint = 0)
            {
                if (m_destroying)
                    return;

                std::lock_guard<std::recursive_mutex> lock(m_mutex);

                // generate the hash
                const uint32_t allocHash = HashMemoryEntry(filename, lineNum);

                // add or update the memory records
                auto findIt = m_counts.find(allocHash);
                if (findIt != m_counts.end())
                {
                    CountRecord& countRecord = findIt->second;
                    ++countRecord.count;
                    if (countRecord.count == breakPoint)
                    {
                        BREAK_INTO_DEBUGGER();
                    }
                    m_records.emplace(reinterpret_cast<size_t>(pPtr), MemoryRecord{ allocHash, pPtr, countRecord.count });
                }
                else
                {
                    if (breakPoint == 1)
                    {
                        BREAK_INTO_DEBUGGER();
                    }
                    m_counts.emplace(allocHash, CountRecord{ filename, lineNum, 1 });
                    m_records.emplace(reinterpret_cast<size_t>(pPtr), MemoryRecord{ allocHash, pPtr, 1 });
                }
            }

            void RemoveRecord(void* pPtr)
            {
                if (m_destroying)
                    return;
                std::lock_guard<std::recursive_mutex> lock(m_mutex);
                m_records.erase(reinterpret_cast<size_t>(pPtr));
            }

            void DumpMemoryRecords()
            {
                static constexpr size_t kBufferLength = 256;

                if (m_destroying)
                    return;

                std::lock_guard<std::recursive_mutex> lock(m_mutex);

                ::OutputDebugStringA("========================================\n");
                ::OutputDebugStringA("Remaining Allocations:\n");

                char buffer[kBufferLength];
                uint64_t rowNum = 0;
                for (const auto& addressRecordPair : m_records)
                {
                    // should be a structured binding, but I want to keep compatible with C++ 14
                    const auto& address = addressRecordPair.first;
                    const auto& record = addressRecordPair.second;

                    std::memset(buffer, 0, kBufferLength);
                    auto findIt = m_counts.find(record.allocLocationHash);
                    if (findIt != m_counts.end())
                        InternalSprintf(buffer, kBufferLength, "%llu> %s(%d)\n    => [0x%x] ID: %llu\n", rowNum, findIt->second.filename.c_str(), findIt->second.line, address, record.id);
                    else
                        InternalSprintf(buffer, kBufferLength, "%llu> (No Record)\n    => [0x%x] ID: %llu\n", rowNum, address, record.id);
                    ::OutputDebugStringA(buffer);
                    ++rowNum;
                }
                ::OutputDebugStringA("========================================\n");
            }

        private:
            static uint32_t HashMemoryEntry(const char* filename, int lineNum)
            {
                uint32_t allocHash = static_cast<uint32_t>(StringHasher()(filename));
                allocHash ^= lineNum;
                return allocHash;
            }

            template <class... Args>
            static void InternalSprintf(char* buffer, size_t sizeOfBuffer, const char* format, Args&&... args)
            {
            #ifdef _MSC_VER
                _sprintf_p(buffer, sizeOfBuffer, format, std::forward<Args>(args)...);
            #else
                sprintf(buffer, format, std::forward<Args>(args)...);
            #endif
            }
        };

        //---------------------------------------------------------------------------------------------------------------------
        // Pointer to the global memory debugger instance.
        //---------------------------------------------------------------------------------------------------------------------
        static MemoryDebugger* g_pMemoryDebugger = nullptr;

        //---------------------------------------------------------------------------------------------------------------------
        // Interface free functions.  These are exposed to the interface, but you should prefer the macros instead.
        //---------------------------------------------------------------------------------------------------------------------
        void InitLeakDetector()
        {
            ::OutputDebugStringA("Initializing Bleach Leak Detector.\n");
            _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
            if (!g_pMemoryDebugger)
                g_pMemoryDebugger = new MemoryDebugger;  // purposefully not using the overloaded version of new
        }

        void DumpMemoryRecords()
        {
            if (g_pMemoryDebugger)
                g_pMemoryDebugger->DumpMemoryRecords();
        }

        void DumpAndDestroyLeakDetector()
        {
            if (g_pMemoryDebugger)
            {
                DumpMemoryRecords();
                delete g_pMemoryDebugger;
                g_pMemoryDebugger = nullptr;  // purposefully not using the overloaded version of delete
                ::OutputDebugStringA("Exiting Bleach Leak Detector.\n");
            }
        }
        
        //---------------------------------------------------------------------------------------------------------------------
        // Internal free functions.
        //---------------------------------------------------------------------------------------------------------------------
        static void AddRecord(void* pPtr, const char* filename, int lineNum, uint64_t breakPoint = 0)
        {
            if (g_pMemoryDebugger)
                g_pMemoryDebugger->AddRecord(pPtr, filename, lineNum, breakPoint);
        }

        static void RemoveRecord(void* pPtr)
        {
            if (g_pMemoryDebugger)
                g_pMemoryDebugger->RemoveRecord(pPtr);
        }
    }

#else  // !ENABLED_MEMORY_DEBUGGING

//---------------------------------------------------------------------------------------------------------------------
// Stubs for the free functions if we're not using logging allocations.
//---------------------------------------------------------------------------------------------------------------------
namespace nes::memory::internal
{
    void InitLeakDetector()
    {
    #if defined(NES_PLATFORM_WINDOWS)
        _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
    #endif
    }

    void DumpAndDestroyLeakDetector() {}

#if NES_LOGGING_ENABLED
    static void AddRecord(void*, const char*, int, uint64_t = 0) {}
    static void RemoveRecord(void*) {}
#endif

}

#endif  // ENABLED_MEMORY_DEBUGGING


namespace nes::memory::internal
{
    void* Allocate(const size_t size)
    {
        NES_ASSERT(size > 0);
        return malloc(size);
    }

    void Free(void* pMemory)
    {
        free(pMemory);
    }

    void* AlignedAllocate(const size_t size, const size_t alignment)
    {
        NES_ASSERT(size > 0 && alignment > 0);

#if defined(NES_PLATFORM_WINDOWS)
        return _aligned_malloc(size, alignment);
#else
#error "AlignedAllocate() not implemented for platform".
#endif
    }

    void AlignedFree(void* pMemory)
    {
#if defined(NES_PLATFORM_WINDOWS)
        return _aligned_free(pMemory);
#else
#error "AlignedFree() not implemented for platform".
#endif
    }
}

#ifdef NES_DEBUG
namespace nes::memory::internal
{
    void* DebugAllocate(size_t size, const char* filename, int lineNum)
    {
#if defined(NES_PLATFORM_WINDOWS)
        void* pMem = _malloc_dbg(size, 1, filename, lineNum);
        AddRecord(pMem, filename, lineNum);
        return pMem;
#else
#error "DebugAllocate() not implemented for platform".
#endif
    }

    void DebugFree(void* pMemory)
    {
        RemoveRecord(pMemory);
        
#if defined(NES_PLATFORM_WINDOWS)
        return _free_dbg(pMemory, 1);
#else
#error "DebugFree() not implemented for platform".
#endif
    }

    void* DebugAlignedAllocate(size_t size, size_t alignment, const char* filename, int lineNum)
    {
#if defined(NES_PLATFORM_WINDOWS)
        void* pMem =  _aligned_malloc_dbg(size, alignment, filename, lineNum);
        AddRecord(pMem, filename, lineNum);
        return pMem;
#else
#error "DebugAlignedAllocate() not implemented for platform".
#endif
    }

    void DebugAlignedFree(void* pMemory)
    {
        RemoveRecord(pMemory);
        
#if defined(NES_PLATFORM_WINDOWS)
        return _aligned_free_dbg(pMemory);
#else
#error "DebugAlignedFree() not implemented for platform".
#endif
    }
}

void* operator new(const size_t size, const char* filename, const int lineNum)
{
    return nes::memory::internal::DebugAllocate(size, filename, lineNum);
}

void operator delete(void* pMemory)
{
    nes::memory::internal::DebugFree(pMemory);
}

void operator delete(void* pMemory, std::align_val_t) noexcept
{
    nes::memory::internal::DebugAlignedFree(pMemory);
}

void operator delete[](void* pMemory)
{
    nes::memory::internal::DebugFree(pMemory);
}

void operator delete(void* pMemory, const char*, int)
{
    return nes::memory::internal::DebugFree(pMemory);
}

void* operator new(const size_t size, std::align_val_t alignment, const char* filename, const int lineNum)
{
    return nes::memory::internal::DebugAlignedAllocate(size, static_cast<size_t>(alignment), filename, lineNum);
}

void operator delete(void* pMemory, [[maybe_unused]] std::align_val_t alignment, const char*, int)
{
    return nes::memory::internal::DebugAlignedFree(pMemory);
}

void* operator new[](const size_t size, const char* filename, const int lineNum)
{
    return nes::memory::internal::DebugAllocate(size, filename, lineNum);
}

void operator delete[](void* pMemory, const char*, int)
{
    return nes::memory::internal::DebugFree(pMemory);
}
#endif
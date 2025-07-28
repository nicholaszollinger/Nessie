// RendererDesc.cpp
#include "RendererDesc.h"
#include "Nessie/Core/Memory/Memory.h"

namespace nes
{
    static void* AlignedMalloc([[maybe_unused]] void* userArg, const size_t size, const size_t alignment)
    {
        return NES_ALIGNED_ALLOC(size, alignment);
    }

    static void* AlignedRealloc([[maybe_unused]] void* userArg, void* pMemory, const size_t size, const size_t alignment)
    {
        return NES_ALIGNED_REALLOC(pMemory, size, alignment);
    }

    static void AlignedFree([[maybe_unused]] void* userArg, void* memory)
    {
        NES_ALIGNED_FREE(memory);
    }

    //----------------------------------------------------------------------------------------------------
    /// @brief : Default message callback to use for the Renderer. Ignores the user arg.
    //----------------------------------------------------------------------------------------------------
    [[maybe_unused]]
    static void DefaultMessageCallback(const ELogLevel level, const char* file, const uint32 line, const char* message, const LogTag& tag, [[maybe_unused]] void* pUserArg)
    {
        if (auto logger = LoggerRegistry::Instance().GetDefaultLogger())
        {
            const internal::LogSource source(file, line, nullptr);
            logger->Log(source, level, tag, message);
        }
    }

    void AllocationCallbacks::EnsureValidCallbacksOrReset()
    {
        // If any are invalid, set to defualt.
        if (m_alloc == nullptr || m_free == nullptr || m_realloc == nullptr)
        {
            *this = AllocationCallbacks();
        }
    }

    DebugMessenger::DebugMessenger()
        : m_callback(nullptr)
        , m_pUserData(nullptr)
    {
        NES_IF_DEBUG(m_callback = DefaultMessageCallback);
    }

    DebugMessenger& DebugMessenger::SetCallback(const DebugMessageCallback& callback)
    {
        if (callback != nullptr)
            m_callback = callback;

        return *this;
    }

    DebugMessenger& DebugMessenger::SetUserData(void* pUserData)
    {
        m_pUserData = pUserData;
        return *this;
    }

    void DebugMessenger::SendMessage(const ELogLevel level, const char* file, const uint32 line, const char* message, const LogTag& tag) const
    {
        m_callback(level, file, line, message, tag, m_pUserData);
    }

    AllocationCallbacks::AllocationCallbacks()
        : m_alloc(AlignedMalloc)
        , m_free(AlignedFree)
        , m_realloc(AlignedRealloc)
        , m_pUserData(nullptr)
    {
        //
    }

    AllocationCallbacks& AllocationCallbacks::SetCallbacks(const AllocateFunction& alloc, const FreeFunction& free, const ReallocationFunction& realloc)
    {
        m_alloc = alloc;
        m_free = free;
        m_realloc = realloc;

        EnsureValidCallbacksOrReset();
        return *this;
    }

    AllocationCallbacks& AllocationCallbacks::SetUserData(void* pUserData)
    {
        m_pUserData = pUserData;
        return *this;
    }

    void* AllocationCallbacks::Allocate(const size_t size, const size_t alignment) const
    {
        return m_alloc(m_pUserData, size, alignment);
    }

    void AllocationCallbacks::Free(void* pMemory) const
    {
        m_free(m_pUserData, pMemory);
    }

    void* AllocationCallbacks::Reallocate(void* pOriginal, const size_t size, const size_t alignment) const
    {
        return m_realloc(m_pUserData, pOriginal, size, alignment);
    }

    RendererDesc::RendererDesc()
        : m_allocationCallbacks(AllocationCallbacks())
        , m_debugMessenger(DebugMessenger())
    {
        // Require a single graphics queue by default.
        m_requiredQueueCountsByFamily[static_cast<size_t>(EQueueType::Graphics)] = 1;
        m_requiredQueueCountsByFamily[static_cast<size_t>(EQueueType::Compute)] = 0;
        m_requiredQueueCountsByFamily[static_cast<size_t>(EQueueType::Transfer)] = 0;
        
#ifdef NES_RELEASE
        m_enableValidationLayer = false;
        m_enableVerbose = false;
#else
        m_enableValidationLayer = true;
        m_enableVerbose = true;
#endif
    }

    RendererDesc& RendererDesc::RequireAPIVersion(const Version& version)
    {
        m_apiVersion = version;
        return *this;
    }

    RendererDesc& RendererDesc::EnableValidationLayer([[maybe_unused]] const bool enable)
    {
#ifdef NES_RELEASE
        m_enableValidationLayer = false;
#else
        m_enableValidationLayer = enable;
#endif
    
        return *this;
    }

    RendererDesc& RendererDesc::SetAllocationCallbacks(AllocationCallbacks allocationCallbacks)
    {
        m_allocationCallbacks = allocationCallbacks;
        m_allocationCallbacks.EnsureValidCallbacksOrReset();
        return *this;
    }

    RendererDesc& RendererDesc::SetDebugMessageCallback(const DebugMessageCallback& debugMessageCallback)
    {
        m_debugMessenger.SetCallback(debugMessageCallback);
        return *this;
    }

    RendererDesc& RendererDesc::SetDebugMessengerUserData(void* pUserData)
    {
        m_debugMessenger.SetUserData(pUserData);
        return *this;
    }

    RendererDesc& RendererDesc::EnableSingleThreaded()
    {
        m_threadPolicy = EThreadPolicy::SingleThreaded;
        return *this;
    }

    RendererDesc& RendererDesc::EnableMultiThreaded()
    {
        m_threadPolicy = EThreadPolicy::Multithreaded;
        return *this;
    }

    RendererDesc& RendererDesc::RequireDedicatedComputeQueue()
    {
        m_requireDedicatedComputeQueue = true;
        return *this;
    }

    RendererDesc& RendererDesc::RequireSeparateComputeQueue()
    {
        m_requireSeparateComputeQueue = true;
        return *this;
    }

    RendererDesc& RendererDesc::RequireDedicatedTransferQueue()
    {
        m_requireDedicatedTransferQueue = true;
        return *this;
    }

    RendererDesc& RendererDesc::RequireSeparateTransferQueue()
    {
        m_requireSeparateTransferQueue = true;
        return *this;
    }
    
    RendererDesc& RendererDesc::RequireQueueType(const EQueueType type, const uint32 count)
    {
        const size_t index = static_cast<size_t>(type);
        NES_ASSERT(index < static_cast<size_t>(EQueueType::MaxNum));
        m_requiredQueueCountsByFamily[index] = count;
        return *this;
    }

    RendererDesc& RendererDesc::ForceGPUAtIndex(const int index)
    {
        m_forceGPU = index;
        return *this;
    }
}

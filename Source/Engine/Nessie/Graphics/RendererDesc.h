// RendererConfig.h
#pragma once
#include "GraphicsCommon.h"
#include "GraphicsCore.h"
#include "Nessie/Core/Version.h"
#include "Nessie/Core/Thread/Thread.h"

namespace nes
{
    //----------------------------------------------------------------------------------------------------
    // [TODO]: Move this class and the static functions to Core/Memory.
    /// @brief : Set of functors (Allocate, Free, and Reallocate) that can be passed into the Renderer to use.
    //----------------------------------------------------------------------------------------------------
    struct AllocationCallbacks
    {
        using AllocateFunction = void* (*)(void* pUserData, size_t size, size_t alignment);
        using FreeFunction = void (*)(void* pUserData, void* pMemory);
        using ReallocationFunction = void* (*)(void* pUserData, void* pOriginal, size_t size, size_t alignment);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Default constructor will set defaults for all callbacks. The default is NES_ALIGNED_""()
        ///     with no user data.
        //----------------------------------------------------------------------------------------------------
        AllocationCallbacks();

        //----------------------------------------------------------------------------------------------------
        /// @brief : Set all callbacks at once. You must provide all callbacks. If any are not provided, the
        ///     default implementation will be used.
        //----------------------------------------------------------------------------------------------------
        AllocationCallbacks&    SetCallbacks(const AllocateFunction& alloc, const FreeFunction& free, const ReallocationFunction& realloc);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Set the user data to pass into the different callbacks. The default is nullptr.
        //----------------------------------------------------------------------------------------------------
        AllocationCallbacks&    SetUserData(void* pUserData);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Allocate memory.
        //----------------------------------------------------------------------------------------------------
        void*                   Allocate(const size_t size, const size_t alignment) const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Free memory.
        //----------------------------------------------------------------------------------------------------
        void                    Free(void* pMemory) const;                                   

        //----------------------------------------------------------------------------------------------------
        /// @brief : Reallocate memory.
        //----------------------------------------------------------------------------------------------------
        void*                   Reallocate(void* pOriginal, const size_t size, const size_t alignment) const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Ensure that *all* the message callbacks are valid. If not, it will be reset to default. 
        //----------------------------------------------------------------------------------------------------
        void                    EnsureValidCallbacksOrReset();
    
        AllocateFunction        m_alloc;
        FreeFunction            m_free;
        ReallocationFunction    m_realloc;
        void*                   m_pUserData = nullptr;
    };

    //----------------------------------------------------------------------------------------------------
    /// @brief : Allocate a type using custom allocation callbacks.
    //----------------------------------------------------------------------------------------------------
    template <typename Type, typename ...CtorParams>
    Type* Allocate(const AllocationCallbacks& callbacks, CtorParams&&...params)
    {
        Type* pObject = static_cast<Type*>(callbacks.Allocate(sizeof(Type), alignof(Type)));
        if (pObject)
        {
            new (pObject) Type(std::forward<CtorParams>(params)...);
        }
        return pObject;
    }

    //----------------------------------------------------------------------------------------------------
    /// @brief : Destroy a type using custom allocation callbacks. The pointer will be set to nullptr.
    //----------------------------------------------------------------------------------------------------
    template <typename Type>
    void Free(const AllocationCallbacks& callbacks, Type*& pObject)
    {
        if (pObject)
        {
            pObject->~Type();           // Destruct
            callbacks.Free(pObject);    // Free
            pObject = nullptr;          // Set pointer to null.
        }
    }

    using DebugMessageCallback = void (*)(ELogLevel level, const char* file, const uint32 line, const char* message, const nes::LogTag& tag, void* pUserArg);

    struct DebugMessenger
    {
        //----------------------------------------------------------------------------------------------------
        /// @brief : Default implementation will post the log to the default logger, and disregard the user data.   
        //----------------------------------------------------------------------------------------------------
        DebugMessenger();

        //----------------------------------------------------------------------------------------------------
        /// @brief : Set the debug callback to use.
        //----------------------------------------------------------------------------------------------------
        DebugMessenger&         SetCallback(const DebugMessageCallback& callback);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Set the user data for the callback.
        //----------------------------------------------------------------------------------------------------
        DebugMessenger&         SetUserData(void* pUserData);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Call the message callback, attaching the set user data.
        //----------------------------------------------------------------------------------------------------
        void                    SendMessage(const ELogLevel, const char* file, const uint32 line, const char* message, const LogTag& tag) const;
    
        DebugMessageCallback    m_callback;
        void*                   m_pUserData = nullptr;
    };
    
    //----------------------------------------------------------------------------------------------------
    /// @brief : Used to request rendering features for the application.
    //----------------------------------------------------------------------------------------------------
    struct RendererDesc
    {
        RendererDesc();

        //----------------------------------------------------------------------------------------------------
        /// @brief : Require an API version for the Graphics API. Default is 1.3.0 for Vulkan.
        //----------------------------------------------------------------------------------------------------
        RendererDesc&               RequireAPIVersion(const Version& version);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Set whether to enable validate layers. Default is true. For release, this is false regardless.
        //----------------------------------------------------------------------------------------------------
        RendererDesc&               EnableValidationLayer(const bool enable = true);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Set custom allocation callbacks for the Renderer. A default is provided.
        //----------------------------------------------------------------------------------------------------
        RendererDesc&               SetAllocationCallbacks(AllocationCallbacks allocationCallbacks);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Set a custom debug allocation callback for the renderer. A default will be provided. 
        //----------------------------------------------------------------------------------------------------
        RendererDesc&               SetDebugMessageCallback(const DebugMessageCallback& debugMessageCallback);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Set the user pointer for the DebugMessenger. 
        //----------------------------------------------------------------------------------------------------
        RendererDesc&               SetDebugMessengerUserData(void* pUserData);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Require a queue family that supports compute operations but not graphics or transfer.
        //----------------------------------------------------------------------------------------------------
        RendererDesc&               RequireDedicatedComputeQueue();

        //----------------------------------------------------------------------------------------------------
        /// @brief : Require a queue family that supports compute operations but not graphics.
        //----------------------------------------------------------------------------------------------------
        RendererDesc&               RequireSeparateComputeQueue();

        //----------------------------------------------------------------------------------------------------
        /// @brief : Require a queue family that supports transfer operations but not graphics or compute.
        //----------------------------------------------------------------------------------------------------
        RendererDesc&               RequireDedicatedTransferQueue();

        //----------------------------------------------------------------------------------------------------
        /// @brief : Require a queue family that supports transfer operations but not graphics.
        //----------------------------------------------------------------------------------------------------
        RendererDesc&               RequireSeparateTransferQueue();

        //----------------------------------------------------------------------------------------------------
        /// @brief : Require that a certain number of queues are available by type. By default, only a single
        ///     graphics queue is requested. Both Compute and Transfer queue counts are set to 0.
        //----------------------------------------------------------------------------------------------------
        RendererDesc&               RequireQueueType(const EQueueType type, const uint32 count = 1);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Force the usage of a specific GPU. If not set, the best GPU will be selected.
        //----------------------------------------------------------------------------------------------------
        RendererDesc&               ForceGPUAtIndex(const int index);

        using QueueFamilyNumArray = std::array<uint32, static_cast<size_t>(EQueueType::MaxNum)>;

        Version                     m_apiVersion = {1, 3, 0};
        std::vector<const char*>    m_instanceExtensions = {};
        std::vector<ExtensionDesc>  m_deviceExtensions = {};
        AllocationCallbacks         m_allocationCallbacks{};
        DebugMessenger              m_debugMessenger{};
        QueueFamilyNumArray         m_requiredQueueCountsByFamily;
        int                         m_forceGPU = -1;                /// If != -1, then the GPU at the given index will be used.
        bool                        m_enableAllFeatures = true;     /// If true, enable all capable 'features' from the GPU.
        bool                        m_useDebugMessenger = true;
        bool                        m_requireDedicatedComputeQueue = false;
        bool                        m_requireDedicatedTransferQueue = false;
        bool                        m_requireSeparateComputeQueue = false;
        bool                        m_requireSeparateTransferQueue = false;
        bool                        m_enableValidationLayer;
        bool                        m_enableVerbose;
    };
}
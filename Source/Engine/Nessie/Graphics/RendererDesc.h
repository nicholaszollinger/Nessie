// RendererConfig.h
#pragma once
#include "GraphicsCore.h"
#include "Nessie/Core/Version.h"
#include "Nessie/Core/Thread/Thread.h"

namespace nes
{
    //----------------------------------------------------------------------------------------------------
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

    using DebugMessageCallback = void (*)(ELogLevel level, const char* file, const uint32 line, const char* functionName, const char* message, void* pUserArg);

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
        void                    SendMessage(const ELogLevel, const char* file, const uint32 line, const char* functionName, const char* message) const;
    
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
        RendererDesc&       RequireAPIVersion(const Version& version);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Set whether to enable validate layers. Default is true. For release, this is false regardless.
        //----------------------------------------------------------------------------------------------------
        RendererDesc&       EnableValidationLayer(const bool enable = true);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Set custom allocation callbacks for the Renderer. A default is provided.
        //----------------------------------------------------------------------------------------------------
        RendererDesc&       SetAllocationCallbacks(AllocationCallbacks allocationCallbacks);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Set a custom debug allocation callback for the renderer. A default will be provided. 
        //----------------------------------------------------------------------------------------------------
        RendererDesc&       SetDebugMessageCallback(const DebugMessageCallback& debugMessageCallback);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Set the user pointer for the DebugMessenger. 
        //----------------------------------------------------------------------------------------------------
        RendererDesc&       SetDebugMessengerUserData(void* pUserData);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Make the renderer run in single-threaded mode.
        //----------------------------------------------------------------------------------------------------
        RendererDesc&       EnableSingleThreaded();

        //----------------------------------------------------------------------------------------------------
        /// @brief : Make the renderer run in multithreaded mode.
        //----------------------------------------------------------------------------------------------------
        RendererDesc&       EnableMultiThreaded();
    
        Version             m_apiVersion = Version(1, 3, 0);
        AllocationCallbacks m_allocationCallbacks{};
        DebugMessenger      m_debugMessenger{};
        EThreadPolicy       m_threadPolicy = EThreadPolicy::Multithreaded;
        bool                m_enableValidationLayer;
    };
}
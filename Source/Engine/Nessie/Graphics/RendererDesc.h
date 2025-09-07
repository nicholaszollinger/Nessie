// RendererConfig.h
#pragma once
#include "GraphicsCommon.h"
#include "GraphicsCore.h"
#include "Nessie/Core/Version.h"
#include "Nessie/Core/Thread/Thread.h"
#include "Nessie/Core/Memory/AllocationCallbacks.h"

namespace nes
{
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

        Version                     m_apiVersion = {1, 4, 0};
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
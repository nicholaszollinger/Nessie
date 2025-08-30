// VulkanObject.h
#pragma once
#include "VulkanCore.h"

namespace nes
{
    //----------------------------------------------------------------------------------------------------
    /// @brief : Checks for a native Vulkan Object type Ex: VkBuffer.
    //----------------------------------------------------------------------------------------------------
    template <typename Type>
    concept VulkanObjectNativeType = requires
    {
        std::same_as<Type, VkBuffer>
        || std::same_as<Type, VkBufferView>
        || std::same_as<Type, VkCommandBuffer>
        || std::same_as<Type, VkCommandPool>
        || std::same_as<Type, VkDescriptorPool>
        || std::same_as<Type, VkDescriptorSet>
        || std::same_as<Type, VkDescriptorSetLayout>
        || std::same_as<Type, VkDevice>
        || std::same_as<Type, VkDeviceMemory>
        || std::same_as<Type, VkFence>
        || std::same_as<Type, VkFramebuffer>
        || std::same_as<Type, VkImage>
        || std::same_as<Type, VkImageView>
        || std::same_as<Type, VkInstance>
        || std::same_as<Type, VkPipeline>
        || std::same_as<Type, VkPipelineCache>
        || std::same_as<Type, VkPipelineLayout>
        || std::same_as<Type, VkQueryPool>
        || std::same_as<Type, VkRenderPass>
        || std::same_as<Type, VkSampler>
        || std::same_as<Type, VkSemaphore>
        || std::same_as<Type, VkShaderModule>
        || std::same_as<Type, VkSurfaceKHR>
        || std::same_as<Type, VkSwapchainKHR>
        || std::same_as<Type, VkPhysicalDevice>
        || std::same_as<Type, VkQueue>
        || std::same_as<Type, VkShaderEXT>
        || std::same_as<Type, VkAccelerationStructureKHR>;
    };

    //----------------------------------------------------------------------------------------------------
    /// @brief : Checks for a Vulkan Object type that is in the vk namespace. Ex: vk::Buffer.
    //----------------------------------------------------------------------------------------------------
    template <typename Type>
    concept VulkanObjectCppType = requires()
    {
        // The Native Type must be a Vulkan Native type.
        VulkanObjectNativeType<typename Type::NativeType>;  
    };

    //----------------------------------------------------------------------------------------------------
    /// @brief : Checks for a Vulkan Object type that is in the vk::raii namespace. Ex: vk::raii::Buffer.
    //----------------------------------------------------------------------------------------------------
    template <typename Type>
    concept VulkanObjectRAIIType = requires()
    {
        VulkanObjectNativeType<typename Type::CType>;
        VulkanObjectCppType<typename Type::CppType>;
        std::same_as<decltype(Type::objectType), vk::ObjectType>;
    };

    //----------------------------------------------------------------------------------------------------
    /// @brief : Checks for a Vulkan object type in any of the forms: VkBuffer, vk::Buffer, vk::raii::Buffer. 
    //----------------------------------------------------------------------------------------------------
    template <typename Type>
    concept ValidVulkanObjectType = VulkanObjectNativeType<Type> || VulkanObjectCppType<Type> || VulkanObjectRAIIType<Type>;
    
    //----------------------------------------------------------------------------------------------------
    /// @brief : Get the vk::ObjectType value from a Vulkan Type.
    //----------------------------------------------------------------------------------------------------
    template <ValidVulkanObjectType Type>
    constexpr vk::ObjectType GetVkObjectType()
    {
        // vk::raii types.
        if constexpr (VulkanObjectRAIIType<Type>)
        {
            return Type::objectType;
        }

        // Objects that are in the vk namespace are implicitly convertible to the Native Types.
        #define SAME_OR_CONVERTIBLE(vkType) std::is_same_v<Type, vkType> || std::is_convertible_v<Type, vkType>
        
        else if constexpr(SAME_OR_CONVERTIBLE(VkBuffer))
            return vk::ObjectType::eBuffer;
        else if constexpr(SAME_OR_CONVERTIBLE(VkBufferView))
            return vk::ObjectType::eBufferView;
        else if constexpr(SAME_OR_CONVERTIBLE(VkCommandBuffer))
            return vk::ObjectType::eCommandBuffer;
        else if constexpr(SAME_OR_CONVERTIBLE(VkCommandPool))
            return vk::ObjectType::eCommandPool;
        else if constexpr(SAME_OR_CONVERTIBLE(VkDescriptorPool))
            return vk::ObjectType::eDescriptorPool;
        else if constexpr(SAME_OR_CONVERTIBLE(VkDescriptorSet))
            return vk::ObjectType::eDescriptorSet;
        else if constexpr(SAME_OR_CONVERTIBLE(VkDescriptorSetLayout))
            return vk::ObjectType::eDescriptorSetLayout;
        else if constexpr(SAME_OR_CONVERTIBLE(VkDevice))
            return vk::ObjectType::eDevice;
        else if constexpr(SAME_OR_CONVERTIBLE(VkDeviceMemory))
            return vk::ObjectType::eDeviceMemory;
        else if constexpr(SAME_OR_CONVERTIBLE(VkFence))
            return vk::ObjectType::eFence;
        else if constexpr(SAME_OR_CONVERTIBLE(VkFramebuffer))
            return vk::ObjectType::eFramebuffer;
        else if constexpr(SAME_OR_CONVERTIBLE(VkImage))
            return vk::ObjectType::eImage;
        else if constexpr(SAME_OR_CONVERTIBLE(VkImageView))
            return vk::ObjectType::eImageView;
        else if constexpr(SAME_OR_CONVERTIBLE(VkInstance))
            return vk::ObjectType::eInstance;
        else if constexpr(SAME_OR_CONVERTIBLE(VkPipeline))
            return vk::ObjectType::ePipeline;
        else if constexpr(SAME_OR_CONVERTIBLE(VkPipelineCache))
            return vk::ObjectType::ePipelineCache;
        else if constexpr(SAME_OR_CONVERTIBLE(VkPipelineLayout))
            return vk::ObjectType::ePipelineLayout;
        else if constexpr(SAME_OR_CONVERTIBLE(VkQueryPool))
            return vk::ObjectType::eQueryPool;
        else if constexpr(SAME_OR_CONVERTIBLE(VkRenderPass))
            return vk::ObjectType::eRenderPass;
        else if constexpr(SAME_OR_CONVERTIBLE(VkSampler))
            return vk::ObjectType::eSampler;
        else if constexpr(SAME_OR_CONVERTIBLE(VkSemaphore))
            return vk::ObjectType::eSemaphore;
        else if constexpr(SAME_OR_CONVERTIBLE(VkShaderModule))
            return vk::ObjectType::eShaderModule;
        else if constexpr(SAME_OR_CONVERTIBLE(VkSurfaceKHR))
            return vk::ObjectType::eSurfaceKHR;
        else if constexpr(SAME_OR_CONVERTIBLE(VkSwapchainKHR))
            return vk::ObjectType::eSwapchainKHR;
        else if constexpr(SAME_OR_CONVERTIBLE(VkPhysicalDevice))
            return vk::ObjectType::ePhysicalDevice;
        else if constexpr(SAME_OR_CONVERTIBLE(VkQueue))
            return vk::ObjectType::eQueue;
        else if constexpr(SAME_OR_CONVERTIBLE(VkShaderEXT))
            return vk::ObjectType::eShaderEXT;
        else if constexpr(SAME_OR_CONVERTIBLE(VkAccelerationStructureKHR))
            return vk::ObjectType::eAccelerationStructureKHR;
        else
        {
            static_assert(false, "Unsupported Vulkan object type");
            return vk::ObjectType::eUnknown;
        }

        #undef SAME_OR_CONVERTIBLE
    }
}
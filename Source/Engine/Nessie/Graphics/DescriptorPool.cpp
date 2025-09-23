// DescriptorPool.cpp
#include "DescriptorPool.h"
#include "PipelineLayout.h"
#include "Renderer.h"

namespace nes
{
    //----------------------------------------------------------------------------------------------------
    /// @brief : If the descriptorCount is non-zero, this will add the Pool Size at the current outPoolSizeIndex,
    ///     then increment it.
    //----------------------------------------------------------------------------------------------------
    static void AddDescriptorPoolSize(std::array<vk::DescriptorPoolSize, 16>& poolSizes, uint32& outPoolSizeIndex, const vk::DescriptorType type, const uint32 descriptorCount)
    {
        if (descriptorCount)
        {
            vk::DescriptorPoolSize& poolSize = poolSizes[outPoolSizeIndex++];
            poolSize.type = type;
            poolSize.descriptorCount = descriptorCount;
        }
    }
    
    DescriptorPool::DescriptorPool(DescriptorPool&& other) noexcept
        : m_pDevice(other.m_pDevice)
        , m_pool(std::move(other.m_pool))
    {
        other.m_pDevice = nullptr;
    }

    DescriptorPool& DescriptorPool::operator=(std::nullptr_t)
    {
        FreePool();
        return *this;
    }

    DescriptorPool& DescriptorPool::operator=(DescriptorPool&& other) noexcept
    {
        if (this != &other)
        {
            FreePool();

            m_pDevice = other.m_pDevice;
            m_pool = std::move(other.m_pool);
            
            other.m_pDevice = nullptr;
        }

        return *this;
    }

    DescriptorPool::~DescriptorPool()
    {
        FreePool();
    }

    DescriptorPool::DescriptorPool(RenderDevice& device, const DescriptorPoolDesc& desc)
        : m_pDevice(&device)
    {
        std::array<vk::DescriptorPoolSize, 16> poolSizes = {};
        uint32 poolSizeCount = 0;

        // Add each of the sizes from the description. If desc's max num == 0, it won't be added.
        AddDescriptorPoolSize(poolSizes, poolSizeCount, vk::DescriptorType::eSampler, desc.m_samplerMaxNum);
        AddDescriptorPoolSize(poolSizes, poolSizeCount, vk::DescriptorType::eUniformBuffer, desc.m_uniformBufferMaxNum);
        AddDescriptorPoolSize(poolSizes, poolSizeCount, vk::DescriptorType::eUniformBufferDynamic, desc.m_dynamicUniformBufferMaxNum);
        AddDescriptorPoolSize(poolSizes, poolSizeCount, vk::DescriptorType::eSampledImage, desc.m_imageMaxNum);
        AddDescriptorPoolSize(poolSizes, poolSizeCount, vk::DescriptorType::eStorageImage, desc.m_storageImageMaxNum);
        AddDescriptorPoolSize(poolSizes, poolSizeCount, vk::DescriptorType::eUniformTexelBuffer, desc.m_bufferMaxNum);
        AddDescriptorPoolSize(poolSizes, poolSizeCount, vk::DescriptorType::eStorageTexelBuffer, desc.m_storageTexelBufferMaxNum);
        AddDescriptorPoolSize(poolSizes, poolSizeCount, vk::DescriptorType::eStorageBuffer, desc.m_storageBufferMaxNum);
        AddDescriptorPoolSize(poolSizes, poolSizeCount, vk::DescriptorType::eAccelerationStructureKHR, desc.m_accelerationStructureMaxNum);

        vk::DescriptorPoolCreateFlags flags = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet;
        if (desc.m_flags & EDescriptorPoolBits::AllowUpdateAfterBound)
            flags |= vk::DescriptorPoolCreateFlagBits::eUpdateAfterBind;
        
        // Create the Pool:
        vk::DescriptorPoolCreateInfo poolInfo = vk::DescriptorPoolCreateInfo()
            .setFlags(flags)
            .setMaxSets(desc.m_descriptorSetMaxNum)
            .setPPoolSizes(poolSizes.data())
            .setPoolSizeCount(poolSizeCount);

        m_pool = vk::raii::DescriptorPool(device, poolInfo, device.GetVkAllocationCallbacks());
    }

    void DescriptorPool::AllocateDescriptorSets(const PipelineLayout& layout, const uint32 setIndex, DescriptorSet* pOutSets, const uint32 numInstances, const uint32 numVariableDescriptors)
    {
        std::unique_lock lock(m_mutex);

        // Get the layout for each of the instances:
        auto& setLayout = layout.GetVkDescriptorSetLayout(setIndex);
        std::vector<vk::DescriptorSetLayout> layouts(numInstances, *setLayout);
        
        const auto& bindingInfo = layout.GetBindingInfo();
        NES_ASSERT(setIndex < bindingInfo.m_setDescs.size());
        const DescriptorSetDesc* pSetDesc = &bindingInfo.m_setDescs[setIndex];
        const bool hasVariableDescriptorCount = bindingInfo.m_hasVariableDescriptorCounts[setIndex];

        vk::DescriptorSetVariableDescriptorCountAllocateInfo variableDescriptorCountInfo = vk::DescriptorSetVariableDescriptorCountAllocateInfo()
            .setPDescriptorCounts(&numVariableDescriptors)
            .setDescriptorSetCount(1);
        
        vk::DescriptorSetAllocateInfo info = vk::DescriptorSetAllocateInfo()
            .setPNext(hasVariableDescriptorCount? &variableDescriptorCountInfo : nullptr)
            .setDescriptorPool(*m_pool)
            .setDescriptorSetCount(numInstances)
            .setSetLayouts(layouts);

        // Allocate the vk::raii::DescriptorSets
        auto sets = m_pDevice->GetVkDevice().allocateDescriptorSets(info);
        NES_ASSERT(sets.size() == static_cast<uint64>(numInstances));

        // Fill the output array of instances: 
        for (uint32 i = 0; i < numInstances; ++i)
        {
            pOutSets[i] = DescriptorSet(*m_pDevice, pSetDesc, std::move(sets[i]));
        }
    }

    void DescriptorPool::Reset()
    {
        std::unique_lock lock(m_mutex);
        m_pool.reset();
    }

    void DescriptorPool::SetDebugName(const std::string& name)
    {
        NES_ASSERT(m_pDevice != nullptr);
        m_pDevice->SetDebugNameVkObject(GetNativeVkObject(), name);
    }

    NativeVkObject DescriptorPool::GetNativeVkObject() const
    {
        return NativeVkObject(*m_pool, vk::ObjectType::eDescriptorPool);
    }

    void DescriptorPool::FreePool()
    {
        if (m_pool != nullptr)
        {
            Renderer::SubmitResourceFree([pool = std::move(m_pool)]() mutable
            {
                pool = nullptr;
            });
        }

        m_pDevice = nullptr;
    }
}

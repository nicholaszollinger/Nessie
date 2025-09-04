// DescriptorSet.cpp
#include "DescriptorSet.h"

#include "RenderDevice.h"
#include "Renderer.h"

namespace nes
{
    static void WriteSamplers(vk::WriteDescriptorSet& writeDescriptorSet, size_t& scratchOffset, uint8* pScratch, const DescriptorBindingUpdateDesc& updateDesc)
    {
        auto* pImageInfos = reinterpret_cast<vk::DescriptorImageInfo*>(pScratch + scratchOffset);
        scratchOffset += updateDesc.m_descriptorCount * sizeof(vk::DescriptorImageInfo);

        for (uint32 i = 0; i < updateDesc.m_descriptorCount; ++i)
        {
            pImageInfos[i].imageView = nullptr;
            pImageInfos[i].imageLayout = vk::ImageLayout::eUndefined;
            pImageInfos[i].sampler = updateDesc.m_pDescriptors[i].GetVkSampler();
        }
        writeDescriptorSet.pImageInfo = pImageInfos;
    }

    static void WriteImages(vk::WriteDescriptorSet& writeDescriptorSet, size_t& scratchOffset, uint8* pScratch, const DescriptorBindingUpdateDesc& updateDesc)
    {
        auto* pImageInfos = reinterpret_cast<vk::DescriptorImageInfo*>(pScratch + scratchOffset);
        scratchOffset += updateDesc.m_descriptorCount * sizeof(vk::DescriptorImageInfo);

        for (uint32 i = 0; i < updateDesc.m_descriptorCount; ++i)
        {
            pImageInfos[i].imageView = updateDesc.m_pDescriptors[i].GetVkImageView();
            pImageInfos[i].imageLayout = updateDesc.m_pDescriptors[i].GetImageDesc().m_imageLayout;
            pImageInfos[i].sampler = nullptr;
        }
        writeDescriptorSet.pImageInfo = pImageInfos;
    }

    static void WriteBuffers(vk::WriteDescriptorSet& writeDescriptorSet, size_t& scratchOffset, uint8* pScratch, const DescriptorBindingUpdateDesc& updateDesc)
    {
        auto* pBufferInfos = reinterpret_cast<vk::DescriptorBufferInfo*>(pScratch + scratchOffset);
        scratchOffset += updateDesc.m_descriptorCount * sizeof(vk::DescriptorBufferInfo);

        for (uint32 i = 0; i < updateDesc.m_descriptorCount; ++i)
        {
            pBufferInfos[i] = updateDesc.m_pDescriptors[i].GetVkBufferInfo();
        }
        writeDescriptorSet.pBufferInfo = pBufferInfos;
    }

    static void WriteTypedBuffers(vk::WriteDescriptorSet& writeDescriptorSet, size_t& scratchOffset, uint8* pScratch, const DescriptorBindingUpdateDesc& updateDesc)
    {
        auto* pBufferViews = reinterpret_cast<vk::BufferView*>(pScratch + scratchOffset);
        scratchOffset += updateDesc.m_descriptorCount * sizeof(vk::BufferView);

        for (uint32 i = 0; i < updateDesc.m_descriptorCount; ++i)
        {
            pBufferViews[i] = updateDesc.m_pDescriptors[i].GetVkBufferView();
        }
        writeDescriptorSet.pTexelBufferView = pBufferViews;
    }

    // [TODO]: 
    static void WriteAccelerationStructures(vk::WriteDescriptorSet& /*writeDescriptorSet*/, size_t& /*scratchOffset*/, uint8* /*pScratch*/, const DescriptorBindingUpdateDesc& /*updateDesc*/)
    {
        NES_ASSERT(false, "Acceleration Structure not implemented yet!");
        //auto* pAccelerationStructures = reinterpret_cast<vk::AccelerationStructureKHR*>(pScratch + scratchOffset);
        //scratchOffset += updateDesc.m_descriptorCount * sizeof(vk::AccelerationStructureKHR);
    }

    using WriteDescriptorsFunc = void (*)(vk::WriteDescriptorSet& writeDescriptorSet, size_t& scratchOffset, uint8* pScratch, const DescriptorBindingUpdateDesc& updateDesc);

    constexpr WriteDescriptorsFunc GetDescriptorWriteFunc(const EDescriptorType type)
    {
        switch (type)
        {
            case EDescriptorType::AccelerationStructure:
                return WriteAccelerationStructures;

            case EDescriptorType::Buffer:
            case EDescriptorType::StorageBuffer:
                return WriteTypedBuffers;

            case EDescriptorType::UniformBuffer:
                return WriteBuffers;

            case EDescriptorType::Image:
            case EDescriptorType::StorageImage:
                return WriteImages;

            case EDescriptorType::Sampler:
                return WriteSamplers;
            
            default: break;
        }

        NES_ASSERT(false, "Unsupported type!");
        return nullptr;
    };
    
    DescriptorSet::DescriptorSet(DescriptorSet&& other) noexcept
        : m_pDevice(other.m_pDevice)
        , m_set(std::move(other.m_set))
        , m_pDesc(other.m_pDesc)
    {
        other.m_pDevice = nullptr;
        other.m_pDesc = nullptr;
    }

    DescriptorSet& DescriptorSet::operator=(std::nullptr_t)
    {
        FreeSet();
        return *this;
    }

    DescriptorSet& DescriptorSet::operator=(DescriptorSet&& other) noexcept
    {
        if (this != &other)
        {
            FreeSet();
            
            m_pDevice = other.m_pDevice;
            m_set = std::move(other.m_set);
            m_pDesc = other.m_pDesc;
            
            other.m_pDevice = nullptr;
            other.m_pDesc = nullptr;
        }

        return *this;
    }

    DescriptorSet::~DescriptorSet()
    {
        FreeSet();
    }

    void DescriptorSet::UpdateBindings(const DescriptorBindingUpdateDesc* pUpdateDescs, uint32 firstBinding, uint32 numBindings)
    {
        // Determine the number of bytes to store a vk::WriteDescriptorSet and the specific info struct for each binding
        // that we are updating.
        uint32 scratchSize = 0;
        for (uint32 i = 0; i < numBindings; ++i)
        {
            const DescriptorBindingUpdateDesc& updateDesc = pUpdateDescs[i];
            const DescriptorBindingDesc& bindingDesc = m_pDesc->m_pBindings[firstBinding + i];

            switch (bindingDesc.m_descriptorType)
            {
                case EDescriptorType::Sampler:
                case EDescriptorType::Image:
                case EDescriptorType::StorageImage:
                {
                    scratchSize += sizeof(vk::DescriptorImageInfo) * updateDesc.m_descriptorCount;
                    break;
                }

                case EDescriptorType::UniformBuffer:
                //case EDescriptorType::StructuredBuffer
                //case EDescriptorType::StorageStructuredBuffer
                {
                    scratchSize += sizeof(vk::DescriptorBufferInfo) * updateDesc.m_descriptorCount;
                    break;
                }

                case EDescriptorType::Buffer:
                case EDescriptorType::StorageBuffer:
                {
                    scratchSize += sizeof(vk::BufferView) * updateDesc.m_descriptorCount;
                    break;
                }

                case EDescriptorType::AccelerationStructure:
                {
                    scratchSize += sizeof(vk::AccelerationStructureKHR) * updateDesc.m_descriptorCount + sizeof(vk::WriteDescriptorSetAccelerationStructureKHR);
                    break;
                }
            }

            scratchSize += sizeof(vk::WriteDescriptorSet);
        }

        // The scratch buffer will contain room for all vk::WriteDescriptorSets and their specific write info structs.
        // - The beginning of the array will contain all vk::WriteDescriptorSet objects contiguously. This is how we are able to submit them
        //   in the update descriptors call below.
        // - The scratchOffset variable is the position of the first info struct following the block of vk::WriteDescriptorSets.
        // - The 'Descriptor Write Functions' move the scratchOffset by the size of the specific info struct.
        std::vector<uint8> scratch(scratchSize);
        size_t scratchOffset = numBindings * sizeof(vk::WriteDescriptorSet);

        // Update Bindings:
        for (uint32 i = 0; i < numBindings; ++i)
        {
            const DescriptorBindingUpdateDesc& updateDesc = pUpdateDescs[i];
            const DescriptorBindingDesc& bindingDesc = m_pDesc->m_pBindings[firstBinding + i];

            vk::WriteDescriptorSet& writeDescriptorSet = *reinterpret_cast<vk::WriteDescriptorSet*>(scratch.data() + i * sizeof(vk::WriteDescriptorSet));
            writeDescriptorSet = vk::WriteDescriptorSet()
                .setDstSet(*m_set)
                .setDescriptorCount(updateDesc.m_descriptorCount)
                .setDescriptorType(GetVkDescriptorType(bindingDesc.m_descriptorType));

            const bool isArray = bindingDesc.m_flags & (EDescriptorBindingBits::Array | EDescriptorBindingBits::VariableSizedArray);
            if (isArray)
            {
                writeDescriptorSet.setDstBinding(bindingDesc.m_bindingIndex);
                writeDescriptorSet.setDstArrayElement(updateDesc.m_baseDescriptorIndex);
            }
            else
            {
                writeDescriptorSet.setDstBinding(bindingDesc.m_bindingIndex);
            }

            // Update the bindings:
            auto* writeFunc = GetDescriptorWriteFunc(bindingDesc.m_descriptorType);
            writeFunc(writeDescriptorSet, scratchOffset, scratch.data(), updateDesc);
        }

        vk::ArrayProxy<const vk::WriteDescriptorSet> writes(numBindings, reinterpret_cast<const vk::WriteDescriptorSet*>(scratch.data()));
        m_pDevice->GetVkDevice().updateDescriptorSets(writes, nullptr);
    }

    void DescriptorSet::SetDebugName(const std::string& name)
    {
        NES_ASSERT(m_pDevice != nullptr);
        m_pDevice->SetDebugNameVkObject(GetNativeVkObject(), name);
    }

    NativeVkObject DescriptorSet::GetNativeVkObject() const
    {
        return NativeVkObject(*m_set, vk::ObjectType::eDescriptorSet);
    }
    
    DescriptorSet::DescriptorSet(RenderDevice& device, const DescriptorSetDesc* pDesc, vk::raii::DescriptorSet&& set)
        : m_pDevice(&device)
        , m_set(std::move(set))
        , m_pDesc(pDesc)
    {
        //
    }

    void DescriptorSet::FreeSet()
    {
        if (m_set != nullptr)
        {
            Renderer::SubmitResourceFree([set = std::move(m_set)]() mutable
            {
                set = nullptr; 
            });
        }

        m_pDevice = nullptr;
        m_pDesc = nullptr;
    }
}

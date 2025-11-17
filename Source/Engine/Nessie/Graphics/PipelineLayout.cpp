// PipelineLayout.cpp
#include "PipelineLayout.h"

#include "RenderDevice.h"
#include "Renderer.h"

namespace nes
{
    PipelineLayout::PipelineLayout(PipelineLayout&& other) noexcept
        : m_pDevice(other.m_pDevice)
        , m_layout(std::move(other.m_layout))
        , m_bindPoint(other.m_bindPoint)
        , m_descriptorSetLayouts(std::move(other.m_descriptorSetLayouts))
        , m_bindingInfo(std::move(other.m_bindingInfo))
    {
        
    }

    PipelineLayout& PipelineLayout::operator=(std::nullptr_t)
    {
        FreeLayout();
        return *this;
    }

    PipelineLayout& PipelineLayout::operator=(PipelineLayout&& other) noexcept
    {
        if (this != &other)
        {
            FreeLayout();

            m_pDevice = other.m_pDevice;
            m_layout = std::move(other.m_layout);
            m_descriptorSetLayouts = std::move(other.m_descriptorSetLayouts);
            m_bindingInfo = std::move(other.m_bindingInfo);
            m_bindPoint = other.m_bindPoint;

            other.m_pDevice = nullptr;
        }

        return *this;
    }

    PipelineLayout::~PipelineLayout()
    {
        FreeLayout();
    }

    PipelineLayout::PipelineLayout(RenderDevice& device, const PipelineLayoutDesc& desc)
        : m_pDevice(&device)
    {
        CreatePipelineLayout(device, desc);
    }
    
    void PipelineLayout::SetDebugName(const std::string& name)
    {
        m_pDevice->SetDebugNameVkObject(GetNativeVkObject(), name);
    }

    const vk::raii::DescriptorSetLayout& PipelineLayout::GetVkDescriptorSetLayout(const uint32 setIndex) const
    {
        NES_ASSERT(setIndex < m_descriptorSetLayouts.size());
        return m_descriptorSetLayouts[setIndex];
    }

    NativeVkObject PipelineLayout::GetNativeVkObject() const
    {
        return NativeVkObject(*m_layout, vk::ObjectType::ePipelineLayout);
    }

    void PipelineLayout::CreatePipelineLayout(const RenderDevice& device, const PipelineLayoutDesc& desc)
    {
        // Binding point:
        if (desc.m_shaderStages & EPipelineStageBits::GraphicsShaders)
            m_bindPoint = vk::PipelineBindPoint::eGraphics;
        else if (desc.m_shaderStages & EPipelineStageBits::ComputeShader)
            m_bindPoint = vk::PipelineBindPoint::eCompute;
        else if (desc.m_shaderStages & EPipelineStageBits::RayTracingShaders)
            m_bindPoint = vk::PipelineBindPoint::eRayTracingKHR;

        // Binding Info:
        size_t bindingCount = 0;
        // [TODO]: size_t dynamicConstantBufferCount = 0;
        for (uint32 i = 0; i < desc.m_descriptorSets.size(); ++i)
        {
            const auto& setDesc = *(desc.m_descriptorSets.begin() + i);
            bindingCount += setDesc.m_numBindings;
            // [TODO]: dynamicConstantBufferCount += setDesc.m_dynamicConstantBufferCount;
        }

        m_bindingInfo.m_setDescs.insert(m_bindingInfo.m_setDescs.begin(), desc.m_descriptorSets.begin(), desc.m_descriptorSets.end());
        m_bindingInfo.m_hasVariableDescriptorCounts.resize(desc.m_descriptorSets.size());
        m_bindingInfo.m_bindingDescs.reserve(bindingCount);
        // [TODO]: m_bindingInfo.m_dynamicConstantBufferDescs.reserve(dynamicConstantBufferCount);
        
        // Descriptor Set Layouts:
        std::vector<vk::DescriptorSetLayout> descriptorLayouts(desc.m_descriptorSets.size());
        for (uint32 i = 0; i < desc.m_descriptorSets.size(); ++i)
        {
            const auto& setDesc = *(desc.m_descriptorSets.begin() + i);
            CreateSetLayout(setDesc);

            // Add the result to the temp array (non-vk::raii version).
            descriptorLayouts[i] = *m_descriptorSetLayouts.back();

            // Update the binding info:
            // Note: We make a copy of the temp data in the set desc.
            m_bindingInfo.m_hasVariableDescriptorCounts[i] = false;
            m_bindingInfo.m_setDescs[i].m_pBindings = m_bindingInfo.m_bindingDescs.data() + m_bindingInfo.m_bindingDescs.size();
            m_bindingInfo.m_bindingDescs.insert(m_bindingInfo.m_bindingDescs.end(), setDesc.m_pBindings, setDesc.m_pBindings + setDesc.m_numBindings);
            
            // [TODO]: Dynamic Constant Buffer Version of above:

            // Check if any binding is a Variable Sized Array and set the m_hasVariableDescriptorCounts[i] = true.
            const DescriptorBindingDesc* pBindings = m_bindingInfo.m_setDescs[i].m_pBindings;
            for (uint32 j = 0; j < setDesc.m_numBindings; ++j)
            {
                if (m_pDevice->GetDesc().m_features.m_descriptorIndexing && (pBindings[j].m_flags & EDescriptorBindingBits::VariableSizedArray))
                {
                    m_bindingInfo.m_hasVariableDescriptorCounts[i] = true;
                    break;
                }
            }
        }
        
        // Push constants:
        m_bindingInfo.m_pushConstantBindings.resize(desc.m_pushConstants.size());
        std::vector<vk::PushConstantRange> pushConstants(desc.m_pushConstants.size());
        uint32 offset = 0;
        for (uint32 i = 0; i < desc.m_pushConstants.size(); ++i)
        {
            const PushConstantDesc& pushConstantDesc = *(desc.m_pushConstants.begin() + i);
            auto& vkPushConstantRange = pushConstants[i];

            const auto shaderStageFlags = GetVkShaderStageFlags(pushConstantDesc.m_shaderStages);
            
            vkPushConstantRange = vk::PushConstantRange()
                .setStageFlags(shaderStageFlags)
                .setOffset(offset)
                .setSize(pushConstantDesc.m_size);

            // Update Binding Info:
            m_bindingInfo.m_pushConstantBindings[i] = { shaderStageFlags, offset };

            // Update the current offset.
            offset += pushConstantDesc.m_size;
        }
        
        // Create the pipeline layout:
        vk::PipelineLayoutCreateInfo pipelineLayoutInfo = vk::PipelineLayoutCreateInfo()
            .setSetLayouts(descriptorLayouts)
            .setPushConstantRanges(pushConstants);
        
        m_layout = vk::raii::PipelineLayout(device, pipelineLayoutInfo, device.GetVkAllocationCallbacks());
    }

    void PipelineLayout::FreeLayout()
    {
        if (m_layout != nullptr)
        {
            // Free the pipeline layout and all of the descriptor set layout objects.
            Renderer::SubmitResourceFree([layout = std::move(m_layout), descriptorLayouts = std::move(m_descriptorSetLayouts)]() mutable
            {
                layout = nullptr;

                for (auto& descriptorLayout : descriptorLayouts)
                {
                    descriptorLayout = nullptr;
                }
            });
        }

        m_pDevice = nullptr;
        m_descriptorSetLayouts.clear();
    }

    void PipelineLayout::CreateSetLayout(const DescriptorSetDesc& setDesc)
    {
        // Calculate Max Binding Counts
        uint32 bindingMaxNum = 0;
        for (uint32 i = 0; i < setDesc.m_numBindings; ++i)
        {
            const DescriptorBindingDesc& binding = setDesc.m_pBindings[i];
            const bool isArray = binding.m_flags & (EDescriptorBindingBits::Array | EDescriptorBindingBits::VariableSizedArray);
            bindingMaxNum += isArray? 1 : binding.m_descriptorCount;
        }
        
        std::vector<vk::DescriptorSetLayoutBinding> bindings(bindingMaxNum);
        std::vector<vk::DescriptorBindingFlags> bindingFlags(bindingMaxNum);

        vk::DescriptorSetLayoutBinding* pBindingsBegin = bindings.data();
        vk::DescriptorSetLayoutBinding* pCurrentBinding = pBindingsBegin;
        vk::DescriptorBindingFlags* pFlagsBegin = bindingFlags.data();
        vk::DescriptorBindingFlags* pCurrentFlags = pFlagsBegin;
        
        // Descriptor Bindings:
        for (uint32 i = 0; i < setDesc.m_numBindings; ++i)
        {
            const DescriptorBindingDesc& bindingDesc = setDesc.m_pBindings[i];
            
            // Flags:
            vk::DescriptorBindingFlags flags = {};
            if (bindingDesc.m_flags & EDescriptorBindingBits::PartiallyBound)
                flags |= vk::DescriptorBindingFlagBits::ePartiallyBound;
            if (bindingDesc.m_flags & EDescriptorBindingBits::AllowUpdateAfterSet)
                flags |= vk::DescriptorBindingFlagBits::eUpdateAfterBind;

            uint32 descriptorCount = 1;
            const bool isArray = bindingDesc.m_flags & (EDescriptorBindingBits::Array | EDescriptorBindingBits::VariableSizedArray);

            if (isArray)
            {
                if (bindingDesc.m_flags & EDescriptorBindingBits::VariableSizedArray)
                    flags |= vk::DescriptorBindingFlagBits::eVariableDescriptorCount;
            }
            else
            {
                descriptorCount = bindingDesc.m_descriptorCount;
            }

            for (uint32 j = 0; j < descriptorCount; ++j)
            {
                *pCurrentFlags++ = flags;

                vk::DescriptorSetLayoutBinding& binding = *pCurrentBinding++;
                binding = vk::DescriptorSetLayoutBinding()
                    .setDescriptorType(GetVkDescriptorType(bindingDesc.m_descriptorType))
                    .setStageFlags(GetVkShaderStageFlags(bindingDesc.m_shaderStages))
                    .setBinding(bindingDesc.m_bindingIndex + j)
                    .setDescriptorCount(isArray ? bindingDesc.m_descriptorCount : 1);
            }
        }

        // [TODO]: Dynamic Constant Buffer Bindings:

        const uint32 bindingCount = static_cast<uint32>(pCurrentBinding - pBindingsBegin);

        // [TODO]: If descriptor indexing is allowed, add the bindingFlagsInfo, otherwise nullptr.
        // vk::DescriptorSetLayoutBindingFlagsCreateInfo bindingFlagsInfo = vk::DescriptorSetLayoutBindingFlagsCreateInfo()
        //     .setBindingCount(bindingCount)
        //     .setPBindingFlags(pFlagsBegin);

        vk::DescriptorSetLayoutCreateInfo info = vk::DescriptorSetLayoutCreateInfo()
            .setPNext(nullptr) // [TODO]: If descriptor indexing is allowed, add the bindingFlagsInfo, otherwise nullptr.
            .setBindingCount(bindingCount)
            .setBindings(bindings)
            .setFlags((setDesc.m_flags & EDescriptorSetBits::AllowUpdateAfterBound) ? vk::DescriptorSetLayoutCreateFlagBits::eUpdateAfterBindPool : vk::DescriptorSetLayoutCreateFlags{}); 

        // Create the DescriptorSetLayout:
        m_descriptorSetLayouts.emplace_back(vk::raii::DescriptorSetLayout(*m_pDevice, info, m_pDevice->GetVkAllocationCallbacks()));
    }
}

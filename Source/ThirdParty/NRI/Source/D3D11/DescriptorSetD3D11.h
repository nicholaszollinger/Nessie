// © 2021 NVIDIA Corporation

#pragma once

namespace nri {

struct PipelineLayoutD3D11;
struct DescriptorD3D11;
struct BindingSet;

struct DescriptorSetD3D11 final : public DebugNameBase {
    inline const DescriptorD3D11* GetDescriptor(uint32_t i) const {
        return m_Descriptors[i];
    }

    uint32_t GetDynamicConstantBufferNum() const;
    void Create(const PipelineLayoutD3D11* pipelineLayout, const BindingSet* bindingSet, const DescriptorD3D11** descriptors);

    //================================================================================================================
    // NRI
    //================================================================================================================

    void UpdateDescriptorRanges(uint32_t rangeOffset, uint32_t rangeNum, const DescriptorRangeUpdateDesc* rangeUpdateDescs);
    void UpdateDynamicConstantBuffers(uint32_t baseDynamicConstantBuffer, uint32_t dynamicConstantBufferNum, const Descriptor* const* descriptors);
    void Copy(const DescriptorSetCopyDesc& descriptorSetCopyDesc);

private:
    const PipelineLayoutD3D11* m_PipelineLayout = nullptr;
    const BindingSet* m_BindingSet = nullptr;
    const DescriptorD3D11** m_Descriptors = nullptr;
};

} // namespace nri

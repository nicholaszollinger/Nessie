// © 2021 NVIDIA Corporation

inline uint32_t DescriptorSetD3D11::GetDynamicConstantBufferNum() const {
    return m_BindingSet->endRangeOfDynamicConstantBuffers - m_BindingSet->startRangeOfDynamicConstantBuffers;
}

void DescriptorSetD3D11::Create(const PipelineLayoutD3D11* pipelineLayout, const BindingSet* bindingSet, const DescriptorD3D11** descriptors) {
    m_PipelineLayout = pipelineLayout;
    m_BindingSet = bindingSet;
    m_Descriptors = descriptors;
}

NRI_INLINE void DescriptorSetD3D11::UpdateDescriptorRanges(uint32_t rangeOffset, uint32_t rangeNum, const DescriptorRangeUpdateDesc* rangeUpdateDescs) {
    rangeOffset += m_BindingSet->startRange;
    CHECK(rangeOffset + rangeNum <= m_BindingSet->endRange, "Out of bounds");

    for (uint32_t i = 0; i < rangeNum; i++) {
        const DescriptorRangeUpdateDesc& range = rangeUpdateDescs[i];
        uint32_t descriptorOffset = range.baseDescriptor;

        const BindingRange& bindingRange = m_PipelineLayout->GetBindingRange(rangeOffset + i);
        descriptorOffset += bindingRange.descriptorOffset;

        const DescriptorD3D11** dstDescriptors = m_Descriptors + descriptorOffset;
        const DescriptorD3D11** srcDescriptors = (const DescriptorD3D11**)range.descriptors;

        memcpy(dstDescriptors, srcDescriptors, range.descriptorNum * sizeof(DescriptorD3D11*));
    }
}

NRI_INLINE void DescriptorSetD3D11::UpdateDynamicConstantBuffers(uint32_t baseDynamicConstantBuffer, uint32_t dynamicConstantBufferNum, const Descriptor* const* descriptors) {
    baseDynamicConstantBuffer += m_BindingSet->startRangeOfDynamicConstantBuffers;
    CHECK(baseDynamicConstantBuffer + dynamicConstantBufferNum <= m_BindingSet->endRangeOfDynamicConstantBuffers, "Out of bounds");

    const DescriptorD3D11** srcDescriptors = (const DescriptorD3D11**)descriptors;
    for (uint32_t i = 0; i < dynamicConstantBufferNum; i++) {
        const BindingRange& bindingRange = m_PipelineLayout->GetBindingRange(baseDynamicConstantBuffer + i);
        uint32_t descriptorOffset = bindingRange.descriptorOffset;

        m_Descriptors[descriptorOffset] = srcDescriptors[i];
    }
}

NRI_INLINE void DescriptorSetD3D11::Copy(const DescriptorSetCopyDesc& descriptorSetCopyDesc) {
    DescriptorSetD3D11& srcSet = (DescriptorSetD3D11&)descriptorSetCopyDesc.srcDescriptorSet;

    uint32_t dstBaseRange = m_BindingSet->startRange + descriptorSetCopyDesc.dstBaseRange;
    uint32_t srcBaseRange = srcSet.m_BindingSet->startRange + descriptorSetCopyDesc.srcBaseRange;
    CHECK(dstBaseRange + descriptorSetCopyDesc.rangeNum <= m_BindingSet->endRange, "Out of bounds");
    CHECK(srcBaseRange + descriptorSetCopyDesc.rangeNum <= srcSet.m_BindingSet->endRange, "Out of bounds");

    for (uint32_t i = 0; i < descriptorSetCopyDesc.rangeNum; i++) {
        const BindingRange& dst = m_PipelineLayout->GetBindingRange(dstBaseRange + i);
        const DescriptorD3D11** dstDescriptors = m_Descriptors + dst.descriptorOffset;

        const BindingRange& src = m_PipelineLayout->GetBindingRange(srcBaseRange + i);
        const DescriptorD3D11** srcDescriptors = srcSet.m_Descriptors + src.descriptorOffset;

        memcpy(dstDescriptors, srcDescriptors, dst.descriptorNum * sizeof(DescriptorD3D11*));
    }

    uint32_t dstBaseDynamicConstantBuffer = m_BindingSet->startRangeOfDynamicConstantBuffers + descriptorSetCopyDesc.dstBaseDynamicConstantBuffer;
    uint32_t srcBaseDynamicConstantBuffer = srcSet.m_BindingSet->startRangeOfDynamicConstantBuffers + descriptorSetCopyDesc.srcBaseDynamicConstantBuffer;
    CHECK(dstBaseDynamicConstantBuffer + descriptorSetCopyDesc.dynamicConstantBufferNum <= m_BindingSet->endRangeOfDynamicConstantBuffers, "Out of bounds");
    CHECK(srcBaseDynamicConstantBuffer + descriptorSetCopyDesc.dynamicConstantBufferNum <= srcSet.m_BindingSet->endRangeOfDynamicConstantBuffers, "Out of bounds");

    for (uint32_t i = 0; i < descriptorSetCopyDesc.dynamicConstantBufferNum; i++) {
        const BindingRange& dst = m_PipelineLayout->GetBindingRange(dstBaseDynamicConstantBuffer + i);
        const BindingRange& src = m_PipelineLayout->GetBindingRange(srcBaseDynamicConstantBuffer + i);

        m_Descriptors[dst.descriptorOffset] = srcSet.m_Descriptors[src.descriptorOffset];
    }
}

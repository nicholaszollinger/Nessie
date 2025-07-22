// © 2021 NVIDIA Corporation

static inline void BuildDescriptorSetMapping(const DescriptorSetDesc& descriptorSetDesc, DescriptorSetMapping& descriptorSetMapping) {
    descriptorSetMapping.descriptorRangeMappings.resize(descriptorSetDesc.rangeNum);

    for (uint32_t i = 0; i < descriptorSetDesc.rangeNum; i++) {
        D3D12_DESCRIPTOR_HEAP_TYPE descriptorHeapType = GetDescriptorHeapType(descriptorSetDesc.ranges[i].descriptorType);
        descriptorSetMapping.descriptorRangeMappings[i].descriptorHeapType = (DescriptorHeapType)descriptorHeapType;
        descriptorSetMapping.descriptorRangeMappings[i].heapOffset = descriptorSetMapping.descriptorNum[descriptorHeapType];
        descriptorSetMapping.descriptorRangeMappings[i].descriptorNum = descriptorSetDesc.ranges[i].descriptorNum;

        descriptorSetMapping.descriptorNum[descriptorHeapType] += descriptorSetDesc.ranges[i].descriptorNum;
    }
}

static inline D3D12_ROOT_SIGNATURE_FLAGS GetRootSignatureStageFlags(const PipelineLayoutDesc& pipelineLayoutDesc, const DeviceD3D12& device) {
    D3D12_ROOT_SIGNATURE_FLAGS flags = D3D12_ROOT_SIGNATURE_FLAG_NONE;

    if (pipelineLayoutDesc.shaderStages & StageBits::VERTEX_SHADER)
        flags |= D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
    else
        flags |= D3D12_ROOT_SIGNATURE_FLAG_DENY_VERTEX_SHADER_ROOT_ACCESS;

    if (!(pipelineLayoutDesc.shaderStages & StageBits::TESS_CONTROL_SHADER))
        flags |= D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS;
    if (!(pipelineLayoutDesc.shaderStages & StageBits::TESS_EVALUATION_SHADER))
        flags |= D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS;
    if (!(pipelineLayoutDesc.shaderStages & StageBits::GEOMETRY_SHADER))
        flags |= D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;
    if (!(pipelineLayoutDesc.shaderStages & StageBits::FRAGMENT_SHADER))
        flags |= D3D12_ROOT_SIGNATURE_FLAG_DENY_PIXEL_SHADER_ROOT_ACCESS;

    // Windows versions prior to 20H1 (which introduced DirectX Ultimate) can
    // produce errors when the following flags are added. To avoid this, we
    // only add these mesh shading pipeline flags when the device
    // (and thus Windows) supports mesh shading.
    if (device.GetDesc().features.meshShader) {
        if (!(pipelineLayoutDesc.shaderStages & StageBits::MESH_CONTROL_SHADER))
            flags |= D3D12_ROOT_SIGNATURE_FLAG_DENY_AMPLIFICATION_SHADER_ROOT_ACCESS;
        if (!(pipelineLayoutDesc.shaderStages & StageBits::MESH_EVALUATION_SHADER))
            flags |= D3D12_ROOT_SIGNATURE_FLAG_DENY_MESH_SHADER_ROOT_ACCESS;
    }

    if (device.GetDesc().shaderModel >= 66) { // TODO: an extension is needed because of VK, or add "samplersDirectlyIndexed" and "resourcesDirectlyIndexed" into "PipelineLayoutDesc"
        bool hasSamplers = false;
        bool hasResources = false;

        for (uint32_t i = 0; i < pipelineLayoutDesc.descriptorSetNum; i++) {
            const DescriptorSetDesc& descriptorSetDesc = pipelineLayoutDesc.descriptorSets[i];

            for (uint32_t j = 0; j < descriptorSetDesc.rangeNum; j++) {
                const DescriptorRangeDesc& descriptorRangeDesc = descriptorSetDesc.ranges[j];

                if (descriptorRangeDesc.descriptorType == DescriptorType::SAMPLER)
                    hasSamplers = true;
                else if (descriptorRangeDesc.descriptorType == DescriptorType::CONSTANT_BUFFER
                    || descriptorRangeDesc.descriptorType == DescriptorType::TEXTURE
                    || descriptorRangeDesc.descriptorType == DescriptorType::STORAGE_TEXTURE
                    || descriptorRangeDesc.descriptorType == DescriptorType::BUFFER
                    || descriptorRangeDesc.descriptorType == DescriptorType::STORAGE_BUFFER
                    || descriptorRangeDesc.descriptorType == DescriptorType::STRUCTURED_BUFFER
                    || descriptorRangeDesc.descriptorType == DescriptorType::STORAGE_STRUCTURED_BUFFER)
                    hasResources = true;
            }
        }

        if (hasSamplers)
            flags |= D3D12_ROOT_SIGNATURE_FLAG_SAMPLER_HEAP_DIRECTLY_INDEXED;

        if (hasResources)
            flags |= D3D12_ROOT_SIGNATURE_FLAG_CBV_SRV_UAV_HEAP_DIRECTLY_INDEXED;
    }

    return flags;
}

PipelineLayoutD3D12::PipelineLayoutD3D12(DeviceD3D12& device)
    : m_DescriptorSetMappings(device.GetStdAllocator())
    , m_DescriptorSetRootMappings(device.GetStdAllocator())
    , m_DynamicConstantBufferMappings(device.GetStdAllocator())
    , m_Device(device) {
}

Result PipelineLayoutD3D12::Create(const PipelineLayoutDesc& pipelineLayoutDesc) {
    m_IsGraphicsPipelineLayout = pipelineLayoutDesc.shaderStages & StageBits::GRAPHICS_SHADERS;

    uint32_t rangeNum = 0;
    uint32_t rangeMaxNum = 0;
    for (uint32_t i = 0; i < pipelineLayoutDesc.descriptorSetNum; i++)
        rangeMaxNum += pipelineLayoutDesc.descriptorSets[i].rangeNum;

    StdAllocator<uint8_t>& allocator = m_Device.GetStdAllocator();
    m_DescriptorSetMappings.resize(pipelineLayoutDesc.descriptorSetNum, DescriptorSetMapping(allocator));
    m_DescriptorSetRootMappings.resize(pipelineLayoutDesc.descriptorSetNum, DescriptorSetRootMapping(allocator));
    m_DynamicConstantBufferMappings.resize(pipelineLayoutDesc.descriptorSetNum);

    Scratch<D3D12_DESCRIPTOR_RANGE1> ranges = AllocateScratch(m_Device, D3D12_DESCRIPTOR_RANGE1, rangeMaxNum);
    Vector<D3D12_ROOT_PARAMETER1> rootParameters(allocator);

    bool enableDrawParametersEmulation = (pipelineLayoutDesc.flags & PipelineLayoutBits::ENABLE_D3D12_DRAW_PARAMETERS_EMULATION) != 0 && (pipelineLayoutDesc.shaderStages & StageBits::VERTEX_SHADER) != 0;

    if (enableDrawParametersEmulation) {
        D3D12_ROOT_PARAMETER1 rootParam = {};
        rootParam.ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
        rootParam.ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
        rootParam.Constants.ShaderRegister = 0;
        rootParam.Constants.RegisterSpace = NRI_BASE_ATTRIBUTES_EMULATION_SPACE;
        rootParam.Constants.Num32BitValues = 2;

        rootParameters.push_back(rootParam);
    }

    for (uint32_t i = 0; i < pipelineLayoutDesc.descriptorSetNum; i++) {
        const DescriptorSetDesc& descriptorSetDesc = pipelineLayoutDesc.descriptorSets[i];
        BuildDescriptorSetMapping(descriptorSetDesc, m_DescriptorSetMappings[i]);
        m_DescriptorSetRootMappings[i].rootOffsets.resize(descriptorSetDesc.rangeNum);

        uint32_t heapIndex = 0;

        D3D12_ROOT_PARAMETER1 rootParam = {};
        rootParam.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;

        uint32_t groupedRangeNum = 0;
        D3D12_DESCRIPTOR_RANGE_TYPE groupedRangeType = {};
        for (uint32_t j = 0; j < descriptorSetDesc.rangeNum; j++) {
            const DescriptorRangeDesc& descriptorRangeDesc = descriptorSetDesc.ranges[j];
            auto& descriptorRangeMapping = m_DescriptorSetMappings[i].descriptorRangeMappings[j];

            D3D12_SHADER_VISIBILITY shaderVisibility = GetShaderVisibility(descriptorRangeDesc.shaderStages);
            D3D12_DESCRIPTOR_RANGE_TYPE rangeType = GetDescriptorRangesType(descriptorRangeDesc.descriptorType);

            if (groupedRangeNum && (rootParam.ShaderVisibility != shaderVisibility || groupedRangeType != rangeType || descriptorRangeMapping.descriptorHeapType != heapIndex)) {
                rootParam.DescriptorTable.NumDescriptorRanges = groupedRangeNum;
                rootParameters.push_back(rootParam);

                rangeNum += groupedRangeNum;
                groupedRangeNum = 0;
            }

            groupedRangeType = rangeType;
            heapIndex = (uint32_t)descriptorRangeMapping.descriptorHeapType;
            m_DescriptorSetRootMappings[i].rootOffsets[j] = groupedRangeNum ? ROOT_PARAMETER_UNUSED : (uint16_t)rootParameters.size();

            rootParam.ShaderVisibility = shaderVisibility;
            rootParam.DescriptorTable.pDescriptorRanges = &ranges[rangeNum];

            // https://microsoft.github.io/DirectX-Specs/d3d/ResourceBinding.html#flags-added-in-root-signature-version-11
            D3D12_DESCRIPTOR_RANGE_FLAGS descriptorRangeFlags = D3D12_DESCRIPTOR_RANGE_FLAG_NONE;

            // "PARTIALLY_BOUND" implies relaxed requirements and validation
            // "ALLOW_UPDATE_AFTER_SET" allows descriptor updates after "bind"
            if (descriptorRangeDesc.flags & (DescriptorRangeBits::PARTIALLY_BOUND | DescriptorRangeBits::ALLOW_UPDATE_AFTER_SET))
                descriptorRangeFlags |= D3D12_DESCRIPTOR_RANGE_FLAG_DESCRIPTORS_VOLATILE;

            // "ALLOW_UPDATE_AFTER_SET" additionally allows to change data, pointed to by descriptors
            // Samplers are always "DATA_STATIC"
            if (rangeType != D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER) {
                if (descriptorRangeDesc.flags & DescriptorRangeBits::ALLOW_UPDATE_AFTER_SET)
                    descriptorRangeFlags |= D3D12_DESCRIPTOR_RANGE_FLAG_DATA_VOLATILE;
                else
                    descriptorRangeFlags |= D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC_WHILE_SET_AT_EXECUTE;
            }

            D3D12_DESCRIPTOR_RANGE1& descriptorRange = ranges[rangeNum + groupedRangeNum];
            descriptorRange.RangeType = rangeType;
            descriptorRange.NumDescriptors = descriptorRangeDesc.descriptorNum;
            descriptorRange.BaseShaderRegister = descriptorRangeDesc.baseRegisterIndex;
            descriptorRange.RegisterSpace = descriptorSetDesc.registerSpace;
            descriptorRange.Flags = descriptorRangeFlags;
            descriptorRange.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
            groupedRangeNum++;
        }

        if (groupedRangeNum) {
            rootParam.DescriptorTable.NumDescriptorRanges = groupedRangeNum;
            rootParameters.push_back(rootParam);
            rangeNum += groupedRangeNum;
        }

        if (descriptorSetDesc.dynamicConstantBufferNum) {
            rootParam = {};
            rootParam.ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
            rootParam.Descriptor.Flags = D3D12_ROOT_DESCRIPTOR_FLAG_NONE; // TODO: better flags?

            m_DynamicConstantBufferMappings[i].rootConstantNum = (uint16_t)descriptorSetDesc.dynamicConstantBufferNum;
            m_DynamicConstantBufferMappings[i].rootOffset = (uint16_t)rootParameters.size();

            for (uint32_t j = 0; j < descriptorSetDesc.dynamicConstantBufferNum; j++) {
                rootParam.Descriptor.ShaderRegister = descriptorSetDesc.dynamicConstantBuffers[j].registerIndex;
                rootParam.Descriptor.RegisterSpace = descriptorSetDesc.registerSpace;
                rootParam.ShaderVisibility = GetShaderVisibility(descriptorSetDesc.dynamicConstantBuffers[j].shaderStages);
                rootParameters.push_back(rootParam);
            }
        } else {
            m_DynamicConstantBufferMappings[i].rootConstantNum = 0;
            m_DynamicConstantBufferMappings[i].rootOffset = 0;
        }
    }

    if (pipelineLayoutDesc.rootConstantNum) {
        m_BaseRootConstant = (uint32_t)rootParameters.size();

        for (uint32_t i = 0; i < pipelineLayoutDesc.rootConstantNum; i++) {
            const RootConstantDesc& rootConstantDesc = pipelineLayoutDesc.rootConstants[i];

            D3D12_ROOT_PARAMETER1 rootParam = {};
            rootParam.ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
            rootParam.ShaderVisibility = GetShaderVisibility(rootConstantDesc.shaderStages);
            rootParam.Constants.ShaderRegister = rootConstantDesc.registerIndex;
            rootParam.Constants.RegisterSpace = pipelineLayoutDesc.rootRegisterSpace;
            rootParam.Constants.Num32BitValues = rootConstantDesc.size / 4;

            rootParameters.push_back(rootParam);
        }
    }

    if (pipelineLayoutDesc.rootDescriptorNum) {
        m_BaseRootDescriptor = (uint32_t)rootParameters.size();

        for (uint32_t i = 0; i < pipelineLayoutDesc.rootDescriptorNum; i++) {
            const RootDescriptorDesc& rootDescriptorDesc = pipelineLayoutDesc.rootDescriptors[i];

            D3D12_ROOT_PARAMETER1 rootParam = {};
            rootParam.ShaderVisibility = GetShaderVisibility(rootDescriptorDesc.shaderStages);
            rootParam.Descriptor.ShaderRegister = rootDescriptorDesc.registerIndex;
            rootParam.Descriptor.RegisterSpace = pipelineLayoutDesc.rootRegisterSpace;
            rootParam.Descriptor.Flags = D3D12_ROOT_DESCRIPTOR_FLAG_NONE; // TODO: better flags?

            if (rootDescriptorDesc.descriptorType == DescriptorType::CONSTANT_BUFFER)
                rootParam.ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
            else if (rootDescriptorDesc.descriptorType == DescriptorType::STORAGE_STRUCTURED_BUFFER)
                rootParam.ParameterType = D3D12_ROOT_PARAMETER_TYPE_UAV;
            else
                rootParam.ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV;

            rootParameters.push_back(rootParam);
        }
    }

    D3D12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc = {};
    rootSignatureDesc.Version = D3D_ROOT_SIGNATURE_VERSION_1_1;
    rootSignatureDesc.Desc_1_1.NumParameters = (UINT)rootParameters.size();
    rootSignatureDesc.Desc_1_1.pParameters = rootParameters.empty() ? nullptr : &rootParameters[0];
    rootSignatureDesc.Desc_1_1.Flags = GetRootSignatureStageFlags(pipelineLayoutDesc, m_Device);

    ComPtr<ID3DBlob> rootSignatureBlob;
    ComPtr<ID3DBlob> errorBlob;
    HRESULT hr = D3D12SerializeVersionedRootSignature(&rootSignatureDesc, &rootSignatureBlob, &errorBlob);
    RETURN_ON_BAD_HRESULT(&m_Device, hr, "D3D12SerializeVersionedRootSignature");

    if (errorBlob)
        REPORT_ERROR(&m_Device, "D3D12SerializeVersionedRootSignature(): %s", (char*)errorBlob->GetBufferPointer());

    hr = m_Device->CreateRootSignature(NODE_MASK, rootSignatureBlob->GetBufferPointer(), rootSignatureBlob->GetBufferSize(), IID_PPV_ARGS(&m_RootSignature));
    RETURN_ON_BAD_HRESULT(&m_Device, hr, "ID3D12Device::CreateRootSignature");

    m_DrawParametersEmulation = enableDrawParametersEmulation;
    if (pipelineLayoutDesc.shaderStages & StageBits::VERTEX_SHADER) {
        Result result = m_Device.CreateDefaultDrawSignatures(m_RootSignature.GetInterface(), enableDrawParametersEmulation);
        RETURN_ON_FAILURE(&m_Device, result == Result::SUCCESS, result, "Failed to create draw signature for pipeline layout");
    }

    return Result::SUCCESS;
}

template <bool isGraphics>
void PipelineLayoutD3D12::SetDescriptorSetImpl(ID3D12GraphicsCommandList& graphicsCommandList, uint32_t setIndex, const DescriptorSet& descriptorSet, const uint32_t* dynamicConstantBufferOffsets) const {
    const DescriptorSetD3D12& descriptorSetImpl = (const DescriptorSetD3D12&)descriptorSet;

    const auto& rootOffsets = m_DescriptorSetRootMappings[setIndex].rootOffsets;
    uint32_t rangeNum = (uint32_t)rootOffsets.size();
    for (uint32_t j = 0; j < rangeNum; j++) {
        uint16_t rootParameterIndex = rootOffsets[j];
        if (rootParameterIndex == ROOT_PARAMETER_UNUSED)
            continue;

        DescriptorPointerGPU descriptorPointerGPU = descriptorSetImpl.GetPointerGPU(j, 0);
        if (isGraphics)
            graphicsCommandList.SetGraphicsRootDescriptorTable(rootParameterIndex, {descriptorPointerGPU});
        else
            graphicsCommandList.SetComputeRootDescriptorTable(rootParameterIndex, {descriptorPointerGPU});
    }

    const auto& dynamicConstantBufferMapping = m_DynamicConstantBufferMappings[setIndex];
    for (uint16_t j = 0; j < dynamicConstantBufferMapping.rootConstantNum; j++) {
        uint16_t rootParameterIndex = dynamicConstantBufferMapping.rootOffset + j;

        DescriptorPointerGPU descriptorPointerGPU = descriptorSetImpl.GetDynamicPointerGPU(j) + dynamicConstantBufferOffsets[j];
        if (isGraphics)
            graphicsCommandList.SetGraphicsRootConstantBufferView(rootParameterIndex, descriptorPointerGPU);
        else
            graphicsCommandList.SetComputeRootConstantBufferView(rootParameterIndex, descriptorPointerGPU);
    }
}

void PipelineLayoutD3D12::SetDescriptorSet(ID3D12GraphicsCommandList& graphicsCommandList, bool isGraphics, uint32_t setIndex, const DescriptorSet& descriptorSet, const uint32_t* dynamicConstantBufferOffsets) const {
    if (isGraphics)
        SetDescriptorSetImpl<true>(graphicsCommandList, setIndex, descriptorSet, dynamicConstantBufferOffsets);
    else
        SetDescriptorSetImpl<false>(graphicsCommandList, setIndex, descriptorSet, dynamicConstantBufferOffsets);
}

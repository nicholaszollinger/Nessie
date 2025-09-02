// DataUploader.cpp
#include "DataUploader.h"
#include "Vulkan/VmaUsage.h"
#include "RenderDevice.h"

namespace nes
{
    DataUploader::DataUploader(RenderDevice& device)
        : m_pDevice(&device)
    {
        m_stagingResources.reserve(32);
    }

    DataUploader::~DataUploader()
    {
        Destroy();
    }

    void DataUploader::AppendUploadBuffer(const UploadBufferDesc& desc, const SemaphoreValue& semaphoreState)
    {
        NES_ASSERT(desc.m_pBuffer != nullptr);

        DeviceBuffer& buffer = *(desc.m_pBuffer);
        
        // No data upload, skip.
        if (desc.m_uploadSize == 0)
            return;

        // Use the entire buffer from the offset, or the given size.
        const uint64 size = desc.m_uploadSize == graphics::kWholeSize? desc.m_pBuffer->GetSize() - desc.m_uploadOffset : desc.m_uploadSize;
        
        NES_ASSERT(desc.m_pData != nullptr);
        NES_ASSERT(desc.m_uploadOffset + size <= desc.m_pBuffer->GetSize());
        NES_ASSERT(buffer.GetVkBuffer());

        // If we have CPU access, copy the data now, and return.
        if (buffer.m_pMappedMemory != nullptr)
        {
            std::memcpy(buffer.m_pMappedMemory + desc.m_uploadOffset, desc.m_pData, size);
            return;
        }

        // Create the staging buffer
        DeviceBufferRange stagingRange;
        AcquireStagingBuffer(desc.m_pData, size, semaphoreState, stagingRange);

        // Add a copy description to use when calling RecordCommands().
        CopyBufferDesc copyDesc;
        copyDesc.m_dstBuffer = &buffer;
        copyDesc.m_dstOffset = desc.m_uploadOffset;
        copyDesc.m_srcBuffer = stagingRange.m_buffer;
        copyDesc.m_srcOffset = stagingRange.m_offset;
        copyDesc.m_size = size;
        m_copyBufferDescs.emplace_back(copyDesc);
    }
    
    void DataUploader::RecordCommands(CommandBuffer& buffer)
    {
        // Record Buffer Copies:
        for (const auto& copyDesc : m_copyBufferDescs)
        {
            buffer.CopyBuffer(copyDesc);
        }

        // [TODO]: Record Copy Buffer to Image

        // Clear pending operations.
        ClearPending();
    }

    bool DataUploader::IsEmpty() const
    {
        return m_copyBufferDescs.empty(); // && m_copyBufferImageInfos.empty();
    }

    void DataUploader::Destroy()
    {
        ClearPending();
        ReleaseStagingBuffers(true);
        NES_ASSERT(m_stagingResources.empty() && m_stagingResourcesSize == 0);
    }

    void DataUploader::AcquireStagingBuffer(const void* pData, uint64 dataSize, const SemaphoreValue& semaphoreState, DeviceBufferRange& outRange)
    {
        AllocateBufferDesc allocDesc{};
        allocDesc.m_size = dataSize;
        allocDesc.m_usage = EBufferUsageBits::None; 
        allocDesc.m_location = EMemoryLocation::HostUpload;
        
        // Create the Staging Buffer
        DeviceBuffer stagingBuffer = DeviceBuffer(*m_pDevice, allocDesc);
        if (!stagingBuffer.m_pMappedMemory)
        {
            stagingBuffer = nullptr;
            NES_ASSERT(false, "Failed to setup staging buffer!");
            return;
        }
        
        if (pData != nullptr)
        {
            std::memcpy(stagingBuffer.m_pMappedMemory, pData, dataSize);
        }

        // Set Range Info:
        outRange.m_offset = 0;
        outRange.m_range = dataSize;
        outRange.m_deviceAddress = stagingBuffer.m_deviceAddress;
        outRange.m_pMapping = stagingBuffer.m_pMappedMemory;

        // Add the staging resource to the array:
        m_stagingResourcesSize += dataSize;
        m_stagingResources.emplace_back(std::move(stagingBuffer), semaphoreState);
        outRange.m_buffer = &(m_stagingResources.back().m_buffer);
    }

    void DataUploader::ClearPending()
    {
        m_copyBufferDescs.clear();
    }

    void DataUploader::ReleaseStagingBuffers(const bool forceAll)
    {
        size_t writeIndex = 0;
        for (size_t readIndex = 0; readIndex < m_stagingResources.size(); ++readIndex)
        {
            StagingResource& stagingResource = m_stagingResources[readIndex];

            // Always release with forceAll.
            // Else, release if semaphoreState is invalid, or test if it was signaled.
            const bool canRelease = forceAll || !stagingResource.m_semaphoreState.IsValid() || stagingResource.m_semaphoreState.IsSignaled();
            
            if (canRelease)
            {
                m_stagingResourcesSize -= stagingResource.m_buffer.GetSize();
                stagingResource.m_buffer = nullptr;
                stagingResource.m_semaphoreState = nullptr;
            }
            else if (readIndex != writeIndex)
            {
                // Move to the previous write position, shrinking the array.
                m_stagingResources[writeIndex++] = std::move(stagingResource);
            }
            else if (!forceAll)
            {
                // If we are not freeing all resources, then move our write position. 
                ++writeIndex;
            }
        }

        m_stagingResources.resize(writeIndex);
    }
}

// DataUploader.cpp
#include "DataUploader.h"

#include "DeviceImage.h"
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
        DeviceBufferRange stagingRange = AcquireStagingBuffer(desc.m_pData, size, semaphoreState);

        // Add a copy description to use when calling RecordCommands().
        CopyBufferDesc copyDesc;
        copyDesc.m_dstBuffer = &buffer;
        copyDesc.m_dstOffset = desc.m_uploadOffset;
        copyDesc.m_srcBuffer = stagingRange.GetBuffer();
        copyDesc.m_srcOffset = stagingRange.GetOffset();
        copyDesc.m_size = size;
        m_copyBufferDescs.emplace_back(copyDesc);
    }

    void DataUploader::AppendUploadImage(const ImageUploadDesc& desc, const SemaphoreValue& semaphoreState)
    {
        NES_ASSERT(desc.m_pImage != nullptr);
        
        if (desc.m_uploadSize == 0)
            return;

        DeviceImage& image = *(desc.m_pImage);

        // Get the actual upload size of the image.
        uint64 size = desc.m_uploadSize;
        const uint64 imageSize = image.GetPixelCount() * image.GetPixelSize();
        if (desc.m_uploadSize == graphics::kWholeSize)
        {
            size = imageSize;
        }
        
        NES_ASSERT(desc.m_uploadOffset + size <= imageSize);
        NES_ASSERT(image.m_image != nullptr);

        // Create the Staging Buffer
        const DeviceBufferRange stagingRange = AcquireStagingBuffer(desc.m_pPixelData, size, semaphoreState);

        // Transition the top mip level from an unknown state to the copy destination state:
        ImageBarrierDesc preBarrier;
        preBarrier.m_pImage = &image;
        preBarrier.m_before = AccessLayoutStage::UnknownState();
        preBarrier.m_after = AccessLayoutStage::CopyDestinationState();
        preBarrier.m_mipCount = image.m_desc.m_mipCount;
        m_preBarriers.m_imageBarriers.emplace_back(preBarrier);

        // Transition from the copy destination state to the final upload layout.
        ImageBarrierDesc postBarrier;
        postBarrier.m_pImage = &image;
        postBarrier.m_before = AccessLayoutStage::CopyDestinationState();
        postBarrier.m_after.m_layout = desc.m_newLayout;
        postBarrier.m_mipCount = image.m_desc.m_mipCount;

        // [TODO]: If we are on a separate staging queue, then we need to transition ownership to the render queue.
        
        m_postBarriers.m_imageBarriers.emplace_back(postBarrier);

        // Add the copy description to use when recording commands.
        CopyBufferToImageDesc copyDesc;
        copyDesc.m_dstImage = &image;
        copyDesc.m_dstImageLayout = EImageLayout::CopyDestination;
        copyDesc.m_imageOffset = {0, 0, 0};
        copyDesc.m_srcBuffer = stagingRange.GetBuffer();
        copyDesc.m_srcOffset = stagingRange.GetOffset();
        copyDesc.m_size = size;
        m_copyBufferToImageDescs.emplace_back(copyDesc);
    }

    void DataUploader::RecordCommands(CommandBuffer& buffer)
    {
        // Apply pre barriers:
        buffer.SetBarriers(m_preBarriers);
        
        // Record buffer copies:
        for (const auto& copyDesc : m_copyBufferDescs)
        {
            buffer.CopyBuffer(copyDesc);
        }

        // Record image copies.
        for (const auto& copyDesc : m_copyBufferToImageDescs)
        {
            buffer.CopyBufferToImage(copyDesc);
        }

        // Apply post barriers.
        buffer.SetBarriers(m_postBarriers);

        // Clear pending operations.
        ClearPending();
    }

    bool DataUploader::IsEmpty() const
    {
        return m_copyBufferDescs.empty() && m_copyBufferToImageDescs.empty();
    }

    void DataUploader::Destroy()
    {
        ClearPending();
        ReleaseStagingBuffers(true);
        NES_ASSERT(m_stagingResources.empty() && m_stagingResourcesSize == 0);
    }

    DeviceBufferRange DataUploader::AcquireStagingBuffer(const void* pData, uint64 dataSize, const SemaphoreValue& semaphoreState)
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
            return {};
        }
        
        if (pData != nullptr)
        {
            std::memcpy(stagingBuffer.m_pMappedMemory, pData, dataSize);
        }
        
        // Add the staging resource to the array:
        m_stagingResourcesSize += dataSize;
        m_stagingResources.emplace_back(std::move(stagingBuffer), semaphoreState);
        
        // Return the Range:
        return DeviceBufferRange(&(m_stagingResources.back().m_buffer), 0, dataSize);
    }

    void DataUploader::ClearPending()
    {
        m_copyBufferDescs.clear();
        m_copyBufferToImageDescs.clear();
        m_preBarriers.m_imageBarriers.clear();
        m_postBarriers.m_imageBarriers.clear();
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

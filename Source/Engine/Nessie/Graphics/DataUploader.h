// DataUploader.h
#pragma once
#include "DeviceBuffer.h"
#include "DeviceSemaphore.h"

namespace nes
{
    //----------------------------------------------------------------------------------------------------
    /// @brief : Parameters for uploading data to a Device Buffer.  
    //----------------------------------------------------------------------------------------------------
    struct UploadBufferDesc
    {
        DeviceBuffer*       m_pBuffer = nullptr;                    // Device Buffer that we are uploading to.
        const void*         m_pData = nullptr;                      // Pointer to the data to upload.
        uint64              m_uploadSize = graphics::kWholeSize;    // Size, in bytes, of the data to upload. If left to kWholeSize, then the entire buffer is used.
        uint64              m_uploadOffset = 0;                     // Byte offset into the destination buffer to begin uploading to.
        // Access after?
    };
    
    struct UploadImageDesc
    {
        DeviceImage*        m_pImage = nullptr;                     // Device Image that we are uploading to.
        const void*         m_pSrcData = nullptr;                   // Array of pixel data to upload to m_layerCount number of layers in the image. The size of each layer is expected to be the image size.
        uint32              m_layerCount = 1;                       // Number of layers to upload. Must be at least 1.
        EImagePlaneBits     m_planes = EImagePlaneBits::Color;      // Which planes to upload to.
        EImageLayout        m_newLayout = EImageLayout::Undefined;  // Destination Layout for the image. 
    };

    //----------------------------------------------------------------------------------------------------
    // [TODO]: Right now, I am allocating a single staging buffer per appended buffer copy. I should instead
    //  queue up all upload operations at once, then create a single staging buffer, save the ranges that I
    //  need to copy from and perform the copy commands using that single source buffer.
    //
    /// @brief : Helper class that manages allocating Staging Buffers for uploading data to buffers and images.
    //----------------------------------------------------------------------------------------------------
    class DataUploader
    {
    public:
        DataUploader(RenderDevice& device);
        ~DataUploader();
        
        //----------------------------------------------------------------------------------------------------
        /// @brief : If the Device Buffer is mappable (CPU can write to it directly), then the associated data
        ///     will be immediately copied to the buffer. Otherwise, this will create a staging buffer and
        ///     append a copy command on "RecordCommands".
        //----------------------------------------------------------------------------------------------------
        void                                AppendUploadBuffer(const UploadBufferDesc& desc, const SemaphoreValue& semaphoreState = {});

        //----------------------------------------------------------------------------------------------------
        /// @brief : Creates a staging buffer to copy the image data into the destination image.
        //----------------------------------------------------------------------------------------------------
        void                                AppendUploadImage(const UploadImageDesc& desc, const SemaphoreValue& semaphoreState = {});

        //----------------------------------------------------------------------------------------------------
        /// @brief : Records all pending upload operations into the command buffer,
        ///     then clears the appending state.
        //----------------------------------------------------------------------------------------------------
        void                                RecordCommands(CommandBuffer& buffer);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get all barriers that need to be sent to the Renderer to acquire resources loaded on the
        ///     Asset Thread.
        //----------------------------------------------------------------------------------------------------
        const std::vector<ImageBarrierDesc>& GetAcquireBarriers() const { return m_pendingAcquireBarriers; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get all semaphores that need to be signaled release. Used for resources that are loaded on the Asset Thread
        //----------------------------------------------------------------------------------------------------
        const std::vector<vk::Semaphore>&   GetSignalSemaphores() const { return m_signalSemaphores; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Check if there are any pending upload operations.
        //----------------------------------------------------------------------------------------------------
        bool                                IsEmpty() const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Clears all pending operations, and frees all staging buffers immediately. Make sure that
        ///     when this is called, the device is idle.
        //----------------------------------------------------------------------------------------------------
        void                                Destroy();
        
    private:
        //----------------------------------------------------------------------------------------------------
        /// @brief : Create a staging buffer that will be used to copy data into the buffer/image.
        //----------------------------------------------------------------------------------------------------
        DeviceBufferRange                   AcquireStagingBuffer(const void* pData, uint64 dataSize, const SemaphoreValue& semaphoreState);
        
        //----------------------------------------------------------------------------------------------------
        /// @brief : Clears all appended commands. 
        //----------------------------------------------------------------------------------------------------
        void                                ClearPending();

        //----------------------------------------------------------------------------------------------------
        /// @brief : All temporary staging resources are associated with a provided Semaphore State.
        ///     This will release all staging buffers that have been signaled (or are invalid).
        ///	@param forceAll : If true, then it is assumed that all buffers can be freed, which typically requires
        ///     that the device is idle.
        //----------------------------------------------------------------------------------------------------
        void                                ReleaseStagingBuffers(const bool forceAll = false);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Contains the staging buffer that will copy to the destination resource, and the
        ///     synchronization semaphore. 
        //----------------------------------------------------------------------------------------------------
        struct StagingResource
        {
            DeviceBuffer                    m_buffer = nullptr;             // Staging Buffer.
            SemaphoreValue                  m_semaphoreState = nullptr;     // Synchronization semaphore.
        };
        
        BarrierGroupDesc                    m_preBarriers;                  // Memory Barriers to apply before the upload commands.
        BarrierGroupDesc                    m_postBarriers;                 // Memory Barriers to apply after the upload commands.
        std::vector<CopyBufferDesc>         m_copyBufferDescs{};
        std::vector<CopyBufferToImageDesc>  m_copyBufferToImageDescs{};
        std::vector<StagingResource>        m_stagingResources{};
        std::vector<ImageBarrierDesc>       m_pendingAcquireBarriers{};
        std::vector<vk::Semaphore>          m_signalSemaphores{};   
        RenderDevice*                       m_pDevice = nullptr;
        size_t                              m_stagingResourcesSize = 0;     // Total size of all staging buffers.
    };
}
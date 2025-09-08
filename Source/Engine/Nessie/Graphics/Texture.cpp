// Texture.cpp
#include "Texture.h"

#include <stb_image.h>

#include "DataUploader.h"
#include "RenderDevice.h"
#include "Renderer.h"
#include "Nessie/Application/Application.h"
#include "Nessie/Application/Device/DeviceManager.h"

namespace nes
{
    Texture::~Texture()
    {
        NES_ASSERT(Platform::IsMainThread());

        m_image = nullptr;
        m_imageData.Free();
    }

    void Texture::SetDeviceDebugName(const char* name)
    {
        if (m_image != nullptr)
            m_image.SetDebugName(name);
    }

    const ImageDesc& Texture::GetDesc() const
    {
        NES_ASSERT(m_image != nullptr);
        return m_image.GetDesc();
    }

    UInt3 Texture::GetExtent() const
    {
        NES_ASSERT(m_image != nullptr);
        const auto extent = m_image.GetExtent();
        return UInt3(extent.width, extent.height, extent.depth);
    }

    ELoadResult Texture::LoadFromFile(const std::filesystem::path& path)
    {
        // Load the image data:
        int width, height, channels;
        void* pData = stbi_load(path.string().c_str(), &width, &height, &channels, STBI_rgb_alpha);
        if (pData == nullptr)
        {
            NES_ERROR("Failed to load texture! Failed to load from file!\n\tPath: {} \n\tError: {}", path.string(), stbi_failure_reason());
            return ELoadResult::Failure;
        }
        
        m_imageData = Buffer(pData, width * height * STBI_rgb_alpha);

        // [TODO]: Allow custom number of mip levels from metadata.
        //const uint32 mipLevels = graphics::CalculateMipLevelCount(static_cast<uint32>(width), static_cast<uint32>(height));     

        // Texture Description
        ImageDesc textureDesc{};
        textureDesc.m_width = math::Max(static_cast<uint32>(width), 1U);
        textureDesc.m_height = math::Max(static_cast<uint32>(height), 1U);
        textureDesc.m_depth = 1;
        textureDesc.m_format = EFormat::RGBA8_UNORM;
        textureDesc.m_layerCount = 1;
        textureDesc.m_mipCount = 1; // // [TODO]: mipLevels from file - not calculated.
        textureDesc.m_sampleCount = 1; // [TODO]: Multisampling
        textureDesc.m_type = EImageType::Image2D;
        textureDesc.m_usage = EImageUsageBits::ShaderResource;
        textureDesc.m_clearValue = ClearValue{}; 

        // Allocation description. 
        const AllocateImageDesc allocDesc
        {
            .m_desc = textureDesc,
            .m_memoryLocation = EMemoryLocation::Device,
            .m_isDedicated = true,
        };

        // Create the Device Texture:
        auto& device = DeviceManager::GetRenderDevice();
        m_image = DeviceImage(device, allocDesc);
        
        auto cmdBuffer = Renderer::BeginTempCommands();
        {
            // Upload image data:
            DataUploader dataUploader(Renderer::GetDevice());
            ImageUploadDesc uploadDesc{};
            uploadDesc.m_uploadSize = m_imageData.GetSize();
            uploadDesc.m_pPixelData = m_imageData.Get();
            uploadDesc.m_pImage = &m_image;
            uploadDesc.m_newLayout = EImageLayout::ShaderResource;
            dataUploader.AppendUploadImage(uploadDesc);
            dataUploader.RecordCommands(cmdBuffer);

            // [TODO]: Uploading Mip Map data:
            // Turns out, the image blitting method that I started with isn't the best option - I should use a stb_image_resize2.h to
            // create the mip map levels that I need instead of relying on the blit image. Not all formats are
            // supported by blit image, and the command has to be submitted on a device queue with graphics capabilities.
            // - When importing a texture into the Engine Application, mip maps should be generated and stored in a single file, with
            // a base level and count at the beginning of the binary file.
            
            // Submit commands:
            Renderer::SubmitAndWaitTempCommands(cmdBuffer);
        }
        
        return ELoadResult::Success;
    }
}

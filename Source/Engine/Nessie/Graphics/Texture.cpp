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
    Texture::Texture(DeviceImage&& image, Buffer&& imageData)
        : m_image(std::move(image))
        , m_imageData(std::move(imageData))
    {
        //
    }

    Texture::Texture(Texture&& other) noexcept
        : m_image(std::move(other.m_image))
        , m_imageData(std::move(other.m_imageData))
    {
        //
    }

    Texture& Texture::operator=(Texture&& other) noexcept
    {
        if (this != &other)
        {
            m_image = std::move(other.m_image);
            m_imageData = std::move(other.m_imageData);
        }

        return *this;
    }

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
        textureDesc.m_mipCount = 1; // [TODO]: mipLevels from file - not calculated.
        textureDesc.m_sampleCount = 1; // [TODO]: Load from data. Also check against allowed number of samples. 
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
            UploadImageDesc uploadDesc{};
            uploadDesc.m_pImage = &m_image;
            uploadDesc.m_pSrcData = pData;
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

    ELoadResult TextureCube::LoadFromFile(const std::filesystem::path& path)
    {
        // Load the YAML file.
        YAML::Node file = YAML::LoadFile(path.string());
        if (!file)
        {
            NES_ERROR("Failed to load TextureCube file! Expecting a YAML file type. Path: {}", path.string());
            return ELoadResult::InvalidArgument;
        }

        auto textureCube = file["TextureCube"];
        if (!textureCube)
        {
            NES_ERROR("Failed to load TextureCube file! Missing TextureCube entry. Path: {}", path.string());
            return ELoadResult::Failure;
        }
        
        return LoadFromYAML(textureCube);
    }

    ELoadResult TextureCube::LoadFromYAML(const YAML::Node& node)
    {
        auto pathsNode = node["Paths"];
        NES_ASSERT(pathsNode);
        
        // I am assuming 6 image paths.
        NES_ASSERT(pathsNode.Type() == YAML::NodeType::Sequence && pathsNode.size() == 6);

        // Load each of the size file paths:
        std::vector<uint8_t> cubeMapBytes{};
        std::array<void*, 6> cubeMapPtrs{};
        int width;
        int height;
        int channels;
        std::string path{};
        
        nes::ImageDesc imageDesc{};
        imageDesc.m_width = 0;  // Set in the loop below:
        imageDesc.m_height = 0; // Set in the loop below:
        imageDesc.m_depth = 1;
        imageDesc.m_format = EFormat::RGBA8_UNORM;
        imageDesc.m_layerCount = 6;
        imageDesc.m_mipCount = 1;       // [TODO]: mipLevels from file - not calculated.
        imageDesc.m_sampleCount = 1;    // [TODO]: Load from data. Also check against allowed number of samples. 
        imageDesc.m_type = EImageType::Image2D;
        imageDesc.m_usage = EImageUsageBits::ShaderResource;
        imageDesc.m_clearValue = ClearValue{};
        
        for (size_t i = 0; i < 6; i++)
        {
            path = NES_CONTENT_DIR;
            path += pathsNode[i].as<std::string>();
            
            stbi_uc* pBytes = stbi_load(path.c_str(), &width, &height, &channels, STBI_rgb_alpha);
            cubeMapPtrs[i] = pBytes;

            // Assert that the widths and heights are matching.
            NES_ASSERT((imageDesc.m_width == 0 && imageDesc.m_height == 0) || (imageDesc.m_width == static_cast<uint32>(width) && imageDesc.m_height == static_cast<uint32>(height)));
            imageDesc.m_width = static_cast<uint32>(width);
            imageDesc.m_height = static_cast<uint32>(height);

            // Add the image data to the buffer:
            auto* pStart = static_cast<const uint8_t*>(pBytes);
            const uint8_t* pEnd = &pStart[static_cast<uint32_t>(width * height * STBI_rgb_alpha)];
            cubeMapBytes.insert(cubeMapBytes.end(), pStart, pEnd);
        }

        // Copy the image data:
        m_imageData = Buffer::Copy(cubeMapBytes.data(), cubeMapBytes.size());
        
        // Create the Device Image:
        nes::AllocateImageDesc allocDesc{};
        allocDesc.m_desc = imageDesc;
        allocDesc.m_memoryLocation = EMemoryLocation::Device;
        allocDesc.m_isDedicated = true;
        
        auto& device = DeviceManager::GetRenderDevice();
        m_image = DeviceImage(device, allocDesc);

        auto cmdBuffer = Renderer::BeginTempCommands();
        {
            // Upload image data:
            DataUploader dataUploader(Renderer::GetDevice());
            UploadImageDesc uploadDesc{};
            uploadDesc.m_pSrcData = m_imageData.Get();
            uploadDesc.m_layerCount = 6;
            uploadDesc.m_pImage = &m_image;
            uploadDesc.m_newLayout = EImageLayout::ShaderResource;
            dataUploader.AppendUploadImage(uploadDesc);
            dataUploader.RecordCommands(cmdBuffer);
            
            // Submit commands:
            Renderer::SubmitAndWaitTempCommands(cmdBuffer);
        }

        // Free the data loaded from stb. We are storing the memory above.
        for (size_t i = 0; i < 6; i++)
        {
            stbi_image_free(cubeMapPtrs[i]);
        }

        return ELoadResult::Success;
    }
}

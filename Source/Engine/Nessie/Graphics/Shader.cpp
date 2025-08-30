// Shader.cpp
#include "Shader.h"

#include <fstream>

#include "RenderDevice.h"
#include "Nessie/Application/Device/DeviceManager.h"

namespace nes
{
    // Shader::~Shader()
    // {
    //     // [TODO]: 
    // }
    //
    // EGraphicsResult Shader::Init(const ShaderDesc& desc)
    // {
    //     if (!desc.IsValid())
    //         return EGraphicsResult::InvalidArgument;
    //     
    //     const VkShaderModuleCreateInfo createInfo
    //     {
    //         .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
    //         .pNext = nullptr,
    //         .flags = static_cast<VkShaderModuleCreateFlags>(0),
    //         .codeSize = desc.m_size,
    //         .pCode = static_cast<const uint32*>(desc.m_pByteCode),
    //     };
    //     NES_VK_FAIL_RETURN(m_device, vkCreateShaderModule(m_device, &createInfo, m_device.GetVkAllocationCallbacks(), &m_handle));
    //     
    //     m_desc = desc;
    //     return EGraphicsResult::Success;
    // }
    //
    // void Shader::SetDebugName(const char* name)
    // {
    //     m_device.SetDebugNameToTrivialObject(m_handle, name);
    // }
    [[maybe_unused]]
    static bool DetermineLanguageFromExtension(EShaderLanguage& lang, const std::filesystem::path& filename)
    {
        struct ExtensionLanguageMapping
        {
            const char8_t* const            m_extension;
            const EShaderLanguage           m_language;
        };

        static constexpr std::array kMappings =
        {
            ExtensionLanguageMapping{ .m_extension = u8".glsl", .m_language = EShaderLanguage::GLSL },
            ExtensionLanguageMapping{ .m_extension = u8".hlsl", .m_language = EShaderLanguage::HLSL },
            ExtensionLanguageMapping{ .m_extension = u8".slang", .m_language = EShaderLanguage::Slang },
            ExtensionLanguageMapping{ .m_extension = u8".spv", .m_language = EShaderLanguage::SPV },
        };

        const auto& ext = filename.extension().u8string();
        for (const auto& [mapExt, mapLang] : kMappings)
        {
            if (mapExt == ext)
            {
                lang = mapLang;
                return true;
            }
        }

        return false;
    }

    // [TODO]: Move to a central location. I'm surprised I don't have this already.
    static bool ReadFile(const std::string& filename, std::vector<char>& outData)
    {
        // Start at the end of the file to get the size of the buffer.
        std::ifstream file(filename, std::ios::ate | std::ios::binary);

        if (!file.is_open())
            return false;

        // Set the size
        outData.resize(file.tellg());

        // Move the front and read into the out buffer.
        file.seekg(0, std::ios::beg);
        file.read(outData.data(), static_cast<std::streamsize>(outData.size()));

        file.close();
        return true;
    }
    
    ELoadResult Shader::LoadFromFile(const std::filesystem::path& path)
    {
        EShaderLanguage language;
        if (!DetermineLanguageFromExtension(language, path))
        {
            NES_ERROR("Failed to load shader. Unhandled file extension!");
            return ELoadResult::InvalidArgument;
        }

        // For now, ensure SPV.
        // In the future, allow debug runtime compilation
        NES_ASSERT(language == EShaderLanguage::SPV);

        // Read the binary data:
        if (!ReadFile(path.string(), m_byteCode))
        {
            NES_ERROR("Failed to load shader! Invalid File: {}", path.string());
            return ELoadResult::InvalidArgument;
        }
        
        return ELoadResult::Success;
    }
}

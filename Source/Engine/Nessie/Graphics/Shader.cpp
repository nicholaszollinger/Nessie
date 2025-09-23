// Shader.cpp
#include "Shader.h"

#include <fstream>
#include "RenderDevice.h"
#include "Renderer.h"
#include "Nessie/Asset/AssetManager.h"

namespace nes
{
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
    
    static bool ReadFile(const std::filesystem::path& path, std::vector<char>& outCode)
    {
        std::ifstream file(path, std::ios::ate | std::ios::binary);
        
        if (!file.is_open())
            return false;
        
        
        outCode.resize(file.tellg());
        file.seekg(0, std::ios::beg);
        file.read(outCode.data(), static_cast<std::streamsize>(outCode.size()));
        file.close();
        return true;
    }

    Shader::Shader(Shader&& other) noexcept
        : m_modules(std::move(other.m_modules))
        , m_name(std::move(other.m_name))
        , m_stages(other.m_stages)
    {
        other.m_stages = EPipelineStageBits::None;
    }

    Shader& Shader::operator=(Shader&& other) noexcept
    {
        if (this != &other)
        {
            FreeShader();
            
            m_modules = std::move(other.m_modules);
            m_name = std::move(other.m_name);
            m_stages = other.m_stages;

            other.m_stages = EPipelineStageBits::None;
        }

        return *this;
    }

    Shader::~Shader()
    {
        FreeShader();
    }

    const ShaderModule* Shader::GetShaderModule(const EPipelineStageBits stage) const
    {
        if (auto it = m_modules.find(stage); it != m_modules.end())
        {
            return &it->second;
        }

        return nullptr;
    }

    GraphicsPipelineShaders Shader::GetGraphicsShaderStages() const
    {
        GraphicsPipelineShaders result;
        result.m_vertex = GetShaderModule(EPipelineStageBits::VertexShader);
        result.m_fragment = GetShaderModule(EPipelineStageBits::FragmentShader);
        result.m_geometry = GetShaderModule(EPipelineStageBits::GeometryShader);
        result.m_meshControl = GetShaderModule(EPipelineStageBits::MeshControlShader);
        result.m_meshEval = GetShaderModule(EPipelineStageBits::MeshEvaluationShader);
        result.m_tessControl = GetShaderModule(EPipelineStageBits::TessControlShader);
        result.m_tessEval = GetShaderModule(EPipelineStageBits::TessEvaluationShader);
        return result;
    }

    ELoadResult Shader::LoadFromYAML(const YAML::Node& node)
    {
        auto& device = Renderer::GetDevice();
        
        auto modules = node["Modules"];
        if (!modules)
        {
            NES_ERROR("Failed to load shader. YAML file invalid! Missing 'Modules' entry!");
            return ELoadResult::InvalidArgument;
        }

        m_name = node["Name"].as<std::string>();

        ShaderModuleDesc desc{};
        std::map<std::filesystem::path, std::vector<char>> shaderPathToBinary{};

        for (auto module : modules)
        {
            desc = {};
            
            const auto stage = static_cast<EPipelineStageBits>(module["Stage"].as<uint32>());
            if (m_stages & stage)
            {
                NES_ERROR("Failed to load shader! Duplicate Shader Stages between modules! Each shader module must be a separate stage!");
                FreeShader();
                return ELoadResult::Failure;
            }
            
            desc.m_stage = stage;
            m_stages |= stage;

            std::filesystem::path shaderPath = NES_SHADER_DIR;
            shaderPath /= module["Path"].as<std::string>();

            // Load the Shader, if necessary:
            if (!shaderPathToBinary.contains(shaderPath))
            {
                shaderPathToBinary.emplace(shaderPath, std::vector<char>());
                
                EShaderLanguage language;
                if (!DetermineLanguageFromExtension(language, shaderPath))
                {
                    NES_ERROR("Failed to load shader. Unhandled file extension!");
                    FreeShader();
                    return ELoadResult::InvalidArgument;
                }

                // For now, ensure SPV.
                // In the future, allow debug runtime compilation
                NES_ASSERT(language == EShaderLanguage::SPV);

                // Load the Shader code:
                if (!ReadFile(shaderPath, shaderPathToBinary.at(shaderPath)))
                {
                    NES_ERROR("Failed to load shader binary! Path: {}", shaderPath.string());
                    FreeShader();
                    return ELoadResult::InvalidArgument;
                }
            }

            desc.m_binary = shaderPathToBinary[shaderPath];

            // Create the Shader Module:
            desc.m_entryPointName = module["EntryPoint"].as<std::string>();
            m_modules.emplace(stage, ShaderModule(device, std::move(desc)));
        }
        
        return ELoadResult::Success;
    }

    void Shader::FreeShader()
    {
        m_modules.clear();
        m_modules.clear();
        m_stages = EPipelineStageBits::None;
        m_name = {};
    }

    ELoadResult Shader::LoadFromFile(const std::filesystem::path& path)
    {
        YAML::Node file = YAML::LoadFile(path.string());
        if (!file)
        {
            NES_ERROR("Failed to load shader. Expecting a YAML file to load each individual module!");
            return ELoadResult::InvalidArgument;
        }

        YAML::Node shader = file["Shader"];
        if (!shader)
        {
            NES_ERROR("Failed to load shader. YAML file invalid! Missing 'Shader' entry!");
            return ELoadResult::InvalidArgument;
        }

        return LoadFromYAML(shader);
    }
}

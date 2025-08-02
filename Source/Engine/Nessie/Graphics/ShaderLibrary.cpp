// ShaderLibrary.cpp
#include "ShaderLibrary.h"

#include <fstream>
#include "RenderDevice.h"
#include "Shader.h"

#include <shaderc/shaderc.hpp>
#pragma comment(lib, "shaderc_shared.lib")

namespace nes
{
    [[maybe_unused]]
    static bool DetermineLanguageFromFilename(shaderc_source_language& lang, const std::filesystem::path& filename)
    {
        struct ExtensionLanguageMapping
        {
            const char8_t* const            m_extension;
            const shaderc_source_language   m_language;
        };

        static constexpr std::array kMappings =
        {
            ExtensionLanguageMapping{ .m_extension = u8".glsl", .m_language = shaderc_source_language_glsl },
            ExtensionLanguageMapping{ .m_extension = u8".hlsl", .m_language = shaderc_source_language_hlsl }
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

    static bool DetermineStageAndKindFromFilename(EPipelineStageBits& outStage, shaderc_shader_kind& outKind, const std::filesystem::path& filename)
    {
        struct ExtensionKindMapping
        {
            const char8_t* const        m_extension;
            const shaderc_shader_kind   m_kind;
            const EPipelineStageBits    m_stage;
        };
        
        static constexpr std::array kMappings =
        {
            ExtensionKindMapping{ .m_extension = u8".vert", .m_kind = shaderc_vertex_shader, .m_stage = EPipelineStageBits::VertexShader },
            ExtensionKindMapping{ .m_extension = u8".vs",   .m_kind = shaderc_vertex_shader, .m_stage = EPipelineStageBits::VertexShader },
            ExtensionKindMapping{ .m_extension = u8".frag", .m_kind = shaderc_fragment_shader, .m_stage = EPipelineStageBits::FragmentShader },
            ExtensionKindMapping{ .m_extension = u8".fs",   .m_kind = shaderc_fragment_shader, .m_stage = EPipelineStageBits::FragmentShader },
            ExtensionKindMapping{ .m_extension = u8".ps",   .m_kind = shaderc_fragment_shader, .m_stage = EPipelineStageBits::FragmentShader },
            ExtensionKindMapping{ .m_extension = u8".geom", .m_kind = shaderc_geometry_shader, .m_stage = EPipelineStageBits::GeometryShader },
            ExtensionKindMapping{ .m_extension = u8".gs",   .m_kind = shaderc_geometry_shader, .m_stage = EPipelineStageBits::GeometryShader },
            ExtensionKindMapping{ .m_extension = u8".comp", .m_kind = shaderc_compute_shader, .m_stage = EPipelineStageBits::ComputeShader },
            ExtensionKindMapping{ .m_extension = u8".tesc", .m_kind = shaderc_tess_control_shader, .m_stage = EPipelineStageBits::TessControlShader },
            ExtensionKindMapping{ .m_extension = u8".tc",   .m_kind = shaderc_tess_control_shader, .m_stage = EPipelineStageBits::TessControlShader },
            ExtensionKindMapping{ .m_extension = u8".tese", .m_kind = shaderc_tess_evaluation_shader, .m_stage = EPipelineStageBits::TessEvaluationShader },
            ExtensionKindMapping{ .m_extension = u8".te",   .m_kind = shaderc_tess_evaluation_shader, .m_stage = EPipelineStageBits::TessEvaluationShader },
            ExtensionKindMapping{ .m_extension = u8".mesh", .m_kind = shaderc_mesh_shader, .m_stage = EPipelineStageBits::MeshControlShader },
            ExtensionKindMapping{ .m_extension = u8".task", .m_kind = shaderc_task_shader, .m_stage = EPipelineStageBits::None },
            ExtensionKindMapping{ .m_extension = u8".rgen", .m_kind = shaderc_raygen_shader, .m_stage = EPipelineStageBits::RayGenShader },
            ExtensionKindMapping{ .m_extension = u8".rint", .m_kind = shaderc_intersection_shader, .m_stage = EPipelineStageBits::IntersectionShader },
            ExtensionKindMapping{ .m_extension = u8".rahit", .m_kind = shaderc_anyhit_shader, .m_stage = EPipelineStageBits::AnyHitShader },
            ExtensionKindMapping{ .m_extension = u8".rchit", .m_kind = shaderc_closesthit_shader, .m_stage = EPipelineStageBits::ClosestHitShader },
            ExtensionKindMapping{ .m_extension = u8".rmiss", .m_kind = shaderc_miss_shader, .m_stage = EPipelineStageBits::MissShader },
            ExtensionKindMapping{ .m_extension = u8".rcall", .m_kind = shaderc_callable_shader, .m_stage = EPipelineStageBits::CallableShader },
        };

        const auto& filenameExt = filename.stem().extension().u8string();
        for (const auto& [mapExt, kind, stage] : kMappings)
        {
            if (mapExt == filenameExt)
            {
                outKind = kind;
                outStage = stage;
                return true;
            }
        }
        
        return false;
    }

    [[maybe_unused]]
    static shaderc::SpvCompilationResult CompileShader(const shaderc_shader_kind kind, const void* pSource, const size_t size, const std::filesystem::path& filename, const char* entryPointName)
    {
        shaderc::Compiler compiler;
        shaderc::CompileOptions options{};
        return compiler.CompileGlslToSpv(static_cast<const char*>(pSource), size, kind, filename.string().c_str(), entryPointName, options);
    }
    
    bool ShaderLibrary::Init(const ShaderLibraryDesc& desc)
    {
        m_desc = desc;

#if NES_SHADER_LIBRARY_ENABLE_COMPILATION
        // If runtime compilation is enabled, add the output directory to the list of search directories.
        if (m_desc.m_enableRuntimeCompilation && !m_desc.m_compileOutDir.empty())
        {
            m_desc.m_searchDirs.insert(m_desc.m_searchDirs.begin(), m_desc.m_compileOutDir);
        }
#endif
        return true;
    }

    void ShaderLibrary::Shutdown()
    {
        UnloadAllShaders();
    }

    Shader* ShaderLibrary::GetShader(const char* name, const char* entryPointName)
    {
        // Check if we have the shader loaded already: 
        const auto it = m_cachedShaders.find(name);
        if (it != m_cachedShaders.end() && it->second)
        {
            return it->second;
        }

        // Load the shader.
        auto shaderDesc = LoadShader(name, entryPointName);

        Shader* pShader;
        EGraphicsResult result = m_device.CreateResource(pShader, shaderDesc);
        if (result != EGraphicsResult::Success)
        {
            return nullptr;
        }
        
        m_cachedShaders[name] = pShader;
        return pShader;
    }

    void ShaderLibrary::UnloadShader(const char* name)
    {
        const auto it = m_cachedShaders.find(name);
        if (it != m_cachedShaders.end() && it->second)
        {
            m_device.WaitUntilIdle();
            m_device.FreeResource(it->second);
            it->second = nullptr;
            
            return;
        }

        NES_GRAPHICS_WARN(m_device, "Attempted to unload shader that isn't loaded! Path: {}", name);
    }

    void ShaderLibrary::UnloadAllShaders()
    {
        for (auto& [_, pShader] : m_cachedShaders)
        {
            if (pShader != nullptr)
                m_device.FreeResource(pShader);
        }
        m_cachedShaders.clear();
    }

    ShaderDesc ShaderLibrary::LoadShader(const std::filesystem::path& name, const char* entryPointName)
    {
        ShaderDesc shaderDesc{};

        // Locate a file, prioritizing the certain extensions.
        std::filesystem::path fullPath = name;
        ResolveShaderPath(fullPath);

        // Determine what kind of shader this is.
        EPipelineStageBits stage;
        shaderc_shader_kind kind = shaderc_glsl_infer_from_source;
        DetermineStageAndKindFromFilename(stage, kind, fullPath);

        // Read file data
        std::vector<std::byte> shaderCode;
        {
            std::ifstream dataFile(fullPath, std::ios::binary);
            if (!dataFile.is_open())
            {
                NES_GRAPHICS_ERROR(m_device, "Failed to open shader file: {}", fullPath.string().c_str());
                return shaderDesc;
            }

            dataFile.seekg(0, std::ios::end);
            shaderCode.resize(dataFile.tellg(), static_cast<std::byte>(0));
            dataFile.seekg(0, std::ios::beg);
            dataFile.read(reinterpret_cast<char*>(shaderCode.data()), static_cast<std::streamsize>(shaderCode.size()));
        }

        // If not spv, try to compile:
    #if NES_SHADER_LIBRARY_ENABLE_COMPILATION
        if (m_desc.m_enableRuntimeCompilation && fullPath.extension().u8string() != u8".spv")
        {
            NES_GRAPHICS_INFO(m_device, "Compiling shader '{}'...", fullPath.filename().string().c_str());

            // Check for language support:
            shaderc_source_language lang = shaderc_source_language_glsl;
            DetermineLanguageFromFilename(lang, fullPath);
            if (lang != shaderc_source_language_glsl)
            {
                NES_GRAPHICS_ERROR(m_device, "Failed to compile shader! Only GLSL is supported.");
                return {};
            }

            // Compile the shader:
            const auto& result = CompileShader(kind, shaderCode.data(), shaderCode.size(), fullPath, entryPointName);
            if (result.GetCompilationStatus() != shaderc_compilation_status_success)
            {
                const auto statusCode = static_cast<int>(result.GetCompilationStatus());
                NES_GRAPHICS_ERROR(m_device, "Shader compilation failed! ({}): {}", statusCode, result.GetErrorMessage());
                return {};
            }

            // Success!
            std::vector<uint32> spv;
            spv.assign(result.cbegin(), result.cend());
            
            // Store the compilation result to the out dir, if set.
            if (!m_desc.m_compileOutDir.empty())
            {
                std::filesystem::create_directories(m_desc.m_compileOutDir);
                std::filesystem::path spirvPath = m_desc.m_compileOutDir / fullPath.stem();
                spirvPath.concat(u8".spv");
                std::ofstream spirvFile(spirvPath, std::ios::binary);
                if (spirvFile.is_open())
                {
                    spirvFile.write(reinterpret_cast<const char*>(spv.data()), sizeof(spv[0]) * static_cast<std::streamsize>(spv.size()));
                    spirvFile.close();
                    NES_GRAPHICS_INFO(m_device, "Stored SPIR-V compilation result to '{}'.", spirvPath.string().c_str());
                }
                else
                {
                    NES_GRAPHICS_ERROR(m_device, " Failed opening file to save SPIR-V compilation result: '{}'.", spirvPath.string().c_str());
                }
            }
            
            NES_GRAPHICS_INFO(m_device, "Compilation successful for '{}'.", fullPath.filename().string().c_str());
            
            return ShaderDesc
            {
                .m_stage = stage,
                .m_pByteCode = spv.data(),
                .m_size = spv.size() * sizeof(spv[0]),
                .m_entryPointName = entryPointName,
            };
        }
    #endif

        return ShaderDesc
        {
            .m_stage = stage,
            .m_pByteCode = shaderCode.data(),
            .m_size = shaderCode.size(),
            .m_entryPointName = entryPointName,
        };
    }

    bool ShaderLibrary::ResolveShaderPath(std::filesystem::path& path) const
    {
        using SearchExtension = const char8_t* const;
        
        // The empty string entry is to support a fully specified filename
        // Only add in (and prefer) GLSL if compilation support is available.
#if NES_SHADER_LIBRARY_ENABLE_COMPILATION
        static constexpr std::array<SearchExtension, 3> kSearchExtensions = { u8".glsl", u8".spv", u8"" };
#else
        static constexpr std::array<SearchExtension, 2> kSearchExtensions = { u8".spv", u8"" };
#endif

        for (const auto& searchExt : kSearchExtensions)
        {
            for (const auto& searchDir : m_desc.m_searchDirs)
            {
                std::filesystem::path candidatePath = searchDir / path;
                candidatePath.concat(searchExt);
                if (std::filesystem::exists(candidatePath))
                {
                    path = candidatePath;
                    return true;
                }
            }
        }

        return false;
    }
}

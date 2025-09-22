// Shader.h
#pragma once
#include "ShaderModule.h"
#include "Nessie/Asset/AssetBase.h"

namespace nes
{
    enum class EShaderLanguage
    {
        Slang,
        GLSL,
        HLSL,
        SPV,    // Compiled shader type.
    };
    
    //----------------------------------------------------------------------------------------------------
    // [TODO]: 
    // - Entry point names
    // - Infer shader stages per entry point name (slang, hlsl) or extension (glsl).
    //
    /// @brief : A Shader Asset contains the compiled code for the shader. 
    //----------------------------------------------------------------------------------------------------
    class Shader final : public AssetBase
    {
        NES_DEFINE_TYPE_INFO(Shader)

    public:
        Shader() = default;
        Shader(Shader&& other) noexcept;
        Shader& operator=(Shader&& other) noexcept;
        virtual ~Shader() override;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the Shader Module for a given stage. Returns nullptr if no module exists for that
        ///     stage.
        //----------------------------------------------------------------------------------------------------
        const ShaderModule*         GetShaderModule(const EPipelineStageBits stage) const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Returns the available shader modules for different stages of a graphics pipeline.
        ///     If there is no module for a stage, the entry will be left as nullptr.
        //----------------------------------------------------------------------------------------------------
        GraphicsPipelineShaders     GetGraphicsShaderStages() const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the name of the shader.
        //----------------------------------------------------------------------------------------------------
        const std::string&          GetName() const { return m_name; }
    
    private:
        //----------------------------------------------------------------------------------------------------
        /// @brief : Load the shader code from a file.
        //----------------------------------------------------------------------------------------------------
        virtual ELoadResult         LoadFromFile(const std::filesystem::path& path) override;
        ELoadResult                 LoadFromYAML(const YAML::Node& node);
        
        //----------------------------------------------------------------------------------------------------
        /// @brief : Free all shader modules and shader code.
        //----------------------------------------------------------------------------------------------------
        void                        FreeShader();

    private:
        std::map<EPipelineStageBits, ShaderModule>  m_modules;
        std::string                                 m_name{};
        EPipelineStageBits                          m_stages{}; // Bitmask of all stages of each module.
    };

    static_assert(ValidAssetType<Shader>);
}

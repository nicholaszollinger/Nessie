// Shader.h
#pragma once
#include "GraphicsCommon.h"
#include "DeviceObject.h"
#include "Nessie/Asset/AssetBase.h"
#include "Nessie/Core/Memory/Buffer.h"

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

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the compiled shader code.
        //----------------------------------------------------------------------------------------------------
        const std::vector<char>&    GetByteCode() const { return m_byteCode; }
    
    private:
        //----------------------------------------------------------------------------------------------------
        /// @brief : Load the shader code from a file.
        //----------------------------------------------------------------------------------------------------
        virtual ELoadResult         LoadFromFile(const std::filesystem::path& path) override;

    private:
        std::vector<char>           m_byteCode{};
        
    };

    static_assert(ValidAssetType<Shader>);
}

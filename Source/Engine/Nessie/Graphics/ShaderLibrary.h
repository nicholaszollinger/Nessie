// ShaderLibrary.h
#pragma once
#include <filesystem>

#include "GraphicsCommon.h"

namespace nes
{
#ifdef NES_DEBUG
    #define NES_SHADER_LIBRARY_ENABLE_COMPILATION 1
#else
    #define NES_SHADER_LIBRARY_ENABLE_COMPILATION 0
#endif

    //----------------------------------------------------------------------------------------------------
    /// @brief : Configuration options for the Shader Library.
    //----------------------------------------------------------------------------------------------------
    struct ShaderLibraryDesc
    {
        /// List of directories to search when resolving a shader file path
        std::vector<std::string> m_searchDirs;

        /// If set and if compilation is enabled, SPIR-V shaders will be
        ///	saved to this directory. This will automatically be searched before
        ///	searchDirs.
        std::string             m_compileOutDir;

        /// Whether to enable runtime compilation support.
        bool                    m_enableRuntimeCompilation = true;
    };
    
    //----------------------------------------------------------------------------------------------------
    /// @brief : Class that manages loading and unloading Shaders. Optionally can compile Shaders during
    ///     runtime as well.
    ///     - For now, this only supports GLSL.
    //----------------------------------------------------------------------------------------------------
    class ShaderLibrary
    {
    public:
        /* Constructor */       ShaderLibrary(RenderDevice& device) : m_device(device) {}
        /* Destructor */        ~ShaderLibrary() = default;

        // No copy or move allowed.
        /* Copy Constructor */  ShaderLibrary(const ShaderLibrary&) = delete;
        /* Move Constructor */  ShaderLibrary(ShaderLibrary&&) noexcept = delete;
        /* Copy Assignment */   ShaderLibrary& operator=(const ShaderLibrary&) = delete;
        /* Move Assignment  */  ShaderLibrary& operator=(ShaderLibrary&&) noexcept = delete;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Initialize the shader library with the RenderDevice and configuration options.
        //----------------------------------------------------------------------------------------------------
        bool                    Init(const ShaderLibraryDesc& desc);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Destroys all created resources. 
        //----------------------------------------------------------------------------------------------------
        void                    Shutdown();

        //----------------------------------------------------------------------------------------------------
        /// @brief : Lookup a shader object given its shader code path.
        ///
        ///     Each ConfigOptions::searchDirs entry will be searched for the provided shader filename,
        ///     first looking for a compiled version that has the extension .spv, then if compilation
        ///     support is enabled, looking for a match with the extension .glsl
        ///
        ///     So if you have a file "shaders/triangle.vert.glsl" you could load this
        ///	    shader in these ways:
        ///	    1. Add "shaders" to searchDirs, enable compilation, and call GetModule("triangle.vert")
        ///	    2. Enable compilation and call GetModule("shaders/triangle.vert")
        ///	    3. Enable compilation and call GetModule("shaders/triangle.vert.glsl")
        ///
        ///     If you want to load a precompiled shader, the flow is the same, except with a .spv file.
        ///	    For example, "shaders/triangle.vert.spv" could be loaded by:
        ///	    1. Add "shaders" to searchDirs and call GetModule("triangle.vert")
        ///	    2. Call GetModule("shaders/triangle.vert")
        ///	    3. Call GetModule("shaders/triangle.vert.spv")
        //----------------------------------------------------------------------------------------------------
        Shader*                 GetShader(const char* name, const char* entryPointName = "main");

        //----------------------------------------------------------------------------------------------------
        /// @brief : Unload a shader, given its path. This will force it to be reloaded on the next use.
        ///     See GetShader more info.
        //----------------------------------------------------------------------------------------------------
        void                    UnloadShader(const char* name);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Unload all loaded shaders. 
        //----------------------------------------------------------------------------------------------------
        void                    UnloadAllShaders();

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the shader library's properties.
        //----------------------------------------------------------------------------------------------------
        const ShaderLibraryDesc& GetDesc() const { return m_desc; }

    private:
        //----------------------------------------------------------------------------------------------------
        /// @brief : Load a shader from disk. If compilation is enabled, this will compile the shader if not already
        ///     in .spv format.
        //----------------------------------------------------------------------------------------------------
        ShaderDesc              LoadShader(const std::filesystem::path& name, const char* entryPointName);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Loop through set search directories to find a shader file and store it in the path.
        ///	@returns : If no file exists, returns false.
        //----------------------------------------------------------------------------------------------------
        bool                    ResolveShaderPath(std::filesystem::path& path) const;
    
    private:
        using ShaderMap = std::unordered_map<std::string, Shader*>;
        
        RenderDevice&           m_device;
        ShaderLibraryDesc       m_desc;             // Behavior settings for the shader library.
        ShaderMap               m_cachedShaders;    // Loaded shaders.
    };
}
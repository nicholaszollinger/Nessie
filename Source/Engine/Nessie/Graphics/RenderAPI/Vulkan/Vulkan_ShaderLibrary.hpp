#pragma once

///
/// VULKAN SHADER LIBRARY FOR GAP-311
/// Written by Josh Stefanski.
///
/// This file implements a very simple caching shader library which also
/// supports compilation on the fly by using the shaderc shared library
/// included with the Vulkan SDK. This on-the-fly compilation is only enabled
/// in debug mode as it relies on other DLLs and its generally only useful
///	for development.
///

#include <unordered_map>
#include <vector>
#include <filesystem>
#include <functional>
#include <cstdarg>
#include <fstream>

#include "Nessie/Graphics/RenderAPI/Vulkan/Vulkan_Core.h"

#if _DEBUG
#define GAP311_SHADERLIB_ENABLE_COMPILATION 1
#endif

#if GAP311_SHADERLIB_ENABLE_COMPILATION
#pragma warning(push)
// 26439: Disabling the noexcept warning in shaderc.hpp
#pragma warning(disable: 26439)
#include <shaderc/shaderc.hpp>
#pragma warning(pop)
#endif

namespace GAP311
{
	class VulkanShaderLibrary
	{
	public:
		struct ConfigOptions
		{
			/// Callback for providing error log messages
			std::function<void(const char*)> logMessage;
			/// List of directories to search when resolving a shader file path
			std::vector<std::string> searchDirs;
			/// Callback when a file is loaded
			std::function<void(const std::filesystem::path&)> onFileLoaded;

#if GAP311_SHADERLIB_ENABLE_COMPILATION
			/// Whether to enable runtime compilation support
			bool enableCompilation = true;
			/// If set and if compilation is enabled, SPIR-V shaders will be
			///	saved to this directory. This will automatically be searched before
			///	searchDirs.
			std::string spirvOutputDir;
#endif
		};

		/// Provide the vulkan device to the library manager and any configuration options
		bool Initialize(vk::Device device, const ConfigOptions& options = {});
		/// Destroys any created resources
		void Shutdown();

		/// Lookup a ShaderModule object given its shader code path
		/// 
		/// Each ConfigOptions::searchDirs entry will be searched for
		///	the provided shader filename, first looking for a compiled version
		///	that has the extension .spv, then if compilation support is enabled,
		///	looking for a match with the extension .glsl
		///
		/// So if you have a file "shaders/triangle.vert.glsl" you could load this
		///	shader in these ways:
		///	1. Add "shaders" to searchDirs, enable compilation, and call GetModule("triangle.vert")
		///	2. Enable compilation and call GetModule("shaders/triangle.vert")
		///	3. Enable compilation and call GetModule("shaders/triangle.vert.glsl")
		/// If you want to load a precompiled shader, the flow is the same, except with a .spv file.
		///	For example "shaders/triangle.vert.spv" could be loaded by:
		///	1. Add "shaders" to searchDirs and call GetModule("triangle.vert")
		///	2. Call GetModule("shaders/triangle.vert")
		///	3. Call GetModule("shaders/triangle.vert.spv")
		vk::ShaderModule GetModule(const char* name);

		/// Manually unload a shader given its path, this will force it to be reloaded on the next use
		bool UnloadModule(const char* name);
		/// Unload everything immediately
		void UnloadAllModules();

		/// The search dirs, including the compilation output dir if specified
		const std::vector<std::string>& GetSearchDirs() const { return m_options.searchDirs; }

		// Trivial construction
		VulkanShaderLibrary() = default;
		~VulkanShaderLibrary() = default;
		// No copying, only move
		VulkanShaderLibrary(const VulkanShaderLibrary&) = delete;
		VulkanShaderLibrary& operator=(const VulkanShaderLibrary&) = delete;
		VulkanShaderLibrary(VulkanShaderLibrary&&) noexcept = default;
		VulkanShaderLibrary& operator=(VulkanShaderLibrary&&) noexcept = default;
	private:
		vk::Device m_device;
		ConfigOptions m_options;
		std::unordered_map<std::string, vk::ShaderModule> m_cachedModules;

		vk::ShaderModule LoadModule(const std::filesystem::path& name);
		void Log(const char* message, ...);

		bool ResolveModulePath(std::filesystem::path& modulePath);

#if GAP311_SHADERLIB_ENABLE_COMPILATION
		static bool DetermineLanguageFromFilename(shaderc_source_language& lang, const std::filesystem::path& filename);
		static bool DetermineKindFromFilename(shaderc_shader_kind& kind, const std::filesystem::path& filename);
		vk::ShaderModule CompileShader(shaderc_shader_kind kind, shaderc_source_language lang, const void* source, size_t sourceSize, const std::filesystem::path& filename = {});
#endif
	};

	////////////////// Implementation //////////////////

	inline bool VulkanShaderLibrary::Initialize(vk::Device device, const ConfigOptions& options)
	{
		m_device = device;
		m_options = options;
#if GAP311_SHADERLIB_ENABLE_COMPILATION
		if (m_options.enableCompilation && !m_options.spirvOutputDir.empty())
		{
			m_options.searchDirs.insert(m_options.searchDirs.begin(), m_options.spirvOutputDir);
		}
#endif
		return true;
	}

	inline void VulkanShaderLibrary::Shutdown()
	{
		UnloadAllModules();
	}

	inline vk::ShaderModule VulkanShaderLibrary::GetModule(const char* name)
	{
		auto cacheIter = m_cachedModules.find(name);
		if (cacheIter != m_cachedModules.end() && cacheIter->second)
			return cacheIter->second;
		
		auto shaderModule = LoadModule(name);
		m_cachedModules[name] = shaderModule;
		return shaderModule;
	}

	inline bool VulkanShaderLibrary::UnloadModule(const char* name)
	{
		if (!m_device)
			return false;

		auto cacheIter = m_cachedModules.find(name);
		if (cacheIter != m_cachedModules.end() && cacheIter->second)
		{
			m_device.waitIdle();
			m_device.destroyShaderModule(cacheIter->second);
			m_cachedModules[name] = nullptr;
			return true;
		}
		return false;
	}

	inline void VulkanShaderLibrary::UnloadAllModules()
	{
		if (!m_device)
			return;

		for (const auto& [_, shader] : m_cachedModules)
		{
			if (shader)
				m_device.destroyShaderModule(shader);
		}

		m_cachedModules.clear();
	}

	inline vk::ShaderModule VulkanShaderLibrary::LoadModule(const std::filesystem::path& name)
	{
		if (!m_device)
			return nullptr;

		// Locate a file, prioritizing certain extensions
		std::filesystem::path modulePath = name;
		ResolveModulePath(modulePath);

		// Read file data
		std::vector<std::byte> shaderData;
		{
			std::ifstream dataFile(modulePath, std::ios::binary);
			if (!dataFile.is_open())
			{
				Log("Failed to open shader file: %s", modulePath.string().c_str());
				return nullptr;
			}

			if (m_options.onFileLoaded)
				m_options.onFileLoaded(modulePath);
		
			dataFile.seekg(0, std::ios::end);
			shaderData.resize(dataFile.tellg(), static_cast<std::byte>(0));
			dataFile.seekg(0, std::ios::beg);
			dataFile.read(reinterpret_cast<char*>(shaderData.data()), shaderData.size());
		}

		// if not spv, try compile
#if GAP311_SHADERLIB_ENABLE_COMPILATION
		if (m_options.enableCompilation && modulePath.extension().u8string() != u8".spv")
		{
			shaderc_source_language lang = shaderc_source_language_glsl;
			shaderc_shader_kind kind = shaderc_glsl_infer_from_source;
			DetermineLanguageFromFilename(lang, modulePath);
			DetermineKindFromFilename(kind, modulePath);
			vk::ShaderModule shader = CompileShader(kind, lang, shaderData.data(), shaderData.size(), modulePath);
			if (shader)
			{
				Log("Compiled shader module: %s", modulePath.string().c_str());
				return shader;
			}
		}
#endif

		// otherwise just try and load it
		vk::ShaderModuleCreateInfo createInfo{};
		createInfo.setPCode(reinterpret_cast<const uint32_t*>(shaderData.data()));
		createInfo.setCodeSize(shaderData.size());
		auto shaderModule = m_device.createShaderModule(createInfo);
		if (shaderModule)
			Log("Loaded shader module: %s", modulePath.string().c_str());
		return shaderModule;
	}

	inline void VulkanShaderLibrary::Log(const char* message, ...)
	{
		if (m_options.logMessage)
		{
			char buffer[1024];
			va_list args;
			va_start(args, message);
			vsprintf_s(buffer, message, args);
			va_end(args);
			m_options.logMessage(buffer);
		}
	}

	inline bool VulkanShaderLibrary::ResolveModulePath(std::filesystem::path& modulePath)
	{
		using SearchExtension = const char8_t* const;
		// The empty string entry is to support a fully-specified filename
		// Only add in (and prefer) GLSL if compilation support is available.
#if GAP311_SHADERLIB_ENABLE_COMPILATION
		constexpr SearchExtension kSearchExtensions[] = { u8".glsl", u8".spv", u8"" };
#else
		const SearchExtension kSearchExtensions[] = { u8".spv", u8"" };
#endif
		for (const auto& searchExt : kSearchExtensions)
		{
			for (const auto& searchDir : m_options.searchDirs)
			{
				std::filesystem::path candidatePath = searchDir / modulePath;
				candidatePath.concat(searchExt);
				if (exists(candidatePath))
				{
					modulePath = candidatePath;
					return true;
				}
			}
		}
		return false;
	}

#if GAP311_SHADERLIB_ENABLE_COMPILATION

	inline bool VulkanShaderLibrary::DetermineLanguageFromFilename(shaderc_source_language& lang, const std::filesystem::path& filename)
	{
		struct ExtensionLanguageMapping
		{
			const char8_t* const extension;
			const shaderc_source_language language;
		} mapping[] = {
			{ u8".glsl", shaderc_source_language_glsl },
			{ u8".hlsl", shaderc_source_language_hlsl },
		};
		const auto& ext = filename.extension().u8string();
		for (const auto& [mapExt, mapLang] : mapping)
		{
			if (mapExt == ext)
			{
				lang = mapLang;
				return true;
			}
		}

		return false;
	}

	inline bool VulkanShaderLibrary::DetermineKindFromFilename(shaderc_shader_kind& kind, const std::filesystem::path& filename)
	{
		struct ExtensionKindMapping
		{
			const char8_t* const extension;
			const shaderc_shader_kind kind;
		} mapping[] = {
			{ u8".vert", shaderc_vertex_shader },
			{ u8".vs",   shaderc_vertex_shader },
			{ u8".frag", shaderc_fragment_shader },
			{ u8".fs",   shaderc_fragment_shader },
			{ u8".ps",   shaderc_fragment_shader },
			{ u8".geom", shaderc_geometry_shader },
			{ u8".gs",   shaderc_geometry_shader },
			{ u8".comp", shaderc_compute_shader },
			{ u8".tesc", shaderc_tess_control_shader },
			{ u8".tc",   shaderc_tess_control_shader },
			{ u8".tese", shaderc_tess_evaluation_shader },
			{ u8".te",   shaderc_tess_evaluation_shader },
			{ u8".mesh", shaderc_mesh_shader },
			{ u8".task", shaderc_task_shader },
			{ u8".rgen", shaderc_raygen_shader },
			{ u8".rint", shaderc_intersection_shader },
			{ u8".rahit", shaderc_anyhit_shader },
			{ u8".rchit", shaderc_closesthit_shader },
			{ u8".rmiss", shaderc_miss_shader },
			{ u8".rcall", shaderc_callable_shader },
		};

		const auto& filenameExt = filename.stem().extension().u8string();
		for (const auto& [mapExt, mapKind] : mapping)
		{
			if (mapExt == filenameExt)
			{
				kind = mapKind;
				return true;
			}
		}
		return false;
	}

	inline vk::ShaderModule VulkanShaderLibrary::CompileShader(shaderc_shader_kind kind, shaderc_source_language lang,
		const void* source, size_t sourceSize, const std::filesystem::path& filename)
	{
		if (!m_options.enableCompilation)
			return nullptr;

		if (lang != shaderc_source_language_glsl)
		{
			Log("CompileShader: Only GLSL is supported.");
			return nullptr;
		}

		shaderc::Compiler compiler;
		shaderc::CompileOptions options{};

		const auto& result = compiler.CompileGlslToSpv(reinterpret_cast<const char*>(source), sourceSize, kind, filename.string().c_str(), "main", options);
		if (result.GetCompilationStatus() != shaderc_compilation_status_success)
		{
			Log("CompileShader: Compilation failed (%d): %s", result.GetCompilationStatus(), result.GetErrorMessage().c_str());
			return nullptr;
		}
		std::vector<uint32_t> spv;
		spv.assign(result.cbegin(), result.cend());

		if (!m_options.spirvOutputDir.empty())
		{
			std::filesystem::create_directories(m_options.spirvOutputDir);
			std::filesystem::path spirvPath = m_options.spirvOutputDir / filename.stem();
			spirvPath.concat(u8".spv");
			std::ofstream spirvFile(spirvPath, std::ios::binary);
			if (spirvFile.is_open())
			{
				spirvFile.write(reinterpret_cast<const char*>(spv.data()), spv.size() * sizeof(spv[0]));
				spirvFile.close();
				Log("CompileShader: Stored SPIR-V compilation result to: %s", spirvPath.string().c_str());
			}
			else
			{
				Log("CompileShader: Failed opening file to save SPIR-V compilation result: %s", spirvPath.string().c_str());
			}
		}

		// This makes an assumption based on the original C API that the underlying
		// memory is contiguous
		vk::ShaderModuleCreateInfo createInfo{};
		createInfo.pCode = spv.data();
		createInfo.codeSize = spv.size() * sizeof(spv[0]);
		return m_device.createShaderModule(createInfo);
	}

#endif

}
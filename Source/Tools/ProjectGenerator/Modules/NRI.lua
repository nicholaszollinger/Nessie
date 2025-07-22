-- Module to build the NRI projects
local utility = require("Utility");
local projectCore = require("ProjectCore");
local vulkanSupport = dofile("VulkanSupport.lua");

premake.modules.NRI = {};
local m = premake.modules.NRI;
utility.SetupModule(m, "NRI", true);

m.NRISourceDirectory = projectCore.ThirdPartyDir .. "NRI\\";
m.LibDir = projectCore.DefaultLibOutDirPath .. "NRI/";

-----------------------------------------------------------------------------------
--- Ensure that we have a valid Graphics API.
-----------------------------------------------------------------------------------
function m.ValidateSupport()
	hasSupport = false;
    hasSupport = vulkanSupport.ValidateVulkanSupport();
  
	-- [TODO] Query Suport for Window SDK files if supporting D3D11/D3D12.

    return hasSuport;
end


-----------------------------------------------------------------------------------
--- Generate the NRI Main NRI project.
-----------------------------------------------------------------------------------
function m._SetNRIProjectDefaults()
    filter{}

    language "C++"
    cppdialect "C++20"
    systemversion "latest"
    staticruntime "Off"
    kind "StaticLib"

    targetdir("$(SolutionDir)Intermediate/Libs/$(Configuration)_$(PlatformTarget)/NRI/")
    objdir(projectCore.DefaultIntermediateDir)
    warnings("Extra")

    defines
    {
        "_CRT_SECURE_NO_WARNINGS",
        "NRI_STATIC_LIBRARY=1",
        "NRI_ENABLE_NVTX_SUPPORT=1",
        "NRI_ENABLE_DEBUG_NAMES_AND_ANNOTATIONS=1",
        "NRI_ENABLE_NONE_SUPPORT=1",
        "NRI_ENABLE_VK_SUPPORT=1",
        "NRI_ENABLE_VALIDATION_SUPPORT=1",
        "NRI_ENABLE_IMGUI_EXTENSION=1",

        "NRI_ENABLE_D3D11_SUPPORT=0",
        "NRI_ENABLE_D3D12_SUPPORT=0",
        "NRI_ENABLE_D3D_EXTENSIONS=0",
        "NRI_ENABLE_AGILITY_SDK_SUPPORT=0",

		--"NRI_ENABLE_NGX_SDK=0",
		--"NRI_ENABLE_FFX_SDK=0",
		--"NRI_ENABLE_XESS_SDK=0",

		-- Linux stuff, ignoreing off for now.
		--"NRI_ENABLE_XLIB_SUPPORT=0",
		--"NRI_ENABLE_WAYLAND_SUPPORT=0",
    }

	disablewarnings
    {
        "4324", -- "'X' : structure was padded due to alignment specifier"
    }

    filter "files:**.hlsl"
        buildaction "None"

    filter "platforms:x64"
        architecture "x64"

    filter "configurations:Debug"
        defines { "_DEBUG", "NES_DEBUG" }
        symbols "On"

    filter "configurations:Release"
        defines { "NDEBUG", "NES_RELEASE" }
        optimize "On"
        runtime "Release"

    filter "system:windows"
        systemversion "latest"
        defines 
        { 
            "WIN32",
            "_WINDOWS",
            "WIN32_LEAN_AND_MEAN",
            "NOMINMAX",
        }

    filter{}
end

return m;

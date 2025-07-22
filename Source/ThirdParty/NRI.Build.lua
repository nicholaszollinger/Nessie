-- NRI Project Configuration.
-- Calling DependencyInjector.Link("NRI") will include all NRI projects.
local projectCore = require("ProjectCore");
local nriCore = require("NRI");
if (nriCore == nil) then
    return nil;
end

local d = {}
d.Name = "NRI";
d.Group = "ThirdParty/NRI"

function d.ConfigureProject(dependencyInjector)
    nriCore._SetNRIProjectDefaults();

	dependencyInjector.Link("NRI_Shared");
	--dependencyInjector.Link("NRI_D3D11");
	--dependencyInjector.Link("NRI_D3D12");
	dependencyInjector.Link("NRI_Vulkan");
	dependencyInjector.Link("NRI_None");
	dependencyInjector.Link("NRI_Validation");

    --     --["ThirdParty/*"] = { projectCore.SourceFolder .. "**.*" },
            --     --["Source/*"] = { target.Directory .. ".**"},
            --     --["*"] = { buildScriptDir .. "*." .. BUILD_EXTENSION, projectCore.SourceFolder}
            --     --{ ["ThirdParty/*"] = projectCore.SourceFolder .. "**.*"},
            --     --{ ["*"] = projectCore.SourceFolder .. "/" .. target.Group .. "/**.*" },
            --     --["*"] = { projectCore.SourceFolder .. "**.*"},
            --     ["Source/*"] = { buildScriptDir .. "**.*"},

    -- Include Files
    files { nriCore.NRISourceDirectory .. "\\Include\\NRI\\**.*"}
    vpaths { ["Include/*"] = { d.BuildDirectory .. "/NRI/Include/NRI/**.*"} }

    files { nriCore.NRISourceDirectory .."\\Resources\\**.*" }
    vpaths { ["Resources/*"] = { d.BuildDirectory .. "/NRI/Resources/**.*"} }

    files { nriCore.NRISourceDirectory .. "\\Source\\Creation\\Creation.cpp" }
    vpaths { ["Source/*"] = { d.BuildDirectory .. "/NRI/Source/Creation/**.*"} }

    includedirs
    {
        nriCore.NRISourceDirectory .. "\\Include\\NRI\\",
        nriCore.NRISourceDirectory .. "\\Source\\Shared\\",
        nriCore.NRISourceDirectory .. "\\_deps\\nvtx-src\\c\\include\\",
        nriCore.NRISourceDirectory .. "\\_deps\\vulkan_headers-src\\include\\"
    }
end

-- Links all NRI Projects.
function d.Link()
    links 
    {
        "NRI",
        "NRI_Shared",
        "NRI_None",
        --"NRI_D3D11",
        --"NRI_D3D12",
        "NRI_Vulkan",
        "NRI_Shaders",
        "NRI_Validation",
    }

    libdirs { nriCore.LibDir }
end

-- Includes for all NRI projects.
function d.Include()
    includedirs
    {
        nriCore.NRISourceDirectory .. "\\Include\\",
        nriCore.NRISourceDirectory .. "\\Source\\",
    }

	-- If this is creating an executeable, then we want to copy the dlls.
	filter {"kind:ConsoleApp"}
		-- These are necessary for AgilitySDK with D3D12.
		--postbuildcommands { "{MKDIR} %[" .. projectCore.DefaultOutDir .. "AgilitySDK]"}
		--postbuildcommands { "{COPYDIR} \"" .. nriCore.NRISourceDirectory .. "\\_deps\\agilitysdk-src\\build\\native\\bin\\x64\" \"" .. projectCore.DefaultOutDir .. "AgilitySDK\\\""};
		--postbuildcommands { "{COPYFILE} \"" .. nriCore.NRISourceDirectory .. "\\_deps\\amdags-src\\ags_lib\\lib\\amd_ags_x64.dll\" \"" .. projectCore.DefaultOutDir .. "\""};
	
	filter{}
end

return d;
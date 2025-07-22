-- NRI D3D12 Project Configuration.
-- Use "DependencyInjector.Link("NRI")" to include all NRI projects.
local nriCore = require("NRI");
if (nriCore == nil) then
    return nil;
end

local d = {}
d.Name = "NRI_D3D12";
d.Group = "ThirdParty/NRI"

-- [TODO]: There is a bunch of work to be done to ensure that D3D works. For now I am going to remove the
-- project. 

--function d.ConfigureProject(dependencyInjector)
--    nriCore._SetNRIProjectDefaults();
--
--    dependencyInjector.Link("NRI_Shared");
--
--	defines
--	{
--		"NRI_AGILITY_SDK_VERSION_MAJOR=616"
--	}
--
--    files
--    {
--        nriCore.NRISourceDirectory .. "\\Source\\D3D12\\**.*",
--        nriCore.NRISourceDirectory .. "\\_deps\\nvapi-src\\nvapi.h",
--        nriCore.NRISourceDirectory .. "\\_deps\\nvapi-src\\nvHLSLExtns.h",
--        nriCore.NRISourceDirectory .. "\\_deps\\nvapi-src\\nvShaderExtnEnums.h",
--        nriCore.NRISourceDirectory .. "\\_deps\\amdags-src\\ags_lib\\hlsl\\ags_shader_intrinsics_dx11.hlsl",
--        nriCore.NRISourceDirectory .. "\\_deps\\amdags-src\\ags_lib\\hlsl\\ags_shader_intrinsics_dx12.hlsl",
--        nriCore.NRISourceDirectory .. "\\_deps\\amdags-src\\ags_lib\\inc\\amd_ags.h",
--        nriCore.NRISourceDirectory .. "\\_deps\\d3d12ma-src\\include\\D3D12MemAlloc.h",
--    }
--
--    includedirs
--    {
--        nriCore.NRISourceDirectory .. "\\Include\\NRI\\",
--        nriCore.NRISourceDirectory .. "\\Source\\Shared\\",
--        nriCore.NRISourceDirectory .. "\\_deps\\nvapi-src\\",
--        nriCore.NRISourceDirectory .. "\\_deps\\amdags-src\\ags_lib\\inc\\",
--        nriCore.NRISourceDirectory .. "\\_deps\\d3d12ma-src\\include\\",
--        nriCore.NRISourceDirectory .. "\\_deps\\d3d12ma-src\\src\\",
--        nriCore.NRISourceDirectory .. "\\_deps\\agilitysdk-src\\build\\native\\include",
--    }
--end

function d.Link()
    -- Use "DependencyInjector.Link("NRI")" to include all NRI projects.
end

function d.Include()
    -- Use "DependencyInjector.Link("NRI")" to include all NRI projects.
end

return d;
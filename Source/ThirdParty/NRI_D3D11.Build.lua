-- NRI D3D11 Project Configuration.
-- Use "DependencyInjector.Link("NRI")" to include all NRI projects.
local nriCore = require("NRI");
if (nriCore == nil) then
    return nil;
end

local d = {}
d.Name = "NRI_D3D11";
d.Group = "ThirdParty/NRI"

-- [TODO]: There is a bunch of work to be done to ensure that D3D works. For now I am going to remove the
-- project. 

--function d.ConfigureProject(dependencyInjector)
--    nriCore._SetNRIProjectDefaults();
--
--    dependencyInjector.Link("NRI_Shared");
--
--    files
--    {
--        nriCore.NRISourceDirectory .. "\\Source\\D3D11\\**.*",
--        nriCore.NRISourceDirectory .. "\\_deps\\nvapi-src\\nvapi.h",
--        nriCore.NRISourceDirectory .. "\\_deps\\nvapi-src\\nvHLSLExtns.h",
--        nriCore.NRISourceDirectory .. "\\_deps\\nvapi-src\\nvShaderExtnEnums.h",
--        nriCore.NRISourceDirectory .. "\\_deps\\amdags-src\\ags_lib\\hlsl\\ags_shader_intrinsics_dx11.hlsl",
--        nriCore.NRISourceDirectory .. "\\_deps\\amdags-src\\ags_lib\\hlsl\\ags_shader_intrinsics_dx12.hlsl",
--        nriCore.NRISourceDirectory .. "\\_deps\\amdags-src\\ags_lib\\inc\\amd_ags.h",
--    }
--
--    includedirs
--    {
--        nriCore.NRISourceDirectory .. "\\Include\\NRI\\",
--        nriCore.NRISourceDirectory .. "\\Source\\Shared\\",
--        nriCore.NRISourceDirectory .. "\\_deps\\nvapi-src\\",
--        nriCore.NRISourceDirectory .. "\\_deps\\amdags-src\\ags_lib\\inc\\",
--    }
--end

function d.Link()
    -- Use "DependencyInjector.Link("NRI")" to include all NRI projects.
end

function d.Include()
    -- Use "DependencyInjector.Link("NRI")" to include all NRI projects.
end

return d;
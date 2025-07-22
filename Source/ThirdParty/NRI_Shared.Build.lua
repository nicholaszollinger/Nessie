-- NRI Shared Project Configuration.
-- Use "DependencyInjector.Link("NRI")" to include all NRI projects.
local dependencyInjector = require("DependencyInjector");
local nriCore = require("NRI");
if (nriCore == nil) then
    return nil;
end

local d = {}
d.Name = "NRI_Shared";
d.Group = "ThirdParty/NRI"

function d.ConfigureProject(dependencyInjector)
    nriCore._SetNRIProjectDefaults();

    dependson
    {
        "NRI_Shaders",
	}

	dependencyInjector.Link("ShaderMakeBlob");

    files
    {
        nriCore.NRISourceDirectory .. "\\Source\\Shared\\**.*"
    }

    includedirs
    {
        nriCore.NRISourceDirectory .. "\\Include\\NRI\\",
        nriCore.NRISourceDirectory .. "\\Source\\Shared\\",
        nriCore.NRISourceDirectory .. "\\_deps\\vulkan_headers-src\\include\\",
        --nriCore.NRISourceDirectory .. "\\_deps\\agilitysdk-src\\build\\native\\include\\",
        nriCore.NRISourceDirectory .. "\\_Shaders\\",
        nriCore.NRISourceDirectory .. "\\_deps\\shadermake-src\\",
    }

    vpaths { ["Source/*"] = { d.BuildDirectory .. "/NRI/Source/Shared/**.*" } }
end

function d.Link()
    -- Use "DependencyInjector.Link("NRI")" to include all NRI projects.
end

function d.Include()
    -- Use "DependencyInjector.Link("NRI")" to include all NRI projects.
end

return d;
-- NRI Shaders Project Configuration.
-- Use "DependencyInjector.Link("NRI")" to include all NRI projects.
local nriCore = require("NRI");
if (nriCore == nil) then
    return nil;
end

local d = {}
d.Name = "NRI_Shaders";
d.Group = "ThirdParty/NRI"

function d.ConfigureProject(dependencyInjector)
    nriCore._SetNRIProjectDefaults();
    kind "Utility"

    dependencyInjector.Link("ShaderMakeBlob");

    files
    {
        nriCore.NRISourceDirectory .. "\\Shaders\\**.*",
    }

    vpaths { ["Source/*"] = { d.BuildDirectory .. "/NRI/Shaders/**.*" } }
end

function d.Link()
    -- Use "DependencyInjector.Link("NRI")" to include all NRI projects.
end

function d.Include()
    -- Use "DependencyInjector.Link("NRI")" to include all NRI projects.
end

return d;
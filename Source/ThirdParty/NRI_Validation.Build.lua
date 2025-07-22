-- NRI Validation Project Configuration.
-- Use "DependencyInjector.Link("NRI")" to include all NRI projects.
local nriCore = require("NRI");
if (nriCore == nil) then
    return nil;
end

local d = {}
d.Name = "NRI_Validation";
d.Group = "ThirdParty/NRI"

function d.ConfigureProject(dependencyInjector)
    nriCore._SetNRIProjectDefaults();

    dependencyInjector.Link("NRI_Shared");

    files
    {
        nriCore.NRISourceDirectory .. "\\Source\\Validation\\**.*",
    }

    includedirs
    {
        nriCore.NRISourceDirectory .. "\\Include\\NRI\\",
        nriCore.NRISourceDirectory .. "\\Source\\Shared\\",
    }

    vpaths { ["Source/*"] = { d.BuildDirectory .. "/NRI/Source/Validation/**.*" } }
end

function d.Link()
    -- Use "DependencyInjector.Link("NRI")" to include all NRI projects.
end

function d.Include()
    -- Use "DependencyInjector.Link("NRI")" to include all NRI projects.
end

return d;
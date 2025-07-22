-- NRI None Project Configuration.
-- Use "DependencyInjector.Link("NRI")" to include all NRI projects.
local dependencyInjector = require("DependencyInjector");
local nriCore = require("NRI");
if (nriCore == nil) then
    return nil;
end

local d = {}
d.Name = "NRI_None";
d.Group = "ThirdParty/NRI"

function d.ConfigureProject(dependencyInjector)
    nriCore._SetNRIProjectDefaults();

    files
    {
        nriCore.NRISourceDirectory .. "\\Source\\NONE\\**.*"
    }

    includedirs
    {
        nriCore.NRISourceDirectory .. "\\Include\\NRI\\",
        nriCore.NRISourceDirectory .. "\\Source\\Shared\\",
    }

    vpaths { ["Source/*"] = { d.BuildDirectory .. "/NRI/Source/NONE/**.*"} }
end

function d.Link()
    -- Use "DependencyInjector.Link("NRI")" to include all NRI projects.
end

function d.Include()
    -- Use "DependencyInjector.Link("NRI")" to include all NRI projects.
end

return d;
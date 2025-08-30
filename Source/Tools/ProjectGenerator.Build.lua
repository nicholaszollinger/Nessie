-- Project Generator Configuration.
-- Premake Documentation: https://premake.github.io/docs/

local projectCore = require("ProjectCore");

local p = {};
p.Name = "ProjectGenerator";

function p.ConfigureProject(dependencyInjector)
    local projectDir = p.ProjectDir;

    projectCore.SetProjectDefaults();
    kind "Makefile"
    buildenabled "Off"

    files 
    {
        projectDir .. "premake5.lua",
        projectDir .. "Modules/**.lua"
    }

    vpaths 
    {
        ["Source/*"] = { p.BuildDirectory .. "/ProjectGenerator/**.*"}
    }

    local premakeDir = projectDir .. "Premake/";
    -- The final Solution Dir argument at the end does not contain an ending quote because when reading the _ARGS[1], it was inserting a quote into the path.
    -- The quotes are added because of folder paths that contain spaces.
    buildcommands { 'call \"' .. premakeDir .. 'premake5.exe\" --file="' .. projectDir .. 'premake5.lua" --cleanTempFiles=true %{_ACTION} ' .. "\"" .. projectCore.SolutionDir};

end

return p;
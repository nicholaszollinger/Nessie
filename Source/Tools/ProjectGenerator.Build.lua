-- Project Generator Configuration.
-- Premake Documentation: https://premake.github.io/docs/

local projectCore = require("ProjectCore");

local p = {};
p.Name = "ProjectGenerator";

function p.ConfigureProject(projectDir, dependencyInjector)
    projectCore.SetProjectDefaults();
    kind "Makefile"
    buildenabled "Off"

    local premakeDir = projectDir .. "Premake/"
    local absoluteSolutionDir = path.getabsolute(projectCore.SolutionDir) .. "/";    
    buildcommands { 'call ' .. premakeDir .. 'premake5.exe --file="' .. projectDir .. 'premake5.lua" --cleanTempFiles=true %{_ACTION} ' .. absoluteSolutionDir}

end

return p;
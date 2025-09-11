-- PBR Sample Project Configuration.
-- Premake Documentation: https://premake.github.io/docs/

local projectCore = require("ProjectCore");

local p = {};
p.Name = "PBR";

function p.ConfigureProject(dependencyInjector)
    local projectDir = p.BuildDirectory .. "/PBR/";

    projectCore.SetProjectDefaults();
    kind "ConsoleApp"
    
    dependencyInjector.Link("Nessie");
    dependencyInjector.Link("Assimp");
    dependencyInjector.Link("Vulkan");

    filter {}

    disablewarnings
    {
        "4324", -- "'X' : structure was padded due to alignment specifier"
    }

    files
    {
        projectDir .. "**.h",
        projectDir .. "**.hpp",
        projectDir .. "**.cpp",
        projectDir .. "**.ixx",
        projectDir .. "**.inl",
    }

    vpaths
    {
        ["Source/*"] = { p.BuildDirectory .. "/PBR/**.*"}
    }
end

return p;
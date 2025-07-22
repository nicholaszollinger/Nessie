-- PhysicsTests Project Configuration.
-- Premake Documentation: https://premake.github.io/docs/

local projectCore = require("ProjectCore");

local p = {};
p.Name = "PhysicsTests";

function p.ConfigureProject(dependencyInjector)
    local projectDir = p.ProjectDir;

    projectCore.SetProjectDefaults();
    kind "ConsoleApp"
    
    dependencyInjector.Link("Nessie");
    dependencyInjector.Link("Assimp");
    dependencyInjector.Include("NRI");

    -- Link RenderAPI libraries if needed.
    -- local renderAPI = projectCore.ProjectSettings["RenderAPI"];
    -- if (renderAPI == "Vulkan") then
    --     dependencyInjector.Link("Vulkan");
    -- end

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
        ["Source/*"] = { p.BuildDirectory .. "/PhysicsTests/**.*"}
    }
end

return p;
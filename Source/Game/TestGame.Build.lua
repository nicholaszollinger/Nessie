-- TestGame Project Configuration.
-- Premake Documentation: https://premake.github.io/docs/

local projectCore = require("ProjectCore");

local p = {};
p.Name = "TestGame";

function p.ConfigureProject(projectDir, dependencyInjector)
    projectCore.SetProjectDefaults();
    kind "ConsoleApp"

    dependencyInjector.Include("BleachLeakDetector");
    dependencyInjector.Include("imgui");
    dependencyInjector.Include("yaml_cpp");
    dependencyInjector.Link("Nessie");
    dependencyInjector.Link("Assimp");

    -- Link RenderAPI libraries if needed.
    local renderAPI = projectCore.ProjectSettings["RenderAPI"];
    if (renderAPI == "Vulkan") then
        dependencyInjector.Link("Vulkan");
    end

    defines { "YAML_CPP_STATIC_DEFINE" }

    filter {}
    
    files
    {
		projectCore.SolutionDir .. "\\Config\\**.*",
        projectCore.SolutionDir .. "\\*" .. projectCore.ProjectFileExtension;
        projectDir .. "**.h",
        projectDir .. "**.cpp",
        projectDir .. "**.ixx",
    }
end

return p;
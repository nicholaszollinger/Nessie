-- Nessie Project Configuration.
-- Premake Documentation: https://premake.github.io/docs/

local projectCore = require ("ProjectCore");

local p = {};
p.Name = "Nessie";

function p.ConfigureProject(projectDir, dependencyInjector)
    projectCore.SetProjectDefaults();

    kind "StaticLib"
    language "C++"
    cppdialect "C++20"

    includedirs
    {
		projectDir
	}
   
    defines 
    {
        "YAML_CPP_STATIC_DEFINE"
		, "NES_LOG_DIR=R\"($(SolutionDir)Saved\\Logs\\)\""        
    }

    filter "system:windows"
        dependencyInjector.Link("glfw");

    filter {}
    
    files
    {
        projectDir .. "**.h",
        projectDir .. "**.cpp",
        projectDir .. "**.ixx",
    }

    dependencyInjector.Link("yaml_cpp");
    dependencyInjector.AddFilesToProject("entt");
    dependencyInjector.AddFilesToProject("BleachLeakDetector");

	prebuildcommands { "{MKDIR} %[" .. projectCore.SolutionDir .. "Saved/]"}
end

function p.Link(projectDir)
    links { "Nessie" }
    libdirs { projectCore.DefaultOutDir }
end

function p.Include(projectDir)
    includedirs { projectDir }
end

return p;
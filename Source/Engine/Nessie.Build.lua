-- Nessie Project Configuration.
-- Premake Documentation: https://premake.github.io/docs/

local projectCore = require ("ProjectCore");
local dependencyInjector = require ("DependencyInjector");

local p = {};
p.Name = "Nessie";

function p.ConfigureProject(dependencyInjector)
    projectCore.SetProjectDefaults();

	targetdir(projectCore.DefaultLibOutDir);
    kind "StaticLib"
    language "C++"
    cppdialect "C++20"
    
    includedirs
    {
        p.BuildDirectory;
	}

    defines 
    {
        "YAML_CPP_STATIC_DEFINE"
		, "NES_LOG_DIR=R\"($(SolutionDir)Saved\\Logs\\)\""
        , "NES_CONTENT_DIR=R\"($(SolutionDir)Content\\)\""
		, "NES_SHADER_DIR=R\"($(SolutionDir)Shaders\\)\""
    }

    disablewarnings
    {
        "4324", -- "'X' : structure was padded due to alignment specifier"
    }

    files
    {
        p.ProjectDir .. "**.h",
        p.ProjectDir .. "**.hpp",
        p.ProjectDir .. "**.cpp",
        p.ProjectDir .. "**.ixx",
        p.ProjectDir .. "**.inl",
        p.ProjectDir .. "**.natvis",
    }
    vpaths { ["Source/*"] = { p.BuildDirectory .. "/Nessie/**.*"} }

    -- I have to convert the solution dir's path parentheses for the vpaths to work...
    local absoluteSolutionDir = path.getabsolute(projectCore.SolutionDir);

    -- Add the Shader files:
    files 
    { 
        projectCore.SolutionDir .. "Shaders\\**.glsl",
        projectCore.SolutionDir .. "Shaders\\**.hlsl",
        projectCore.SolutionDir .. "Shaders\\**.slang",
    }
    vpaths { ["Shaders/*"] = { absoluteSolutionDir .. "/Shaders/**.glsl", absoluteSolutionDir .. "/Shaders/**.slang"} }

    -- Add ThirdParty files and link
    dependencyInjector.AddFilesToProject("imgui");
    dependencyInjector.AddFilesToProject("stb");
    dependencyInjector.AddFilesToProject("fmt");
    dependencyInjector.AddFilesToProject("Vulkan");
    dependencyInjector.Include("Assimp");
    dependencyInjector.Link("glfw");
    dependencyInjector.Link("yaml_cpp");
    vpaths { ["ThirdParty/*"] = { path.getabsolute(projectCore.ThirdPartyDir) .. "/**.*" } }

    prebuildcommands { "{MKDIR} %[" .. projectCore.DefaultOutDir .. "]"}
	prebuildcommands { "{MKDIR} %[" .. projectCore.SolutionDir .. "Saved/]"}
    
    filter{}
end

function p.Link()
    links { "Nessie" }
    libdirs { projectCore.DefaultLibOutDirPath .. "Nessie/" }
end

function p.SetIncludeThirdPartyDirs()
    -- Include directories for third party files.
    dependencyInjector.Include("imgui");
    dependencyInjector.Include("yaml_cpp");
    dependencyInjector.Include("fmt");
    dependencyInjector.Include("glfw");
    dependencyInjector.Include("Vulkan");
end

function p.Include()
    includedirs { p.BuildDirectory }
    p.SetIncludeThirdPartyDirs();
end

return p;
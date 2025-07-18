-- Nessie Project Configuration.
-- Premake Documentation: https://premake.github.io/docs/

local projectCore = require ("ProjectCore");
local dependencyInjector = require ("DependencyInjector");

local p = {};
p.Name = "Nessie";

function p.ConfigureProject(projectDir, dependencyInjector)
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

    -- Platform specific project set up.
    p.InitializePlatform(projectDir, dependencyInjector);

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
        projectDir .. "**.natvis",

        -- Add the Shader files:
        projectCore.SolutionDir .. "Shaders\\**.glsl",
    }

    dependencyInjector.AddFilesToProject("imgui");
    dependencyInjector.AddFilesToProject("stb");
    dependencyInjector.AddFilesToProject("fmt");
    dependencyInjector.Link("yaml_cpp");
    dependencyInjector.Include("Assimp");
    
    prebuildcommands { "{MKDIR} %[" .. projectCore.DefaultOutDir .. "]"}
	prebuildcommands { "{MKDIR} %[" .. projectCore.SolutionDir .. "Saved/]"}
    
    filter {"configurations:Debug"}
        -- Copy YAML PDB
        postbuildcommands { "{COPYFILE} \"" .. projectCore.ThirdPartyDir .. "yaml_cpp\\lib\\Debug\\yaml-cppd.pdb\" \"" .. projectCore.DefaultLibOutDirPath .. "Nessie\\\""};
    
    filter{}
end

-----------------------------------------------------------------------------------------
-- Platform specific project set up.
---@param projectDir string Path to the project's build 8rector.
---@param dependencyInjector table Dependency Injector module.
---@return boolean Success If false, then we have no valid Render API set.
-----------------------------------------------------------------------------------------
function p.InitializePlatform(projectDir, dependencyInjector)
    if (_TARGET_OS == "windows") then
        defines
        {
            "_RENDER_API_VULKAN"
        }

        dependencyInjector.Link("glfw");
        dependencyInjector.Include("Vulkan");
        return true;
    end

    projectCore.PrintError("Unsupported Platform!");
    return false;
end

function p.Link(projectDir)
    links { "Nessie" }
    libdirs { projectCore.DefaultLibOutDirPath .. "Nessie/" }
end

function p.SetIncludeThirdPartyDirs()
    -- Include directories for third party files.
    dependencyInjector.Include("imgui");
    dependencyInjector.Include("yaml_cpp");
    defines { "YAML_CPP_STATIC_DEFINE" }

    dependencyInjector.Include("fmt");

    -- Platform specific options.
    p.InitializePlatform(p.BuildDirectory, dependencyInjector);
end

function p.Include(projectDir)
    includedirs { p.BuildDirectory }
    p.SetIncludeThirdPartyDirs();
end

return p;
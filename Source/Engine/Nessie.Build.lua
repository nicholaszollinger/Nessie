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

    -- Platform specific project set up.
    p.InitializePlatform(dependencyInjector);

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
    files { projectCore.SolutionDir .. "Shaders\\**.glsl", }
    vpaths { ["Shaders/*"] = { absoluteSolutionDir .. "/Shaders/**.glsl"} }

    -- Add ThirdParty files and link
    dependencyInjector.AddFilesToProject("imgui");
    dependencyInjector.AddFilesToProject("stb");
    dependencyInjector.AddFilesToProject("fmt");
    dependencyInjector.Link("yaml_cpp");
    dependencyInjector.Include("Assimp");
    vpaths { ["ThirdParty/*"] = { path.getabsolute(projectCore.ThirdPartyDir) .. "/**.*" } }

    
    prebuildcommands { "{MKDIR} %[" .. projectCore.DefaultOutDir .. "]"}
	prebuildcommands { "{MKDIR} %[" .. projectCore.SolutionDir .. "Saved/]"}
    
    filter {"configurations:Debug"}
        -- Copy PDB files into output library folder.
        postbuildcommands { "{COPYFILE} \"" .. projectCore.ThirdPartyDir .. "yaml_cpp\\lib\\Debug\\yaml-cppd.pdb\" \"" .. projectCore.DefaultLibOutDirPath .. "Nessie\\\""};
        --postbuildcommands { "{COPYFILE} \"" .. projectCore.DefaultLibOutDirPath .. "NRI\\NRI.pdb\" \"" .. projectCore.DefaultLibOutDirPath .. "Nessie\\\""};
    
    filter{}
end

-----------------------------------------------------------------------------------------
-- Platform specific project set up.
---@param dependencyInjector table Dependency Injector module.
---@return boolean Success If false, then we have no valid Render API set.
-----------------------------------------------------------------------------------------
function p.InitializePlatform(dependencyInjector)
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

function p.Link()
    links { "Nessie" }
    libdirs { projectCore.DefaultLibOutDirPath .. "Nessie/" }
end

function p.SetIncludeThirdPartyDirs()
    -- Include directories for third party files.
    dependencyInjector.Include("imgui");
    dependencyInjector.Include("yaml_cpp");
    defines { "YAML_CPP_STATIC_DEFINE" }

    dependencyInjector.Include("fmt");
    --dependencyInjector.Include("glfw");
    --dependencyInjector.Include("NRI");

    -- Platform specific options.
    p.InitializePlatform(dependencyInjector);
end

function p.Include()
    includedirs { p.BuildDirectory }
    p.SetIncludeThirdPartyDirs();
end

return p;
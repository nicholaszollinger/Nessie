-- Nessie Project Configuration.
-- Premake Documentation: https://premake.github.io/docs/

local projectCore = require ("ProjectCore");

local libFolder = projectCore.ProjectIntermediateLibsLocation .. "Nessie\\";

local p = {};
p.Name = "Nessie";

function p.ConfigureProject(projectDir, dependencyInjector)
    projectCore.SetProjectDefaults();

    targetdir(libFolder)
    kind "StaticLib"
    language "C++"
    cppdialect "C++20"

    filter {}
    
    includedirs
    {
		projectDir
	}

    defines 
    {
        "YAML_CPP_STATIC_DEFINE"
		, "NES_LOG_DIR=R\"($(SolutionDir)Saved\\Logs\\)\""
        , "NES_CONTENT_DIR=R\"($(SolutionDir)Content\\)\""
		, "NES_SHADER_DIR=R\"($(SolutionDir)Shaders\\)\""
    }

    -- Set the Render API based on the Project Settings:
    p.SetRenderAPI(projectDir, dependencyInjector);

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
end

-----------------------------------------------------------------------------------------
-- Load the RenderAPI based on the Project.json settings file.
---@param projectDir string Path to the project's Directory.
---@param dependencyInjector table Dependency Injector module.
---@return boolean Success If false, then we have no valid Render API set.
-----------------------------------------------------------------------------------------
function p.SetRenderAPI(projectDir, dependencyInjector)
    local renderAPI = projectCore.ProjectSettings["RenderAPI"];
    
    if (renderAPI == nil) then
        projectCore.PrintError("Failed to load RenderAPI! No RenderAPI value set in the Project File!");
        return false;
    end

    -- Ensure that the RenderAPI is valid on the Platform.
    if (renderAPI == "SDL") then
        -- Only available on Windows for now.
        if (_TARGET_OS == "windows") then
            defines
            {
                "_RENDER_API_SDL"
            }
            dependencyInjector.Link("SDL");
            return true;
        end
    elseif (renderAPI == "Vulkan") then
        -- Only available on Windows for now.
        if (_TARGET_OS == "windows") then
            defines
            {
                "_RENDER_API_VULKAN"
            }

            dependencyInjector.Link("glfw");
            dependencyInjector.Include("Vulkan");
            return true;
        end
    end

    projectCore.PrintError("Failed to set RenderAPI! Se1ected RenderAPI, " .. renderAPI .. ", is not valid for this platform!");
    return false;
end

function p.Link(projectDir)
    links { "Nessie" }
    libdirs { libFolder }

    local renderAPI = projectCore.ProjectSettings["RenderAPI"];
    if (renderAPI == "Vulkan") then
        defines { "_RENDER_API_VULKAN" }
    elseif (renderAPI == "SDL") then
        defines { "_RENDER_API_SDL" }
    end
end

function p.Include(projectDir)
    includedirs { projectDir }

end

return p;
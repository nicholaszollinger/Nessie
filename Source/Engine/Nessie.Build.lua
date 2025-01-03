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
    
    -- Set the Render API based on the Project Settings:
    p.SetRenderAPI(projectDir, dependencyInjector);

    filter {}
    
    files
    {
        projectDir .. "**.h",
        projectDir .. "**.cpp",
        projectDir .. "**.ixx",
    }

    dependencyInjector.Link("yaml_cpp");
    dependencyInjector.AddFilesToProject("imgui");
    dependencyInjector.AddFilesToProject("entt");
    dependencyInjector.AddFilesToProject("BleachLeakDetector");

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
        -- Only available on Windows for now. I have only made sure
        -- that it works on Windows.
        if (_TARGET_OS == "windows") then
            defines
            {
                "_RENDER_API_SDL"
            }
            dependencyInjector.Link("SDL");
            return true;
        end
    end

    projectCore.PrintError("Failed to set RenderAPI! Se1ected RenderAPI, " .. renderAPI .. ", is not valid for this platform!");
    return false;
end

function p.Link(projectDir)
    links { "Nessie" }
    libdirs { projectCore.DefaultOutDir }
end

function p.Include(projectDir)
    includedirs { projectDir }
end

return p;
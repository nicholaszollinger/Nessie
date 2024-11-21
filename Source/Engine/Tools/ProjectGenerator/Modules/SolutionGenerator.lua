--SolutionGenerator.lua

local utility = require("Utility");
local ps = require("ProjectSettings");

premake.modules.SolutionGenerator = {};
local m = premake.modules.SolutionGenerator;

-- Setup the module with base functionality.
utility.SetupModule(m, "SolutionGenerator", true);

m.DefaultOutDir = "$(SolutionDir)Build/$(Configuration)_$(PlatformTarget)/";
m.DefaultIntermediateDir = "$(SolutionDir)Intermediate/$(ProjectName)/$(Configuration)_$(PlatformTarget)/"

---------------------------------------------------------------------------------------------------------
--- Generate the Solution files for the project.
---@return boolean Success True if the solution was properly generated.
---------------------------------------------------------------------------------------------------------
function m.GenerateSolution()
    if (ps == nil or ps.IsValid == false) then
        m.PrintError("Failed to generate Solution! ProjectSettings were invalid!");
        return false;
    end

    -- Create the new Solution
    m.PrintInfo("Creating Solution...")
    m.CreateSolution();
    m.AddPremakeProject();

    -- Add Projects
    if m.AddProjects() == false then
        return false;
    end

    m.PrintSuccessOrFail("Solution Generation", true);
    return true;
end

---------------------------------------------------------------------------------------------------------
--- Create the new Visual Studio Solution.
---------------------------------------------------------------------------------------------------------
function m.CreateSolution()
    workspace (ps.ProjectSettings["ProjectName"])
        configurations { "Debug", "Test", "Release" }
        location(ps.SolutionDir)
        platforms {"x64"}
        startproject(ps.ProjectSettings["StartupProject"]);
        staticruntime "Off"
        flags { "MultiProcessorCompile" }

        -- System SDK version.
        filter "system:windows"
            systemversion "latest"
        
        -- Solution-wide Debug config settings
        filter "configurations:Debug"
            defines { "DEBUG", "NES_DEBUG" }
            symbols "On"

        -- Optimized, but with Logs
        filter "configurations:Test"
            defines { "NDEBUG", "NES_TEST" }
            optimize "On"

        filter "configurations:Release"
            defines { "NDEBUG" , "NES_RELEASE" }
            optimize "On"

        filter "platforms:x64"
            system "Windows"
            architecture "x64"

        -- Reset the filter.
        filter {}

        defines
        {
            "YAML_CPP_STATIC_DEFINE"
        }
end

---------------------------------------------------------------------------------------------------------
--- Add a Premake Project to the Solution that can regenerate the projects directly from VS.
---------------------------------------------------------------------------------------------------------
function m.AddPremakeProject()
    local premakeProjectDir = ps.SolutionDir .. "Source/Engine/Tools/ProjectGenerator/"
    group("_Premake")
        project ("Regenerate")
            location(premakeProjectDir)
            targetdir(m.DefaultOutDir)
            objdir(m.DefaultIntermediateDir)
            kind "Makefile"
            buildcommands { 'call Premake\\premake5.exe --file="Actions\\RegenerateProjects.lua" %{_ACTION} %{table.concat(_ARGS, " ")}'  }
            files { premakeProjectDir .. "Actions/RegenerateProjects.lua" }
end

---------------------------------------------------------------------------------------------------------
---Iterate through the Projects array and attempt to run the premake5.lua file in the ProjectDir.
---comment
---@param guidTable table|nil Optional table of guids in the event that we are regenerating the projects.
---@return boolean Success False if there was an error.
---------------------------------------------------------------------------------------------------------
function m.AddProjects(guidTable)
    local projectsArray = ps.ProjectSettings["Projects"];

    for i = 1, #projectsArray do
        local projectName = projectsArray[i].Name;
        local projectDir = ps.SolutionDir .. projectsArray[i].ProjectDir;
        
        m.PrintInfo("Loading " .. projectName);

        local buildScript = projectDir .. projectName .. ".Project.lua";
        local guid = nil;

        if guidTable ~= nil then
            assert(type(guidTable) == "table");
            guid = guidTable[projectName];
        end

        -- Ensure the directory exists.
        if (os.isdir(projectDir) == false) then
            os.mkdir(projectDir);
            -- [TODO] Generate the "ProjectName.Project.lua" file?
                
        elseif (os.isfile(buildScript) == false) then
            m.PrintError("Failed to find '" .. projectName .. "' premake file! Skipping...");
            -- [TODO] Generate the "ProjectName.Project.lua" file?

        else
            m.PrintMessage("Running project script...");
            local projectData = dofile(buildScript);

            local projectFilepath = projectDir .. projectName .. ".vcxproj";
            if (os.isfile(projectFilepath) == true) then
                m.AddExistingProject(projectData, projectDir, projectFilepath);
            else
                m.CreateNewProject(projectData, projectDir, guid);
                filter {}

                -- Add the Build Script itself to the project, for ease of use.
                files { buildScript }
            end
        end
    end


    return true;
end

---------------------------------------------------------------------------------------------------------
---Add an existing project located in the Source folder.
---@param projectData table Table of Data about the Project.
---@param projectDir string Name of the Project.
---@param projectFilepath string Path to to the Project File (.vcxproj)
---------------------------------------------------------------------------------------------------------
function m.AddExistingProject(projectData, projectDir, projectFilepath)
    filter {}
    
    group(projectData.Group)
        externalproject(projectData.Name)
        location(projectDir)
        uuid(m.GetProjectUUID(projectFilepath))
        kind(projectData.TargetType)
        language(projectData.Language)

        -- It would be nice if loading an existing project would 
        -- report mismatches between the projectData and the actual
        -- settings in the project.
end

---------------------------------------------------------------------------------------------------------
---Create a new project in the Source Folder.
---@param projectData table Table of information about the Project.
---@param projectDir string Name of the Project.
---@param guid string|nil Optional guid value to set for the project.
---------------------------------------------------------------------------------------------------------
function m.CreateNewProject(projectData, projectDir, guid)
    filter {}
    group(projectData.Group)
        project(projectData.Name)
            location(projectDir)
            if (guid ~= nil) then
                uuid(guid)
            end

            kind(projectData.TargetType)
            language(projectData.Language)
            if (projectData.Language == "C++") then
                cppdialect(projectData.CppDialect)
            end
            -- Reset the filter.
            filter{}

            projectData.ConfigureProject(projectDir);
end

---------------------------------------------------------------------------------------------------------
--- Get the Project UUID from the .vcxproj file.
---@param projectFile string : Path to the project file.
---@return any UUID : UUID value for the project, or nil if nothing was found.
---------------------------------------------------------------------------------------------------------
function m.GetProjectUUID(projectFile)
    -- Read the vcxproj file.
    local fileData = utility.ReadFile(projectFile);

    -- Get the contents of the ProjectGuid tag.
    local uuid = string.match(fileData, "^.+<ProjectGuid>{(.+)}</ProjectGuid>.+$")
    if uuid == nil then
        m.PrintError("Failed to find UUID for Project File: " .. projectFile)
        return nil;
    end

    return uuid;
end

-- Return the Module.
return m;
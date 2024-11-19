--SolutionGenerator.lua

local utility = require("Utility");
--local luaYAML, _ = require("LuaYaml");

premake.modules.SolutionGenerator = {};
local m = premake.modules.SolutionGenerator;

-- Setup the module with base functionality.
utility.SetupModule(m, "SolutionGenerator", true);

m.DefaultOutDir = "$(SolutionDir)Build/$(Configuration)_$(PlatformTarget)/";
m.DefaultIntermediateDir = "$(SolutionDir)Intermediate/$(ProjectName)/$(Configuration)_$(PlatformTarget)/"

---------------------------------------------------------------------------------------------------------
--- Generate the Solution files for the project.
---@param solutionDirectory string Absolute directory path to create the solution in.
---@return boolean Success True if the solution was properly generated.
---------------------------------------------------------------------------------------------------------
function m.GenerateSolution(solutionDirectory)
    -- Load the ProjectSettings.ini:
    if m.LoadProjectSettings(solutionDirectory) == false then
        return false;
    end

    -- Cleanup old files.
    if m.CleanSolutionFolder() == false then
        return false;
    end

    -- Create the new Solution
    m.CreateSolution();

    -- Add Projects
    if m.AddProjects() == false then
        return false;
    end

    return true;
end

---------------------------------------------------------------------------------------------------------
--- Cleanup the .vs, Build, Intermediate, & Saved folders, and delete the old .sln file.
---------------------------------------------------------------------------------------------------------
function m.CleanSolutionFolder()
    m.PrintInfo("Cleaning up old solution files...");

    local success = m.DeleteFolderIfExists(m.solutionDir .. ".vs/", ".vs");
    if success == false then
        return false;
    end

    success = m.DeleteFolderIfExists(m.solutionDir .. "Build/", "Build");
    if success == false then
        return false;
    end

    success = m.DeleteFolderIfExists(m.solutionDir .. "Intermediate/", "Intermediate");
    if success == false then
        return false;
    end

    success = m.DeleteFolderIfExists(m.solutionDir .. "Saved/", "Saved");
    if success == false then
        return false;
    end

    -- Delete the old .sln.
    local existingSlnFilepath = m.solutionDir .. m.projectSettings["ProjectName"] .. ".sln";
    if os.locate(existingSlnFilepath) ~= nil then
        m.PrintMessage("Deleting old .sln...");
        local errorMsg;
        success, errorMsg = os.remove(existingSlnFilepath);
        if success == false then
            m.PrintError("Failed to delete old Solution File! Error msg: " .. errorMsg);
        end
    end

    return true;
end

---------------------------------------------------------------------------------------------------------
--- Sets the m.projectName variable.
---@param solutionDirectory string SolutionDir.
---@return boolean Success False if we failed to load the project settings!
---------------------------------------------------------------------------------------------------------
function m.LoadProjectSettings(solutionDirectory)
    -- Get ProjectName field from the ProjectSettings.yaml file.
    m.solutionDir = solutionDirectory;
    m.PrintInfo("Loading ProjectSettings...");
    local projectSettingsPath = m.solutionDir .. "Config\\ProjectSettings.json";
    if os.locate(projectSettingsPath) == nil then
        m.PrintError("Failed to find ProjectSettings file!");
        return false;
    end

    local errorMsg;
    local jsonData = utility.ReadFile(projectSettingsPath);
    m.projectSettings, errorMsg = json.decode(jsonData);
    if (m.projectSettings == nil) then
        m.PrintError("Failed to load ProjectSettings file! Error: " .. errorMsg);
        return false;
    end

    return true;
end

---------------------------------------------------------------------------------------------------------
--- Delete a folder and its contents, if it exists.
---------------------------------------------------------------------------------------------------------
function m.DeleteFolderIfExists(folderPath, name)
    local success = true;
    local errorMsg;
    local dirs = os.matchdirs(folderPath);

    for _, match in pairs(dirs) do
        m.PrintMessage("Deleting ".. name .. " folder...");
        success, errorMsg = os.rmdir(match);
        if success == false then
            m.PrintError("Failed to delete " .. name .. " folder! Error msg: " .. errorMsg);
        end
    end

    return success;
end

---------------------------------------------------------------------------------------------------------
--- Create the new Visual Studio Solution.
---------------------------------------------------------------------------------------------------------
function m.CreateSolution()
    m.PrintInfo("Creating Solution...")
    workspace (m.projectSettings["ProjectName"])
        configurations { "Debug", "Test", "Release" }
        location(m.solutionDir)
        platforms {"x64"}
        startproject(m.projectSettings["ProjectName"]);
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
---Iterate through the Projects array and attempt to run the premake5.lua file in the ProjectDir.
---------------------------------------------------------------------------------------------------------
function m.AddProjects()
    local projectsArray = m.projectSettings["Projects"];

    for i = 1, #projectsArray do
        local projectName = projectsArray[i].Name;
        local projectDir = m.solutionDir .. projectsArray[i].ProjectDir;
        
        m.PrintInfo("Loading " .. projectName);

        -- Ensure the directory exists.
        if (os.isdir(projectDir) == false) then
            os.mkdir(projectDir);
            -- Generate the premake5.lua file...
        
        elseif (os.isfile(projectDir .. "premake5.lua") == false) then
            m.PrintError("Failed to find '" .. projectName .. "' premake file! Skipping...");
            -- Generate the premake5.lua file.

        else
            m.PrintMessage("Running premake5 script...");
            local projectData = include(projectDir);

            local projectFilepath = projectDir .. projectName .. ".vcxproj";
            if (os.isfile(projectFilepath) == true) then
                m.AddExistingProject(projectData, projectDir, projectFilepath);
            else
                m.CreateNewProject(projectData, projectDir);
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
---------------------------------------------------------------------------------------------------------
function m.CreateNewProject(projectData, projectDir)
    filter {}

    group(projectData.Group)
        project(projectData.Name)
            location(projectDir)
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
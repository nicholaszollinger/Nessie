-- Project.lua
-- This module will be able to initialize a Project script with
-- all the necessary information.
local utility = require("Utility");

premake.modules.ProjectIntializer = {};
local m = premake.modules.ProjectIntializer;
utility.SetupModule(m, "ProjectIntializer", true);

-- Table of registered Projects
m.projects = {};
m.DefaultOutDir = "$(SolutionDir)Build/$(Configuration)_$(PlatformTarget)/";
m.DefaultIntermediateDir = "!$(SolutionDir)Intermediate/$(ProjectName)/$(Configuration)_$(PlatformTarget)/"

-----------------------------------------------------------------------------------
---Create a new ProjectData table. This table is used in the build process to properly
---add projects to the VS solution.
---@param name string Name of the Project to initialize.
---@param projectDir string The Project's directory. This should be grabbed from the 
---     the script's location who called this.
---@return table ProjectData Table that contains all the default variables for creating 
---     a project with Premake.
-----------------------------------------------------------------------------------
function m.CreateProjectData(name, projectDir)
    local projectData = {};
    m.PrintMessage("Registering " .. name .. "...");
    projectData.Name = name;
    projectData.Dir = projectDir;
    projectData.UUID = nil;
    projectData.Type = nil;
    projectData.Language = "C++";
    projectData.CppDialect = "C++20";
    projectData.WarningLevel = "Extra";
    projectData.TargetType = "ConsoleApp";
    projectData.Dependencies = {};
    
    -- Add the project to our map.
    m.projects[name] = projectData;

    return projectData;
end


return m;
--SolutionGenerator.lua

local utility = require("Utility");
local core = require("ProjectCore");
local cleaner = require("Cleaner");
local dependencyInjector = require("DependencyInjector");

premake.modules.SolutionGenerator = {};
local m = premake.modules.SolutionGenerator;

-- Setup the module with base functionality.
utility.SetupModule(m, "SolutionGenerator", true);

---------------------------------------------------------------------------------------------------------
--- Generate the Solution files for the project.
---@return boolean Success True if the solution was properly generated.
---------------------------------------------------------------------------------------------------------
function m.GenerateSolution()
    if (core == nil or core.ProjectConfigurations == false) then
        m.PrintError("Failed to generate Solution! ProjectSettings were invalid!");
        return false;
    end

    local cleanOption = utility.GetOptionValueOrDefault("cleanTempFiles");
    if (cleanOption ~= nil) then
        cleaner.CleanTempFiles();
    end

    -- Create the new Solution
    m.PrintInfo("Creating Solution...")
    m.CreateSolution();
    dependencyInjector.AddProjectsToWorkspace();

    m.PrintSuccessOrFail("Solution Generation", true);
    return true;
end

---------------------------------------------------------------------------------------------------------
--- Create the new Visual Studio Solution.
---------------------------------------------------------------------------------------------------------
function m.CreateSolution()
    workspace (core.ProjectSettings["ProjectName"])
        configurations (core.ProjectConfigurations)
        location(core.SolutionDir)
        platforms {"x64"}
        startproject(core.ProjectSettings["StartupProject"]);
        staticruntime "Default"
        flags { "MultiProcessorCompile" }

        filter "platforms:x64"
            system "Windows"
            architecture "x64"

        -- Reset the filter.
        filter {}

        defines
        {
            "YAML_CPP_STATIC_DEFINE"
            , "NES_CONFIG_DIR=R\"($(SolutionDir)Config\\)\""
        }
end

-- Return the Module.
return m;
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
        m.PrintInfo("Cleaning Temp Files...");
        cleaner.CleanTempFiles();
    end

    -- Create the new Solution
    m.PrintInfo("Creating Solution...")
    m.CreateSolution();

    -- Add Registerd projects to the solution:
    if (dependencyInjector.AddProjectsToWorkspace() == false) then
        m.PrintSuccessOrFail("Solution Generation", false);
        return false;
    end

    m.PrintSuccessOrFail("Solution Generation", true);
    return true;
end

---------------------------------------------------------------------------------------------------------
--- Create the new Visual Studio Solution.
---------------------------------------------------------------------------------------------------------
function m.CreateSolution()
    workspace(core.ProjectSettings["ProjectName"])
    configurations(core.ProjectConfigurations)
    location(core.SolutionDir)
    platforms { "x64" }
    startproject(core.ProjectSettings["StartupProject"]);
    staticruntime "Off"
    flags { "MultiProcessorCompile" }

    -- Windows x64
    filter "platforms:x64"
        system "Windows"
        architecture "x64"

    -- Reset the filter.
    filter {}
    
    m.EnableInstructionSets();

    defines
    {
        --"YAML_CPP_STATIC_DEFINE"
        "NES_PROJECT_DIR=R\"($(SolutionDir))\"",
        "NES_SAVED_DIR=R\"($(SolutionDir)Saved\\)\"",
        "NES_CONFIG_DIR=R\"($(SolutionDir)Config\\)\"",
        "NES_CONTENT_DIR=R\"($(SolutionDir)Content\\)\"",
        "NES_SHADER_DIR=R\"($(SolutionDir)Shaders\\)\"",
    }
end

function m.EnableInstructionSets()
    filter "platforms:x64"
        defines
        {
            "WIN32",
            "_WINDOWS",
            "WIN32_LEAN_AND_MEAN",
            "NOMINMAX",

            "NES_USE_SSE",
            "NES_USE_SSE4_1",
            "NES_USE_SSE4_2",

            "NES_USE_AVX",
            "NES_USE_AVX2",
            "NES_USE_FMADD",

            "NES_USE_LZCNT",
            "NES_USE_TZCNT",
        }
    
        vectorextensions "SSE2"; -- Minimum
        vectorextensions "SSE4.1";
        vectorextensions "SSE4.2";
        vectorextensions "AVX";
        vectorextensions "AVX2";
    
    -- Reset the filter.
    filter{}
end

-- Return the Module.
return m;
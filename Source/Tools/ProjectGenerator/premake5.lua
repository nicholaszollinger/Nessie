-- premake5.lua
-- This is the Kick off file for Generating solution files.

-- Get the Solution file from the args.
local solutionDirectory = _ARGS[1];

-- Add a searcher for our Modules and Actions.
package.path = solutionDirectory.. 'Source/Tools/ProjectGenerator/Modules/?.lua;' .. package.path;

-- Require the Actions from Git and Cleaner.
require("Git");
require("Cleaner");

local projectCore = require("ProjectCore");
local dependencyInjector = require("DependencyInjector");

-- Project Core:
if (projectCore.Init() == false) then
    error("Failed to Create Project!");
    return;
end

-- Dependency Injector
if (dependencyInjector.Init() == false) then
    error("Failed to Create Project!");
    return;
end

-- -- Initialize the SolutionGenerator.
local solutionGenerator = require("SolutionGenerator");

-- If this is a build action generate the solution.
if (_ACTION == "vs2022") then
    if (solutionGenerator.GenerateSolution() == false) then
        error("Failed to create Nessie Project!");
    end
end

-- Note: Executes the _ACTION on exit.
-- premake5.lua
-- This is the Kick off file for Generating solution files.

-- Get the Solution file from the args.
local solutionDirectory = _ARGS[1];

-- Add a searcher for our Modules and Actions.
package.path = solutionDirectory.. 'Source/Engine/Tools/ProjectGenerator/Modules/?.lua;' .. solutionDirectory.. 'Source/Engine/Tools/ProjectGenerator/Actions/?.lua;' .. package.path;


-- Initialize the Project Settings.
local ps = require("ProjectSettings");
if (ps.Init(solutionDirectory) == false) then
    error("Failed to load the Project Settings! There must be a ProjectSettings.json file in the Config folder.");
    return;
end

-- Initialize the Cleaner
require("Cleaner");

-- Initialize the SolutionGenerator.
local solutionGenerator = require("SolutionGenerator");

-- If this is a build action generate the solution.
if (_ACTION == "vs2022") then
    if (solutionGenerator.GenerateSolution() == false) then
        error("Failed to create Nessie Engine Project!");
    end
end


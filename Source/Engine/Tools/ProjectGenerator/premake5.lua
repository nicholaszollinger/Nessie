-- GenerateSolution.lua
-- This is the Kick off file for Generating solution files.

local solutionDirectory = _ARGS[1];
local solutionGenerator = include("Modules/SolutionGenerator.lua");
local clean = include("Actions/Clean.lua");

-- If the CleanOnly flag is set to 1, just clean the solution folder.
if (_ACTION == "clean") then
    --_ACTION = nil; -- Clear out the Action, so that premake doesn't try to do something.
    if solutionGenerator.LoadProjectSettings(solutionDirectory) == false then
        error("Failed to load project settings!");
    end

    if (solutionGenerator.CleanSolutionFolder() == false) then
        error("Failed to CleanSolutionFolder!");
    end

-- Otherwise, generate the full solution.
else
    if (solutionGenerator.GenerateSolution(solutionDirectory) == false) then
        error("Failed to create Nessie Engine Project!");
    end
end


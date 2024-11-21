-- RegenerateProjects.lua

-- Add searchers to get the modules/actions we need.
package.path = '../Modules/?.lua;' .. '../Actions/?.lua;' .. package.path;

local utility = require("Utility");
local cleaner = require("Cleaner");
local ps = require("ProjectSettings");
local solutionGenerator = require("SolutionGenerator");

premake.modules.RegenerateProjects = {}
local m = premake.modules.RegenerateProjects
utility.SetupModule(m, "RegenerateProjects", true);

------------------------------------------------------------------------------------------------------
--- Deletes the current projects, regenerates them, then adds them back into the Solution.
---@return boolean Success Returns false in the event of a failure.
------------------------------------------------------------------------------------------------------
function m.RegenerateProjects()
	local solutionDir = path.getabsolute("../" .. _ARGS[1]) .. "/";
	if ps.Init(solutionDir) == false then
		m.PrintError("Failed to Regenerate Projects! Failed to Initialize Project Settings!");
		return false;
	end

    local projectsArray = ps.ProjectSettings["Projects"];

    local guidTable = {};

    for i = 1, #projectsArray do
        local projectName = projectsArray[i].Name;
        local projectDir = ps.SolutionDir .. projectsArray[i].ProjectDir;

        local filepath = projectDir .. projectName .. ".vcxproj";
        if (os.locate(filepath) == true) then
            local guid = solutionGenerator.GetProjectUUID(filepath);
            guidTable[projectName] = guid;
        end
    end

    cleaner.CleanProjects();

    local ok, errMsg = os.chdir(ps.SolutionDir .. ps.ProjectSettings["ProjectName"]);
    if (ok == false) then
        m.PrintError("Failed to Regenerate Projects! ErrorMsg: " .. errMsg);
        return false;
    end

    solutionGenerator.CreateSolution();
    solutionGenerator.AddPremakeProject();
    if (solutionGenerator.AddProjects(guidTable) == false) then
        return false;
    end

    return true;
end

m.RegenerateProjects();
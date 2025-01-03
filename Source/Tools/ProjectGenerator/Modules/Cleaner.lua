-- Clean.lua

local utility = require("Utility");
local core = require("ProjectCore");

premake.modules.Cleaner = {}
local m = premake.modules.Cleaner
utility.SetupModule(m, "Cleaner", true);

newoption
{
    trigger     = "cleanTempFiles",
    description = "True or false value. If set, This will clean the Build, Intermediate, and Saved folders."
}

---------------------------------------------------------------------------------------------------------
--- Deletes the Build, Intermediate, and Saved Folders.
---@return boolean Success False if something went wrong in the removal.
---------------------------------------------------------------------------------------------------------
function m.CleanTempFiles()
    if (core == nil) then
        m.PrintError("Failed to Clean Project! ProjectSettings nil!");
        return false;
    end

    if (core.ProjectSettings == nil) then
        m.PrintError("Failed to Clean Project! ProjectSettings are invalid!");
        return false;
    end

    if utility.DeleteFolderIfExists(core.SolutionDir .. "Build/", "Build") == false then
        return false;
    end

    if utility.DeleteFolderIfExists(core.SolutionDir .. "Intermediate/", "Intermediate") == false then
        return false;
    end

    if utility.DeleteFolderIfExists(core.SolutionDir .. "Saved/", "Saved") == false then
        return false;
    end

    return true;
end

function m.CleanBuildFolder()
    -- Build Folder
    if utility.DeleteFolderIfExists(core.SolutionDir .. "Build/", "Build") == false then
        return false;
    end
end

function m.CleanSavedFolder()
    if utility.DeleteFolderIfExists(core.SolutionDir .. "Saved/", "Saved") == false then
        return false;
    end
end

---------------------------------------------------------------------------------------------------------
--- Deletes the Build, Intermediate, and Saved Folders.
---@return boolean Success False if something went wrong in the removal.
---------------------------------------------------------------------------------------------------------
function m.CleanProjectFiles()
    if (utility.DeleteFolderIfExists(core.ProjectFilesLocation, "ProjectFiles") == false) then
        return false;
    end

    return true;
end

---------------------------------------------------------------------------------------------------------
--- Deletes the Solution and .vs folder.
---@return boolean Success False if something went wrong in the removal.
---------------------------------------------------------------------------------------------------------
function m.CleanSolution()
    -- .vs folder
    if utility.DeleteFolderIfExists(core.SolutionDir .. ".vs/", ".vs") == false then
        return false;
    end

    -- .idea folder (Rider)
    if utility.DeleteFolderIfExists(core.SolutionDir .. ".idea/", ".idea") == false then
        return false;
    end

    -- .sln File.
    if utility.DeleteFileIfExists(core.SolutionDir .. core.ProjectSettings["ProjectName"] .. ".sln") == false then
        return false;
    end

    return true;
end

newaction {
	trigger = "clean_all",
	description = "Deletes the Build, Intermediate Saved, and .vs folders, as well as the Solution and Projects.",

	execute = function()
		if (m.CleanTempFiles() == false) then
            m.PrintSuccessOrFail("Clean Intermediate Files", false);
            return;
        end

        if (m.CleanSolution() == false) then
            m.PrintSuccessOrFail("Clean Solution", false);
            return;
        end

        m.PrintSuccessOrFail("Clean All", true);
	end
}

newaction {
	trigger = "clean_projects",
	description = "Deletes the Build, Intermediate and Saved folders, but does not delete the Solution.",

	execute = function()
        if (m.CleanTempFiles() == false) then
            m.PrintSuccessOrFail("Clean Intermediate Files", false);
            return;
        end

        m.PrintSuccessOrFail("Clean Projects", true);
	end
}

return m
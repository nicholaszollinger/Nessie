-- Clean.lua

local utility = require("Utility");
local ps = require("ProjectSettings");

premake.modules.Cleaner = {}
local m = premake.modules.Cleaner
utility.SetupModule(m, "Cleaner", true);

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
--- Delete a File, if it exists.
---@param filepath string Filepath to delete.
---------------------------------------------------------------------------------------------------------
function m.DeleteFileIfExists(filepath)
    local success = true;
    local errorMsg;

    if os.locate(filepath) ~= nil then
        success, errorMsg = os.remove(filepath);
        if success == false then
            m.PrintError("Failed to delete '" .. filepath .. "'! Error msg: " .. errorMsg);
        end
    end

    return success;
end

---------------------------------------------------------------------------------------------------------
--- Deletes the Build, Intermediate, and Saved Folders.
---@return boolean
---------------------------------------------------------------------------------------------------------
function m.CleanProjects()
    if (ps == nil) then
        m.PrintError("Failed to Clean Projects! ProjectSettings nil!");
        return false;
    end

    if (ps.ProjectSettings == nil) then
        m.PrintError("Failed to Clean Projects! ProjectSettings are invalid!");
        return false;
    end

    local projectsArray = ps.ProjectSettings["Projects"];

    for i = 1, #projectsArray do
        local projectName = projectsArray[i].Name;
        local projectDir = ps.SolutionDir .. projectsArray[i].ProjectDir;

        local filepath = projectDir .. projectName .. ".vcxproj";
        m.DeleteFileIfExists(filepath);
        filepath = projectDir .. projectName .. ".vcxproj.user";
        m.DeleteFileIfExists(filepath);
        filepath = projectDir .. projectName .. ".vcxproj.filters";
        m.DeleteFileIfExists(filepath);
    end

    return true;
end

---------------------------------------------------------------------------------------------------------
--- Deletes the Build, Intermediate, and Saved Folders.
---@return boolean
---------------------------------------------------------------------------------------------------------
function m.CleanIntermediateFiles()
    if (ps == nil) then
        m.PrintError("Failed to Clean Project! ProjectSettings nil!");
        return false;
    end

    if (ps.ProjectSettings == nil) then
        m.PrintError("Failed to Clean Project! ProjectSettings are invalid!");
        return false;
    end

    if m.DeleteFolderIfExists(ps.SolutionDir .. "Build/", "Build") == false then
        return false;
    end

    if m.DeleteFolderIfExists(ps.SolutionDir .. "Intermediate/", "Intermediate") == false then
        return false;
    end

    if m.DeleteFolderIfExists(ps.SolutionDir .. "Saved/", "Saved") == false then
        return false;
    end

    return true;
end

---------------------------------------------------------------------------------------------------------
--- Deletes the Solution and .vs folder.
---@return boolean
---------------------------------------------------------------------------------------------------------
function m.CleanSolution()
    if m.DeleteFolderIfExists(ps.SolutionDir .. ".vs/", ".vs") == false then
        return false;
    end

    if m.DeleteFileIfExists(ps.SolutionDir .. ps.ProjectSettings["ProjectName"] .. ".sln") == false then
        return false;
    end

    return true;
end

newaction {
	trigger = "clean_all",
	description = "Deletes the Build, Intermediate Saved, and .vs folders, as well as the Solution and Projects.",

	execute = function()
		if (m.CleanIntermediateFiles() == false) then
            m.PrintSuccessOrFail("Clean Intermediate Files", false);
            return;
        end

        if (m.CleanProjects() == false) then
            m.PrintSuccessOrFail("Clean Projects", false);
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
        if (m.CleanIntermediateFiles() == false) then
            m.PrintSuccessOrFail("Clean Intermediate Files", false);
            return;
        end

        if (m.CleanProjects == false) then
            m.PrintSuccessOrFail("Clean Projects", false);
        end

        m.PrintSuccessOrFail("Clean Projects", true);

	end
}

return m
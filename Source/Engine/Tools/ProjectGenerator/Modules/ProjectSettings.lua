-- ProjectSettings.lua
-- Loads a table of the ProjectSettings to be used.

local utility = require("Utility");

premake.modules.ProjectSettings = {}
local m = premake.modules.ProjectSettings

utility.SetupModule(m, "ProjectSettings", true);
m.IsValid = false;
m.ProjectSettings = nil;
m.SolutionDir = nil;

---------------------------------------------------------------------------------------------------------
--- Intializes the ProjectSettings module. If this succeeds, the returned table will contain the 
---     SolutionDir (string), ProjectSettings (table loaded from json), and IsValid will equal true.
---     If it fails, IsValid will be set to false, and the other fields will be set to nil.
---@param solutionDirectory string SolutionDir.
---@return boolean Success False if we failed to load the project settings!
---------------------------------------------------------------------------------------------------------
function m.Init(solutionDirectory)
    -- Get ProjectName field from the ProjectSettings.yaml file.
    m.SolutionDir = solutionDirectory;
    m.IsValid = true;

    local projectSettingsPath = m.SolutionDir .. "Config\\ProjectSettings.json";
    if os.locate(projectSettingsPath) == nil then
        m.PrintError("Failed to find ProjectSettings file! Path: " .. projectSettingsPath);
        m.IsValid = false;
        m.SolutionDir = nil;
        m.ProjectSettings = nil;
        return false;
    end

    local errorMsg;
    local jsonData = utility.ReadFile(projectSettingsPath);
    m.ProjectSettings, errorMsg = json.decode(jsonData);
    if (m.ProjectSettings == nil) then
        m.PrintError("Failed to load ProjectSettings file! Error: " .. errorMsg);
        m.IsValid = false;
        m.SolutionDir = nil;
        m.ProjectSettings = nil;
        return false;
    end

    m.PrintSuccessOrFail("Loaded Project Settings", m.IsValid);
    return true;
end

return m;
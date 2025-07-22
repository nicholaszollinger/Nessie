-- ProjectCore
local utility = require("Utility")

premake.modules.ProjectCore = {};
local m = premake.modules.ProjectCore;

-- [TODO]: Have options for the ProjectLocation and the Project Name (The whole Project, not a VS project).
-- Option to set the projectLocation.
-- newoption
-- {
--     trigger = "projectLocation",
--     description = "The Directory that the solution will be built in."
-- }

-- [TODO] : Make sure this is still correct for the Final Location.
-- Better yet, I would use the projectLocation option above.
-- - This isn't correct at all btw. I need to fix this.
local DefaultProjectDirectory = path.getabsolute("../../../../../")

-----------------------------------------------------------------------------------
--- Initialize the Project Core module. This will load the Project file.
-----------------------------------------------------------------------------------
function m.Init()
    utility.SetupModule(m, "ProjectCore", true);

    m.PrintMessage("Loading Project Settings...");
    m.BuildFileExtension = ".Build.lua";
    m.ProjectFileExtension = ".Project.json";
    m.SolutionDir = utility.GetArgOrDefault(1, DefaultProjectDirectory);
    m.ThirdPartyDir = m.SolutionDir .. "Source\\ThirdParty\\";
    m.SourceFolder = m.SolutionDir .. "Source\\";
    m.ProjectFilesLocation = m.SolutionDir .. "Intermediate\\ProjectFiles\\";
    m.DefaultOutDir = "$(SolutionDir)Build/$(Configuration)_$(PlatformTarget)/";
    m.DefaultLibOutDir = "$(SolutionDir)Intermediate/Libs/$(Configuration)_$(PlatformTarget)/$(ProjectName)/";
    m.DefaultLibOutDirPath = "%{wks.location}/Intermediate/Libs/%{cfg.buildcfg}_%{cfg.platform}/";
    m.DefaultIntermediateDir = "!$(SolutionDir)Intermediate/Obj/$(Configuration)_$(PlatformTarget)/$(ProjectName)/"
    m.ProjectConfigurations = {"Debug", "Release"};
    m.ProjectSettings = nil;

    local match = os.matchfiles(m.SolutionDir .. "*" .. m.ProjectFileExtension);
    local projectFile = match[1] or nil;
    if (projectFile == nil) then
        m.PrintError("Failed to find Project File in Project Directory: '" .. m.SolutionDir .. "'");
        return false;
    end

    -- local projectSettingsPath = m.SolutionDir .. PROJECT_FILE_EXTENSION;
    -- if os.locate(projectSettingsPath) == nil then
    --     m.PrintError("Failed to find Project file! Path: " .. projectSettingsPath);
    --     m.SolutionDir = nil;
    --     m.ProjectSettings = nil;
    --     return false;
    -- end

    local errorMsg;
    local jsonData = utility.ReadFile(projectFile);
    m.ProjectSettings, errorMsg = json.decode(jsonData);
    if (m.ProjectSettings == nil) then
        m.PrintError("Failed to load Project file! Error: " .. errorMsg);
        m.ProjectSettings = nil;
        return false;
    end
end

---------------------------------------------------------------------------------------------------------
--- Get the path to the intermediate library directory for a given project.  
---@param name string : Path to the project file.
---@return string 
---------------------------------------------------------------------------------------------------------
function m.GetLibraryOutDirPath(name)
    
    local result = m.SolutionDir .. "Intermediate\\Libs\\";
    filter {"configurations:Debug" , "platforms:x64"}
        result = result .. "Debug_x64\\" .. name .. "\\";

    filter {"configurations:Release" , "platforms:x64"}
        result = result .. "Release_x64\\" .. name .. "\\";
    
    return result;
end

-----------------------------------------------------------------------------------
--- Sets common properties for Nessie Projects. Call this first, then make any changes
--- that you might need afterward to override.
--- Must be called within the scope of the premake call "project".
-----------------------------------------------------------------------------------
function m.SetProjectDefaults()
    filter {}

    language "C++"
    cppdialect "C++20"
    systemversion "latest"
    staticruntime "Off"

    targetdir(m.DefaultOutDir)
    objdir(m.DefaultIntermediateDir)
    warnings("Extra")

    filter "platforms:x64"
        architecture "x64"

    filter "configurations:Debug"
        defines { "_DEBUG", "NES_DEBUG" }
        symbols "On"

    filter "configurations:Release"
        defines { "NDEBUG", "NES_RELEASE" }
        optimize "On"
        runtime "Release"

    filter "system:windows"
        systemversion "latest"
        defines { "NES_PLATFORM_WINDOWS" }

    -- Reset the Filter
    filter {}
end

---------------------------------------------------------------------------------------------------------
--- Get the Project UUID from the .vcxproj file.
---@param projectFile string : Path to the project file.
---@return any UUID : UUID value for the project, or nil if nothing was found.
---------------------------------------------------------------------------------------------------------
function m.GetVSProjectUUID(projectFile)
    -- Read the vcxproj file.
    local fileData = utility.ReadFile(projectFile);

    -- Get the contents of the ProjectGuid tag.
    local uuid = string.match(fileData, "^.+<ProjectGuid>{(.+)}</ProjectGuid>.+$")
    if uuid == nil then
        m.PrintError("Failed to find UUID for Project File: " .. projectFile)
        return nil;
    end

    return uuid;
end

return m;
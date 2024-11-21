-- Dependencies.lua
-- [TODO] This isn't currently used. I am planning on using this to build a table of 
--        Dependencies that a project can use to add what they need. The Project would give it a table
--        of Dependency names, and if those dependencies are available, then they would be linked to the project.
--        This includes projects and libraries.

local utility = require("Utility")

premake.modules.Dependencies = {};
local m = premake.modules.Dependencies;
utility.SetupModule(m, "Dependencies", true);
m.dependencies = {};

-----------------------------------------------------------------------------------
---Create and register a new Dependency.
---@param dependency table Dependcy table that was created with CreateNewDependency()
---@param isDebug boolean Whether we are in the Debug Configuration and should attempt
---     to use Debug Library Name, if applicable.
---@return boolean Success Whether the dependency was successfully linked.
-----------------------------------------------------------------------------------
function LinkDependency(dependency, isDebug)
    -- Add the Lib Directory.
    if dependency.LibDir ~= nil then
        libdirs { dependency.LibDir }
    end

    local libraryName = dependency.LibDir;

    -- If in a Debug Configuration and we have a debug library name, set that.
    if dependency.DebugLibName ~= nil and isDebug then
        libraryName = dependency.DebugLibName;
    end
    
    if (libraryName == nil) then
        m.PrintError("Failed to Link Dependency! Null Library Name!");
        return false;
    end

    links { libraryName };
    return true;
end

-----------------------------------------------------------------------------------
---Create and register a new Dependency.
---@param name string Name of the Dependency to create.
---@return table dependency Returns a table with the following values set
-----------------------------------------------------------------------------------
function m.CreateNewDependency(name)
    local dependency = {};
    dependency.LibName = name;
    dependency.DebugLibName = name;
    dependency.IncludeDir = "";
    dependency.LibDir = "";

    ---Add the Library Directory and LinkDependency to a Project
    ---@param isDebug boolean Whether we are adding to a Debug Configuration.
    ---@return boolean Success Whether the dependency was successfully linked.
    dependency.Link = function(isDebug)
        return LinkDependency(dependency, isDebug);
    end

    -- Register the Dependency.
    m.dependencies[name] = dependency;

    return dependency;
end

return m;

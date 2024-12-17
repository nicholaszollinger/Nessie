-- DependencyInjector.lua

local utility = require("Utility")
local projectCore = require("ProjectCore");

premake.modules.DependencyInjector = {};
local m = premake.modules.DependencyInjector;

local BUILD_EXTENSION = ".Build.lua";

-----------------------------------------------------------------------------------
--- Initialize the Depenedency Injector, registering all dependencies in the Engine and Game folders.
---@return boolean Success False if there was an error registering Depenedencies.
-----------------------------------------------------------------------------------
function m.Init()
    utility.SetupModule(m, "DependencyInjector", true);
    m._targets = {};

    local matches = os.matchdirs(projectCore.SourceFolder .. "\\*");

    m.PrintInfo("Registering Build Targets");
    for i = 1, #matches do
        local folder = matches[i];
        if (m._RegisterBuildTargetsFromFolder(folder) == false) then
            m.PrintError("Failed to run Build Targets from folder: '" .. folder .. "'!");
            return false;
        end
    end

    return true;
end

-----------------------------------------------------------------------------------
---Link a registered Dependency. This will call Link and Include in the Dependency's 
---Build script.
---@param targetName string Name of the Dependency
---@return boolean Success Whether the dependency was successfully linked.
-----------------------------------------------------------------------------------
function m.Link(targetName)
    if (m._targets[targetName] ~= nil) then
        local target = m._targets[targetName];

        -- Ensure Include function
        if (target.Include == nil) then
            m.PrintError("Failed to Link Dependency: '" .. targetName .. "'! No Include() function found!");
            return false;
        end

        -- Ensure Link function
        if (target.Link == nil) then
            m.PrintError("Failed to Link Dependency: '" .. targetName .. "'! No Link() function found!");
            return false;
        end

        target.Include(target.Directory);
        target.Link(target.Directory);

        -- If the target is a Project, set it as a build dependency for the current project.
        if (target.ConfigureProject ~= nil) then
            dependson(targetName);
        end

        return true;
    end

    m.PrintError("Failed to Link Dependency: '" .. targetName .. "'! No Dependency with that name was found!");
    return false;
end

-----------------------------------------------------------------------------------
---Include the files from a registered Dependency. This will call the Include() from
---the Dependency's Build script.
---@param targetName string Name of the Dependency
---@return boolean Success Whether the dependency was successfully linked.
-----------------------------------------------------------------------------------
function m.Include(targetName)
    if (m._targets[targetName] ~= nil) then
        local target = m._targets[targetName];

        -- Ensure Include function
        if (target.Include == nil) then
            m.PrintError("Failed to Link Dependency: '" .. targetName .. "'! No Include() function found!");
            return false;
        end

        target.Include(target.Directory);

        return true;
    end

    m.PrintError("Failed to Include Dependency: '" .. targetName .. "'! No Dependency with that name was found!");
    return false;
end

-----------------------------------------------------------------------------------
--- Add the Dependencies' Files into the current project.
---@param dependencyName string Name of the Depedency.
---@return boolean Success Whether the files were successfully added.
-----------------------------------------------------------------------------------
function m.AddFilesToProject(dependencyName)
    if (m._targets[dependencyName] == nil) then
        m.PrintError("Failed to add Files to Current Project from Dependency: '" .. "'! No Dependency with that name was found!");
        return false;
    end

    local dependency = m._targets[dependencyName];
    if (dependency.Include == nil) then
        m.PrintError("Failed to add Files to Current Project from Target: '" .. "'! No Include() function was found!");
        return false;
    end

    if (dependency.AddFilesToProject == nil) then
        m.PrintError("Failed to add Files to Current Project from Target: '" .. "'! No AddFilesToProject() function was found!");
        return false;
    end

    dependency.AddFilesToProject(dependency.Directory);
    dependency.Include(dependency.Directory);

    return true;
end

-----------------------------------------------------------------------------------
--- Add all registered Projects to the current Workspace.
-----------------------------------------------------------------------------------
function m.AddProjectsToWorkspace()
    filter{};
    m.PrintInfo("Adding Projects to Solution...");

    for name, target in pairs(m._targets) do
        if (target.ConfigureProject == nil) then
            goto skip
        end

        m.PrintMessage("Adding Project: " .. name);
        group(target.Group)
            project(target.Name);
            location(projectCore.ProjectFilesLocation)

            if (target.UUID ~= nil) then
                uuid(target.UUID);
            end

            target.ConfigureProject(target.Directory, m);

            -- Reset the Filter.
            filter {};

            -- Add the Build Script
            files {target.BuildScript};

            local buildScriptDir = path.getdirectory(target.BuildScript);

            -- [TODO]: I want:
            --      - The Project's folder that contains the Source files to be in a filter called "Source"
            --      - The Build Script to be on the outermost project filter.
            --      - Any ThirdParty files that were added to be in a ThirdParty filter.
            -- I have tried a bunch of combinations, but the order of how these rules are applied is non-determinent, so it's a bit
            -- frustrating. The best I was able to get is having all of the included files' in a single source filter.
            -- Not the best, but is the closest to what I want.
            vpaths
            {
                --["ThirdParty/*"] = { projectCore.SourceFolder .. "**.*" },
                --["Source/*"] = { target.Directory .. ".**"},
                --["*"] = { buildScriptDir .. "*." .. BUILD_EXTENSION, projectCore.SourceFolder}
                --{ ["ThirdParty/*"] = projectCore.SourceFolder .. "**.*"},
                --{ ["*"] = projectCore.SourceFolder .. "/" .. target.Group .. "/**.*" },
                --["*"] = { projectCore.SourceFolder .. "**.*"},
                ["Source/*"] = { buildScriptDir .. "**.*"},
            }


        -- End of Loop
        ::skip::
    end
end

-----------------------------------------------------------------------------------
---Validate that the returned result from a Dependency Script is valid.
---@param file string Filepath for the script that produced the rawDependency.
---@param rawDependency table|nil Table returned from Depenedency Script.
---@return boolean IsValid If false, then the script was invalid.
-----------------------------------------------------------------------------------
function m._ValidateTarget(file, rawDependency)
    if (rawDependency == nil) then
        m.PrintError("Invalid Dependency returned from file '" .. file .. "'! Dependency Script must return a table result!");
        return false;
    end

    if (rawDependency.Name == nil or type(rawDependency.Name) ~= "string" or rawDependency.Name == "") then
        m.PrintError("Invalid Dependency returned from file '" .. file .. "'! Invalid Name field!");
        return false;
    end

    return true;
end

-----------------------------------------------------------------------------------
---Add all Dependencies that are located in the provided folder.
---@param folder string Folder path to begin the search.
---@return boolean Success False if a found Depenedency result was invalid.
-----------------------------------------------------------------------------------
function m._RegisterBuildTargetsFromFolder(folder)
    local groupName = path.getbasename(folder);
    local matches = os.matchfiles(folder .. "\\*" .. BUILD_EXTENSION);

    for i = 1, #matches do
        local file = matches[i];
        local target = dofile(file);

        -- Make sure that it is a valid result.
        if (m._ValidateTarget(file, target) == false) then
            return false;
        end

        m.PrintMessage("Registering: " .. target.Name);

        -- Save the Directory that it was found in.
        local scriptDirectory = path.getdirectory(file);
        target.Directory = scriptDirectory  .. "/" .. target.Name .. "/";
        target.BuildScript = file;
        target.UUID = nil;
        target.Group = groupName;

        -- If this target defined a ConfigureProject function, try to get an existing UUID.
        if (os.isdir(projectCore.ProjectFilesLocation) and target.ConfigureProject ~= nil and type(target.ConfigureProject) == "function") then
            -- Check for an existing project that was created previously, and save the UUID.
            local projectFilepath = projectCore.ProjectFilesLocation .. target.Name .. ".vcxproj";
            if (os.isfile(projectFilepath)) then
                target.UUID = projectCore.GetVSProjectUUID(projectFilepath);
            end
        end

        -- Add the Dependency to the map.
        m._targets[target.Name] = target;

    end

    return true;
end

-- [Note]: I opted to not deal with the Git Submodules for now. I'd have to be able to save some kind of 
-- versioning for the dependencies, whether I want to update them, etc. For now, it is not a issue I want to 
-- entirely tackle.
-- function m._UpdateGitSubmodule(dependency)
--     if (dependency == nil) then
--         m.PrintError("Failed to Update Git Dependency! Dependency was nil!")
--         return false;
--     end

--     -- if (dependency.GitURL == nil) then
--     --     m.PrintError("Failed to Update Git Dependency: '" .. dependency.Name .. "'! GitURL was nil!")
--     --     return false;
--     -- end

--     -- [TODO]: Do I need to move to the solution folder? 
--     local currentFolder = path.getabsolute("");
--     os.chdir(projectCore.SolutionDir);

--     -- If the Dependecy folder already exists, we need to delete it in order to update the git submodule.
--     if (os.isdir(dependency.Directory)) then
--         os.rmdir(dependency.Directory);

--     -- Else, add the Submodule. (?)
--     else

--     end


--     -- Return to the operating folder.
--     os.chdir(currentFolder);

--     return true;
-- end

return m;
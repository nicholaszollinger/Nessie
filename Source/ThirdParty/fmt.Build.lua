-- fmt Build Script
-- fmt is used for logging.

local d = {};
d.Name = "fmt";
d.ConfgureProject = nil;

function d.AddFilesToProject()
    local directory = d.ProjectDir;

    files
    {
        directory .. "include/**.h",
        directory .. "src/**.*",
    }
end

function d.Include()
    local directory = d.ProjectDir;
    
    includedirs { directory .. "include/" }
    
    buildoptions
    {
        "/utf-8" -- Unicode support
    }
end

return d;
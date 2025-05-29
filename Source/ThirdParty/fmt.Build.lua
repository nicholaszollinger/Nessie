-- fmt Build Script
-- fmt is used for logging.

local d = {};
d.Name = "fmt";
d.ConfgureProject = nil;

function d.AddFilesToProject(directory)
    files
    {
        directory .. "include/**.h",
        directory .. "src/**.*",
    }
end

function d.Include(directory)
    includedirs { directory .. "include/" }
    
    buildoptions
    {
        "/utf-8" -- Unicode support
    }
end

return d;
-- BleackLeakDetector Dependency Settings

local d = {};
d.Name = "BleachLeakDetector";
d.ConfgureProject = nil;

function d.AddFilesToProject(directory)
    files
    {
        directory .. "include\\**.h"
        , directory .. "include\\**.cpp"
    }
end

function d.Include(directory)
    includedirs { directory .. "include" }
end

return d;
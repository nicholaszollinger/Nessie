-- stb Build Script

local d = {};
d.Name = "stb";
d.ConfgureProject = nil;

function d.AddFilesToProject(directory)
    files
    {
        directory .. "stb_image.h",
        directory .. "*.cpp",
    }
end

function d.Include(directory)
    includedirs { directory }
end

return d;
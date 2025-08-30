-- stb Build Script

local d = {};
d.Name = "stb";
d.ConfgureProject = nil;

function d.AddFilesToProject()
    files
    {
        d.ProjectDir .. "stb_image.h",
        d.ProjectDir .. "*.cpp",
    }
end

function d.Include()
    includedirs { d.ProjectDir }
end

return d;
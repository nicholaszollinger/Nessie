-- Entt Dependency Configuration

local d = {};
d.Name = "entt";
d.ConfgureProject = nil;

function d.AddFilesToProject(directory)
    files
    {
        directory .. "include\\entt\\**.hpp"
    }
end

function d.Include(directory)    
    includedirs { directory .. "include" }
end

return d;
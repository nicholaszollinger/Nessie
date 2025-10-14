-- entt Build Script

local d = {};
d.Name = "entt";
d.ConfgureProject = nil;

function d.AddFilesToProject()
    local sourcePath = d.ProjectDir .. "src/entt/"

    files
    {
        sourcePath .. "entity/**.hpp", -- Just the ECS portion of entt.
        sourcePath .. "natvis/entity.natvis",
    }
end

function d.Include()
    local sourcePath = d.ProjectDir .. "src/"
    includedirs { sourcePath }
end

return d;
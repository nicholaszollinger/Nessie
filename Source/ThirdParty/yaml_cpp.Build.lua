-- Dependency Configuration for yaml_cpp

local d = {};
d.Name = "yaml_cpp";

function d.Include()
    includedirs { d.ProjectDir .. "include\\" }
end

function d.Link()
    filter {"configurations:Debug"}
        links { "yaml-cppd.lib"}
        libdirs { d.ProjectDir .. "lib\\Debug" }

    filter {"configurations:Release"}
        links {"yaml-cpp.lib"}
        libdirs { d.ProjectDir .. "lib\\Release\\"}

    -- Reset the filter.
    filter{};
end

return d;
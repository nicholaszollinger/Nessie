-- Dependency Configuration for yaml_cpp

local d = {};
d.Name = "yaml_cpp";

function d.Include(rootDir)
    includedirs { rootDir .. "include\\" }
end

function d.Link(rootDir)
    filter {"configurations:Debug"}
        links { "yaml-cppd.lib"}
        libdirs { rootDir .. "lib\\Debug" }

    filter {"configurations:Release"}
        links {"yaml-cpp.lib"}
        libdirs { rootDir .. "lib\\Release\\"}

    filter {"configurations:Test"}
        links {"yaml-cpp.lib"}
        libdirs { rootDir .. "lib\\Release\\"}

    -- Reset the filter.
    filter{};
end

return d;
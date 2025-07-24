-- Dependency Configuration for yaml_cpp

local d = {};
d.Name = "yaml_cpp";

function d.Include()
    includedirs { d.ProjectDir .. "include\\" }
    defines { "YAML_CPP_STATIC_DEFINE" }
end

function d.Link()
    filter {"configurations:Debug"}
        links { "yaml-cppd.lib"}
        libdirs { d.ProjectDir .. "lib\\Debug" }
        postbuildcommands { "{COPYFILE} \"" .. d.ProjectDir .. "\\lib\\Debug\\yaml-cppd.pdb\" \"%{cfg.buildtarget.directory}\""};

    filter {"configurations:Release"}
        links {"yaml-cpp.lib"}
        libdirs { d.ProjectDir .. "lib\\Release\\"}

    -- Reset the filter.
    filter{};
end

return d;
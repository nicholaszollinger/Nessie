-- Imgui Dependency Config
local projectCore = require("ProjectCore");

local d = {};
d.Name = "imgui";
d.IsOptional = true;

function d.AddFilesToProject()
    local projectDir = d.ProjectDir;

    files
    {
        -- Everything in the root folder:
        projectDir .. "*.h",
        projectDir .. "*.cpp",

        -- Backends for Vulkan and GLFW
        projectDir .. "backends\\imgui_impl_glfw.h",
        projectDir .. "backends\\imgui_impl_glfw.cpp",
        projectDir .. "backends\\imgui_impl_vulkan.h",
        projectDir .. "backends\\imgui_impl_vulkan.cpp",

        -- VS Debugging files:
        projectDir .. "misc\\debuggers\\imgui.natvis",
        projectDir .. "misc\\debuggers\\imgui.natstepfilter",

        -- Ease of use with std::string
        projectDir .. "misc\\cpp\\imgui_stdlib.*",
    };
end

function d.Include()
    includedirs { d.ProjectDir }
end

return d;
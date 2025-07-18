-- Imgui Dependency Config
local projectCore = require("ProjectCore");

local d = {};
d.Name = "imgui";

function d.AddFilesToProject(projectDir)
    files
    {
        -- Everything in the root folder:
        projectDir .. "*.h",
        projectDir .. "*.cpp",

        -- VS Debugging files:
        projectDir .. "misc\\debuggers\\imgui.natvis",
        projectDir .. "misc\\debuggers\\imgui.natstepfilter",

        -- Ease of use with std::string
        projectDir .. "misc\\cpp\\imgui_stdlib.*",
    };

    -- Add the Backend files based on the RenderAPI:
    local renderAPI = projectCore.ProjectSettings["RenderAPI"];
    d._AddBackendFiles(projectDir, renderAPI);
end

function d._AddBackendFiles(projectDir, renderAPI)
    if (renderAPI == "Vulkan") then
        files
        {
            projectDir .. "backends\\imgui_impl_glfw.h",
		    projectDir .. "backends\\imgui_impl_glfw.cpp",
            projectDir .. "backends\\imgui_impl_vulkan.h",
            projectDir .. "backends\\imgui_impl_vulkan.cpp",
        }
        return true;
    end

    projectCore.PrintError("Failed to Add Imgui backend! No Backend supported for RenderAPI: " .. renderAPI);
    return false;
end

function d.Include(projectDir)
    includedirs { projectDir }
end

return d;
-- Imgui Dependency Config
local projectCore = require("ProjectCore");

local d = {};
d.Name = "imgui";

-- local libFolder = projectCore.ProjectIntermediateLibsLocation .. "imgui\\";

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

--function d.ConfigureProject(projectDir)
--    projectCore.SetProjectDefaults();
--
--    targetdir(libFolder);
--    kind "StaticLib";
--    cppdialect "C++17"
--    staticruntime "off"
--    warnings "off"
--
--    includedirs { projectDir };
--
--    files
--    {
--        -- Everything in the root folder:
--        projectDir .. "*.h",
--        projectDir .. "*.cpp",
--
--        -- VS Debugging files:
--        projectDir .. "misc\\debuggers\\imgui.natvis",
--        projectDir .. "misc\\debuggers\\imgui.natstepfilter",
--
--        -- Ease of use with std::string
--        projectDir .. "misc\\cpp\\imgui_stdlib.*",
--    };
--
--    -- Add the Backend files based on the RenderAPI:
--    local renderAPI = projectCore.ProjectSettings["RenderAPI"];
--    d._AddBackend(projectDir, renderAPI);
--
--    filter "configurations:Debug"
--		runtime "Debug"
--		symbols "on"
--
--    filter "configurations:Release"
--        runtime "Release"
--        optimize "speed"
--
--    filter "configurations:Test"
--		runtime "Release"
--		optimize "speed"
--        symbols "on"
--end

function d._AddBackendFiles(projectDir, renderAPI)
    if (renderAPI == "SDL") then
        files
        {
            projectDir .. "backends\\imgui_impl_sdl2.h",
		    projectDir .. "backends\\imgui_impl_sdl2.cpp",
            projectDir .. "backends\\imgui_impl_sdlrenderer2.h",
            projectDir .. "backends\\imgui_impl_sdlrenderer2.cpp",
        }
        return true;
    end

    projectCore.PrintError("Failed to Add Imgui backend! No Backend supported for RenderAPI: " .. renderAPI);
    return false;
end

function d.Include(projectDir)
    includedirs { projectDir }
end

--function d.Link(projectDir)
--    links { "imgui.lib" }
--    libdirs { libFolder }
--end

return d;
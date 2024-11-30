-- GLFW Dependency Config
local projectCore = require("ProjectCore");

local d = {};
d.Name = "glfw";

function d.ConfigureProject(projectDir)
    projectCore.SetProjectDefaults();

    kind "StaticLib"
    language "C"
    warnings "off"
    targetname "glfw3"

    files
    {
        projectDir .. "include/GLFW/glfw3.h",
		projectDir .. "include/GLFW/glfw3native.h",
        projectDir .. "include/GLFW/internal.h",
        projectDir .. "include/GLFW/mappings.h",
        projectDir .. "include/GLFW/null_joystick.h",
        projectDir .. "include/GLFW/null_platform.h",
        projectDir .. "include/GLFW/platform.h",

		projectDir .. "src/glfw_config.h",
		projectDir .. "src/context.c",
		projectDir .. "src/init.c",
		projectDir .. "src/input.c",
		projectDir .. "src/monitor.c",

		projectDir .. "src/null_init.c",
		projectDir .. "src/null_joystick.c",
		projectDir .. "src/null_monitor.c",
		projectDir .. "src/null_window.c",

		projectDir .. "src/platform.c",
		projectDir .. "src/vulkan.c",
		projectDir .. "src/window.c",
    }

    filter "system:windows"
        files
        {
            projectDir .. "include/GLFW/win32_joystick.h",
            projectDir .. "include/GLFW/win32_platform.h",
            projectDir .. "include/GLFW/win32_thread.h",
            projectDir .. "include/GLFW/win32_time.h",

            projectDir .. "src/win32_init.c",
			projectDir .. "src/win32_joystick.c",
			projectDir .. "src/win32_module.c",
			projectDir .. "src/win32_monitor.c",
			projectDir .. "src/win32_time.c",
			projectDir .. "src/win32_thread.c",
			projectDir .. "src/win32_window.c",
			projectDir .. "src/wgl_context.c",
			projectDir .. "src/egl_context.c",
			projectDir .. "src/osmesa_context.c"
        }

        defines 
        {
            "_GLFW_WIN32",
            "_CRT_SECURE_NO_WARNINGS"
        }

    filter "configurations:Debug"
		runtime "Debug"
		symbols "on"

    filter "configurations:Release"
        runtime "Release"
        optimize "speed"

    filter "configurations:Test"
		runtime "Release"
		optimize "speed"
        symbols "off"
    
end

function d.Include(projectDir)
    includedirs { projectDir .. "include" }
end

function d.Link(projectDir)
    filter "system:windows"
        links { "glfw3.lib"}
        libdirs { projectCore.DefaultOutDir }
end

return d;
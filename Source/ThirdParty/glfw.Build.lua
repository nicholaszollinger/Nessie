-- GLFW Dependency Config
local projectCore = require("ProjectCore");

local d = {};
d.Name = "glfw";
d.IsOptional = true;

function d.ConfigureProject()
    local projectDir = d.ProjectDir;

    projectCore.SetProjectDefaults();

    targetdir(projectCore.DefaultLibOutDir);
    kind "StaticLib"
    language "C"
    warnings "off"
    targetname "glfw3"

    files
    {
        projectDir .. "include/GLFW/glfw3.h",
		projectDir .. "include/GLFW/glfw3native.h",
        
        projectDir .. "src/internal.h",
        projectDir .. "src/mappings.h",
        projectDir .. "src/null_joystick.h",
        projectDir .. "src/null_platform.h",
        projectDir .. "src/platform.h",

		--projectDir .. "src/glfw_config.h",
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
            projectDir .. "src/win32_joystick.h",
            projectDir .. "src/win32_platform.h",
            projectDir .. "src/win32_thread.h",
            projectDir .. "src/win32_time.h",

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
        optimize "Speed"

    filter{}

    vpaths
    {
        ["Include/*"] = {d.BuildDirectory .. "/glfw/include/GLFW/**.h", d.BuildDirectory .. "/glfw/src/**.h"},
        ["Source/*"] = { d.BuildDirectory .. "/glfw/src/**.c"},
    }

end

function d.Include()
    includedirs { d.ProjectDir .. "include" }
end

function d.Link()
    filter "system:windows"
        links { "glfw3.lib" }
        libdirs { projectCore.DefaultLibOutDirPath .. "glfw/" }
end

return d;
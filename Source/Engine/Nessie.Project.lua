-- Nessie Project Configuration.
-- Premake Documentation: https://premake.github.io/docs/

local p = {};
p.Name = "Nessie";
p.Group = "Engine"; -- [TODO] Groups should be handled by the ProjectSettings
p.TargetType = "StaticLib"
p.Language = "C++";
p.CppDialect = "C++20";

function p.ConfigureProject(projectDir)
    targetdir("$(SolutionDir)Build/$(Configuration)_$(PlatformTarget)/")
    objdir("!$(SolutionDir)Intermediate/$(ProjectName)/$(Configuration)_$(PlatformTarget)/")
    includedirs { "$(ProjectDir)Nessie\\", "$(ProjectDir)_ThirdParty\\yaml_cpp\\include\\", "$(ProjectDir)_ThirdParty\\BleachLeakDetector\\include\\" }
    warnings("Extra")
    links {}

    defines { "YAML_CPP_STATIC_DEFINE" }

    filter "system:windows"
        defines { "NES_PLATFORM_WINDOWS" };

    filter "configurations:Debug"
        links { "yaml-cppd.lib" }
        libdirs { "$(ProjectDir)_ThirdParty\\yaml_cpp\\lib\\$(Configuration)\\" }

    filter "configurations:Release"
        links { "yaml-cpp.lib" }
        libdirs { "$(ProjectDir)_ThirdParty\\yaml_cpp\\lib\\$(Configuration)\\" }

    filter "configurations:Test"
        links { "yaml-cpp.lib" }
        libdirs { "$(ProjectDir)_ThirdParty\\yaml_cpp\\lib\\Release\\" }

    filter {}
    
    files 
    {
        projectDir .. "Nessie\\**.h",
        projectDir .. "Nessie\\**.cpp",
        projectDir .. "Nessie\\**.ixx",

        projectDir .. "_ThirdParty\\BleachLeakDetector\\include\\**.h",
        projectDir .. "_ThirdParty\\BleachLeakDetector\\include\\**.cpp",
    }
end

return p;
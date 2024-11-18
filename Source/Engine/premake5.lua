-- Nessie Project Info.

local p = {};
p.Name = "Nessie";
p.Group = "Engine"; -- TODO: Get rid, this should be in the ProjectSettings.json.
p.TargetType = "StaticLib"
p.Language = "C++";
p.CppDialect = "C++20";

function p.ConfigureProject(projectDir)
    targetdir("$(SolutionDir)Build/$(Configuration)_$(PlatformTarget)/")
    objdir("!$(SolutionDir)Intermediate/$(ProjectName)/$(Configuration)_$(PlatformTarget)/")
    includedirs { "$(ProjectDir)Nessie\\", "$(ProjectDir)_ThirdParty\\yaml_cpp\\include\\", "$(ProjectDir)_ThirdParty\\BleachLeakDetector\\include\\" }
    warnings("Extra")
    links {}
    libdirs { "$(ProjectDir)_ThirdParty\\yaml_cpp\\lib\\$(Configuration)\\" }

    defines { "YAML_CPP_STATIC_DEFINE" }

    filter "system:windows"
        defines { "NES_PLATFORM_WINDOWS" };

    filter "configurations:Debug"
        defines { "NES_DEBUG" , "NES_LOGGING_ENABLED"}
        links { "yaml-cppd.lib" }

    filter "configurations:Release"
        defines { "NES_RELEASE" }
        links { "yaml-cpp.lib" }

    filter "configurations:Test"
        defines { "NES_TEST" , "NES_LOGGING_ENABLED" }
        links { "yaml-cpp.lib" }

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

-- local p = {};

-- function p.Do()

-- end

-- return p;
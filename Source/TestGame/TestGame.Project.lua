-- TestGame Project Configuration.
-- Premake Documentation: https://premake.github.io/docs/

local p = {};

p.Name = "TestGame";
p.Group = "Game"; -- [TODO] Groups should be handled by the ProjectSettings
p.TargetType = "ConsoleApp";
p.Language = "C++";
p.CppDialect = "C++20";

function p.ConfigureProject(projectDir)
    targetdir("$(SolutionDir)Build/$(Configuration)_$(PlatformTarget)/")
    objdir("!$(SolutionDir)Intermediate/$(ProjectName)/$(Configuration)_$(PlatformTarget)/")
    includedirs { "$(SolutionDir)Source/Engine/Nessie/", "$(SolutionDir)Source/Engine/_ThirdParty/yaml_cpp/include/", "$(SolutionDir)Source/Engine/_ThirdParty/BleachLeakDetector/include/" }
    warnings("Extra");
    links { "Nessie" }
    libdirs { "$(OutDir)"}
    dependson { "Nessie" }

    defines { "YAML_CPP_STATIC_DEFINE" }

    filter {}
    
    files 
	{
        projectDir .. "TestGame\\**.h",
        projectDir .. "TestGame\\**.cpp",
        projectDir .. "TestGame\\**.ixx",
    }
end

return p;
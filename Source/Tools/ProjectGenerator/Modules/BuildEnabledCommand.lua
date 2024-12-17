-- Creates a buildenabled attribute for projects, so that it isn't built when building the solution.
-- This is useful for disabling my ProjectGenerator project, for example.
-- Source: https://stackoverflow.com/questions/63227008/exclude-project-from-build-by-premake
require('vstudio')

premake.api.register
{
    name = "buildenabled",
    scope = "config",
    kind = "string",
    allowed =
    {
        "Default",
        "Off",
        "On"
    },
}

-- \modules\vstudio\vs2005_solution.lua
premake.override(premake.vstudio.sln2005.elements, "projectConfigurationPlatforms", function(base, cfg, context)
    
    if context.prjCfg.buildenabled and context.prjCfg.buildenabled == "Off" then
        return { premake.vstudio.sln2005.activeCfg }
    else
        return base(cfg, context)
    end
  end
)
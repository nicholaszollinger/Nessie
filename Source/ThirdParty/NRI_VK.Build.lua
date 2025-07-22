-- NRI Vulkan Project Configuration.
-- Use "DependencyInjector.Link("NRI")" to include all NRI projects.
local nriCore = require("NRI");
if (nriCore == nil) then
    return nil;
end

local d = {}
d.Name = "NRI_Vulkan";
d.Group = "ThirdParty/NRI"

function d.ConfigureProject(dependencyInjector)
    nriCore._SetNRIProjectDefaults();

    dependencyInjector.Link("NRI_Shared");

    files
    {
        nriCore.NRISourceDirectory .. "\\Source\\VK\\**.*",
        nriCore.NRISourceDirectory .. "\\_deps\\vma-src\\include\\vk_mem_alloc.h",
    }

    includedirs
    {
        nriCore.NRISourceDirectory .. "\\Include\\NRI\\",
        nriCore.NRISourceDirectory .. "\\Source\\Shared\\",
        nriCore.NRISourceDirectory .. "\\_deps\\vma-src\\include\\",
        nriCore.NRISourceDirectory .. "\\_deps\\vulkan_headers-src\\include\\",
    }

	filter {"system:windows"}
		defines { "VK_USE_PLATFORM_WIN32_KHR" }

	filter {}

    vpaths { ["Source/*"] = { d.BuildDirectory .. "/NRI/Source/VK/**.*" } }
    vpaths { ["vma/*"] = { d.BuildDirectory .. "/NRI/_deps/vma-src/include/**.*" } }
end

function d.Link()
    -- Use "DependencyInjector.Link("NRI")" to include all NRI projects.
end

function d.Include()
    -- Use "DependencyInjector.Link("NRI")" to include all NRI projects.
end

return d;
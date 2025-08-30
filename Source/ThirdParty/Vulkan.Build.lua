-- Vulkan Build Script

local projectCore = require("ProjectCore");
local vulkanSupport = require("VulkanSupport");

-- If we are going to build Project, validate that Vulkan is installed, and with a valid version.
if (_ACTION == "vs2022") then
    if (vulkanSupport.ValidateVulkanSupport() == false) then
        return nil;
    end
end

-- Vulkan Dependency for Linking.
local d = {};
d.Name = "Vulkan";
d.IsOptional = true;

function d.Include()
    includedirs
    {
        vulkanSupport.VulkanSDKPath .. "\\Include",
        d.BuildDirectory .. "\\VMA\\include",
        --d.BuildDirectory .. "\\Volk\\",
    }

    defines
    {
        "VULKAN_NO_EXCEPTIONS",

        -- Define VK_NO_PROTOTYPES to avoid including Vulkan prototypes
        -- This is necessary because we are using volk to load Vulkan functions
        --"VK_NO_PROTOTYPES",
    }

    filter "system:windows"
        defines { "VK_USE_PLATFORM_WIN32_KHR" }

    filter{}
end

function d.AddFilesToProject()
    files
    {
        d.BuildDirectory .. "\\VMA\\include\\vk_mem_alloc.h",
        --d.BuildDirectory .. "\\Volk\\volk.h",
        --d.BuildDirectory .. "\\Volk\\volk.c",
        d.BuildScript,
    };
end

function d.Link()
    defines
    {
        "VULKAN_NO_EXCEPTIONS",
    }

    links 
    {
        "vulkan-1.lib",
        "shaderc_shared.lib"
    }

    libdirs
    {
        vulkanSupport.VulkanSDKPath .. "\\Lib\\",
    }

    filter "system:windows"
        defines { "VK_USE_PLATFORM_WIN32_KHR" }
end

return d;
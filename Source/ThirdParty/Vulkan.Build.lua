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
    }

    defines
    {
        "VULKAN_NO_EXCEPTIONS",
    }

    filter "system:windows"
        defines { "VK_USE_PLATFORM_WIN32_KHR" }
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

    -- Add the BuildScript to the Project (Nessie)
    files { d.BuildScript };

    filter "system:windows"
        defines { "VK_USE_PLATFORM_WIN32_KHR" }
end

return d;
-- Vulkan Build Script

local projectCore = require("ProjectCore");
local utility = require("Utility");

-- Vulkan Module for managing the Vulkan
premake.modules.Vulkan = {};
local m = premake.modules.Vulkan;
utility.SetupModule(m, "Vulkan", true);
m.VulkanSDKPath = os.getenv("VK_SDK_PATH");
m.MinimumVulkanVersion = 1.3; -- Version 1.3 and up.

-- If no Vulkan Version present, then try to install this one.
m.InstallVulkanVersion = "1.4.304.0";
-- https://sdk.lunarg.com/sdk/download/1.4.304.0/windows/VulkanSDK-1.4.304.0-Installer.exe
m.InstallVulkanURL = "https://sdk.lunarg.com/sdk/download/" .. m.InstallVulkanVersion .. "/windows/VulkanSDK-" .. m.InstallVulkanVersion .. "-Installer.exe";
m.InstallVulkanLocation = projectCore.ThirdPartyDir .. "VulkanSDK";

-------------------------------------------------------------------------------------
---Ask the User if they want to install Vulkan.
---@return boolean VulkanInstalled False if the user declines or installation failed.
-------------------------------------------------------------------------------------
function m.TryInstallVulkan()
    m.PrintInfo("You need to install Vulkan. Use the link provided in the README to install Vulkan, then run \"GenerateSolution.bat\" again.");
    return false;
    -- [TODO]: 
    -- local wantsToInstall = utility.YesOrNoPrompt("Would you like to install the Vulkan SDK? It will be installed to: \"" .. m.InstallVulkanLocation .. "\".");
    -- if (wantsToInstall) then
    --     -- Download the installer:

    --     -- Run the Installer

    --     -- Recheck support?

    --     return true;
    -- else
    --     return false;
    -- end
end

---------------------------------------------------------------------------
---Validate that Vulkan is installed and that the version is compatible.
---@return boolean Success False if No Vulkan Support is valid.
---------------------------------------------------------------------------
function m.ValidateVulkanSupport()
    -- Vulkan not Installed:
    if (m.VulkanSDKPath == nil) then
        m.PrintError("No Vulkan SDK installed!");
        return m.TryInstallVulkan();
    end

    -- Get the current version
    local majorMinorVersion = string.gmatch(m.VulkanSDKPath, "1%.%d");
    local version = tonumber(majorMinorVersion() or 0);
    m.PrintMessage("Found Vulkan SDK version: " .. version);

    -- Invalid Version:
    if (version < m.MinimumVulkanVersion) then
        m.PrintError("Incompatible version SDK installed! Nessie requires at least " .. m.MinimumVulkanVersion .. "!");
        return m.TryInstallVulkan();

    -- Compatible Version Found:
    else
        m.PrintInfo("Compatible Vulkan SDK installed.");
        return true;
    end
end

-- Vulkan Dependency for Linking.
local d = {};
d.Name = "Vulkan";
d.IsOptional = true;

function d.Include(projectDir)
    includedirs
    {
        m.VulkanSDKPath .. "\\Include",
        projectCore.ThirdPartyDir .. "vk-bootstrap-1.3.302\\src\\"
        -- [TODO] vma
    }

    defines
    {
        "VULKAN_NO_EXCEPTIONS",
    }

    filter "system:windows"
        defines { "VK_USE_PLATFORM_WIN32_KHR" }

    -- Add the BuildScript to the Project
    files { d.BuildScript };
end

function d.Link(projectDir)

    defines
    {
        "VULKAN_NO_EXCEPTIONS",
    }

    links 
    {
        "vulkan-1.lib",
        "vk-bootstrap.lib",
        "shaderc_shared.lib"
        --vma
    }

    libdirs
    {
        m.VulkanSDKPath .. "\\Lib\\",
        --vma
    }

    filter "system:windows"
        defines { "VK_USE_PLATFORM_WIN32_KHR" }


    filter "configurations:Debug"
        libdirs 
        {
            -- vkBootstrap Debug folder
            projectCore.ThirdPartyDir .. "vk-bootstrap-1.3.302\\lib\\Debug\\"
        }

    filter "configurations:Release"
        libdirs 
        {
            -- vkBootstrap Release folder
            projectCore.ThirdPartyDir .. "vk-bootstrap-1.3.302\\lib\\Release\\"
        }

    filter "configurations:Test"
        libdirs 
        {
            -- vkBootstrap Release folder
            projectCore.ThirdPartyDir .. "vk-bootstrap-1.3.302\\lib\\Release\\"
        }
end

-- If we are going to build Project, validate that Vulkan is installed, and with a valid version.
if (_ACTION == "vs2022") then
    if (m.ValidateVulkanSupport() == false) then
        return nil;
    end
end

return d;
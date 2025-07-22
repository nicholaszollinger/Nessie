-- NRI Shared Project Configuration.
-- Use "DependencyInjector.Link("NRI")" to include all NRI projects.
local dependencyInjector = require("DependencyInjector");
local vulkanSupport = require("VulkanSupport");
local nriCore = require("NRI");
if (nriCore == nil) then
    return nil;
end

local shadermakeDXCVersion = "v1.8.2505"    -- DXC to download from 'GitHub/DirectXShaderCompiler' releases
local shadermakeDXCDate = "2025_05_24"      -- DXC release date, needed because DXC releases on GitHub have this in the download links :(
local shadermakeSlangVersion = "2025.12.1"  -- Slang to download from 'GitHub/Shader-slang' releases

local d = {}
d.Name = "ShaderMakeBlob";
d.Group = "ThirdParty/NRI"

function d.ConfigureProject(dependencyInjector)

    nriCore._SetNRIProjectDefaults();

    files
    {
        nriCore.NRISourceDirectory .. "\\_deps\\shadermake-src\\ShaderMake\\ShaderBlob.h",
        nriCore.NRISourceDirectory .. "\\_deps\\shadermake-src\\ShaderMake\\ShaderBlob.cpp",
    }

    includedirs
    {
        nriCore.NRISourceDirectory .. "\\_deps\\shadermake-src\\",
    }

    vpaths { ["Source/*"] = { d.BuildDirectory .. "/NRI/_deps/shadermake-src/ShaderMake/**.*" } }
end

function d.Link()
    links { "ShaderMakeBlob"}
    libdirs { nriCore.LibDir }
end

function d.Include()
    includedirs
    {
        nriCore.NRISourceDirectory .. "\\_deps\\shadermake-src\\",
    }
end

function d.FindShadermakeCompilers()
    -- Find DXC in Vulkan:
    if (vulkanSupport.VulkanSDKPath == nil) then
        return false;
    end

    local dxcExecutable = vulkanSupport.VulkanSDKPath .. "\\Bin\\dxc.exe"
    vulkanSupport.PrintInfo("DXC Path: " .. dxcExecutable);

    return true;
end

-- Test.
if (_ACTION == "vs2022") then
    if (d.FindShadermakeCompilers() == false) then
        return nil;
    end
end

return d;
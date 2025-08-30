-- Assimp 5.4.3 build script

local projectCore = require("ProjectCore");

local d = {};
d.Name = "Assimp";
d.IsOptional = true;

function d.Include()
    includedirs
    {
        projectCore.ThirdPartyDir .. "assimp-5.4.3\\include\\"
    }
end

function d.Link()
    
    libdirs { projectCore.ThirdPartyDir .. "assimp-5.4.3\\lib\\" }
    links { "assimp-vc143-mt.lib" }

    -- Post build command to copy over the dll.
    postbuildcommands { "{COPYFILE} \"" .. projectCore.ThirdPartyDir .. "assimp-5.4.3\\bin\\assimp-vc143-mt.dll\" \"" .. projectCore.DefaultOutDir .. "\""};
end


return d;
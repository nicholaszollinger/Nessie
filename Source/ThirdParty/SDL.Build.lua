-- SDL Dependency Config

local projectCore = require("ProjectCore");
local d = {};
d.Name = "SDL";
d.IsOptional = true;

function d.Include(projectDir)
    includedirs { projectDir .. "SDL\\include\\"}
end

function d.Link(projectDir)
    local libFolder = projectDir .. "SDL\\lib\\";

    links { "SDL2.lib" }
    libdirs { libFolder };

    -- Add a postbuildcommand to copy over the dll.
    postbuildcommands { "{COPYFILE} \"" .. libFolder .. "SDL2.dll\" \"" .. projectCore.DefaultOutDir .. "\""};
end

return d;
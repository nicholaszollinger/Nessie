-- Git Module for Lua

local utility = require("Utility");

premake.modules.Git = {}
local m = premake.modules.Git
utility.SetupModule(m, "Git", true);

newaction {
	trigger = "update_submodules",
	description = "Updates all git submodules",

	execute = function()
        m.PrintInfo("Updating Submodules...");
		os.execute("git submodule update --init --recursive --progress");
        m.PrintInfo("Submodule Status:")
        os.execute("git submodule status");
        --os.execute("git submodule foreach --recursive --quiet \"echo - $name Commit: `git rev-parse HEAD`\"");
        m.PrintSuccessOrFail("Update Submodules", true);
	end
}

return m;
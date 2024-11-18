-- Clean.lua
-- Right now, I just made this print out a message on completion. 
-- The cleaning logic happens in the SolutionGenerator.lua at the moment.
-- In order to move it here, I need to have a more central place for loading the ProjectData.
-- I will get to this later.
local utility = require("Utility");

premake.modules.Clean = {}
local m = premake.modules.Clean
utility.SetupModule(m, "Clean", true);

newaction {
	trigger = "clean",
	description = "",

	onEnd = function()
		m.PrintInfo("Clean complete");
	end
}

return m
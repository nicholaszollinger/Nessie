--Utility.lua
premake.modules.Utility = {};
local m = premake.modules.Utility;

-----------------------------------------------------------------------------------
---Setup a module with the following members
--- - module.moduleName : Name of the module
--- - module.DEBUG : Boolean to toggle Debug operations.
--- - module.PrintInfo(...) : Function to print out a message to the console.
--- - module.PrintDebug(...) : Function that prints only if module.DEBUG is true.
---@param module table Module that was registered with premake.
---@param moduleName string Name of the module.
---@param debugSetting boolean Value for the DEBUG settings for the module.
-----------------------------------------------------------------------------------
function m.SetupModule(module, moduleName, debugSetting)
    module.moduleName = moduleName;
    module.DEBUG = debugSetting;

    m.RegisterPrintFunctions(module);
end

-----------------------------------------------------------------------------------
---Registers PrintMessage, PrintInfo, PrintWarning and PrintError to the Module.
---@param module table Module we are setting variables for.
-----------------------------------------------------------------------------------
function m.RegisterPrintFunctions(module)
    -----------------------------------------------------------------------------------
    --- Print out a message from this module. If module.DEBUG is false, nothing
    --- will print.
    ---@param ... any : Any additional arguments you want to print.
    -----------------------------------------------------------------------------------
    module.PrintMessage = function(...)
        m.PrintMessage(module, ...);
        end;

    -----------------------------------------------------------------------------------
    --- Print out a info message, with the text brightened.
    ---@param ... any : Any additional arguments you want to print.
    -----------------------------------------------------------------------------------
    module.PrintInfo = function(...)
        m.PrintInfo(module, ...);
        end;

    -----------------------------------------------------------------------------------
    --- Print out a warning message.
    ---@param ... any : Any additional arguments you want to print.
    -----------------------------------------------------------------------------------
    module.PrintWarning = function(...)
        m.PrintWarning(module, ...);
        end;
    
    -----------------------------------------------------------------------------------
    --- Print out an error message.
    ---@param ... any : Any additional arguments you want to print.
    -----------------------------------------------------------------------------------
    module.PrintError = function(...)
        m.PrintError(module, ...);
        end;

    -----------------------------------------------------------------------------------
    --- Print out a message from this module. If module.DEBUG is false, nothing
    --- will print.
    ---@param taskString string : String that describes the task.
    ---@param success boolean : Whether the task failed or succeeded.
    -----------------------------------------------------------------------------------
    module.PrintSuccessOrFail = function(taskString, success)
        m.PrintSuccessOrFail(module, taskString, success);
        end
end

-----------------------------------------------------------------------------------
--- Print out a message from this module. If module.DEBUG is false, nothing
--- will print.
---@param ... any : Any additional arguments you want to print.
-----------------------------------------------------------------------------------
function m.PrintMessage(module, ...)
    assert(module.DEBUG);

    if module.DEBUG == false then
        return;
    end

    term.setTextColor(term.gray); -- Default Color
    print("[" .. module.moduleName .. "] " .. ...)
end

-----------------------------------------------------------------------------------
--- Print out some info from the module. It will be highlighted green.
---@param module table Module we are printing info for.
---@param ... any  Any arguments that we want to pass in to print.
-----------------------------------------------------------------------------------
function m.PrintInfo(module, ...)
    assert(module.moduleName);

    term.setTextColor(term.white);
    print("[INFO:" .. module.moduleName .. "] " ..  ...);
    term.setTextColor(term.gray); -- Default Color
end

-----------------------------------------------------------------------------------
--- Print a success or fail message in the format: "[ModuleName] taskString... successMsg"
---@param module table Module we are printing info for.
---@param taskString string Describes the Task.
---@param success boolean Whether the task succeeded or failed.
-----------------------------------------------------------------------------------
function m.PrintSuccessOrFail(module, taskString, success)
    assert(module.moduleName);
    local successMsg;

    if (success == true) then
        term.setTextColor(term.green);
        successMsg = "SUCCESS."
    else
        term.setTextColor(term.red);
        successMsg = "FAILED."
    end

    print("[" .. module.moduleName .. "] " .. taskString .. " ... " .. successMsg);
    term.setTextColor(term.gray); -- Default Color
end

-----------------------------------------------------------------------------------
--- Print out a warning message from the module. It will be highlighted green.
---@param module table Module we are printing info for.
---@param ... any  Any arguments that we want to pass in to print.
-----------------------------------------------------------------------------------
function m.PrintWarning(module, ...)
    assert(module.moduleName);

    term.setTextColor(term.yellow);
    print("[WARNING:" .. module.moduleName .. "] " ..  ...);
    term.setTextColor(term.gray); -- Default Color
end

-----------------------------------------------------------------------------------
--- Print out an error message from the Module. It will be highlighted red.
---@param module table Module we are printing info for.
---@param ... any  Any arguments that we want to pass in to print.
-----------------------------------------------------------------------------------
function m.PrintError(module, ...)
    assert(module.moduleName);

    term.setTextColor(term.red);
    print("[ERROR:" .. module.moduleName .. "] " ..  ...);
    term.setTextColor(term.gray); -- Default Color
end

-----------------------------------------------------------------------------------
--- Returns the file as a raw set of data.
---@param filename string : Name of the file we are trying to read.
-----------------------------------------------------------------------------------
function m.ReadFile(filename)
    -- Attempt to open and read the file into the 'data' var, then close it.
    local file = assert(io.open(filename, "rb"));
    local data = file:read("*all");
    file:close();

    return data;
end

-----------------------------------------------------------------------------------
--- Reads each setting in an .ini file into a table, and returns the table as a result.
--- The key and value will both be a string. The key value is equal to the key value 
--- in the config file.
---@param filename string Filepath to the .ini file.
-----------------------------------------------------------------------------------
function m.ReadIniFile(filename)
    local fileData = {};

    for line in io.lines(filename) do
        -- If we aren't looking at a comment or an empty sequence, then
        if string.len(line) > 0 and string.startswith(line, "#") == false then
            -- Check for Array.
            if string.startswith(line, "+") == true then
                line = string.sub(line, 2); -- Get the string without the +
                local arrayKey, value = m.GetKeyValuePair(line);

                if (arrayKey == nil or value == nil) then
                    return nil;
                end
                
                if (fileData[arrayKey] == nil) then
                    fileData[arrayKey] = {};
                end

                print("Adding value to array: " .. arrayKey);

                local array = fileData[arrayKey];
                local nextIndex = #array + 1;
                array[nextIndex] = value;

            else
                local key, value = m.GetKeyValuePair(line);
                if (key == nil or value == nil) then
                    return nil;
                end

                --print("Key: " .. tostring(key) .. " Val: " .. tostring(value));
                fileData[key] = value;
            end
            
        end
    end

    -- Return the filled out table.
    return fileData;
end

--------------------------------------------------------------------------------------------------------------
--- Get a Key Value pair from a string. The Key is on the left of the equal sign, the value is on the right. 
---@param stringValue any
---@return string|nil Key Name of the Key, or nil on failure.
---@return any|nil Value Value or nil on failure. The value could be a table, boolean, integer, float.
--------------------------------------------------------------------------------------------------------------
function m.GetKeyValuePair(stringValue)
    local key = "";
    local value = nil;

    local start, patternEnd = string.find(stringValue, "=");
    if (start == nil or patternEnd == nil) then
        return nil, nil;
    end

    key = string.sub(stringValue, 1, patternEnd - 1);
    local valueString = string.sub(stringValue, patternEnd + 1);

    -- Check for a table
    -- Example Table Syntax: "(Name="Nessie",ProjectDir="Source/Engine/")"
    if (string.startswith(valueString, "(") == true) then
        value = {};
        local strLength = string.len(valueString) - 1; -- without the parentheses.
        local tableElements = string.sub(valueString, 2, strLength);

        for _, elementPairString in next, string.explode(tableElements, ",") do
            local innerKey, innerValue = m.GetKeyValuePair(elementPairString);
            if (innerKey == nil or innerValue == nil) then
                return nil, nil;
            end

            value[innerKey] = innerValue;
        end

    -- Number value.
    elseif tonumber(valueString) then
        value = tonumber(valueString);

    --Boolean conversions
    elseif (string.lower(valueString) == "true") then
        value = true;

    elseif (string.lower(valueString) == "false") then
        value = false;

    -- Quoted String
    -- Strip the Quotation marks around the value.
    elseif string.startswith(valueString, "\"") then
        value = string.sub(valueString, 2, string.len(valueString) - 1);
    else
        value = stringValue;
    end

    return key, value;
end

-- Return the module.
return m;
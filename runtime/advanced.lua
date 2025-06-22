-- advanced.lua - Advanced utilities for trex runtime
local advanced = {}

-- Import utils from the same directory (relative import test)
local utils = require("utils")

function advanced.complex_operation(data)
    utils.log("Performing complex operation on: " .. tostring(data))
    return data * 2 + 1
end

function advanced.test_terminal_functions()
    print("Testing terminal functions from module...")
    
    -- Test terminal functions work from modules
    local text = xterm.terminal.get_text()
    utils.log("Retrieved " .. #text .. " characters from terminal")
    
    xterm.terminal.message("Message from advanced module!")
    
    return "Terminal functions test complete"
end

function advanced.demonstrate_imports()
    utils.log("Advanced module successfully imported utils")
    print("Relative imports working correctly within modules")
end

return advanced
-- utils.lua - Utility functions for trex runtime
local utils = {}

function utils.greet(name)
    print("Hello from " .. name .. "!")
    xterm.terminal.message("Module loaded: " .. name)
end

function utils.log(message)
    print("[UTILS] " .. message)
end

return utils
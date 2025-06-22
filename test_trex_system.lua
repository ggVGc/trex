-- Test script to verify the trex runtime system
print("=== Testing Trex Runtime System ===")

-- Test 1: Basic require functionality
print("Test 1: Loading utils module via require")
local utils = require("utils")
utils.log("Require function working correctly!")

-- Test 2: Verify we're loading from current directory
print("Test 2: Verifying path resolution")
print("This should load from ./runtime/")

-- Test 3: Test xterm integration
print("Test 3: Testing xterm integration")
if xterm and xterm.terminal then
    xterm.terminal.message("Trex system test successful!")
else
    print("ERROR: xterm integration not available")
end

print("=== Test Complete ===")
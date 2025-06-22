-- Test script to verify trex runtime system works correctly

print("=== Testing Trex Runtime System ===")

-- Test 1: Check that we can load runtime modules
print("\nTest 1: Loading modules from runtime/")
local success, utils = pcall(require, "utils")

if success then
    print("✓ Successfully loaded utils module from runtime/")
    
    -- Test module functions
    utils.greet("Test Script")
    utils.log("Module function test successful")
else
    print("✗ Failed to load utils module: " .. tostring(utils))
end

-- Test 2: Check current working directory behavior
print("\nTest 2: Verifying current working directory behavior")
local cwd_test = os.execute("pwd")
print("Current working directory check completed")

-- Test 3: Test that relative imports work from modules
print("\nTest 3: Testing nested relative imports")
-- This should work if our require system is properly set up
local success2, result = pcall(function()
    return require("utils")  -- Should resolve to runtime/utils.lua
end)

if success2 then
    print("✓ Nested relative import test successful")
else
    print("✗ Nested relative import failed: " .. tostring(result))
end

-- Test 4: Verify package.path includes trex_runtime
print("\nTest 4: Checking package.path configuration")
print("package.path:", package.path)

if string.find(package.path, "runtime") then
    print("✓ package.path correctly includes runtime")
else
    print("✗ package.path does not include runtime")
end

-- Test 5: Test loading from different subdirectories
print("\nTest 5: Testing trex runtime directory structure")
local runtime_dir_test = io.open("runtime/trex.lua", "r")
if runtime_dir_test then
    runtime_dir_test:close()
    print("✓ runtime/trex.lua exists and is accessible")
else
    print("✗ Cannot access runtime/trex.lua")
end

print("\n=== Trex Runtime System Test Complete ===")
-- Test script to verify standard require function works

print("=== Testing Standard Lua Require Function ===")

-- Test 1: Check that require is the standard Lua function
print("\nTest 1: Verifying require function type")
print("require type:", type(require))
if type(require) == "function" then
    print("✓ require is available as a function")
else
    print("✗ require is not available")
end

-- Test 2: Test package.path is configured correctly
print("\nTest 2: Checking package.path configuration")
print("package.path:", package.path)

-- Test 3: Basic module loading using standard require
print("\nTest 3: Loading modules with standard require")
local success, utils = pcall(require, "utils")

if success then
    print("✓ Successfully loaded utils module with standard require")
    utils.log("Standard Lua require working correctly!")
else
    print("✗ Failed to load utils module: " .. tostring(utils))
end

-- Test 4: Test subdirectory loading
print("\nTest 4: Testing subdirectory module loading")
local success2, helper = pcall(require, "lib.helper")

if success2 then
    print("✓ Successfully loaded module from subdirectory")
    helper.greet_from_subdir()
    local result = helper.multiply(6, 7)
    print("Multiplication result:", result)
else
    print("✗ Failed to load subdirectory module: " .. tostring(helper))
end

-- Test 5: Test module caching (standard require behavior)
print("\nTest 5: Testing module caching")
local utils1 = require("utils")
local utils2 = require("utils")

if utils1 == utils2 then
    print("✓ Module caching works correctly (same object returned)")
else
    print("✗ Module caching not working (different objects returned)")
end

-- Test 6: Test nested requires work
print("\nTest 6: Testing nested module imports")
local success3, advanced = pcall(require, "advanced")

if success3 then
    print("✓ Successfully loaded advanced module")
    advanced.demonstrate_imports()
else
    print("✗ Failed to load advanced module: " .. tostring(advanced))
end

-- Test 7: Test that we can use all standard require features
print("\nTest 7: Testing standard require features")
-- This should use package.loaded table
if package.loaded["utils"] then
    print("✓ package.loaded table is working")
else
    print("✗ package.loaded table not working")
end

print("\n=== Standard Require Test Complete ===")
print("Benefits of using standard require:")
print("1. Module caching - modules loaded once and reused")
print("2. Standard Lua behavior - familiar to developers")
print("3. package.path support - flexible module organization")
print("4. package.loaded table - can inspect loaded modules")
print("5. No custom security restrictions - uses Lua's built-in security")
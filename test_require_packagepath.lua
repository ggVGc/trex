-- Test script to verify the updated require function uses package.path

print("=== Testing Package.path-based Require Function ===")

-- Test 1: Check package.path configuration
print("\nTest 1: Checking package.path configuration")
print("package.path:", package.path)

if string.find(package.path, "runtime") then
    print("✓ package.path includes runtime directory")
else
    print("✗ package.path does not include runtime directory")
end

-- Test 2: Basic module loading
print("\nTest 2: Basic module loading via package.path")
local success, utils = pcall(require, "utils")

if success then
    print("✓ Successfully loaded utils module using package.path")
    utils.log("Package.path require working correctly!")
else
    print("✗ Failed to load utils module: " .. tostring(utils))
end

-- Test 3: Nested module imports
print("\nTest 3: Testing nested module imports")
local success2, advanced = pcall(require, "advanced")

if success2 then
    print("✓ Successfully loaded advanced module")
    advanced.demonstrate_imports()
    
    -- Test that advanced module can use require internally
    local result = advanced.complex_operation(5)
    print("Complex operation result:", result)
else
    print("✗ Failed to load advanced module: " .. tostring(advanced))
end

-- Test 4: Test security - attempt to load from outside runtime
print("\nTest 4: Testing security restrictions")
local security_test1 = pcall(require, "../test_require_packagepath")
if not security_test1 then
    print("✓ Security working: Cannot load modules from outside runtime directory")
else
    print("✗ Security issue: Loaded module from outside runtime directory")
end

-- Test 5: Test subdirectory module loading
print("\nTest 5: Testing subdirectory module loading")
local success3, helper = pcall(require, "lib.helper")
if success3 then
    print("✓ Successfully loaded module from subdirectory")
    helper.greet_from_subdir()
    local result = helper.multiply(7, 8)
    print("Multiplication result:", result)
else
    print("✗ Failed to load module from subdirectory: " .. tostring(helper))
end

-- Test 6: Test consistent module loading behavior
print("\nTest 6: Testing consistent module loading behavior")
local success4, utils2 = pcall(require, "utils")
if success4 then
    print("✓ Module loading works consistently across calls")
else
    print("✗ Inconsistent module loading")
end

print("\n=== Package.path Require Test Complete ===")
print("The require function now:")
print("1. Uses package.path for module resolution")
print("2. Supports standard Lua require behavior") 
print("3. Maintains security by validating paths")
print("4. Allows for more flexible module organization")
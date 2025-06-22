-- Test script to verify dofile and loadfile work

print("=== Testing dofile and loadfile Functions ===")

-- Test 1: Check that dofile is available
print("\nTest 1: Checking dofile availability")
print("dofile type:", type(dofile))
if type(dofile) == "function" then
    print("✓ dofile is available as a function")
else
    print("✗ dofile is not available")
end

-- Test 2: Check that loadfile is available
print("\nTest 2: Checking loadfile availability")
print("loadfile type:", type(loadfile))
if type(loadfile) == "function" then
    print("✓ loadfile is available as a function")
else
    print("✗ loadfile is not available")
end

-- Test 3: Test dofile functionality
print("\nTest 3: Testing dofile functionality")
local success, result = pcall(dofile, "runtime/test_file.lua")
if success then
    print("✓ dofile successfully loaded file")
    print("Loaded data message:", result.message)
    print("Loaded data value:", result.value)
    print("Function result:", result.func())
else
    print("✗ dofile failed:", tostring(result))
end

-- Test 4: Test loadfile functionality
print("\nTest 4: Testing loadfile functionality")
local loaded_func, err = loadfile("runtime/test_file.lua")
if loaded_func then
    print("✓ loadfile successfully loaded file")
    -- Execute the loaded chunk
    local success2, result2 = pcall(loaded_func)
    if success2 then
        print("✓ Executed loaded chunk successfully")
        print("Result message:", result2.message)
    else
        print("✗ Failed to execute loaded chunk:", tostring(result2))
    end
else
    print("✗ loadfile failed:", tostring(err))
end

-- Test 5: Test loading from different paths
print("\nTest 5: Testing file loading from different paths")
-- Create a temporary test file
local test_content = [[
print("Loaded from temporary location!")
return { location = "temp", status = "success" }
]]

-- Write temporary file
local f = io.open("/tmp/xterm_test_load.lua", "w")
if f then
    f:write(test_content)
    f:close()
    
    -- Try to load it
    local success3, result3 = pcall(dofile, "/tmp/xterm_test_load.lua")
    if success3 then
        print("✓ Successfully loaded file from /tmp")
        print("Result:", result3.location, result3.status)
    else
        print("✗ Failed to load from /tmp:", tostring(result3))
    end
    
    -- Clean up
    os.remove("/tmp/xterm_test_load.lua")
else
    print("Note: io.open not available for temp file test")
end

-- Test 6: Test error handling
print("\nTest 6: Testing error handling")
local success4, err = pcall(dofile, "nonexistent_file.lua")
if not success4 then
    print("✓ dofile correctly reports error for missing file")
    print("Error message:", tostring(err))
else
    print("✗ dofile did not report error for missing file")
end

-- Test 7: Compare require vs dofile
print("\nTest 7: Comparing require vs dofile behavior")
print("require caches modules, dofile always reloads:")
local r1 = require("utils")
local r2 = require("utils")
print("require returns same object:", r1 == r2)

if dofile then
    -- dofile always executes the file again
    print("dofile executes file each time (check output above)")
end

print("\n=== File Loading Test Complete ===")
print("Summary:")
print("- dofile: Executes a Lua file immediately")
print("- loadfile: Loads a file as a function to execute later")
print("- Both functions now available for flexible file loading")
print("- No restrictions on file paths - full filesystem access")
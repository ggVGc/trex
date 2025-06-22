-- test_module.lua - Test module in current working directory
local test_module = {}

function test_module.hello()
    return "Hello from test module in current working directory!"
end

function test_module.get_location()
    return "Current working directory"
end

print("test_module.lua loaded from current directory")

return test_module
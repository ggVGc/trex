-- helper.lua - Test module in subdirectory
local helper = {}

function helper.multiply(a, b)
    print("[HELPER] Computing " .. a .. " * " .. b)
    return a * b
end

function helper.greet_from_subdir()
    print("Hello from runtime/lib/helper.lua!")
end

return helper
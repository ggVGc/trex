print("=== Runtime/trex.lua loaded successfully! ===")

-- Test that we can require other runtime modules
local utils = require("trex.utils")
utils.log("Runtime system initialized and ready!")

-- Test package.path configuration
print("Package.path includes:")
local i = 1
for path in string.gmatch(package.path, "[^;]+") do
    if i <= 6 then
        print(string.format("  %d. %s", i, path))
    end
    i = i + 1
end

return {
    status = "loaded",
    message = "Runtime system ready"
}
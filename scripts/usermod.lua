-- Example user module that can be required
local usermod = {}

function usermod.hello()
    return "Hello from usermod in ~/.config/trex/"
end

function usermod.get_info()
    return {
        location = "~/.config/trex/usermod.lua",
        purpose = "Demonstrate user module loading via require()",
        loaded_by = "require('usermod')"
    }
end

print("usermod.lua loaded from ~/.config/trex/")

return usermod
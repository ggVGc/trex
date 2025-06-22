-- User configuration for trex
print("Loading user config from ~/.config/trex/init.lua")

if trex then
    trex.utils.log("User configuration loaded - trex API available!")
    
    -- Test that package.path includes ~/.config/trex
    trex.utils.log("Testing package.path includes user config directory...")
    
    -- Add user-specific customizations
    trex.user = {
        config_dir = os.getenv("HOME") .. "/.config/trex",
        
        greet = function()
            trex.message("Hello from ~/.config/trex/init.lua!")
        end,
        
        show_paths = function()
            print("Current package.path:")
            local i = 1
            for path in string.gmatch(package.path, "[^;]+") do
                print(string.format("  %d. %s", i, path))
                i = i + 1
                if i > 10 then break end
            end
        end,
        
        test_require = function()
            -- Test loading a module from user config dir
            local success, usermod = pcall(require, "usermod")
            if success then
                trex.utils.log("Successfully loaded usermod from ~/.config/trex/")
                return usermod
            else
                trex.utils.log("Could not load usermod: " .. tostring(usermod))
                return nil
            end
        end
    }
    
    trex.utils.log("User configuration complete - functions available as trex.user.*")
else
    print("Warning: trex API not available in user config")
end

print("User configuration loaded successfully!")
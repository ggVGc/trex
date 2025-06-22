-- Example user script in scripts directory
print("Loading example user script from ./scripts/example.lua")

if trex then
    trex.utils.log("Example script: Adding utility functions")
    
    -- Add more utility functions
    trex.user = trex.user or {}
    
    trex.user.timestamp = function()
        local date = os.date("%Y-%m-%d %H:%M:%S")
        trex.write(date)
    end
    
    trex.user.clear_and_message = function(msg)
        -- This would need a clear function, just demo for now
        trex.message("DEMO: " .. msg)
    end
    
    trex.utils.log("Example script loaded - added timestamp and clear_and_message functions")
else
    print("Warning: trex API not available in example script")
end
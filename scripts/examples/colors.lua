-- Color Theme Example
-- This script demonstrates dynamic color management

-- Function to set a dark theme
function set_dark_theme()
    xterm.config.set("background", "#001122")
    xterm.config.set("foreground", "#ccddee")
    xterm.terminal.write("\nDark theme applied\n")
end

-- Function to set a light theme  
function set_light_theme()
    xterm.config.set("background", "#ffffff")
    xterm.config.set("foreground", "#000000")
    xterm.terminal.write("\nLight theme applied\n")
end

-- Time-based theme switching
function auto_theme()
    local hour = tonumber(os.date("%H"))
    if hour >= 18 or hour < 6 then
        set_dark_theme()
    else
        set_light_theme()
    end
end

-- Hook for focus events to update theme
xterm.events.on("focus_in", function()
    auto_theme()
end)

print("Color theme script loaded. Use set_dark_theme(), set_light_theme(), or auto_theme()")
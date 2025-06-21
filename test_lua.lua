-- Simple test script to verify Lua scripting works
print("Lua scripting is working in xterm!")

-- Test basic xterm API if available
if xterm then
    print("xterm API is available")
    if xterm.terminal then
        print("terminal API is available")
    end
    if xterm.config then
        print("config API is available")
    end
else
    print("xterm API not available")
end
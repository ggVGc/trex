-- Test script for the new xterm.terminal.message function

print("Testing xterm.terminal.message function...")

-- Test the new message function
xterm.terminal.message("Hello from xterm.terminal.message!")
xterm.terminal.message("This is a test message")

-- Compare with write function
print("Now testing xterm.terminal.write for comparison:")
xterm.terminal.write("This text from write has no automatic newlines")
xterm.terminal.write(" - continues on same line\n")

-- Test message again
xterm.terminal.message("Another message with proper line separation")

print("Test completed successfully!")
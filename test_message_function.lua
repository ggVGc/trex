-- Test script for the corrected xterm.terminal.message function

print("Testing corrected xterm.terminal.message function...")

-- Test the new message function (should appear as terminal output, not shell input)
xterm.terminal.message("Hello from corrected xterm.terminal.message!")
xterm.terminal.message("This message should appear as terminal output")
xterm.terminal.message("Not as text typed into the shell")

-- Compare with write function (this still sends to shell)
print("Comparing with xterm.terminal.write (sends to shell):")
xterm.terminal.write("This text from write goes to shell input")
xterm.terminal.write(" - and continues on same line\n")

-- Test more messages
xterm.terminal.message("Message 1: I appear as terminal output")
xterm.terminal.message("Message 2: Each message gets its own line")
xterm.terminal.message("Message 3: Shell prompt appears below")

print("Test completed - check that messages appear as output, not input!")
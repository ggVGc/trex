#!/bin/bash

echo "=== XTerm Lua Command Mode Demo (Clean Version) ==="
echo
echo "The Lua command mode is now active with these features:"
echo
echo "1. Press Ctrl+T to enter command mode"
echo "   - The terminal will beep to indicate command mode is active"
echo "   - Your keystrokes will be captured for the Lua command"
echo
echo "2. In command mode:"
echo "   - Type any Lua command (it won't appear on screen to avoid interference)"
echo "   - Press Enter to execute the command"
echo "   - Press Escape to cancel and exit command mode"
echo "   - Backspace/Delete work for editing"
echo
echo "3. Example commands to try:"
echo "   print('Hello from Lua!')"
echo "   xterm.terminal.bell()"
echo "   xterm.terminal.set_title('New Title')"
echo "   for i=1,5 do print(i) end"
echo
echo "The command mode operates silently to avoid display conflicts."
echo "Output from executed commands will appear normally."
echo

# Keep terminal open
exec bash
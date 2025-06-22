#!/bin/bash

echo "=== XTerm Lua Command Mode with Display ==="
echo
echo "The command mode now shows what you type!"
echo
echo "Instructions:"
echo "1. Press Ctrl+T to enter Lua command mode"
echo "2. You'll see 'Lua> ' prompt appear"
echo "3. Type your command - it will be displayed as you type"
echo "4. Press Enter to execute"
echo "5. Press Escape to cancel"
echo
echo "Try these commands:"
echo "  print('Hello World')"
echo "  xterm.terminal.bell()"
echo "  2+2"
echo "  xterm.terminal.set_title('My Terminal')"
echo

exec bash
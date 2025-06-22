#!/bin/bash

echo "=== Testing XTerm Lua Command Mode with Visual Prompt ==="
echo
echo "Press Ctrl+T to enter Lua command mode (configured in init.lua)"
echo "You should see a 'Lua> ' prompt at the bottom of the screen"
echo
echo "Once in command mode:"
echo "- Type Lua commands like: print('Hello!')"
echo "- Press Enter to execute"
echo "- Press Escape to exit command mode"
echo
echo "The prompt will appear at the bottom of the terminal and disappear when you exit."
echo

# Keep terminal open
exec bash
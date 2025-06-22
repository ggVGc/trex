-- Test script for xterm.terminal.get_text() function

print("Testing xterm.terminal.get_text() function...")

-- Clear screen first
xterm.terminal.clear()

-- Add some test content
print("Line 1: This is test content")
print("Line 2: For testing get_text function")
print("Line 3: With various characters: !@#$%^&*()")
print("Line 4: And some numbers: 1234567890")

-- Get the current terminal text
local terminal_text = xterm.terminal.get_text()

print("\n--- Terminal Text Output ---")
print("Length:", #terminal_text)
print("Content:")
print(terminal_text)
print("--- End Terminal Text ---")

-- Test that we can find our test content
if string.find(terminal_text, "test content") then
    print("✓ Successfully found test content in terminal text")
else
    print("✗ Could not find test content in terminal text")
end

if string.find(terminal_text, "1234567890") then
    print("✓ Successfully found numbers in terminal text")
else
    print("✗ Could not find numbers in terminal text")
end

-- Test getting text after more output
print("Adding more content after get_text call...")
print("This should appear in subsequent get_text calls")

local terminal_text2 = xterm.terminal.get_text()
print("\n--- Second Terminal Text Output ---")
print("Length:", #terminal_text2)
print("--- End Second Terminal Text ---")

if string.find(terminal_text2, "subsequent get_text") then
    print("✓ Successfully captured new content")
else
    print("✗ New content not captured")
end

print("get_text() function test completed!")
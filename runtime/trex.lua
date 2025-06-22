-- trex.lua - Main trex runtime initialization
print("=== Trex Runtime Loading ===")
print("Loading trex runtime from current working directory!")

-- Test the custom require function by loading a module
print("\n1. Loading utils module...")
local utils = require("trex/utils")
utils.greet("Trex Runtime")

-- Test relative imports work
print("\n2. Testing relative module loading...")
local utils2 = require("trex/utils")  -- Should load from trex_runtime/utils.lua
utils2.log("Relative import test successful")

-- Test advanced module with nested imports
print("\n3. Loading advanced module (tests nested imports)...")
local advanced = require("trex/advanced")
advanced.demonstrate_imports()

-- Test terminal functions from modules
print("\n4. Testing terminal functions from modules...")
local result = advanced.test_terminal_functions()
print("Result:", result)

-- Test complex operation
print("\n5. Testing module functionality...")
local complex_result = advanced.complex_operation(10)
utils.log("Complex operation result: " .. complex_result)

-- Register some basic hooks
print("\n6. Trex runtime setup complete!")
print("=== Runtime Ready ===")

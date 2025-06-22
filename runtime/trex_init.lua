print("=== Runtime/trex_init.lua loaded successfully! ===")

-- Load core runtime modules
local utils = require("trex.utils")
local advanced = require("trex.advanced")
utils.log("Runtime system initialized!")

-- Add user config directory to package.path
local function setup_user_package_path()
  local user_config_dir = os.getenv("HOME") .. "/.config/trex"

  -- Add user config directory patterns to package.path
  local user_patterns = {
    user_config_dir .. "/?.lua",
    user_config_dir .. "/?/init.lua"
  }

  for _, pattern in ipairs(user_patterns) do
    if not package.path:find(pattern, 1, true) then
      package.path = pattern .. ";" .. package.path
      utils.log("Added to package.path: " .. pattern)
    end
  end
end

-- Load user scripts (simplified to focus on ~/.config/trex)
local function load_user_scripts()
  local user_config_dir = os.getenv("HOME") .. "/.config/trex"
  local init_script = user_config_dir .. "/init.lua"

  utils.log("Loading user scripts from " .. user_config_dir)

  -- First, try to load init.lua if it exists
  local file = io.open(init_script, "r")
  if file then
    file:close()
    utils.log("Loading user init script: " .. init_script)
    local success, err = pcall(dofile, init_script)
    if not success then
      utils.log("Error loading " .. init_script .. ": " .. tostring(err))
    else
      utils.log("Successfully loaded " .. init_script)
    end
  else
    utils.log("No user init script found at " .. init_script)
  end

  utils.log("User script loading complete")
end

-- Set up user package path first
setup_user_package_path()

-- Load user scripts (package.path is set, they can require("trex") normally)
load_user_scripts()

utils.log("Runtime system ready with user script support!")

print("=== Runtime/trex.lua loaded successfully! ===")

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

-- Export trex API for user scripts BEFORE loading them
_G.trex = {
  utils = utils,
  advanced = advanced,

  -- Terminal API shortcuts
  message = xterm.terminal.message,
  write = xterm.terminal.write,
  get_text = xterm.terminal.get_text,

  -- Configuration helpers
  reload_scripts = function()
    utils.log("Reloading user scripts...")
    setup_user_package_path()
    load_user_scripts()
  end,

  version = "1.0.0",
  loaded_scripts = {}
}

-- Load user scripts (now that trex API is available and package.path is set)
load_user_scripts()

utils.log("Runtime system ready with user script support!")

return {
  status = "loaded",
  message = "Runtime system ready with user scripts",
  api = _G.trex
}

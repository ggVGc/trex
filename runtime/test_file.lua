-- test_file.lua - Test file for dofile and loadfile
print("This file was loaded via dofile or loadfile!")

local test_data = {
    message = "Successfully loaded external file",
    value = 42,
    func = function()
        return "Function from loaded file works!"
    end
}

return test_data
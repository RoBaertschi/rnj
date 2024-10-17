---@meta

---Escapes a string for use in ninja.
---@param string string
---@return string
function escape(string)
	return string
end

---Generates a gitignore for all build outputs
function generate_gitignore() end

---Rnj functions, that are not specific to the build system and more of utilities.
rnj = {
	os = {},
}

---Retruns the current builddir or nil
---@return nil|string
function rnj.get_builddir() end

---Sets the builddir to the specified path
function rnj.builddir(path) end

---Create a directory at path
---@param path string
function rnj.os.mkdir(path) end

---Removes file or an empty directory
---@param path string
function rnj.os.unlink(path) end

---Returns the path seperator for this os.
---@return string
function rnj.os.sep() end

---Checks if the specified directory exists.
---@param path string
---@return boolean
function rnj.os.dir_exists(path) end

---Returns the absolute path for path
---@param path string
---@return string
function rnj.os.realpath(path) end

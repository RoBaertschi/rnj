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

---Create a directory at path
---@param path string
function rnj.os.mkdir(path) end

---Removes file or an empty directory
---@param path string
function rnj.os.unlink(path) end

---Returns the path seperator for this os.
function rnj.os.sep() end

local arg_mode_none = 0
local arg_mode_done = 99

local arg_mode = arg_mode_done

local function help_and_exit(error)
	print("rnj.lua [args...]")
	print("args:")
	print("\t--help - print this help")
	os.exit(error)
end

if 0 < #arg then
	for _, a in ipairs(arg) do
		if a == "--help" then
			help_and_exit(0)
		end

		if arg_mode == arg_mode_done then
			print('unexpected additonal argument after DIR "' .. a .. '"')
			help_and_exit(1)
		end
	end
end

---@type { [string]: string }
vars = {}

---Adds a new variable that can be used in ninja using the $variablename syntax
---@param name string
---@param default string
function var(name, default)
	vars[name] = default or ""
end

---@class rule_options
---@field command string
---@field depfile string?
---@field deps "gcc"|"msvc"|nil

---@type { [string]: rule_options }
rules = {}

---@param name string
---@param options rule_options
function rule(name, options)
	assert(options.command ~= nil, "rule arg2 options should have a command field")
	assert(type(options.command) == "string", "rule arg2 options command should be a string")

	if options.depfile ~= nil then
		assert(type(options.depfile) == "string", "rule(..., options) options.depfile should be a string")
	end
	if options.deps ~= nil then
		assert(type(options.deps) == "string", "rule(..., options) options.deps should be a string")
		assert(
			options.deps == "msvc" or options.deps == "gcc",
			'rule(..., options) options.deps should be either "msvc" or "gcc"'
		)
	end
	rules[name] = options
end

---@type { output: string, rule: string, inputs: string[] }[]
builds = {}

function build(output, rule, ...)
	local arg = { ... }
	table.insert(builds, { output = output, rule = rule, inputs = arg })
end

-- function generate_gitignore()
-- 	local file, err = io.open(".gitignore", "w")
-- 	if err ~= nil or file == nil then
-- 		error('could not create or open ".gitignore", error: ' .. err)
-- 	end
--
-- 	for _, b in ipairs(builds) do
-- 		file:write(b.output .. "\n")
-- 	end
-- 	file:write("build.ninja\n.ninja_logs\n.ninja_deps\n")
--
-- 	file:close()
-- end

file, err = io.open("rnj.lua", "r")

if err ~= nil or file == nil then
	error('could not open "rnj.lua", error: ' .. err)
end

file:close()

local rnj, err = loadfile("rnj.lua")

if err ~= nil then
	error('could not execute lua file "rnj.lua", error: ' .. err)
end

local success

-- Yes it matches you stupid
---@diagnostic disable-next-line: param-type-mismatch
success, err = pcall(rnj)

if not success then
	error('could not lua file "rnj.lua", error: ' .. err)
end

-- Ninja Tab
local t = "    "
local build_ninja = "ninja_required_version = 1.3\n"
for variable, default in pairs(vars) do
	build_ninja = build_ninja .. variable .. " = " .. default .. "\n"
end

for rule, options in pairs(rules) do
	build_ninja = build_ninja .. "rule " .. rule .. "\n"
	build_ninja = build_ninja .. t .. "command = " .. options.command .. "\n"
	if options.depfile ~= nil then
		build_ninja = build_ninja .. t .. "depfile = " .. options.depfile .. "\n"
	end
	if options.deps ~= nil then
		build_ninja = build_ninja .. t .. "deps = " .. options.deps .. "\n"
	end
end

for _, build in ipairs(builds) do
	build_ninja = build_ninja .. "build " .. build.output .. ": " .. build.rule
	for _, input in ipairs(build.inputs) do
		build_ninja = build_ninja .. " " .. input
	end
	build_ninja = build_ninja .. "\n"
end

print(build_ninja)

build_ninja_file = io.open("build.ninja", "w+")
build_ninja_file:write(build_ninja)
build_ninja_file:close()

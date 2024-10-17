rnj.builddir("builddir")

var("cflags", "-O2 -Wall -Wextra -g -I /usr/include/lua5.4/")
var("ldflags", "-llua5.4 -lm -lc")

rule("cc", {
	command = "gcc $cflags $in -c -o $out -MD -MF $out.d",
	description = "gcc $in",
	depfile = "$out.d",
	deps = "gcc",
})

rule("ld", {
	command = "gcc -o $out $in $ldflags",
	description = "ld $in",
})

local rnj_o = build("rnj.o", "cc", "src/rnj.c")
local args_o = build("args.o", "cc", "src/args.c")
local rdir_o = build("rdir.o", "cc", "src/rdir.c")
build("rnj", "ld", rnj_o, args_o, rdir_o)

generate_gitignore()

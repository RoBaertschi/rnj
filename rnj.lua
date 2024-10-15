var("cflags", "-O2 -Wall -Wextra -llua -lm")

rule("cc", {
	command = "gcc $cflags $in -o $out -MD -MF $out.d",
	description = "gcc $in $out",
	depfile = "$out.d",
	deps = "gcc",
})

build("rnj", "cc", "src/rnj.c")

generate_gitignore()

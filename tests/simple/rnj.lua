var("cflags", "-O2 -Wall -Wextra")

rule("cc", {
	command = "gcc $cflags $in -o $out",
	description = "gcc $in $out",
})

build("simple", "cc", "simple.c")

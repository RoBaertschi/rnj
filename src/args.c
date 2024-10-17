#include "args.h"
#include "rnj.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

enum rnj_arg_type {
    RNJ_ARG_NONE,
    RNJ_ARG_BUILDDIR,
};


void NORETURN help_and_exit(int err) {
    printf("rnj.lua [args...]\n");
    printf("args:\n");
    printf("\t--help - print this help\n");
    printf("\t-b BUILDDIR - set the builddir\n");
    exit(err);
}

rnj_args parse_args(int argc, char *argv[]) {
    rnj_args args = {0};
    int current_arg = RNJ_ARG_NONE;

    for (int i = 1; i < argc; i++) {
        if (current_arg == RNJ_ARG_BUILDDIR) {
            args.builddir = argv[i];
            current_arg = RNJ_ARG_NONE;
            continue;
        }

        if (strcmp(argv[i], "--help") == 0) {
            help_and_exit(0);
        } else if(strcmp(argv[i], "-b")) {
            if (args.builddir != NULL) {
                printf("duplicated argument -b, -b can only be specified once\n");
                help_and_exit(1);
            }

            current_arg = RNJ_ARG_BUILDDIR;
            continue;
        } else {
            printf("unexpected, unknown argument \"%s\"\n", argv[i]);
            help_and_exit(1);
        }
    }

    return args;
}

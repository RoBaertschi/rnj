#include "args.h"
#include "rnj.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

/*enum rnj_arg_type {*/
/*    RNJ_ARG_HELP,*/
/*};*/

bool cmp_strings(const char* a, const char* b) {
    if (strlen(a) != strlen(b)) {
        return false;
    }

    if (memcmp(a, b, strlen(a))) {
        return true;
    }
    return false;
}

void NORETURN help_and_exit(int err) {
    printf("rnj.lua [args...]\n");
    printf("args:\n");
    printf("\t--help - print this help\n");
    exit(err);
}

rnj_args parse_args(int argc, char *argv[]) {
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--help") == 0) {
            help_and_exit(0);
        } else {
            printf("unexpected, unknown argument \"%s\"\n", argv[i]);
            help_and_exit(1);
        }
    }

    return (rnj_args){};
}

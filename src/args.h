#ifndef RNJ_ARGS_H_
#define RNJ_ARGS_H_

typedef struct {
    // optional, can be NULL
    char* builddir;
} rnj_args;

rnj_args parse_args(int argc, char *argv[]);

#endif // RNJ_ARGS_H_

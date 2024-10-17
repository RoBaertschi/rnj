#ifndef RNJ_RNJ_H_
#define RNJ_RNJ_H_

#if defined (__GNUC__) || defined (__clang__)
#define NORETURN __attribute__ ((noreturn))
#else
#define NORETURN
#endif

#define RNJ_REGISTRY_TABLE "rnj_state"
#define RNJ_REGISTRY_BUILDDIR "builddir"
#define RNJ_REGISTRY_GENERATE_GITIGNORE "generate_gitignore"

#endif // RNJ_RNJ_H_

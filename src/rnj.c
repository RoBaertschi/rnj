#include "args.h"
#include "rdir.h"
#include <assert.h>
#include <stddef.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdlib.h>
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "rnj.h"


void NORETURN error(lua_State* L, const char* fmt, ...) {
    const char* prefix = "FATAL ERROR [rnj] %s\n";
    int str_size = snprintf(NULL, 0, prefix, fmt) + 1 /* \0 byte */;
    char* buf = malloc(str_size * sizeof(char));
    snprintf(buf, str_size, prefix, fmt);


    va_list args;
    va_start(args, fmt);
    vfprintf(stderr, buf, args);
    lua_close(L);
    exit(1);
}

int lua_panic(lua_State* L) {
    const char * msg = lua_tostring(L, 1);
    fprintf(stderr, "LUA PANIC [rnj] %s", msg);
    return 0;
}

void call_file(lua_State* L, const char* filename) {
    int result = luaL_loadfilex(L, filename, NULL);
    if (result != LUA_OK) {
        error(L, "failed to open file %s due to %s", filename, lua_tostring(L, -1));
    }
    int top = lua_gettop(L)-1;
    result = lua_pcall(L, 0, LUA_MULTRET, 0);
    if (result != LUA_OK) {
        error(L, "failed to execute the file %s due to %s", filename, lua_tostring(L, -1));
    }
    int new_top = lua_gettop(L);
    lua_pop(L, new_top-top);
}


// Reads the complete file.
char* fall(FILE* file) {
    char c = 0;
    char* buffer = malloc(1);
    size_t buffer_capacity = 1;
    size_t buffer_size = 0;
    while ((c = fgetc(file)) != EOF) {
        if (buffer_size >= buffer_capacity) {
            buffer = realloc(buffer, buffer_capacity * 2);
            buffer_capacity = buffer_capacity * 2;
        }
        buffer[buffer_size] = c;
        buffer_size += 1;
    }

    if (buffer_size >= buffer_capacity) {
        buffer = realloc(buffer, buffer_capacity + 1);
    }
    buffer[buffer_size] = '\0';

    return buffer;
}

int enable_generate_gitignore(lua_State* L) {
    luaL_getsubtable(L, LUA_REGISTRYINDEX, RNJ_REGISTRY_TABLE);
    lua_pushboolean(L, true);
    lua_setfield(L, -2, RNJ_REGISTRY_GENERATE_GITIGNORE);

    return 0;
}

int rnj_internal_is_generate_gitignore(lua_State* L) {
    luaL_getsubtable(L, LUA_REGISTRYINDEX, RNJ_REGISTRY_TABLE);
    lua_getfield(L, -1, RNJ_REGISTRY_GENERATE_GITIGNORE);

    return 1;
}

int rnj_internal_generate_gitignore(lua_State* L) {
    luaL_argcheck(L, lua_gettop(L) == 0, 1, "generate_gitignore should not get any arguments");
    lua_getglobal(L, "builds");
    luaL_checktype(L, 1, LUA_TTABLE);

    FILE* file = fopen(".gitignore", "r");

    bool full_content_allocated = file != NULL;
    char* full_content = file != NULL ? fall(file) : "";

    if (file) fclose(file);
    file = fopen(".gitignore", "w+");

    if (file == NULL) {
        lua_pushstring(L, "could not open the .gitignore file");
        lua_error(L);
    }

    const char search_string[] = "# RNJ BEGIN\n";
    const char end_string[] = "# RNJ END\n";

    int64_t search_pos = strstr(full_content, search_string) - full_content;
    int64_t end_pos = strstr(full_content, end_string) - full_content + sizeof(end_string) - 1;

    if (search_pos < 0) {
        for (size_t i = 0; i < strlen(full_content); i++) {
            fputc(full_content[i], file);
        }
        end_pos = strlen(full_content);
    } else {
        if (end_pos < 0) {
            end_pos = 0;
            for (int64_t i = 0; i < search_pos; i++) {
                end_pos++;
                fputc(full_content[i], file);
            }
        } else {
            for (int64_t i = 0; i < search_pos; i++) {
                fputc(full_content[i], file);
            }
        }
    }
    fprintf(file, "%s", search_string);

    // 2
    lua_pushnil(L);
    // 2 3
    while (lua_next(L, 1)) {
        // 4
        lua_getfield(L, 3, "output");
        // 5
        lua_pushstring(L, "\n");
        // 3
        lua_concat(L, 2);
        size_t output_size = 0;
        const char* output = lua_tolstring(L, -1, &output_size);
        lua_pop(L, 2);
        fwrite(output, sizeof(char), output_size, file);
    }

    fprintf(file, "\n.ninja_log\n.ninja_deps\n");

    fprintf(file, "%s", end_string);

    for (size_t i = end_pos; i < strlen(full_content); i++) {
        fputc(full_content[i], file);
    }

    fclose(file);
    if (full_content_allocated) {
        free(full_content);
    }

    return 0;
}

int escape(lua_State* L) {
    luaL_checktype(L, 1, LUA_TSTRING);

    size_t string_size = 0;
    const char* string = lua_tolstring(L, 1, &string_size);
    lua_pushstring(L, "");

    for (size_t i = 0; i < string_size; i++) {
        char c = string[i];
        char str[2] = {c, '\0'};

        switch (c) {
        case '$':
            lua_pushstring(L, "$$");
            break;
        case ' ':
            lua_pushstring(L, "$ ");
            break;
        case ':':
            lua_pushstring(L, "$:");
            break;
        default:
            lua_pushstring(L, str);
        }
        lua_concat(L, 2);
    }

    return 1;
}

int rnj_os_mkdir(lua_State* L) {
    const char* path = luaL_checkstring(L, 1);

    const char* err;
    if((err = rdir_mdkir(path)) != NULL) {
        luaL_error(L, "could not create path %s because of %s", path, err);
    }

    return 0;
}

int rnj_os_unlink(lua_State* L) {
    const char* path = luaL_checkstring(L, 1);

    const char* err;
    if((err = rdir_unlink(path)) != NULL) {
        luaL_error(L, "could not remove directory %s because of %s", path, err);
    }

    return 0;
}

int rnj_os_sep(lua_State* L) {
    const char sep[2] = { rdir_path_seperator(), '\0' };
    lua_pushstring(L, sep);

    return 1;
}

int rnj_os_dir_exists(lua_State* L) {
    const char* path = luaL_checkstring(L, 1);
    lua_pushboolean(L, rdir_exists(path));

    return 1;
}

int rnj_get_builddir(lua_State* L) {
    luaL_getsubtable(L, LUA_REGISTRYINDEX, RNJ_REGISTRY_TABLE);
    lua_getfield(L, 1, RNJ_REGISTRY_BUILDDIR);
    return 1;
}

int rnj_builddir(lua_State* L) {
    const char* path = luaL_checkstring(L, 1);
    char* realpath = rdir_realpath(path);
    if (realpath == NULL) {
        const char* result = rdir_mdkir(path);
        if (result != NULL) {
            luaL_error(L, "could not create \"%s\" because: %s", path, result);
        }
        realpath = rdir_realpath(path);
        if (result != NULL) {
            luaL_error(L, "could not get absolute path for \"%s\"", path);
        }
    }
    /*printf("path: %s, realpath: %s, error: %s\n", path, realpath, strerror(errno));*/
    luaL_getsubtable(L, LUA_REGISTRYINDEX, RNJ_REGISTRY_TABLE);
    lua_pushstring(L, realpath);
    free(realpath);
    lua_setfield(L, -2, RNJ_REGISTRY_BUILDDIR);
    return 0;
}

int rnj_os_realpath(lua_State* L) {
    const char* path = luaL_checkstring(L, 1);
    char* string = rdir_realpath(path);
    lua_pushstring(L, string);
    free(string);
    return 1;
}

static const luaL_Reg rnjlib[] = {
    {"get_builddir", rnj_get_builddir},
    {"builddir", rnj_builddir},
    {NULL, NULL},
};

static const luaL_Reg rnj_oslib[] = {
    {"mkdir", rnj_os_mkdir},
    {"unlink", rnj_os_unlink},
    {"sep", rnj_os_sep},
    {"dir_exists", rnj_os_dir_exists},
    {"realpath", rnj_os_realpath},
    {NULL, NULL},
};

static const luaL_Reg rnj_internallib[] = {
    {"generate_gitignore", rnj_internal_generate_gitignore},
    {"is_generate_gitignore", rnj_internal_is_generate_gitignore},
    {NULL, NULL},
};

int open_rnj(lua_State* L) {
    luaL_newlib(L, rnjlib);
    luaL_newlib(L, rnj_oslib);
    lua_setfield(L, -2, "os");
    luaL_newlib(L, rnj_internallib);
    lua_setfield(L, -2, "internal");
    return 1;
}

int setup_registry(lua_State* L) {
    lua_gettable(L, LUA_REGISTRYINDEX); // 1 + top_args
    lua_createtable(L, 0, 0); // 2 + top_args
    lua_setfield(L, 1, RNJ_REGISTRY_TABLE); // registry = { rnj_state = { builddir = nil } }

    return 0;
}

int main(int argc, char *argv[]) {
    rnj_args args = parse_args(argc, argv);
    if (args.builddir != NULL) {
        
    }

    lua_State* L = luaL_newstate();
    luaL_openlibs(L);
    lua_atpanic(L, lua_panic);

    luaL_requiref(L, "rnj", open_rnj, 1);
    lua_pop(L, 1);

    lua_pushcfunction(L, setup_registry);
    lua_call(L, 0, 0);

    lua_pushcfunction(L, enable_generate_gitignore);
    lua_setglobal(L, "generate_gitignore");
    lua_pushcfunction(L, escape);
    lua_setglobal(L, "escape");

    call_file(L, "src/rnj.lua");


    lua_close(L);
}

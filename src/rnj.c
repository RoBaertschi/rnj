#include "args.h"
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

// Requires the size of search_term_size to be bigger than 0.
int64_t find_string(const char* string, int64_t string_size, const char* search_term, int64_t search_term_size) {
    int64_t pos = 0;
    int64_t search_pos = 0;
    assert(search_term_size > 0);
    while (pos < string_size) {
        if (string[pos] == search_term[search_pos]) {
            search_pos += 1;
            if (search_pos >= search_term_size) {
                return pos - search_pos;
            }
        } else {
            search_pos = 0;
        }
        pos++;
    }

    return -1;
}

int generate_gitignore(lua_State* L) {
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

    int64_t search_pos = find_string(full_content, strlen(full_content), search_string, sizeof(search_string) / sizeof(char));
    int64_t end_pos = find_string(full_content, strlen(full_content), end_string, sizeof(end_string) / sizeof(char));

    for (int64_t i = 0; i < search_pos; i++) {
        fputc(full_content[i], file);
    }

    if (search_pos < 0) {
        fwrite(search_string, sizeof(char), sizeof(search_string), file);
    }

    // 2
    lua_pushnil(L);
    // 2 3
    while (lua_next(L, 1)) {
        // 4
        lua_getfield(L, 3, "output");
        // 5
        lua_pushstring(L, "\n");
        // 4
        lua_concat(L, 2);
        size_t output_size = 0;
        const char* output = lua_tolstring(L, -1, &output_size);
        lua_pop(L, 2);
        fwrite(output, sizeof(char), output_size, file);
    }

    fprintf(file, "build.ninja\n.ninja_logs\n.ninja_deps\n");

    if (end_pos < 0) {
        fwrite(end_string, sizeof(char), sizeof(end_string), file);
    }

    if (end_pos > 0) {
        for (size_t i = end_pos; i < strlen(full_content); i++) {
            fputc(full_content[i], file);
        }
    }

    fclose(file);
    if (full_content_allocated) {
        free(full_content);
    }

    return 0;
}

int main(int argc, char *argv[]) {
    parse_args(argc, argv);

    lua_State* L = luaL_newstate();
    luaL_openlibs(L);
    lua_atpanic(L, lua_panic);

    lua_pushcfunction(L, generate_gitignore);
    lua_setglobal(L, "generate_gitignore");

    call_file(L, "src/rnj.lua");


    lua_close(L);
}

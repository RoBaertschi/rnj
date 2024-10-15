#include <stddef.h>
#include <stdarg.h>
#include <stdlib.h>
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#if defined (__GNUC__) || defined (__clang__)
#define NORETURN __attribute__ ((noreturn))
#else
#define NORETURN
#endif

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

int generate_gitignore(lua_State* L) {
    luaL_argcheck(L, lua_gettop(L) == 0, 1, "generate_gitignore should not get any arguments");
    lua_getglobal(L, "builds");
    luaL_checktype(L, 1, LUA_TTABLE);

    FILE* file = fopen(".gitignore", "r");
    if (file == NULL) {
        lua_pushstring(L, "could not open the .gitignore file");
        lua_error(L);
    }

    char* full_content = fall(file);

    fclose(file);
    file = fopen(".gitignore", "w");
    if (file == NULL) {
        lua_pushstring(L, "could not open the .gitignore file");
        lua_error(L);
    }

    const char search_string[] = "# RNJ BEGIN\n";
    const char end_string[] = "# RNJ END\n";

    bool found = false;

    size_t pos = 0;
    size_t content_size = strlen(full_content);
    while (true) {
        if (content_size <= pos) {
            break;
        }
        if (full_content[pos] == search_string[0] && content_size - pos >= sizeof(search_string)) {
            fputc(full_content[pos], file);
            size_t pos_search_string = 1;
            pos += 1;

            while (search_string[pos_search_string] == full_content[pos] && pos_search_string < sizeof(search_string)) {
                fputc(full_content[pos], file);
                pos_search_string += 1;
                pos += 1;
            }
            if (pos_search_string >= sizeof(search_string)) {
                found = true;
                break;
            }
        } else {
            fputc(full_content[pos], file);
            pos += 1;
        }
    }


    if (!found) {
        fwrite(search_string, sizeof(char), sizeof(search_string), file);
    }

    lua_pushnil(L);
    while (lua_next(L, 1)) {
        lua_getfield(L, 1, "output");
        lua_pushstring(L, "\n");
        lua_concat(L, 2);
        size_t output_size = 0;
        const char* output = lua_tolstring(L, -1, &output_size);
        lua_pop(L, 1);
        fwrite(output, sizeof(char), output_size, file);
    }

    fprintf(file, "build.ninja\n.ninja_log\n.ninja_deps\n");

    if (!found) {
        fwrite(end_string, sizeof(char), sizeof(end_string), file);
    }

    while (true) {
        if (content_size <= pos) {
            break;
        }
        if (full_content[pos] == end_string[0] && content_size - pos >= sizeof(end_string)) {
            size_t pos_search_string = 1;
            pos += 1;

            while (end_string[pos_search_string] == full_content[pos] && pos_search_string < sizeof(end_string)) {
                pos_search_string += 1;
                pos += 1;
            }
            if (pos_search_string >= sizeof(end_string)) {
                found = true;
                break;
            }
        } else {
            pos += 1;
        }
    }

    if (!found) {
        fprintf(file, "%s", end_string);
    }

    while (content_size <= pos) {
        fputc(full_content[pos], file);
        pos++;
    }

    return 0;
}

int main(int argc, char *argv[]) {
    lua_State* L = luaL_newstate();
    luaL_openlibs(L);
    lua_atpanic(L, lua_panic);

    lua_createtable(L, argc, 0);
    for (int i = 1; i < argc; i++) {
        lua_pushstring(L, argv[i]);
        lua_rawseti(L, -2, i);
    }
    lua_setglobal(L, "arg");
    lua_pushcfunction(L, generate_gitignore);
    lua_setglobal(L, "generate_gitignore");

    call_file(L, "src/rnj.lua");
    call_file(L, "rnj.lua");


    lua_close(L);
}

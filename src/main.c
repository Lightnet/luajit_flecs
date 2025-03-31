#include <stdio.h>
#include <stdlib.h>
#include "flecs.h"
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
#include "flecs_lua.h"

static void log_lua_error(lua_State *L, const char *filename) {
    FILE *log = fopen("error.log", "a");
    if (log) {
        fprintf(log, "Lua error in %s: %s\n", filename, lua_tostring(L, -1));
        fclose(log);
    } else {
        fprintf(stderr, "Failed to open error.log\n");
    }
    fprintf(stderr, "Lua error: %s\n", lua_tostring(L, -1));
}

int main(int argc, char *argv[]) {
    const char *script_path = "script.lua";

    if (argc > 1) {
        script_path = argv[1];
        const char *ext = strrchr(script_path, '.');
        if (!ext || strcmp(ext, ".lua") != 0) {
            fprintf(stderr, "Error: Script '%s' must have a .lua extension\n", script_path);
            return 1;
        }
    }

    printf("init lua\n");
    lua_State *L = luaL_newstate();
    if (!L) {
        fprintf(stderr, "Failed to create Lua state\n");
        return 1;
    }
    printf("init luaL_openlibs\n");
    luaL_openlibs(L);

    printf("init flecs_lua\n");
    init_flecs_lua(L);

    printf("Loading script '%s'...\n", script_path);
    if (luaL_loadfile(L, script_path) != LUA_OK) {
        log_lua_error(L, script_path);
        lua_close(L);
        return 1;
    }

    printf("init lua_pcall\n");
    int status = lua_pcall(L, 0, 0, 0);
    if (status != LUA_OK) {
        log_lua_error(L, script_path);
        lua_close(L);
        return 1;
    }

    printf("lua_close\n");
    lua_close(L);

    printf("Test completed successfully.\n");
    return 0;
}
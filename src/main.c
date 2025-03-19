#include <stdio.h>
#include <string.h>
#include <flecs.h>
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
#include "flecs_lua.h"
#include "flecs_lua_ctypes.h" // Include the new header

// Custom error handler for Lua
static int lua_error_handler(lua_State *L) {
    const char *msg = lua_tostring(L, -1);
    printf("Lua Error: %s\n", msg ? msg : "Unknown error");
    luaL_traceback(L, L, msg, 1);
    printf("%s\n", lua_tostring(L, -1));
    lua_pop(L, 2);
    return 0;
}

int main(int argc, char *argv[]) {
    lua_State *L = luaL_newstate();
    if (L == NULL) {
        printf("Failed to create Lua state\n");
        return 1;
    }

    luaL_openlibs(L);

    lua_getglobal(L, "require");
    lua_pushstring(L, "ffi");
    if (lua_pcall(L, 1, 1, 0) != LUA_OK) {
        printf("Debug Error: Failed to require 'ffi': %s\n", lua_tostring(L, -1));
        lua_close(L);
        return 1;
    }
    lua_setglobal(L, "ffi");
    lua_getglobal(L, "ffi");
    if (!lua_istable(L, -1)) {
        printf("Debug Error: FFI library not loaded after require\n");
        lua_close(L);
        return 1;
    }
    printf("FFI library loaded successfully\n");

    // Register FFI types
    if (flecs_lua_register_ctypes(L) != 0) {
        lua_close(L);
        return 1;
    }

    // Test the type definition
    lua_getglobal(L, "ffi");
    lua_getfield(L, -1, "new");
    lua_pushstring(L, "l_point_t");
    if (lua_pcall(L, 1, 1, 0) != LUA_OK) {
        printf("Debug Error: Failed to create l_point_t: %s\n", lua_tostring(L, -1));
        lua_close(L);
        return 1;
    }
    lua_getglobal(L, "tostring");
    lua_pushvalue(L, -2);
    lua_call(L, 1, 1);
    printf("Test l_point_t from C: %s\n", lua_tostring(L, -1));
    lua_pop(L, 2);

    lua_getglobal(L, "_VERSION");
    const char *lua_version = lua_tostring(L, -1);
    printf("LuaJIT Version: %s\n", lua_version ? lua_version : "Unknown");
    lua_pop(L, 1);

    lua_getglobal(L, "print");
    if (!lua_isfunction(L, -1)) {
        printf("Debug Error: Standard library 'print' not loaded\n");
        lua_close(L);
        return 1;
    }
    lua_pop(L, 1);

    lua_getglobal(L, "package");
    lua_getfield(L, -1, "preload");
    lua_pushcfunction(L, luaopen_flecs);
    lua_setfield(L, -2, "flecs");
    lua_pop(L, 2);
    lua_getglobal(L, "require");
    lua_pushstring(L, "flecs");
    if (lua_pcall(L, 1, 1, 0) != LUA_OK) {
        printf("Debug Error: Failed to preload Flecs module: %s\n", lua_tostring(L, -1));
        lua_close(L);
        return 1;
    }
    printf("Flecs module preloaded successfully\n");
    lua_pop(L, 1);

    lua_pushcfunction(L, lua_error_handler);
    int err_handler_idx = lua_gettop(L);

    const char *lua_file = "main.lua";
    if (argc > 1) {
        const char *ext = strrchr(argv[1], '.');
        if (ext && strcmp(ext, ".lua") == 0) {
            lua_file = argv[1];
        } else {
            printf("Warning: '%s' does not have a .lua extension, using default 'main.lua'\n", argv[1]);
        }
    }

    if (luaL_loadfile(L, lua_file) != LUA_OK) {
        printf("Error loading script: %s\n", lua_tostring(L, -1));
        lua_close(L);
        return 1;
    }

    if (lua_pcall(L, 0, 0, err_handler_idx) != LUA_OK) {
        lua_close(L);
        return 1;
    }

    lua_close(L);
    return 0;
}
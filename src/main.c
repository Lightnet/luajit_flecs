#include <stdio.h>
#include <string.h>
#include <flecs.h>
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
#include "flecs_lua.h"

// Custom error handler for Lua
static int lua_error_handler(lua_State *L) {
    const char *msg = lua_tostring(L, -1);
    printf("Lua Error: %s\n", msg ? msg : "Unknown error");
    luaL_traceback(L, L, msg, 1); // Add stack traceback
    printf("%s\n", lua_tostring(L, -1));
    lua_pop(L, 2); // Remove traceback and error message
    return 0; // No values returned to Lua
}

int main(int argc, char *argv[]) {
    // Initialize Lua state
    lua_State *L = luaL_newstate();
    if (L == NULL) {
        printf("Failed to create Lua state\n");
        return 1;
    }

    // Load standard libraries (includes package)
    luaL_openlibs(L);

    // Load and verify FFI
    lua_getglobal(L, "require");
    lua_pushstring(L, "ffi");
    if (lua_pcall(L, 1, 1, 0) != LUA_OK) {
        printf("Debug Error: Failed to require 'ffi': %s\n", lua_tostring(L, -1));
        lua_close(L);
        return 1;
    }
    lua_setglobal(L, "ffi"); // Make ffi global
    lua_getglobal(L, "ffi");
    if (!lua_istable(L, -1)) {
        printf("Debug Error: FFI library not loaded after require\n");
        lua_close(L);
        return 1;
    }
    printf("FFI library loaded successfully\n");

    // Define l_point_t via ffi.cdef with explicit struct name
    lua_getfield(L, -1, "cdef");
    lua_pushstring(L, "struct l_point_t { double x, y; }; typedef struct l_point_t l_point_t;");
    if (lua_pcall(L, 1, 0, 0) != LUA_OK) {
        printf("Debug Error: Failed to define l_point_t: %s\n", lua_tostring(L, -1));
        lua_close(L);
        return 1;
    }
    printf("l_point_t defined via ffi.cdef in C\n");

    // Test the type definition
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
    lua_pop(L, 2); // Remove tostring result and cdata

    lua_pop(L, 1); // Remove ffi table

    // Debug check: Verify LuaJIT version
    lua_getglobal(L, "_VERSION");
    const char *lua_version = lua_tostring(L, -1);
    printf("LuaJIT Version: %s\n", lua_version ? lua_version : "Unknown");
    lua_pop(L, 1);

    // Debug check: Verify standard libraries
    lua_getglobal(L, "print");
    if (!lua_isfunction(L, -1)) {
        printf("Debug Error: Standard library 'print' not loaded\n");
        lua_close(L);
        return 1;
    }
    lua_pop(L, 1);

    // Register Flecs module
    lua_getglobal(L, "package");
    lua_getfield(L, -1, "preload");
    lua_pushcfunction(L, luaopen_flecs);
    lua_setfield(L, -2, "flecs");
    lua_pop(L, 2); // Remove preload and package tables
    lua_getglobal(L, "require");
    lua_pushstring(L, "flecs");
    if (lua_pcall(L, 1, 1, 0) != LUA_OK) {
        printf("Debug Error: Failed to preload Flecs module: %s\n", lua_tostring(L, -1));
        lua_close(L);
        return 1;
    }
    printf("Flecs module preloaded successfully\n");
    lua_pop(L, 1); // Remove flecs table

    // Set up error handler
    lua_pushcfunction(L, lua_error_handler);
    int err_handler_idx = lua_gettop(L);

    // Determine which Lua file to run
    const char *lua_file = "main.lua"; // Default file
    if (argc > 1) {
        const char *ext = strrchr(argv[1], '.');
        if (ext && strcmp(ext, ".lua") == 0) {
            lua_file = argv[1];
        } else {
            printf("Warning: '%s' does not have a .lua extension, using default 'main.lua'\n", argv[1]);
        }
    }

    // Load and run the Lua file with error handler
    if (luaL_loadfile(L, lua_file) != LUA_OK) {
        printf("Error loading script: %s\n", lua_tostring(L, -1));
        lua_close(L);
        return 1;
    }

    if (lua_pcall(L, 0, 0, err_handler_idx) != LUA_OK) {
        // Error handler already printed the message
        lua_close(L);
        return 1;
    }

    // Clean up
    lua_close(L);
    return 0;
}
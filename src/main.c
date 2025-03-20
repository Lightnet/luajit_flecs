#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
#include <stdio.h>

extern int luaopen_flecs_lua(lua_State *L);

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
    if (!L) {
        fprintf(stderr, "Failed to create Lua state\n");
        return 1;
    }
    
    luaL_openlibs(L);
    
    // Preload the flecs_lua module into package.preload
    lua_getglobal(L, "package");
    lua_getfield(L, -1, "preload");
    lua_pushcfunction(L, luaopen_flecs_lua);
    lua_setfield(L, -2, "flecs_lua");
    lua_pop(L, 2); // Pop package.preload and package tables

    // Push error handler onto stack
    lua_pushcfunction(L, lua_error_handler);
    int err_handler_idx = lua_gettop(L);

    // Run the script
    const char *script = (argc > 1) ? argv[1] : "main.lua";
    if (luaL_loadfile(L, script) != LUA_OK) {
        fprintf(stderr, "Error loading script '%s': %s\n", script, lua_tostring(L, -1));
        lua_pop(L, 1);
        lua_close(L);
        return 1;
    }

    if (lua_pcall(L, 0, LUA_MULTRET, err_handler_idx) != LUA_OK) {
        // Error already printed by lua_error_handler
        lua_close(L);
        return 1;
    }
    
    lua_close(L);
    return 0;
}
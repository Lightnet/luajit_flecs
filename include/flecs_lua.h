#ifndef FLECS_LUA_H
#define FLECS_LUA_H

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
#include "flecs.h"

// Register the Flecs Lua module
int luaopen_ecs(lua_State *L);

// Initialize the Lua state with Flecs bindings (optional, if not fully managed in Lua)
void init_flecs_lua(lua_State *L);

#endif // FLECS_LUA_H
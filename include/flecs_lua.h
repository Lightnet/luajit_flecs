#ifndef FLECS_LUA_H
#define FLECS_LUA_H

// Metatable name for ecs_world_t
//#define ECS_WORLD_MT "ecs_world_t"

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
#include "flecs.h"

// Register the Flecs Lua module
int luaopen_flecs_lua(lua_State *L);

#endif // FLECS_LUA_H
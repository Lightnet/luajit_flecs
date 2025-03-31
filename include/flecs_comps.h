#ifndef FLECS_COMPS_H
#define FLECS_COMPS_H

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
#include "flecs.h"

// Define the Position component structure
typedef struct {
    float x, y, z;
} Position;

// Define the Velocity component structure
typedef struct {
    float x, y, z;
} Velocity;

// Component IDs
extern ecs_entity_t PositionId;
extern ecs_entity_t VelocityId;

// Register components with Flecs and set IDs
void register_flecs_components(ecs_world_t *world);

// Lua binding functions (exposed for use in flecs_lua.c)
int lua_ecs_new(lua_State *L);
int lua_ecs_check_type(lua_State *L);
int lua_ecs_set(lua_State *L);
int lua_ecs_get(lua_State *L);
int lua_ecs_system(lua_State *L);

#endif // FLECS_COMPS_H
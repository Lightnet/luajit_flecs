#ifndef FLECS_LUA_COMPS_H
#define FLECS_LUA_COMPS_H

// Metatable names
#define FLECS_WORLD_MT "FlecsWorld"
#define FLECS_ENTITY_MT "FlecsEntity"
#define FLECS_COMPONENT_MT "FlecsComponent"

#include "flecs.h"
#include <lua.h>
#include <lauxlib.h>

typedef struct {
    float x;
    float y;
} Position;

typedef struct {
    float x;
    float y;
} Velocity;

// Global component IDs
extern ecs_entity_t PositionId;
extern ecs_entity_t VelocityId;

// Component registration helper
void register_flecs_components(ecs_world_t *world);

// Lua helper functions
int push_position(lua_State *L, Position *pos);
Position* check_position(lua_State *L, int idx);
int push_velocity(lua_State *L, Velocity *vel);
Velocity* check_velocity(lua_State *L, int idx);

#endif
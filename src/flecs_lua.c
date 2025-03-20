#include "flecs_lua_comps.h"
#include <stdio.h>

// Global variables
static ecs_world_t *global_world = NULL;
ecs_entity_t PositionId = 0;
ecs_entity_t VelocityId = 0;

// Hard-coded move system
static void move_sys(ecs_iter_t *it) {
    // printf("move_sys called, entity count: %d\n", it->count);
    Position *p = ecs_field(it, Position, 0);
    Velocity *v = ecs_field(it, Velocity, 1);

    // char *type_str = ecs_table_str(it->world, it->table);
    // printf("Move entities with [%s]\n", type_str);
    // ecs_os_free(type_str);

    for (int i = 0; i < it->count; i++) {
        p[i].x += v[i].x * it->delta_time;
        p[i].y += v[i].y * it->delta_time;
        // printf("Entity %llu: Pos (%.1f, %.1f) updated with Vel (%.1f, %.1f)\n",
        //        (unsigned long long)it->entities[i], p[i].x, p[i].y, v[i].x, v[i].y);
    }
}

// Component helpers implementation
void register_flecs_components(ecs_world_t *world) {
  PositionId = ecs_component_init(world, &(ecs_component_desc_t){
      .entity = ecs_entity(world, { .name = "Position" }),
      .type.size = sizeof(Position),
      .type.alignment = ECS_ALIGNOF(Position)
  });
  VelocityId = ecs_component_init(world, &(ecs_component_desc_t){
      .entity = ecs_entity(world, { .name = "Velocity" }),
      .type.size = sizeof(Velocity),
      .type.alignment = ECS_ALIGNOF(Velocity)
  });

  ecs_system_desc_t sys_desc = {0};
  sys_desc.entity = ecs_entity(world, { .name = "MoveSystem" });
  ecs_add_id(world, sys_desc.entity, ecs_dependson(EcsOnUpdate));
  sys_desc.query.terms[0].id = PositionId;
  sys_desc.query.terms[1].id = VelocityId;
  sys_desc.callback = move_sys;
  ecs_system_init(world, &sys_desc);
}

int push_position(lua_State *L, Position *pos) {
    lua_newtable(L);
    lua_pushnumber(L, pos->x);
    lua_setfield(L, -2, "x");
    lua_pushnumber(L, pos->y);
    lua_setfield(L, -2, "y");
    
    luaL_getmetatable(L, "Position");
    lua_setmetatable(L, -2);
    return 1;
}

Position* check_position(lua_State *L, int idx) {
    Position *pos = lua_newuserdata(L, sizeof(Position));
    luaL_getmetatable(L, "Position");
    lua_setmetatable(L, -2);
    
    lua_getfield(L, idx, "x");
    pos->x = luaL_checknumber(L, -1);
    lua_pop(L, 1);
    
    lua_getfield(L, idx, "y");
    pos->y = luaL_checknumber(L, -1);
    lua_pop(L, 1);
    
    return pos;
}

int push_velocity(lua_State *L, Velocity *vel) {
    lua_newtable(L);
    lua_pushnumber(L, vel->x);
    lua_setfield(L, -2, "x");
    lua_pushnumber(L, vel->y);
    lua_setfield(L, -2, "y");
    
    luaL_getmetatable(L, "Velocity");
    lua_setmetatable(L, -2);
    return 1;
}

Velocity* check_velocity(lua_State *L, int idx) {
    Velocity *vel = lua_newuserdata(L, sizeof(Velocity));
    luaL_getmetatable(L, "Velocity");
    lua_setmetatable(L, -2);
    
    lua_getfield(L, idx, "x");
    vel->x = luaL_checknumber(L, -1);
    lua_pop(L, 1);
    
    lua_getfield(L, idx, "y");
    vel->y = luaL_checknumber(L, -1);
    lua_pop(L, 1);
    
    return vel;
}

// Lua bindings
static int l_init(lua_State *L) {
    printf("check\n");
    if (global_world) {
        ecs_fini(global_world);
    }
    printf("init world\n");
    global_world = ecs_init();
    printf("lua_pushlightuserdata\n");
    lua_pushlightuserdata(L, global_world);
    printf("register_flecs_components\n");
    register_flecs_components(global_world);
    return 1;
}

static int l_ecs_new(lua_State *L) {
    ecs_world_t *world = lua_touserdata(L, 1);
    ecs_entity_t e = ecs_new(world);
    printf("Created entity: %llu\n", (unsigned long long)e);
    lua_pushinteger(L, e);
    return 1;
}

static int l_progress(lua_State *L) {
    ecs_world_t *world = lua_touserdata(L, 1);
    float delta_time = luaL_optnumber(L, 2, 0.0f);
    printf("Progressing world with delta_time: %.1f\n", delta_time);
    ecs_progress(world, delta_time);
    return 0;
}

static int l_set(lua_State *L) {
    printf("set comp...\n");
    ecs_world_t *world = lua_touserdata(L, 1);
    ecs_entity_t e = lua_tointeger(L, 2);
    const char *comp_name = luaL_checkstring(L, 3);
    printf("name: %s\n", comp_name);
    
    if (strcmp(comp_name, "Position") == 0) {
        Position *pos = check_position(L, 4);
        ecs_set_id(world, e, PositionId, sizeof(Position), pos);
        printf("Set Position on entity %llu: (%.1f, %.1f)\n", 
               (unsigned long long)e, pos->x, pos->y);
    } else if (strcmp(comp_name, "Velocity") == 0) {
        Velocity *vel = check_velocity(L, 4);
        ecs_set_id(world, e, VelocityId, sizeof(Velocity), vel);
        printf("Set Velocity on entity %llu: (%.1f, %.1f)\n", 
               (unsigned long long)e, vel->x, vel->y);
    }
    
    return 0;
}

static int l_get(lua_State *L) {
    ecs_world_t *world = lua_touserdata(L, 1);
    ecs_entity_t e = lua_tointeger(L, 2);
    const char *comp_name = luaL_checkstring(L, 3);
    
    if (strcmp(comp_name, "Position") == 0) {
        const Position *pos = ecs_get_id(world, e, PositionId);
        if (pos) {
            push_position(L, (Position*)pos);
            printf("Get Position for entity %llu: (%.1f, %.1f)\n", 
                   (unsigned long long)e, pos->x, pos->y);
            return 1;
        } else {
            lua_pushnil(L);
            printf("No Position found for entity %llu\n", (unsigned long long)e);
            return 1;
        }
    } else if (strcmp(comp_name, "Velocity") == 0) {
        const Velocity *vel = ecs_get_id(world, e, VelocityId);
        if (vel) {
            push_velocity(L, (Velocity*)vel);
            printf("Get Velocity for entity %llu: (%.1f, %.1f)\n", 
                   (unsigned long long)e, vel->x, vel->y);
            return 1;
        } else {
            lua_pushnil(L);
            printf("No Velocity found for entity %llu\n", (unsigned long long)e);
            return 1;
        }
    }
    
    lua_pushnil(L);
    return 1;
}

static int l_component(lua_State *L) {
    ecs_world_t *world = lua_touserdata(L, 1);
    const char *name = luaL_checkstring(L, 2);
    lua_pushstring(L, name);
    return 1;
}

// Module registration
static const luaL_Reg flecs_funcs[] = {
    {"init", l_init},
    {"ecs_new", l_ecs_new},
    {"progress", l_progress},
    {"set", l_set},
    {"get", l_get},
    {"component", l_component},
    {NULL, NULL}
};

int luaopen_flecs_lua(lua_State *L) {
    luaL_newmetatable(L, "Position");
    lua_pop(L, 1);
    luaL_newmetatable(L, "Velocity");
    lua_pop(L, 1);
    
    luaL_newlib(L, flecs_funcs);
    return 1;
}
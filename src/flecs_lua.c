#include "flecs_lua.h"
#include "flecs_comps.h"

static int lua_ecs_world_gc(lua_State *L) {
    ecs_world_t *world = *(ecs_world_t**)luaL_checkudata(L, 1, "ecs_world");
    ecs_fini(world);
    return 0;
}

static int lua_ecs_new_world(lua_State *L) {
    ecs_world_t *world = ecs_init();
    ecs_world_t **world_ptr = (ecs_world_t**)lua_newuserdata(L, sizeof(ecs_world_t*));
    *world_ptr = world;

    register_flecs_components(world);

    ecs_set_ctx(world, L, NULL);  // Set Lua state as world context

    luaL_getmetatable(L, "ecs_world");
    if (lua_isnil(L, -1)) {
        lua_pop(L, 1);
        luaL_newmetatable(L, "ecs_world");
        lua_pushcfunction(L, lua_ecs_world_gc);
        lua_setfield(L, -2, "__gc");
    }
    lua_setmetatable(L, -2);

    // Store the world userdata in the registry
    lua_pushvalue(L, -1);  // Duplicate the world userdata
    lua_setfield(L, LUA_REGISTRYINDEX, "flecs_world");  // Store as "flecs_world"

    return 1;
}

static int lua_ecs_entity(lua_State *L) {
    ecs_world_t *world = *(ecs_world_t**)luaL_checkudata(L, 1, "ecs_world");
    ecs_entity_t e = ecs_new(world);
    lua_pushinteger(L, e);
    return 1;
}

static int lua_ecs_progress(lua_State *L) {
    ecs_world_t *world = *(ecs_world_t**)luaL_checkudata(L, 1, "ecs_world");
    float delta_time = (float)luaL_optnumber(L, 2, 0.0);
    ecs_progress(world, delta_time);
    return 0;
}

static const struct luaL_Reg ecs_lib[] = {
    {"new_world", lua_ecs_new_world},
    {"entity", lua_ecs_entity},
    {"new", lua_ecs_new},
    {"check_type", lua_ecs_check_type},
    {"set", lua_ecs_set},
    {"get", lua_ecs_get},
    {"ecs_system", lua_ecs_system},
    {"progress", lua_ecs_progress},
    {NULL, NULL}
};

int luaopen_ecs(lua_State *L) {
    luaL_newlib(L, ecs_lib);
    return 1;
}

void init_flecs_lua(lua_State *L) {
    lua_getglobal(L, "package");
    lua_getfield(L, -1, "preload");
    lua_pushcfunction(L, luaopen_ecs);
    lua_setfield(L, -2, "ecs");
    lua_pop(L, 2);
}
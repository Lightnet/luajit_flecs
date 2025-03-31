#include "flecs_comps.h"

// Component IDs
ecs_entity_t PositionId = 0;
ecs_entity_t VelocityId = 0;

// --- Position Bindings ---
static int lua_position_index(lua_State *L) {
    ComponentPtr *cp = (ComponentPtr *)luaL_checkudata(L, 1, "Position");
    Position *pos = (Position *)cp->ptr;
    const char *field = luaL_checkstring(L, 2);
    if (strcmp(field, "x") == 0) {
        lua_pushnumber(L, pos->x);
    } else if (strcmp(field, "y") == 0) {
        lua_pushnumber(L, pos->y);
    } else if (strcmp(field, "z") == 0) {
        lua_pushnumber(L, pos->z);
    } else {
        lua_pushnil(L);
    }
    return 1;
}

static int lua_position_newindex(lua_State *L) {
    ComponentPtr *cp = (ComponentPtr *)luaL_checkudata(L, 1, "Position");
    Position *pos = (Position *)cp->ptr;
    const char *field = luaL_checkstring(L, 2);
    float value = (float)luaL_checknumber(L, 3);
    if (strcmp(field, "x") == 0) {
        pos->x = value;
    } else if (strcmp(field, "y") == 0) {
        pos->y = value;
    } else if (strcmp(field, "z") == 0) {
        pos->z = value;
    } else {
        luaL_error(L, "Unknown field '%s' in Position", field);
    }
    return 0;
}

static int lua_position_gc(lua_State *L) {
    ComponentPtr *cp = (ComponentPtr *)luaL_checkudata(L, 1, "Position");
    if (cp->is_owned) {
        free(cp->ptr);
    }
    return 0;
}

// --- Velocity Bindings ---
static int lua_velocity_index(lua_State *L) {
    ComponentPtr *cp = (ComponentPtr *)luaL_checkudata(L, 1, "Velocity");
    Velocity *vel = (Velocity *)cp->ptr;
    const char *field = luaL_checkstring(L, 2);
    if (strcmp(field, "x") == 0) {
        lua_pushnumber(L, vel->x);
    } else if (strcmp(field, "y") == 0) {
        lua_pushnumber(L, vel->y);
    } else if (strcmp(field, "z") == 0) {
        lua_pushnumber(L, vel->z);
    } else {
        lua_pushnil(L);
    }
    return 1;
}

static int lua_velocity_newindex(lua_State *L) {
    ComponentPtr *cp = (ComponentPtr *)luaL_checkudata(L, 1, "Velocity");
    Velocity *vel = (Velocity *)cp->ptr;
    const char *field = luaL_checkstring(L, 2);
    float value = (float)luaL_checknumber(L, 3);
    if (strcmp(field, "x") == 0) {
        vel->x = value;
    } else if (strcmp(field, "y") == 0) {
        vel->y = value;
    } else if (strcmp(field, "z") == 0) {
        vel->z = value;
    } else {
        luaL_error(L, "Unknown field '%s' in Velocity", field);
    }
    return 0;
}

static int lua_velocity_gc(lua_State *L) {
    ComponentPtr *cp = (ComponentPtr *)luaL_checkudata(L, 1, "Velocity");
    if (cp->is_owned) {
        free(cp->ptr);
    }
    return 0;
}

// --- Unified Constructor ---
int lua_ecs_new(lua_State *L) {
    const char *type = luaL_checkstring(L, 1);
    ComponentPtr *cp;

    if (strcmp(type, "Position") == 0) {
        cp = (ComponentPtr *)lua_newuserdata(L, sizeof(ComponentPtr));
        cp->ptr = malloc(sizeof(Position));
        cp->is_owned = 1;
        Position *pos = (Position *)cp->ptr;
        pos->x = 0.0f;
        pos->y = 0.0f;
        pos->z = 0.0f;

        luaL_getmetatable(L, "Position");
        if (lua_isnil(L, -1)) {
            lua_pop(L, 1);
            luaL_newmetatable(L, "Position");
            lua_pushcfunction(L, lua_position_index);
            lua_setfield(L, -2, "__index");
            lua_pushcfunction(L, lua_position_newindex);
            lua_setfield(L, -2, "__newindex");
            lua_pushcfunction(L, lua_position_gc);
            lua_setfield(L, -2, "__gc");
        }
        lua_setmetatable(L, -2);
    } else if (strcmp(type, "Velocity") == 0) {
        cp = (ComponentPtr *)lua_newuserdata(L, sizeof(ComponentPtr));
        cp->ptr = malloc(sizeof(Velocity));
        cp->is_owned = 1;
        Velocity *vel = (Velocity *)cp->ptr;
        vel->x = 0.0f;
        vel->y = 0.0f;
        vel->z = 0.0f;

        luaL_getmetatable(L, "Velocity");
        if (lua_isnil(L, -1)) {
            lua_pop(L, 1);
            luaL_newmetatable(L, "Velocity");
            lua_pushcfunction(L, lua_velocity_index);
            lua_setfield(L, -2, "__index");
            lua_pushcfunction(L, lua_velocity_newindex);
            lua_setfield(L, -2, "__newindex");
            lua_pushcfunction(L, lua_velocity_gc);
            lua_setfield(L, -2, "__gc");
        }
        lua_setmetatable(L, -2);
    } else {
        luaL_error(L, "Unknown component type: '%s'", type);
    }

    return 1;
}

// --- Type Checking ---
int lua_ecs_check_type(lua_State *L) {
    if (luaL_testudata(L, 1, "Position")) {
        lua_pushstring(L, "Position");
    } else if (luaL_testudata(L, 1, "Velocity")) {
        lua_pushstring(L, "Velocity");
    } else if (luaL_testudata(L, 1, "ecs_world")) {
        lua_pushstring(L, "ecs_world");
    } else {
        lua_pushnil(L);
    }
    return 1;
}

// --- Set Component ---
int lua_ecs_set(lua_State *L) {
    ecs_world_t *world = *(ecs_world_t**)luaL_checkudata(L, 1, "ecs_world");
    ecs_entity_t entity = (ecs_entity_t)luaL_checkinteger(L, 2);
    ComponentPtr *cp = (ComponentPtr *)lua_touserdata(L, 3);

    if (luaL_testudata(L, 3, "Position")) {
        ecs_set_id(world, entity, PositionId, sizeof(Position), cp->ptr);
    } else if (luaL_testudata(L, 3, "Velocity")) {
        ecs_set_id(world, entity, VelocityId, sizeof(Velocity), cp->ptr);
    } else {
        luaL_error(L, "Invalid component type for ecs.set");
    }

    return 0;
}

// --- Get Component ---
int lua_ecs_get(lua_State *L) {
    ecs_world_t *world = *(ecs_world_t**)luaL_checkudata(L, 1, "ecs_world");
    ecs_entity_t entity = (ecs_entity_t)luaL_checkinteger(L, 2);
    const char *type = luaL_checkstring(L, 3);

    if (strcmp(type, "Position") == 0) {
        Position *pos = (Position *)ecs_get_id(world, entity, PositionId);
        if (pos) {
            ComponentPtr *cp = (ComponentPtr *)lua_newuserdata(L, sizeof(ComponentPtr));
            cp->ptr = pos;
            cp->is_owned = 0;  // Flecs owns this memory
            luaL_getmetatable(L, "Position");
            lua_setmetatable(L, -2);
            return 1;
        }
    } else if (strcmp(type, "Velocity") == 0) {
        Velocity *vel = (Velocity *)ecs_get_id(world, entity, VelocityId);
        if (vel) {
            ComponentPtr *cp = (ComponentPtr *)lua_newuserdata(L, sizeof(ComponentPtr));
            cp->ptr = vel;
            cp->is_owned = 0;  // Flecs owns this memory
            luaL_getmetatable(L, "Velocity");
            lua_setmetatable(L, -2);
            return 1;
        }
    } else {
        luaL_error(L, "Unknown component type: '%s'", type);
    }

    lua_pushnil(L);
    return 1;
}

// --- System Callback ---
static void lua_system_callback(ecs_iter_t *it) {
    lua_State *L = (lua_State *)ecs_get_ctx(it->world);
    if (!L) return;

    // Retrieve the function reference from the system's context
    lua_rawgeti(L, LUA_REGISTRYINDEX, (int)(intptr_t)it->ctx);

    // Create an iterator table
    lua_newtable(L);

    // Add delta_time
    lua_pushnumber(L, it->delta_time);
    lua_setfield(L, -2, "delta_time");

    // Add world from registry
    lua_getfield(L, LUA_REGISTRYINDEX, "flecs_world");
    lua_setfield(L, -2, "world");

    // Populate entity IDs
    lua_newtable(L);
    for (int i = 0; i < it->count; i++) {
        lua_pushinteger(L, i + 1);
        lua_pushinteger(L, it->entities[i]);
        lua_settable(L, -3);
    }
    lua_setfield(L, -2, "entities");

    // Populate Position components (index 0 in terms)
    Position *positions = ecs_field(it, Position, 0);
    lua_newtable(L);
    for (int i = 0; i < it->count; i++) {
        lua_pushinteger(L, i + 1);
        ComponentPtr *cp = (ComponentPtr *)lua_newuserdata(L, sizeof(ComponentPtr));
        cp->ptr = &positions[i];
        cp->is_owned = 0;  // Flecs owns this memory
        luaL_getmetatable(L, "Position");
        lua_setmetatable(L, -2);
        lua_settable(L, -3);
    }
    lua_setfield(L, -2, "Position");

    // Populate Velocity components (index 1 in terms)
    Velocity *velocities = ecs_field(it, Velocity, 1);
    lua_newtable(L);
    for (int i = 0; i < it->count; i++) {
        lua_pushinteger(L, i + 1);
        ComponentPtr *cp = (ComponentPtr *)lua_newuserdata(L, sizeof(ComponentPtr));
        cp->ptr = &velocities[i];
        cp->is_owned = 0;  // Flecs owns this memory
        luaL_getmetatable(L, "Velocity");
        lua_setmetatable(L, -2);
        lua_settable(L, -3);
    }
    lua_setfield(L, -2, "Velocity");

    // Call the Lua function with the iterator table
    if (lua_pcall(L, 1, 0, 0) != LUA_OK) {
        fprintf(stderr, "System error: %s\n", lua_tostring(L, -1));
        lua_pop(L, 1);
    }
}

// --- System Binding ---
int lua_ecs_system(lua_State *L) {
    ecs_world_t *world = *(ecs_world_t**)luaL_checkudata(L, 1, "ecs_world");
    luaL_checktype(L, 2, LUA_TFUNCTION);  // Ensure second arg is a function
    const char *comp1 = luaL_checkstring(L, 3);
    const char *comp2 = luaL_checkstring(L, 4);

    // Store the Lua function in the registry and get a reference
    lua_pushvalue(L, 2);  // Duplicate the function
    int func_ref = luaL_ref(L, LUA_REGISTRYINDEX);  // Store in registry, get reference

    // Define the system with both Position and Velocity
    ecs_system_desc_t desc = {
        .entity = ecs_entity(world, { 
            .name = "LuaSystem",
            .add = ecs_ids(ecs_dependson(EcsOnUpdate))
        }),
        .query.terms = {
            { .id = PositionId },
            { .id = VelocityId }
        },
        .callback = lua_system_callback,
        .ctx = (void *)(intptr_t)func_ref
    };
    ecs_entity_t system = ecs_system_init(world, &desc);

    return 0;
}

void register_flecs_components(ecs_world_t *world) {
    printf("Registering components with Flecs\n");
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
}
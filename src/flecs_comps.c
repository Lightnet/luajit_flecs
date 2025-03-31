#include "flecs_comps.h"

// Component IDs
ecs_entity_t PositionId = 0;
ecs_entity_t VelocityId = 0;

// --- Position Bindings ---
static int lua_position_index(lua_State *L) {
    Position *pos = (Position *)luaL_checkudata(L, 1, "Position");
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
    Position *pos = (Position *)luaL_checkudata(L, 1, "Position");
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

// --- Velocity Bindings ---
static int lua_velocity_index(lua_State *L) {
    Velocity *vel = (Velocity *)luaL_checkudata(L, 1, "Velocity");
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
    Velocity *vel = (Velocity *)luaL_checkudata(L, 1, "Velocity");
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

// --- Unified Constructor ---
int lua_ecs_new(lua_State *L) {
    const char *type = luaL_checkstring(L, 1);

    if (strcmp(type, "Position") == 0) {
        Position *pos = (Position *)lua_newuserdata(L, sizeof(Position));
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
        }
        lua_setmetatable(L, -2);
    } else if (strcmp(type, "Velocity") == 0) {
        Velocity *vel = (Velocity *)lua_newuserdata(L, sizeof(Velocity));
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
    void *component = lua_touserdata(L, 3);

    if (luaL_testudata(L, 3, "Position")) {
        ecs_set_id(world, entity, PositionId, sizeof(Position), component);
    } else if (luaL_testudata(L, 3, "Velocity")) {
        ecs_set_id(world, entity, VelocityId, sizeof(Velocity), component);
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
        const Position *pos = ecs_get_id(world, entity, PositionId);
        if (pos) {
            Position *new_pos = (Position *)lua_newuserdata(L, sizeof(Position));
            *new_pos = *pos;
            luaL_getmetatable(L, "Position");
            lua_setmetatable(L, -2);
            return 1;
        }
    } else if (strcmp(type, "Velocity") == 0) {
        const Velocity *vel = ecs_get_id(world, entity, VelocityId);
        if (vel) {
            Velocity *new_vel = (Velocity *)lua_newuserdata(L, sizeof(Velocity));
            *new_vel = *vel;
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
        lua_newtable(L);
        lua_pushnumber(L, positions[i].x);
        lua_setfield(L, -2, "x");
        lua_pushnumber(L, positions[i].y);
        lua_setfield(L, -2, "y");
        lua_pushnumber(L, positions[i].z);
        lua_setfield(L, -2, "z");
        lua_settable(L, -3);
    }
    lua_setfield(L, -2, "Position");

    // Populate Velocity components (index 1 in terms)
    Velocity *velocities = ecs_field(it, Velocity, 1);
    lua_newtable(L);
    for (int i = 0; i < it->count; i++) {
        lua_pushinteger(L, i + 1);
        lua_newtable(L);
        lua_pushnumber(L, velocities[i].x);
        lua_setfield(L, -2, "x");
        lua_pushnumber(L, velocities[i].y);
        lua_setfield(L, -2, "y");
        lua_pushnumber(L, velocities[i].z);
        lua_setfield(L, -2, "z");
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
            .add = ecs_ids(ecs_dependson(EcsOnUpdate))  // Run on update
        }),
        .query.terms = {  // v4.x
            { .id = PositionId },
            { .id = VelocityId }
        },
        .callback = lua_system_callback,
        .ctx = (void *)(intptr_t)func_ref  // Store function reference as context
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
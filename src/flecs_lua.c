#include "flecs_lua.h"
#include "lauxlib.h"
#include "lualib.h"
#include <stdio.h>
#include "flecs_lua_ctypes.h"

// Metatable name for ecs_world_t
#define ECS_WORLD_MT "ecs_world_t"

#define DEBUG 1 // Set to 0 to disable debug output

#ifdef DEBUG
#define DEBUG_PRINT(...) printf(__VA_ARGS__)
#else
#define DEBUG_PRINT(...)
#endif



// Lua function: flecs.new_world()
static int l_flecs_new_world(lua_State *L) {
    ecs_world_t *world = ecs_init();
    ecs_world_t **world_ud = (ecs_world_t **)lua_newuserdata(L, sizeof(ecs_world_t *));
    *world_ud = world;
    luaL_getmetatable(L, ECS_WORLD_MT);
    if (lua_isnil(L, -1)) {
        lua_pop(L, 1);
        luaL_newmetatable(L, ECS_WORLD_MT);
        lua_pushstring(L, "__index");
        lua_pushvalue(L, -2);
        lua_settable(L, -3);
    }
    lua_setmetatable(L, -2);
    return 1;
}

// Lua function: flecs.delete_world(world)
static int l_flecs_delete_world(lua_State *L) {
    ecs_world_t **world_ud = (ecs_world_t **)luaL_checkudata(L, 1, ECS_WORLD_MT);
    if (*world_ud) {
        ecs_fini(*world_ud);
        *world_ud = NULL;
    }
    return 0;
}

static int lua_ecs_component_init_name(lua_State *L) {
  ecs_world_t **world_ud = (ecs_world_t **)luaL_checkudata(L, 1, ECS_WORLD_MT);
  ecs_world_t *world = *world_ud;
  const char *type_name = luaL_checkstring(L, 2);

  const FlecsComponentType *type = flecs_lua_get_component_type(type_name);
  if (!type) {
      return luaL_error(L, "Unknown component type: %s", type_name);
  }

  ecs_entity_t existing_id = ecs_lookup(world, type_name);
  if (existing_id != 0) {
      printf("Component '%s' already registered with ID: %llu\n", type_name, (unsigned long long)existing_id);
  } else {
      printf("Registering new component '%s':\n", type_name);
  }

  ecs_entity_t comp_id = ecs_component_init(world, &(ecs_component_desc_t){
      .entity = ecs_entity(world, {.name = type_name}),
      .type.size = type->size,
      .type.alignment = type->alignment
  });

  char reg_key[32];
  snprintf(reg_key, sizeof(reg_key), "Lua_%s_ID", type_name);
  lua_pushinteger(L, comp_id);
  lua_setfield(L, LUA_REGISTRYINDEX, reg_key);
  return 0;
}


static int lua_ecs_component_init_velocity(lua_State *L) {
  ecs_world_t **world_ud = (ecs_world_t **)luaL_checkudata(L, 1, ECS_WORLD_MT);
  ecs_world_t *world = *world_ud;

  l_velocity_t vel_buffer;
  l_velocity_t *vel = (l_velocity_t *)flecs_lua_extract_cdata(L, 2, "l_velocity_t*", sizeof(l_velocity_t), &vel_buffer, "ecs_component_init_velocity");
  if (!vel) {
      return luaL_error(L, "Failed to extract l_velocity_t cdata");
  }

  ecs_entity_t existing_id = ecs_lookup(world, "l_velocity_t");
  if (existing_id != 0) {
      printf("Component 'l_velocity_t' already registered with ID: %llu\n", (unsigned long long)existing_id);
  } else {
      printf("Registering new component 'l_velocity_t':\n");
  }

  printf("Component Name: l_velocity_t\n");
  printf("Type: l_velocity_t (size: %zu, alignment: %zu)\n", sizeof(l_velocity_t), ECS_ALIGNOF(l_velocity_t));
  printf("Sample values: vx = %f, vy = %f\n", vel->vx, vel->vy);

  ecs_entity_t comp_id = ecs_component_init(world, &(ecs_component_desc_t){
      .entity = ecs_entity(world, {.name = "l_velocity_t"}),
      .type.size = sizeof(l_velocity_t),
      .type.alignment = ECS_ALIGNOF(l_velocity_t)
  });

  lua_pushinteger(L, comp_id);
  lua_setfield(L, LUA_REGISTRYINDEX, "Lua_Velocity_ID");
  return 0;
}

// Lua function: lua_ecs_set(world, entity, ffi_component)
static int lua_ecs_set(lua_State *L) {
  ecs_world_t **world_ud = (ecs_world_t **)luaL_checkudata(L, 1, ECS_WORLD_MT);
  ecs_world_t *world = *world_ud;
  ecs_entity_t entity = luaL_checkinteger(L, 2);
  const char *type_name = luaL_checkstring(L, 3);

  const FlecsComponentType *type = flecs_lua_get_component_type(type_name);
  if (!type) {
      return luaL_error(L, "Unknown component type: %s", type_name);
  }

  void *buffer = malloc(type->size);
  if (!buffer) {
      return luaL_error(L, "Failed to allocate memory for %s", type_name);
  }

  char type_name_ptr[32];
  snprintf(type_name_ptr, sizeof(type_name_ptr), "%s*", type_name);
  if (!flecs_lua_extract_cdata(L, 4, type_name_ptr, type->size, buffer, "ecs_set")) {
      free(buffer);
      return luaL_error(L, "Expected %s cdata", type_name);
  }

  char reg_key[32];
  snprintf(reg_key, sizeof(reg_key), "Lua_%s_ID", type_name);
  lua_getfield(L, LUA_REGISTRYINDEX, reg_key);
  ecs_entity_t comp_id = lua_tointeger(L, -1);
  lua_pop(L, 1);

  printf("Setting %s component for entity %llu:\n", type_name, (unsigned long long)entity);
  printf("Type: %s (size: %zu, alignment: %zu)\n", type_name, type->size, type->alignment);
  if (strcmp(type_name, "l_point_t") == 0) {
      l_point_t *point = (l_point_t *)buffer;
      printf("Values: x = %f, y = %f\n", point->x, point->y);
  } else if (strcmp(type_name, "l_velocity_t") == 0) {
      l_velocity_t *vel = (l_velocity_t *)buffer;
      printf("Values: vx = %f, vy = %f\n", vel->vx, vel->vy);
  } else if (strcmp(type_name, "l_acceleration_t") == 0) {
      l_acceleration_t *acc = (l_acceleration_t *)buffer;
      DEBUG_PRINT("Values: ax = %f, ay = %f\n", acc->ax, acc->ay);
  }

  ecs_set_id(world, entity, comp_id, type->size, buffer);
  free(buffer);
  return 0;
}


// Lua function: lua_ecs_get(world, entity)
static int lua_ecs_get(lua_State *L) {
  ecs_world_t **world_ud = (ecs_world_t **)luaL_checkudata(L, 1, ECS_WORLD_MT);
  ecs_world_t *world = *world_ud;
  ecs_entity_t entity = luaL_checkinteger(L, 2);
  const char *type_name = luaL_checkstring(L, 3);

  const FlecsComponentType *type = flecs_lua_get_component_type(type_name);
  if (!type) {
      return luaL_error(L, "Unknown component type: %s", type_name);
  }

  char reg_key[32];
  snprintf(reg_key, sizeof(reg_key), "Lua_%s_ID", type_name);
  lua_getfield(L, LUA_REGISTRYINDEX, reg_key);
  ecs_entity_t comp_id = lua_tointeger(L, -1);
  lua_pop(L, 1);

  const void *value = ecs_get_id(world, entity, comp_id);
  if (!value) {
      lua_pushnil(L);
      return 1;
  }

  printf("Getting %s component for entity %llu:\n", type_name, (unsigned long long)entity);
  printf("Type: %s (size: %zu, alignment: %zu)\n", type_name, type->size, type->alignment);
  if (strcmp(type_name, "l_point_t") == 0) {
      const l_point_t *point = (const l_point_t *)value;
      printf("Values: x = %f, y = %f\n", point->x, point->y);
  } else if (strcmp(type_name, "l_velocity_t") == 0) {
      const l_velocity_t *vel = (const l_velocity_t *)value;
      printf("Values: vx = %f, vy = %f\n", vel->vx, vel->vy);
  }

  lua_getglobal(L, "ffi");
  lua_getfield(L, -1, "new");
  lua_pushstring(L, type_name);
  if (lua_pcall(L, 1, 1, 0) != LUA_OK) {
      printf("FFI new error: %s\n", lua_tostring(L, -1));
      luaL_error(L, "failed to create %s cdata", type_name);
  }

  lua_getglobal(L, "ffi");
  lua_getfield(L, -1, "copy");
  lua_pushvalue(L, -3);
  lua_pushlightuserdata(L, (void *)value);
  lua_pushinteger(L, type->size);
  if (lua_pcall(L, 3, 0, 0) != LUA_OK) {
      printf("FFI copy error: %s\n", lua_tostring(L, -1));
      luaL_error(L, "failed to copy %s data", type_name);
  }

  lua_pop(L, 1);
  return 1;
}



// Lua function: flecs.new_entity(world)
static int l_flecs_new_entity(lua_State *L) {
    ecs_world_t **world_ud = (ecs_world_t **)luaL_checkudata(L, 1, ECS_WORLD_MT);
    ecs_world_t *world = *world_ud;
    ecs_entity_t e = ecs_new(world);
    lua_pushinteger(L, e);
    return 1;
}

// Lua function: flecs.progress(world, delta_time)
static int l_flecs_progress(lua_State *L) {
    ecs_world_t **world_ud = (ecs_world_t **)luaL_checkudata(L, 1, ECS_WORLD_MT);
    ecs_world_t *world = *world_ud;
    float delta_time = (float)luaL_optnumber(L, 2, 0.0);
    ecs_progress(world, delta_time);
    return 0;
}

// Lua function: flecs.list_registry()
static int l_flecs_list_registry(lua_State *L) {
    printf("Listing Lua registry contents:\n");
    lua_pushvalue(L, LUA_REGISTRYINDEX);
    lua_pushnil(L);
    while (lua_next(L, -2) != 0) {
        printf("Key: ");
        switch (lua_type(L, -2)) {
            case LUA_TNUMBER: printf("%g", lua_tonumber(L, -2)); break;
            case LUA_TSTRING: printf("\"%s\"", lua_tostring(L, -2)); break;
            case LUA_TLIGHTUSERDATA: printf("lightuserdata: %p", lua_touserdata(L, -2)); break;
            case LUA_TTABLE: printf("table: %p", lua_topointer(L, -2)); break;
            default: printf("%s: %p", lua_typename(L, lua_type(L, -2)), lua_topointer(L, -2)); break;
        }
        printf(", Value: ");
        switch (lua_type(L, -1)) {
            case LUA_TNUMBER: printf("%g", lua_tonumber(L, -1)); break;
            case LUA_TSTRING: printf("\"%s\"", lua_tostring(L, -1)); break;
            case LUA_TLIGHTUSERDATA: printf("lightuserdata: %p", lua_touserdata(L, -1)); break;
            case LUA_TTABLE: printf("table: %p", lua_topointer(L, -1)); break;
            case LUA_TUSERDATA: printf("userdata: %p", lua_touserdata(L, -1)); break;
            default: printf("%s: %p", lua_typename(L, lua_type(L, -1)), lua_topointer(L, -1)); break;
        }
        printf("\n");
        lua_pop(L, 1);
    }
    lua_pop(L, 1);
    return 0;
}

// Lua function: flecs.list_lua_types()
static int l_flecs_list_lua_types(lua_State *L) {
    lua_newtable(L);
    lua_pushstring(L, "nil"); lua_pushinteger(L, LUA_TNIL); lua_settable(L, -3);
    lua_pushstring(L, "number"); lua_pushinteger(L, LUA_TNUMBER); lua_settable(L, -3);
    lua_pushstring(L, "boolean"); lua_pushinteger(L, LUA_TBOOLEAN); lua_settable(L, -3);
    lua_pushstring(L, "string"); lua_pushinteger(L, LUA_TSTRING); lua_settable(L, -3);
    lua_pushstring(L, "table"); lua_pushinteger(L, LUA_TTABLE); lua_settable(L, -3);
    lua_pushstring(L, "function"); lua_pushinteger(L, LUA_TFUNCTION); lua_settable(L, -3);
    lua_pushstring(L, "userdata"); lua_pushinteger(L, LUA_TUSERDATA); lua_settable(L, -3);
    lua_pushstring(L, "thread"); lua_pushinteger(L, LUA_TTHREAD); lua_settable(L, -3);
    lua_pushstring(L, "lightuserdata"); lua_pushinteger(L, LUA_TLIGHTUSERDATA); lua_settable(L, -3);
    return 1;
}

// Table of functions to register with Lua
static const luaL_Reg flecs_funcs[] = {
    {"new_world", l_flecs_new_world},
    {"delete_world", l_flecs_delete_world},
    {"lua_ecs_component_init_name", lua_ecs_component_init_name},
    {"lua_ecs_component_init_velocity", lua_ecs_component_init_velocity},
    {"lua_ecs_set", lua_ecs_set},
    {"lua_ecs_get", lua_ecs_get},
    {"new_entity", l_flecs_new_entity},
    {"progress", l_flecs_progress},
    {"list_registry", l_flecs_list_registry},
    {"list_lua_types", l_flecs_list_lua_types},
    {NULL, NULL}
};

// Entry point for the Lua module
int luaopen_flecs(lua_State *L) {
    luaL_newlib(L, flecs_funcs);
    return 1;
}
#include "flecs_lua.h"
#include "lauxlib.h"
#include "lualib.h"
#include <stdio.h>

// Structure for l_point_t (must match Lua's ffi.cdef)
typedef struct {
    double x;
    double y;
} l_point_t;

// Metatable name for ecs_world_t
#define ECS_WORLD_MT "ecs_world_t"

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


static int lua_ecs_component_init(lua_State *L) {
  ecs_world_t **world_ud = (ecs_world_t **)luaL_checkudata(L, 1, ECS_WORLD_MT);
  ecs_world_t *world = *world_ud;

  // Log the raw type and type name
  int type = lua_type(L, 2);
  const char *type_name = lua_typename(L, type);
  printf("ecs_component_init: arg type=%d, type_name=%s\n", type, type_name ? type_name : "nil");

  // Check if the second argument is a cdata (FFI type)
  if (type != 10) { // 10 is LuaJIT's cdata type ID
      luaL_argerror(L, 2, "expected FFI cdata struct");
  }

  // Debug: Inspect the original cdata
  printf("Original cdata: ");
  lua_getglobal(L, "tostring");
  lua_pushvalue(L, 2);
  lua_call(L, 1, 1);
  printf("%s\n", lua_tostring(L, -1));
  lua_pop(L, 1);

  // Use ffi.cast to get the pointer
  lua_getglobal(L, "ffi");
  lua_getfield(L, -1, "cast");
  lua_pushstring(L, "l_point_t*");
  lua_pushvalue(L, 2); // The cdata argument
  if (lua_pcall(L, 2, 1, 0) != LUA_OK) {
      printf("FFI cast error: %s\n", lua_tostring(L, -1));
      luaL_argerror(L, 2, "expected l_point_t FFI cdata struct");
  }

  // Debug: Inspect the casted result
  printf("Casted cdata: ");
  lua_getglobal(L, "tostring");
  lua_pushvalue(L, -2);
  lua_call(L, 1, 1);
  printf("%s\n", lua_tostring(L, -1));
  lua_pop(L, 1);

  // Extract the pointer using ffi.copy or direct access
  l_point_t point_data;
  lua_getfield(L, -2, "copy");
  lua_pushlightuserdata(L, &point_data); // Destination
  lua_pushvalue(L, -3); // Source (casted cdata)
  lua_pushinteger(L, sizeof(l_point_t));
  if (lua_pcall(L, 3, 0, 0) != LUA_OK) {
      printf("FFI copy error: %s\n", lua_tostring(L, -1));
      luaL_argerror(L, 2, "failed to copy l_point_t data");
  }
  l_point_t *point = &point_data;

  printf("Extracted pointer: %p\n", (void *)point);

  // Clean up stack
  lua_pop(L, 2); // Remove cast result and ffi table

  // Log raw value for debugging
  printf("Argument 2 raw value: %p\n", (void *)point);

  // Check if the component is already registered
  ecs_entity_t existing_id = ecs_lookup(world, "l_point_t");
  if (existing_id != 0) {
      printf("Component 'l_point_t' already registered with ID: %llu\n", (unsigned long long)existing_id);
  } else {
      printf("Registering new component 'l_point_t':\n");
  }

  // Log type and name details
  printf("Component Name: l_point_t\n");
  printf("Type: l_point_t (size: %zu, alignment: %zu)\n", sizeof(l_point_t), ECS_ALIGNOF(l_point_t));
  printf("Sample values: x = %f, y = %f\n", point->x, point->y);
  printf("Lua Type: %s\n", type_name);

  // Register the component
  ecs_entity_t comp_id = ecs_component_init(world, &(ecs_component_desc_t){
      .entity = ecs_entity(world, {.name = "l_point_t"}),
      .type.size = sizeof(l_point_t),
      .type.alignment = ECS_ALIGNOF(l_point_t)
  });

  lua_pushinteger(L, comp_id);
  lua_setfield(L, LUA_REGISTRYINDEX, "Lua_Point_ID");
  return 0;
}


// Lua function: lua_ecs_set(world, entity, ffi_component)

static int lua_ecs_set(lua_State *L) {
  ecs_world_t **world_ud = (ecs_world_t **)luaL_checkudata(L, 1, ECS_WORLD_MT);
  ecs_world_t *world = *world_ud;
  ecs_entity_t entity = luaL_checkinteger(L, 2);

  if (lua_type(L, 3) != 10) { // Check for cdata
      luaL_argerror(L, 3, "expected FFI cdata struct");
  }

  // Use ffi.cast to get the pointer
  lua_getglobal(L, "ffi");
  lua_getfield(L, -1, "cast");
  lua_pushstring(L, "l_point_t*");
  lua_pushvalue(L, 3); // The cdata argument
  if (lua_pcall(L, 2, 1, 0) != LUA_OK) {
      printf("FFI cast error: %s\n", lua_tostring(L, -1));
      luaL_argerror(L, 3, "expected l_point_t FFI cdata struct");
  }

  // Extract the data using ffi.copy
  l_point_t point_data;
  lua_getfield(L, -2, "copy");
  lua_pushlightuserdata(L, &point_data); // Destination
  lua_pushvalue(L, -3); // Source (casted cdata)
  lua_pushinteger(L, sizeof(l_point_t));
  if (lua_pcall(L, 3, 0, 0) != LUA_OK) {
      printf("FFI copy error: %s\n", lua_tostring(L, -1));
      luaL_argerror(L, 3, "failed to copy l_point_t data");
  }
  l_point_t *point = &point_data;

  // Clean up stack
  lua_pop(L, 2); // Remove cast result and ffi table

  lua_getfield(L, LUA_REGISTRYINDEX, "Lua_Point_ID");
  ecs_entity_t comp_id = lua_tointeger(L, -1);
  lua_pop(L, 1);

  printf("Setting l_point_t component for entity %llu:\n", (unsigned long long)entity);
  printf("Type: l_point_t (size: %zu, alignment: %zu)\n", sizeof(l_point_t), ECS_ALIGNOF(l_point_t));
  printf("Values: x = %f, y = %f\n", point->x, point->y);

  ecs_set_id(world, entity, comp_id, sizeof(l_point_t), point);
  return 0;
}


// Lua function: lua_ecs_get(world, entity)
static int lua_ecs_get(lua_State *L) {
  ecs_world_t **world_ud = (ecs_world_t **)luaL_checkudata(L, 1, ECS_WORLD_MT);
  ecs_world_t *world = *world_ud;
  ecs_entity_t entity = luaL_checkinteger(L, 2);

  lua_getfield(L, LUA_REGISTRYINDEX, "Lua_Point_ID");
  ecs_entity_t comp_id = lua_tointeger(L, -1);
  lua_pop(L, 1);

  const void *value = ecs_get_id(world, entity, comp_id);
  if (!value) {
      lua_pushnil(L);
      return 1;
  }

  const l_point_t *point = (const l_point_t *)value;

  printf("Getting l_point_t component for entity %llu:\n", (unsigned long long)entity);
  printf("Type: l_point_t (size: %zu, alignment: %zu)\n", sizeof(l_point_t), ECS_ALIGNOF(l_point_t));
  printf("Values: x = %f, y = %f\n", point->x, point->y);

  // Create an FFI cdata instead of plain userdata
  lua_getglobal(L, "ffi");
  lua_getfield(L, -1, "new");
  lua_pushstring(L, "l_point_t");
  if (lua_pcall(L, 1, 1, 0) != LUA_OK) {
      printf("FFI new error: %s\n", lua_tostring(L, -1));
      luaL_error(L, "failed to create l_point_t cdata");
  }

  // Copy the retrieved data into the cdata
  lua_getglobal(L, "ffi");
  lua_getfield(L, -1, "copy");
  lua_pushvalue(L, -3); // The new cdata
  lua_pushlightuserdata(L, (void *)point); // Source (const l_point_t *)
  lua_pushinteger(L, sizeof(l_point_t));
  if (lua_pcall(L, 3, 0, 0) != LUA_OK) {
      printf("FFI copy error: %s\n", lua_tostring(L, -1));
      luaL_error(L, "failed to copy l_point_t data");
  }

  // Clean up stack (leave only the cdata)
  lua_pop(L, 1); // Remove ffi table
  // cdata is now at the top of the stack

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
    {"lua_ecs_component_init", lua_ecs_component_init},
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
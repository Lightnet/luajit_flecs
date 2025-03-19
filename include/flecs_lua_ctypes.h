// flecs_lua_ctypes.h
#ifndef FLECS_LUA_CTYPES_H
#define FLECS_LUA_CTYPES_H

#include "lua.h"
#include "lauxlib.h"
#include <stdio.h>
#include <string.h>
#include <flecs.h>

// Structure definitions
typedef struct {
    double x;
    double y;
} l_point_t;

typedef struct {
    float vx;
    float vy;
} l_velocity_t;

typedef struct {
  float ax;
  float ay;
} l_acceleration_t;

// Component type descriptor
typedef struct {
    const char *name;
    size_t size;
    size_t alignment;
} FlecsComponentType;



// Registry of known component types (terminated by {NULL, 0, 0})
static const FlecsComponentType flecs_component_types[] = {
  {"l_point_t", sizeof(l_point_t), ECS_ALIGNOF(l_point_t)},
  {"l_velocity_t", sizeof(l_velocity_t), ECS_ALIGNOF(l_velocity_t)},
  {"l_acceleration_t", sizeof(l_acceleration_t), ECS_ALIGNOF(l_acceleration_t)},
  {NULL, 0, 0}
};

// Lookup a component type by name
static const FlecsComponentType *flecs_lua_get_component_type(const char *type_name) {
    for (int i = 0; flecs_component_types[i].name != NULL; i++) {
        if (strcmp(flecs_component_types[i].name, type_name) == 0) {
            return &flecs_component_types[i];
        }
    }
    return NULL;
}

typedef void * (*FlecsLuaCDataExtractor)(lua_State *L, int arg_idx, const char *type_name, size_t size, void *buffer, const char *func_name);

static void *flecs_lua_extract_cdata(lua_State *L, int arg_idx, const char *type_name, size_t size, void *buffer, const char *func_name) {
    int type = lua_type(L, arg_idx);
    const char *lua_type_name = lua_typename(L, type);
    printf("%s: arg type=%d, type_name=%s\n", func_name, type, lua_type_name ? lua_type_name : "nil");

    if (type != 10) {
        luaL_argerror(L, arg_idx, "expected FFI cdata struct");
        return NULL;
    }

    printf("%s: Original cdata: ", func_name);
    lua_getglobal(L, "tostring");
    lua_pushvalue(L, arg_idx);
    lua_call(L, 1, 1);
    printf("%s\n", lua_tostring(L, -1));
    lua_pop(L, 1);

    lua_getglobal(L, "ffi");
    lua_getfield(L, -1, "cast");
    lua_pushstring(L, type_name);
    lua_pushvalue(L, arg_idx);
    if (lua_pcall(L, 2, 1, 0) != LUA_OK) {
        printf("%s: FFI cast error: %s\n", func_name, lua_tostring(L, -1));
        luaL_argerror(L, arg_idx, "expected FFI cdata struct of specified type");
        return NULL;
    }

    printf("%s: Casted cdata: ", func_name);
    lua_getglobal(L, "tostring");
    lua_pushvalue(L, -2);
    lua_call(L, 1, 1);
    printf("%s\n", lua_tostring(L, -1));
    lua_pop(L, 1);

    lua_getfield(L, -2, "copy");
    lua_pushlightuserdata(L, buffer);
    lua_pushvalue(L, -3);
    lua_pushinteger(L, size);
    if (lua_pcall(L, 3, 0, 0) != LUA_OK) {
        printf("%s: FFI copy error: %s\n", func_name, lua_tostring(L, -1));
        luaL_argerror(L, arg_idx, "failed to copy cdata");
        return NULL;
    }

    lua_pop(L, 2);
    printf("%s: Extracted pointer: %p\n", func_name, buffer);
    return buffer;
}

static int flecs_lua_register_ctypes(lua_State *L) {
    lua_getglobal(L, "ffi");
    if (!lua_istable(L, -1)) {
        printf("Error: FFI not loaded before registering ctypes\n");
        lua_pop(L, 1);
        return 1;
    }

    lua_getfield(L, -1, "cdef");
    // In flecs_lua_register_ctypes
    lua_pushstring(L, "struct l_point_t { double x, y; }; typedef struct l_point_t l_point_t;"
                      "struct l_velocity_t { float vx, vy; }; typedef struct l_velocity_t l_velocity_t;"
                      "struct l_acceleration_t { float ax, ay; }; typedef struct l_acceleration_t l_acceleration_t;");
    if (lua_pcall(L, 1, 0, 0) != LUA_OK) {
        printf("Debug Error: Failed to define ctypes: %s\n", lua_tostring(L, -1));
        lua_pop(L, 2);
        return 1;
    }
    printf("l_point_t and l_velocity_t defined via ffi.cdef in C\n");

    lua_pop(L, 1);
    return 0;
}

#endif // FLECS_LUA_CTYPES_H
// flecs_lua_ctypes.h
#ifndef FLECS_LUA_CTYPES_H
#define FLECS_LUA_CTYPES_H

#include "lua.h"
#include "lauxlib.h"
#include <stdio.h>

// Structure for l_point_t (must match Lua's ffi.cdef)
typedef struct {
    double x;
    double y;
} l_point_t;

typedef void * (*FlecsLuaCDataExtractor)(lua_State *L, int arg_idx, const char *type_name, size_t size, void *buffer, const char *func_name);

/**
 * Extracts a C struct from a LuaJIT FFI cdata object into a provided buffer.
 * @param L Lua state.
 * @param arg_idx Stack index of the cdata argument.
 * @param type_name FFI type name (e.g., "l_point_t*").
 * @param size Size of the struct in bytes.
 * @param buffer Caller-provided buffer to store the extracted data.
 * @param func_name Name of the calling function for debug logging.
 * @return Pointer to the filled buffer, or NULL on error.
 */
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
    lua_pushlightuserdata(L, buffer); // Use provided buffer
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

/**
 * Registers FFI type definitions with LuaJIT.
 * @param L Lua state.
 * @return 0 on success, 1 on failure.
 */
static int flecs_lua_register_ctypes(lua_State *L) {
    lua_getglobal(L, "ffi");
    if (!lua_istable(L, -1)) {
        printf("Error: FFI not loaded before registering ctypes\n");
        lua_pop(L, 1);
        return 1;
    }

    lua_getfield(L, -1, "cdef");
    lua_pushstring(L, "struct l_point_t { double x, y; }; typedef struct l_point_t l_point_t;");
    if (lua_pcall(L, 1, 0, 0) != LUA_OK) {
        printf("Debug Error: Failed to define l_point_t: %s\n", lua_tostring(L, -1));
        lua_pop(L, 2);
        return 1;
    }
    printf("l_point_t defined via ffi.cdef in C\n");

    lua_pop(L, 1);
    return 0;
}

#endif // FLECS_LUA_CTYPES_H
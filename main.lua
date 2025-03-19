print("Running main.lua from embedded LuaJIT 2.1!")

-- Test Flecs integration
local flecs = require "flecs"
print("Flecs module loaded:", flecs)

-- Load FFI
local ffi = require("ffi")
print("FFI loaded:", ffi)

-- Test the type
local test_point = ffi.new("l_point_t")
print("Test l_point_t:", tostring(test_point))

-- Create a new Flecs world
local world = flecs.new_world()
print("Flecs world created:", world)

-- Register l_point_t with a default instance
flecs.lua_ecs_component_init(world, ffi.new("l_point_t"))

-- Create an entity and set l_point_t
local entity = flecs.new_entity(world)
local lpos = ffi.new("l_point_t")
lpos.x = 10
lpos.y = 20
flecs.lua_ecs_set(world, entity, lpos)
print("Entity created and set with l_point_t:", entity)

-- Retrieve l_point_t
local retrieved_point = flecs.lua_ecs_get(world, entity)
if retrieved_point then
    print("Retrieved l_point_t: x =", retrieved_point.x, "y =", retrieved_point.y)
else
    print("Failed to retrieve l_point_t")
end

-- Progress the world
flecs.progress(world, 0.1)

-- Clean up
flecs.delete_world(world)
print("Flecs world deleted")
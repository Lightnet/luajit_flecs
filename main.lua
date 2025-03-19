print("Running main.lua from embedded LuaJIT 2.1!")

local flecs = require "flecs"
print("Flecs module loaded:", flecs)

local ffi = require("ffi")
print("FFI loaded:", ffi)

local function set_component(world, entity, type_name, data)
  local cdata = ffi.new(type_name)
  if type(data) == "table" then
      if #data > 0 then -- Positional table
          if type_name == "l_point_t" and #data == 2 then
              cdata.x = data[1]
              cdata.y = data[2]
          elseif type_name == "l_velocity_t" and #data == 2 then
              cdata.vx = data[1]
              cdata.vy = data[2]
          else
              error("Invalid positional table size for " .. type_name)
          end
      else -- Named fields
          for k, v in pairs(data) do
              if cdata[k] ~= nil then -- Check if field exists
                  cdata[k] = v
              else
                  error("Unknown field '" .. k .. "' for " .. type_name)
              end
          end
      end
  end
  flecs.lua_ecs_set(world, entity, type_name, cdata)
end

local world = flecs.new_world()
print("Flecs world created:", world)

flecs.lua_ecs_component_init_name(world, "l_point_t")
flecs.lua_ecs_component_init_name(world, "l_velocity_t")
flecs.lua_ecs_component_init_name(world, "l_acceleration_t")

local entity = flecs.new_entity(world)

-- Test both named and positional tables
set_component(world, entity, "l_point_t", {x=10, y=20})
print("Entity created and set with l_point_t (named):", entity)

set_component(world, entity, "l_velocity_t", {1.5, 2.5})
print("Entity set with l_velocity_t (positional):", entity)

set_component(world, entity, "l_acceleration_t", {ax=0.5, ay=0.8})
local retrieved_acc = flecs.lua_ecs_get(world, entity, "l_acceleration_t")
if retrieved_acc then
    print("Retrieved l_acceleration_t: ax =", retrieved_acc.ax, "ay =", retrieved_acc.ay)
end

local retrieved_point = flecs.lua_ecs_get(world, entity, "l_point_t")
if retrieved_point then
    print("Retrieved l_point_t: x =", retrieved_point.x, "y =", retrieved_point.y)
else
    print("Failed to retrieve l_point_t")
end

local retrieved_vel = flecs.lua_ecs_get(world, entity, "l_velocity_t")
if retrieved_vel then
    print("Retrieved l_velocity_t: vx =", retrieved_vel.vx, "vy =", retrieved_vel.vy)
else
    print("Failed to retrieve l_velocity_t")
end

flecs.progress(world, 0.1)
flecs.delete_world(world)
print("Flecs world deleted")
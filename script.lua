print("Loading ecs module...")
local ecs = require 'ecs'
print("ecs module loaded")

-- Create a new Flecs world
local world = ecs.new_world()
print("World created")

-- Create an entity
local entity = ecs.entity(world)
print("Entity created: " .. tostring(entity))

-- Create and set a Position component
local pos = ecs.new("Position")
pos.x = 10.0
ecs.set(world, entity, pos)
print("Set Position x: " .. pos.x)

-- Create and set a Velocity component
local vel = ecs.new("Velocity")
vel.x = 5.0
ecs.set(world, entity, vel)
print("Set Velocity x: " .. vel.x)

-- Define a system
function test(it)
    local updates = {}
    updates.Position = {}
    for i, entity in ipairs(it.entities) do
        local p = it.Position[i]
        local v = it.Velocity[i]
        print("Entity " .. entity .. ": Pos x=" .. p.x .. ", Vel x=" .. v.x)
        
        -- Update Position based on Velocity and delta_time
        updates.Position[i] = {
            x = p.x + v.x * it.delta_time,
            y = p.y + v.y * it.delta_time,
            z = p.z + v.z * it.delta_time
        }
    end
    return updates
end

ecs.ecs_system(world, test, "Position", "Velocity")

-- Run the world for 5 frames with 0.1s delta time
for i = 1, 5 do
    ecs.progress(world, 0.1)
end

-- Verify the final Position
local final_pos = ecs.get(world, entity, "Position")
print("Final Position x: " .. final_pos.x)
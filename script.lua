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
    for i, entity in ipairs(it.entities) do
        local p = it.Position[i]
        local v = it.Velocity[i]
        print("Entity " .. entity .. ": Pos x=" .. p.x .. ", Vel x=" .. v.x)
    end
end

ecs.ecs_system(world, test, "Position", "Velocity")

-- Run the world once to trigger the system
ecs.progress(world, 0)
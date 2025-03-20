print("init")
local flecs = require("flecs_lua")
local world = flecs.init()
print("init entity")
local e = flecs.ecs_new(world)
flecs.set(world, e, "Position", {x = 10, y = 20})
flecs.set(world, e, "Velocity", {x = 1.5, y = 2.5})

-- Helper function to convert table to string
local function table_to_string(t)
    if not t then return "nil" end
    return string.format("{x=%.1f, y=%.1f}", t.x, t.y)
end
print("get entity")
-- Get and print initial values
local pos = flecs.get(world, e, "Position")
local vel = flecs.get(world, e, "Velocity")
print("Initial Position: " .. table_to_string(pos))
print("Initial Velocity: " .. table_to_string(vel))

for i = 1, 6 do
    flecs.progress(world, 1.0)
    pos = flecs.get(world, e, "Position")
    vel = flecs.get(world, e, "Velocity")
    print(string.format("Step %d - Position: %s, Velocity: %s", i, table_to_string(pos), table_to_string(vel)))
end
print("finish")
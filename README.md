# luajit_flecs

# License

MIT License - See LICENSE for details.

# Information

luajit_flecs is a prototype integration of the [Flecs ECS library](https://github.com/SanderMertens/flecs) (v4.x) with LuaJIT, designed to explore embedding an Entity-Component-System (ECS) framework into a Lua scripting environment without relying on LuaJIT’s FFI (Foreign Function Interface). This project serves as a proof-of-concept for bridging C-based ECS functionality with Lua’s dynamic scripting capabilities.

# Goals
- No FFI: Avoid using LuaJIT’s FFI to maintain control over the C-to-Lua boundary and ensure compatibility with standard Lua if needed.
- Metatable-Based Components: Use Lua metatables to wrap C structs (Position, Velocity) for intuitive access in Lua (e.g., pos.x).
- Bidirectional Communication: Enable Lua to create, read, and modify Flecs components, with changes reflected back in the ECS world.
- Systems in Lua: Allow Lua functions to define Flecs systems, processing entities and components dynamically.

# Approach

This prototype wraps Flecs functionality in a custom C API, exposing it to Lua via a module (ecs). It handles the translation between C structs and Lua userdata, using metatables for type safety and ease of use. The code has been refined iteratively to address challenges like type checking, system callbacks, and component modification.

# Current State

- Components: Supports Position and Velocity components with x, y, z fields, registered with Flecs and accessible in Lua.
- Entities: Lua can create entities and attach/get components using ecs.set and ecs.get.
- Systems: Lua functions can define systems that iterate over entities with specific components, with support for modifying components during execution.
- Limitations: The wrapper adds some bloat due to manual C-to-Lua translation, and performance isn’t optimized yet (e.g., copying data instead of direct pointers).

# Future Considerations

- Refinement: Streamline the API by rethinking metatable usage and type checking for better efficiency.
- Research: Investigate optimal patterns for C-Lua interop (e.g., reducing overhead, using lightuserdata, or integrating FFI optionally).
- Applications: Could be extended for scenarios like SQL-like triggers or game logic scripting.

# Results

- Metatables and Type Checking: Successfully used metatables to wrap C structs, with ecs.check_type ensuring type safety between C and Lua. This adds overhead but simplifies Lua-side usage.
- API Design: Required significant effort to balance Lua’s flexibility with Flecs’ C-based ECS logic, especially for systems.
- Flecs Integration: Correctly implemented ecs_system to run Lua callbacks on EcsOnUpdate, with component modification working via returned update tables.
    

# How It Works

The project bridges three layers: Flecs (C ECS), a custom C wrapper, and Lua. Here’s a visual diagram of the data flow:

```text
+--------------------+        +--------------------+        +--------------------+
|       Lua          |        |        C           |        |       Flecs        |
|--------------------|        |--------------------|        |--------------------|
| - ecs.new("Pos")   |<----->| - lua_ecs_new()    |<----->| - ecs_component_init() |
| - pos.x = 10       |        | - Metatable setup  |        | - PositionId       |
| - ecs.set(world, e)|        | - lua_ecs_set()    |------>| - ecs_set_id()     |
| - ecs.get(world, e)|<------| - lua_ecs_get()    |<------| - ecs_get_id()     |
| - ecs_system(world,|        | - lua_ecs_system() |        |                    |
|   fn, "Pos", "Vel")|<----->| - lua_system_cb()  |------>| - ecs_system_init()|
| - ecs.progress()   |------>| - lua_ecs_progress()|<---->| - ecs_progress()   |
+--------------------+        +--------------------+        +--------------------+

Legend:
  -----> : Function call or data flow
  <----> : Bidirectional interaction
```

# Flow Details

1. Component Creation:
    - Lua: local pos = ecs.new("Position"); pos.x = 10
    - C: lua_ecs_new creates userdata with a metatable, allowing field access (__index, __newindex).
    - Flecs: Components are registered with ecs_component_init during world setup.
        
2. Entity Management:
    - Lua: ecs.set(world, entity, pos) attaches components; ecs.get(world, entity, "Position") retrieves them.
    - C: lua_ecs_set and lua_ecs_get translate between Lua userdata and Flecs ecs_set_id/ecs_get_id.
    - Flecs: Stores and retrieves component data for entities.
        
3. Systems:
    - Lua: ecs.ecs_system(world, test, "Position", "Velocity") defines a system; test(it) processes entities.
    - C: lua_ecs_system registers the Lua function and sets up a Flecs system with ecs_system_init. lua_system_callback calls the Lua function, passing an it table with entities, Position, Velocity, and delta_time.
    - Flecs: Runs the system on ecs_progress, invoking the callback for matching entities.
    - Modification: Lua returns an updates table, which C applies back to Flecs via ecs_set_id.

# Example

lua
```lua
local ecs = require 'ecs'
local world = ecs.new_world()
local entity = ecs.entity(world)

local pos = ecs.new("Position")
pos.x = 10
ecs.set(world, entity, pos)

local vel = ecs.new("Velocity")
vel.x = 5
ecs.set(world, entity, vel)

function test(it)
    local updates = { Position = {} }
    for i, e in ipairs(it.entities) do
        local p = it.Position[i]
        local v = it.Velocity[i]
        print("Entity " .. e .. ": Pos x=" .. p.x .. ", Vel x=" .. v.x)
        updates.Position[i] = { x = p.x + v.x * it.delta_time }
    end
    return updates
end

ecs.ecs_system(world, test, "Position", "Velocity")
ecs.progress(world, 0.1)  -- Moves Position.x to 10.5
```

# Building and Running

1. Dependencies:
    - LuaJIT (or standard Lua)
    - Flecs (v4.x, fetched via CMake)
    - CMake
        
2. Build:
    
    bash
    ```bash
    mkdir build
    cd build
    cmake ..
    cmake --build .
    ```
    
3. Run:
    
    bash
    ```bash
    cd Debug
    flecs_lua_test.exe
    ```
    

# Output Example

```text
init lua
init luaL_openlibs
init flecs_lua
Loading script 'script.lua'...
init lua_pcall
Loading ecs module...
ecs module loaded
Registering components with Flecs
World created
Entity created: 533
Set Position x: 10
Set Velocity x: 5
Entity 533: Pos x=10, Vel x=5
Updated Position x: 10.5
lua_close
Test completed successfully.
```

# Future Improvements

- Optimize Data Transfer: Reduce copying by using pointers or lightuserdata (with safety trade-offs).
- Dynamic Queries: Parse component lists in ecs_system for flexibility (e.g., "Position, Velocity").
- Error Handling: Improve Lua error reporting in systems.
- Documentation: Expand with more examples and API details.

# luajit_flecs

# License: MIT

# Information:
  Prototype test. Trying to get c struct to work well with the lua. Althought I over building the layers.

  To filter out the casting type to access struct from c to lua and lua to c to deal with update component data.

  There are limited on ffi which LuaJIT has. Not much docs. Examples.

  After some thinking some time. It bloated. Well think of better methods.

# Results:
 * It take a while to do something light cide but more detail required c more coding.
 * adding helpers but it add more bloat for type checks for components.
 * required time to research for better code between lua and c for api.
 * Flecs is a bit tricky to layers and args as it tricky.
 * Doing some simple test build.
 * Required Struct in c to lua api to work with check type or match handle struct.
    * relate to lua metatable.

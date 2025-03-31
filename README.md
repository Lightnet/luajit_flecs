# luajit_flecs

# License: MIT

# Information:
  Prototype test. 
  
  Not using the ffi.
  
  After refining the code. It will be bloat but still required wrapper to handle the translate some degree.
  
  Checking types to handle c to lua and lua to c. Reason is metatables for lua.
  
  I can think of SQL and trigger functions.
  
# Results:
 * Rethink how to use metatable and check types.
 * required time to research for better code between lua and c for api.
 * required flecs logics to code correctly to handle ecs_system.

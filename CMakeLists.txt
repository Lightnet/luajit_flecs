cmake_minimum_required(VERSION 3.10)
project(FlecsLuaTest C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Enable FetchContent module
include(FetchContent)

# Fetch LuaJIT
FetchContent_Declare(
    luajit
    GIT_REPOSITORY https://github.com/openresty/luajit2.git  # Using openresty/luajit2 as per your original request
    GIT_TAG v2.1-agentzh
)
FetchContent_MakeAvailable(luajit)

# Define LuaJIT paths
set(LUAJIT_SRC_DIR ${luajit_SOURCE_DIR}/src)
set(LUAJIT_LIB ${LUAJIT_SRC_DIR}/lua51.lib)
set(LUAJIT_DLL ${LUAJIT_SRC_DIR}/lua51.dll)
include_directories(${LUAJIT_SRC_DIR})

# Custom command to build LuaJIT with msvcbuild.bat in VS2022 environment
add_custom_command(
    OUTPUT ${LUAJIT_LIB} ${LUAJIT_DLL}
    COMMAND cmd.exe /C " \"C:/Program Files/Microsoft Visual Studio/2022/Community/Common7/Tools/VsDevCmd.bat\" -arch=x64 && ${LUAJIT_SRC_DIR}/msvcbuild.bat"
    WORKING_DIRECTORY ${LUAJIT_SRC_DIR}
    DEPENDS ${luajit_SOURCE_DIR}/src/msvcbuild.bat
    COMMENT "Building LuaJIT with msvcbuild.bat"
)

# Custom target to ensure LuaJIT is built
add_custom_target(
    BuildLuaJIT
    DEPENDS ${LUAJIT_LIB} ${LUAJIT_DLL}
)

# Fetch Flecs
FetchContent_Declare(
    flecs
    GIT_REPOSITORY https://github.com/SanderMertens/flecs.git
    GIT_TAG v4.0.4
)
FetchContent_MakeAvailable(flecs)

# Add the main executable
add_executable(flecs_lua_test 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/main.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/flecs_lua.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/flecs_comps.c
)
target_include_directories(flecs_lua_test PUBLIC 
    ${flecs_SOURCE_DIR}/include  # Flecs include directory
    ${luajit_SOURCE_DIR}/src     # LuaJIT include directory
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)
target_link_libraries(flecs_lua_test PRIVATE 
    ${LUAJIT_LIB}               # Link against LuaJIT static library
    flecs_static                # Link against Flecs static library
)

# Ensure LuaJIT is built before the executable
add_dependencies(flecs_lua_test BuildLuaJIT)

# Post-build step to copy lua51.dll and script.lua to the output directory
add_custom_command(TARGET flecs_lua_test POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${LUAJIT_DLL}
        $<TARGET_FILE_DIR:flecs_lua_test>
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_CURRENT_SOURCE_DIR}/script.lua
        $<TARGET_FILE_DIR:flecs_lua_test>
    COMMENT "Copying lua51.dll and script.lua to output directory"
)
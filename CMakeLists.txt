cmake_minimum_required(VERSION 3.15)
project(FlecsLuaProject C)

# Set debug build by default
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
endif()

# Enable FetchContent
include(FetchContent)

# Fetch LuaJIT
FetchContent_Declare(
    luajit
    GIT_REPOSITORY https://github.com/openresty/luajit2.git
    GIT_TAG v2.1-20250117
)
FetchContent_MakeAvailable(luajit)

# Define LuaJIT DLL and import library paths
set(LUAJIT_DLL "${luajit_SOURCE_DIR}/src/lua51.dll")
set(LUAJIT_LIB "${luajit_SOURCE_DIR}/src/lua51.lib")

# Ensure LuaJIT src directory exists and build DLL only if missing
if(NOT EXISTS ${luajit_SOURCE_DIR}/src)
    message(FATAL_ERROR "LuaJIT source directory not found: ${luajit_SOURCE_DIR}/src")
endif()

if(NOT EXISTS ${LUAJIT_DLL} OR NOT EXISTS ${LUAJIT_LIB})
    message(STATUS "Building LuaJIT DLL...")
    execute_process(
        COMMAND cmd /c msvcbuild.bat dll
        WORKING_DIRECTORY ${luajit_SOURCE_DIR}/src
        RESULT_VARIABLE LUAJIT_BUILD_RESULT
        OUTPUT_VARIABLE LUAJIT_BUILD_OUTPUT
        ERROR_VARIABLE LUAJIT_BUILD_ERROR
    )
    if(NOT LUAJIT_BUILD_RESULT EQUAL 0)
        message(FATAL_ERROR "Failed to build LuaJIT:\nOutput: ${LUAJIT_BUILD_OUTPUT}\nError: ${LUAJIT_BUILD_ERROR}")
    endif()
    if(NOT EXISTS ${LUAJIT_DLL} OR NOT EXISTS ${LUAJIT_LIB})
        message(FATAL_ERROR "LuaJIT DLL or LIB not found after build: ${LUAJIT_DLL}, ${LUAJIT_LIB}")
    endif()
else()
    message(STATUS "LuaJIT DLL and LIB already exist, skipping build.")
endif()

# Fetch Flecs
FetchContent_Declare(
    flecs
    GIT_REPOSITORY https://github.com/SanderMertens/flecs.git
    GIT_TAG v4.0.4
)
set(BUILD_SHARED_LIBS ON CACHE BOOL "Build shared libraries" FORCE) # Force Flecs as DLL
FetchContent_MakeAvailable(flecs)

# Set C standard
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Add source files
set(SOURCES
    src/main.c
    src/flecs_lua.c
    #flecs_lua_ctypes.h
)

# Add include directories
include_directories(
    ${flecs_SOURCE_DIR}/include
    ${luajit_SOURCE_DIR}/src
    include
)

# Add executable
add_executable(${PROJECT_NAME} ${SOURCES})

# Link LuaJIT and Flecs as DLLs
target_link_libraries(${PROJECT_NAME} PRIVATE
    ${LUAJIT_LIB}  # Link against the import library
    flecs          # Flecs shared library
)

# Ensure output directory exists and copy LuaJIT DLL
set(OUTPUT_DIR $<TARGET_FILE_DIR:${PROJECT_NAME}>)
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory ${OUTPUT_DIR}
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${LUAJIT_DLL} ${OUTPUT_DIR}
    COMMENT "Copying LuaJIT DLL to output directory"
)

# Ensure Flecs DLL is copied (if built in a different location)
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:flecs> ${OUTPUT_DIR}
    COMMENT "Copying Flecs DLL to output directory"
)


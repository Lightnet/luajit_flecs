@echo off
echo Copying current main.lua to build folder...
copy main.lua build\main.lua
cd build
echo Running FlecsLuaProject with main.lua...
FlecsLuaProject.exe main.lua
@REM pause
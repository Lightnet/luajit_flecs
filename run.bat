@REM @echo off
setlocal
set APPNAME=flecs_lua_test
set APPPATH=build\Debug\%APPNAME%.exe
set EXECUTABLE=%APPNAME%.exe
set DEMO_SRC=script.lua
set DEMO_DEST=build\Debug\script.lua

echo %DEMO_SRC%
echo %DEMO_DEST%

if not exist %APPPATH% (
    echo Executable not found! Please build the project first.
    exit /b 1
)
echo copying %DEMO_SRC%
copy %DEMO_SRC% %DEMO_DEST%

echo Running %EXECUTABLE%...

cd build\Debug\

@REM %EXECUTABLE% %DEMO_SRC%
%EXECUTABLE%
endlocal
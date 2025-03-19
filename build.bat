@echo off
setlocal EnableDelayedExpansion

:: Set build type to Debug
set BUILD_TYPE=Debug

:: Define the path to VS2022's vcvarsall.bat (adjust if not Community Edition)
set VS_DEV_ENV="C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat"

:: Check if vcvarsall.bat exists
if not exist %VS_DEV_ENV% (
    echo ERROR: Could not find vcvarsall.bat at %VS_DEV_ENV%.
    echo Please ensure Visual Studio 2022 is installed and adjust the path in this script.
    exit /b 1
)

:: Initialize the VS2022 Developer environment (x64 target)
call %VS_DEV_ENV% x64
if !ERRORLEVEL! neq 0 (
    echo ERROR: Failed to initialize Visual Studio 2022 Developer environment.
    exit /b !ERRORLEVEL!
)

:: Create build directory if it doesn't exist
if not exist build (
    mkdir build
)

:: Change to build directory
cd build

:: Run CMake to configure the project with NMake Makefiles
cmake -G "NMake Makefiles" -DCMAKE_BUILD_TYPE=%BUILD_TYPE% ..
if !ERRORLEVEL! neq 0 (
    echo CMake configuration failed!
    exit /b !ERRORLEVEL!
)

:: Build the project using NMake
nmake
if !ERRORLEVEL! neq 0 (
    echo Build failed!
    exit /b !ERRORLEVEL!
)

echo Build completed successfully!
cd ..

endlocal
@echo off
title DSA Image Processor
echo ============================================
echo   DSA Image Processor - Build and Run
echo ============================================
echo.

:: Bypass Qt license check if license is expired
set "QTFRAMEWORK_BYPASS_LICENSE_CHECK=1"

:: Set Qt, CMake, Ninja, and MinGW path variables
set "QT_DIR=C:\Qt\6.11.0\mingw_64"
set "CMAKE_DIR=C:\Qt\Tools\CMake_64\bin"
set "NINJA_DIR=C:\Qt\Tools\Ninja"
set "MINGW_DIR=C:\Qt\Tools\mingw1310_64\bin"
set "PATH=%QT_DIR%\bin;%CMAKE_DIR%;%NINJA_DIR%;%MINGW_DIR%;%PATH%"

:: Detect if directory has changed
set "CURRENT_DIR_FORWARD=%~dp0"
set "CURRENT_DIR_FORWARD=%CURRENT_DIR_FORWARD:\=/%"
if "%CURRENT_DIR_FORWARD:~-1%"=="/" set "CURRENT_DIR_FORWARD=%CURRENT_DIR_FORWARD:~0,-1%"

if exist "%~dp0build\CMakeCache.txt" (
    findstr /I /C:"%CURRENT_DIR_FORWARD%" "%~dp0build\CMakeCache.txt" >nul
    if %ERRORLEVEL% NEQ 0 (
        echo [WARNING] Project directory has changed or cache is invalid.
        echo [WARNING] Cleaning build cache and reconfiguring...
        rmdir /s /q "%~dp0build"
    )
)

:: Reconfigure if build directory was cleaned or does not exist
if not exist "%~dp0build" (
    mkdir "%~dp0build"
    echo [1/2] Configuring CMake project...
    cmake -S "%~dp0." -B "%~dp0build" -G Ninja -DCMAKE_BUILD_TYPE=Debug -DCMAKE_PREFIX_PATH="%QT_DIR%" -DCMAKE_MAKE_PROGRAM="%NINJA_DIR%\ninja.exe" -DCMAKE_C_COMPILER="%MINGW_DIR%\gcc.exe" -DCMAKE_CXX_COMPILER="%MINGW_DIR%\g++.exe"
    if %ERRORLEVEL% NEQ 0 (
        echo.
        echo  CONFIGURATION FAILED! Fix errors above and try again.
        echo.
        pause
        exit /b 1
    )
)

:: Navigate to build directory
pushd "%~dp0build"

:: Build the project
echo [1/2] Building project...
cmake --build . --config Debug
if %ERRORLEVEL% NEQ 0 (
    echo.
    echo  BUILD FAILED! Fix errors above and try again.
    echo.
    pause
    popd
    exit /b 1
)

echo.
echo [2/2] Launching DSA Image Processor...
echo.

:: Run the executable
start "" "DSA_Image_Project.exe"

popd

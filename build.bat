@echo off
:: Blash C++ Build Script for Windows
:: This script builds the C++ version of Blash on Windows

setlocal enabledelayedexpansion

echo 🚀 Blash C++ Build Script for Windows
echo ===================================

:: Parse command line arguments
set "BUILD_TYPE=Release"
set "CLEAN_BUILD=false"
set "VERBOSE=false"

:parse_args
if "%~1"=="" goto :args_done
if /i "%~1"=="-d" (
    set "BUILD_TYPE=Debug"
    shift
    goto :parse_args
)
if /i "%~1"=="--debug" (
    set "BUILD_TYPE=Debug"
    shift
    goto :parse_args
)
if /i "%~1"=="-c" (
    set "CLEAN_BUILD=true"
    shift
    goto :parse_args
)
if /i "%~1"=="--clean" (
    set "CLEAN_BUILD=true"
    shift
    goto :parse_args
)
if /i "%~1"=="-v" (
    set "VERBOSE=true"
    shift
    goto :parse_args
)
if /i "%~1"=="--verbose" (
    set "VERBOSE=true"
    shift
    goto :parse_args
)
if /i "%~1"=="-h" (
    echo Usage: %0 [OPTIONS]
    echo Options:
    echo   -d, --debug     Build in Debug mode (default: Release)
    echo   -c, --clean     Clean build directory before building
    echo   -v, --verbose   Verbose build output
    echo   -h, --help      Show this help message
    exit /b 0
)
if /i "%~1"=="--help" (
    echo Usage: %0 [OPTIONS]
    echo Options:
    echo   -d, --debug     Build in Debug mode (default: Release)
    echo   -c, --clean     Clean build directory before building
    echo   -v, --verbose   Verbose build output
    echo   -h, --help      Show this help message
    exit /b 0
)
echo [ERROR] Unknown option: %~1
echo Use -h or --help for usage information.
exit /b 1

:args_done

echo [INFO] Build type: %BUILD_TYPE%

:: Check if we're in the right directory
if not exist "CMakeLists.txt" (
    echo [ERROR] CMakeLists.txt not found. Please run this script from the project root directory.
    exit /b 1
)

:: Check for required dependencies
echo [INFO] Checking dependencies...

:: Check for CMake
cmake --version >nul 2>&1
if errorlevel 1 (
    echo [ERROR] CMake is not installed or not in PATH. Please install CMake 3.20 or later.
    exit /b 1
)

for /f "tokens=3" %%i in ('cmake --version ^| findstr /C:"cmake version"') do set CMAKE_VERSION=%%i
echo [INFO] Found CMake version: %CMAKE_VERSION%

:: Check for Qt6 (basic check)
where qmake >nul 2>&1
if errorlevel 1 (
    echo [WARN] Qt6 qmake not found in PATH. Make sure Qt6 is installed and in your PATH.
) else (
    echo [INFO] Found Qt6 qmake
)

:: Create build directory
set "BUILD_DIR=build"

if "%CLEAN_BUILD%"=="true" (
    if exist "%BUILD_DIR%" (
        echo [INFO] Cleaning build directory...
        rmdir /s /q "%BUILD_DIR%"
    )
)

if not exist "%BUILD_DIR%" (
    echo [INFO] Creating build directory...
    mkdir "%BUILD_DIR%"
)

cd "%BUILD_DIR%"

:: Configure with CMake
echo [INFO] Configuring with CMake...
set CMAKE_ARGS=-DCMAKE_BUILD_TYPE=%BUILD_TYPE%

:: Add Windows-specific configurations
echo [INFO] Detected Windows platform

cmake %CMAKE_ARGS% ..
if errorlevel 1 (
    echo [ERROR] CMake configuration failed!
    exit /b 1
)

:: Build
echo [INFO] Building Blash C++...
set BUILD_ARGS=--build . --config %BUILD_TYPE%

if "%VERBOSE%"=="true" (
    set BUILD_ARGS=%BUILD_ARGS% --verbose
)

cmake %BUILD_ARGS%
if errorlevel 1 (
    echo [ERROR] Build failed!
    exit /b 1
)

echo [INFO] Build completed successfully! 🎉

:: Find the executable
set "EXECUTABLE="
if "%BUILD_TYPE%"=="Debug" (
    set "EXECUTABLE=Debug\BlashCpp.exe"
) else (
    set "EXECUTABLE=Release\BlashCpp.exe"
)

if exist "%EXECUTABLE%" (
    echo [INFO] Executable location: %EXECUTABLE%
    echo.
    echo ✅ Build completed successfully!
    echo To run Blash C++:
    echo   cd build ^&^& %EXECUTABLE%
    echo.
    echo ⚠️  Remember to configure your API keys before running!
    echo    Edit src/Config.cpp and rebuild, or wait for the settings GUI.
) else (
    echo [WARN] Executable not found at expected location: %EXECUTABLE%
    echo [INFO] Check the build directory for the executable.
)

echo.
echo 🔧 Development Tips:
echo   • Use '%~nx0 --debug' for debug builds
echo   • Use '%~nx0 --clean' to force a clean build
echo   • Use '%~nx0 --verbose' for detailed build output
echo   • Check README_CPP.md for more information

cd ..
pause
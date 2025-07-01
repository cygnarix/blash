#!/bin/bash

# Blash C++ Build Script
# This script builds the C++ version of Blash

set -e  # Exit on any error

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

echo -e "${BLUE}🚀 Blash C++ Build Script${NC}"
echo -e "${BLUE}========================${NC}"

# Function to print colored output
print_status() {
    echo -e "${GREEN}[INFO]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARN]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# Check if we're in the right directory
if [ ! -f "CMakeLists.txt" ]; then
    print_error "CMakeLists.txt not found. Please run this script from the project root directory."
    exit 1
fi

# Parse command line arguments
BUILD_TYPE="Release"
CLEAN_BUILD=false
VERBOSE=false

while [[ $# -gt 0 ]]; do
    case $1 in
        -d|--debug)
            BUILD_TYPE="Debug"
            shift
            ;;
        -c|--clean)
            CLEAN_BUILD=true
            shift
            ;;
        -v|--verbose)
            VERBOSE=true
            shift
            ;;
        -h|--help)
            echo "Usage: $0 [OPTIONS]"
            echo "Options:"
            echo "  -d, --debug     Build in Debug mode (default: Release)"
            echo "  -c, --clean     Clean build directory before building"
            echo "  -v, --verbose   Verbose build output"
            echo "  -h, --help      Show this help message"
            exit 0
            ;;
        *)
            print_error "Unknown option: $1"
            echo "Use -h or --help for usage information."
            exit 1
            ;;
    esac
done

print_status "Build type: $BUILD_TYPE"

# Check for required dependencies
print_status "Checking dependencies..."

# Check for CMake
if ! command -v cmake &> /dev/null; then
    print_error "CMake is not installed. Please install CMake 3.20 or later."
    exit 1
fi

CMAKE_VERSION=$(cmake --version | head -n1 | cut -d' ' -f3)
print_status "Found CMake version: $CMAKE_VERSION"

# Check for Qt6
if ! command -v qmake6 &> /dev/null && ! command -v qmake &> /dev/null; then
    print_warning "Qt6 qmake not found in PATH. Make sure Qt6 is installed."
fi

# Check for nlohmann/json (basic check)
if [ -f "/usr/include/nlohmann/json.hpp" ] || [ -f "/usr/local/include/nlohmann/json.hpp" ]; then
    print_status "Found nlohmann/json library"
else
    print_warning "nlohmann/json library not found in standard locations."
    print_warning "Make sure it's installed or available to CMake."
fi

# Create build directory
BUILD_DIR="build"

if [ "$CLEAN_BUILD" = true ] && [ -d "$BUILD_DIR" ]; then
    print_status "Cleaning build directory..."
    rm -rf "$BUILD_DIR"
fi

if [ ! -d "$BUILD_DIR" ]; then
    print_status "Creating build directory..."
    mkdir "$BUILD_DIR"
fi

cd "$BUILD_DIR"

# Configure with CMake
print_status "Configuring with CMake..."
CMAKE_ARGS=("-DCMAKE_BUILD_TYPE=$BUILD_TYPE")

# Add platform-specific configurations
if [[ "$OSTYPE" == "linux-gnu"* ]]; then
    print_status "Detected Linux platform"
elif [[ "$OSTYPE" == "msys" ]] || [[ "$OSTYPE" == "win32" ]]; then
    print_status "Detected Windows platform"
    # Add Windows-specific CMake args if needed
fi

if ! cmake "${CMAKE_ARGS[@]}" ..; then
    print_error "CMake configuration failed!"
    exit 1
fi

# Build
print_status "Building Blash C++..."
BUILD_ARGS=("--build" "." "--config" "$BUILD_TYPE")

if [ "$VERBOSE" = true ]; then
    BUILD_ARGS+=("--verbose")
fi

if ! cmake "${BUILD_ARGS[@]}"; then
    print_error "Build failed!"
    exit 1
fi

print_status "Build completed successfully! 🎉"

# Find the executable
EXECUTABLE=""
if [[ "$OSTYPE" == "linux-gnu"* ]]; then
    EXECUTABLE="./BlashCpp"
elif [[ "$OSTYPE" == "msys" ]] || [[ "$OSTYPE" == "win32" ]]; then
    if [ "$BUILD_TYPE" = "Debug" ]; then
        EXECUTABLE="./Debug/BlashCpp.exe"
    else
        EXECUTABLE="./Release/BlashCpp.exe"
    fi
fi

if [ -f "$EXECUTABLE" ]; then
    print_status "Executable location: $EXECUTABLE"
    echo ""
    echo -e "${GREEN}✅ Build completed successfully!${NC}"
    echo -e "${BLUE}To run Blash C++:${NC}"
    echo -e "  cd build && $EXECUTABLE"
    echo ""
    echo -e "${YELLOW}⚠️  Remember to configure your API keys before running!${NC}"
    echo -e "   Edit src/Config.cpp and rebuild, or wait for the settings GUI."
else
    print_warning "Executable not found at expected location: $EXECUTABLE"
    print_status "Check the build directory for the executable."
fi

echo ""
echo -e "${BLUE}🔧 Development Tips:${NC}"
echo -e "  • Use '$0 --debug' for debug builds"
echo -e "  • Use '$0 --clean' to force a clean build"
echo -e "  • Use '$0 --verbose' for detailed build output"
echo -e "  • Check README_CPP.md for more information"
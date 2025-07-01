cmake_minimum_required(VERSION 3.20)
project(BlashCpp)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find Qt6 components
find_package(Qt6 REQUIRED COMPONENTS Core Widgets Gui Network)

# Find nlohmann/json
find_package(nlohmann_json REQUIRED)

# Sources
set(SOURCES
    src/main.cpp
    src/BlashApp.cpp
    src/StealthToolbar.cpp
    src/HookManager.cpp
    src/ScreenshotViewer.cpp
    src/ResponseDialog.cpp
    src/DialogBox.cpp
    src/AIWorker.cpp
    src/Config.cpp
    src/ThumbnailLabel.cpp
)

# Headers
set(HEADERS
    include/BlashApp.h
    include/StealthToolbar.h
    include/HookManager.h
    include/ScreenshotViewer.h
    include/ResponseDialog.h
    include/DialogBox.h
    include/AIWorker.h
    include/Config.h
    include/ThumbnailLabel.h
)

# Create executable
add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

# Include directories
target_include_directories(${PROJECT_NAME} PRIVATE include)

# Link libraries
target_link_libraries(${PROJECT_NAME} 
    Qt6::Core 
    Qt6::Widgets 
    Qt6::Gui 
    Qt6::Network
    nlohmann_json::nlohmann_json
)

# Platform-specific libraries
if(WIN32)
    target_link_libraries(${PROJECT_NAME} user32 kernel32)
elseif(UNIX AND NOT APPLE)
    target_link_libraries(${PROJECT_NAME} X11 Xtst)
endif()

# Qt6 specific settings
set_target_properties(${PROJECT_NAME} PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
)

# Copy config file to build directory
configure_file(${CMAKE_SOURCE_DIR}/config/kb_sht.json ${CMAKE_BINARY_DIR}/kb_sht.json COPYONLY)
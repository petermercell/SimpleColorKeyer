cmake_minimum_required(VERSION 3.18)
project(SimpleColorKeyer LANGUAGES CXX)

# Set C++ standard to 17 (required for Nuke 16.0v4)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Paths
set(NUKE_SDK_PATH "/opt/Nuke14.1v2" CACHE PATH "Path to Nuke SDK")

# Check if Nuke SDK exists
if(NOT EXISTS ${NUKE_SDK_PATH}/include/DDImage)
    message(FATAL_ERROR "Nuke SDK not found at ${NUKE_SDK_PATH}")
endif()

# Include directories
include_directories(${NUKE_SDK_PATH}/include)

# Source files - CPU only
set(CPP_SOURCES SimpleColorKeyer.cpp)

# Create shared library
add_library(SimpleColorKeyer SHARED ${CPP_SOURCES})

# Set library properties
set_target_properties(SimpleColorKeyer PROPERTIES
    PREFIX ""
    SUFFIX ".so"
    POSITION_INDEPENDENT_CODE ON
)

# Uncomment Nuke14 only
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_GLIBCXX_USE_CXX11_ABI=0")

# Optimized compiler options
target_compile_options(SimpleColorKeyer PRIVATE
    -O3
    -fPIC
    -DUSE_GLEW
    -DFNUKE_INTERNAL
)

# Link directories and libraries
target_link_directories(SimpleColorKeyer PRIVATE ${NUKE_SDK_PATH})
target_link_libraries(SimpleColorKeyer DDImage)

# Install target
install(TARGETS SimpleColorKeyer
    LIBRARY DESTINATION $ENV{HOME}/.nuke
)

# Print configuration
message(STATUS "=== Simple Color Keyer Build Configuration ===")
message(STATUS "Nuke SDK Path: ${NUKE_SDK_PATH}")
message(STATUS "CPU-only version (no CUDA)")
message(STATUS "==============================================")

# Custom targets
add_custom_target(install-user
    COMMAND ${CMAKE_COMMAND} --build . --target install
    COMMENT "Installing Simple Color Keyer to user's .nuke directory"
)

add_custom_target(test-simple
    COMMAND echo "=== Simple Color Keyer Built Successfully ==="
    COMMAND file SimpleColorKeyer.so
    COMMAND echo "=============================================="
    DEPENDS SimpleColorKeyer
)

cmake_minimum_required(VERSION 3.11)

project(SimpleColorKeyer)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT DEFINED NUKE_VERSION)
    set(NUKE_VERSION 16.0v4)
endif()

set(NDKDIR "C:/Program Files/Nuke${NUKE_VERSION}")

if (MSVC)
    add_definitions(/D_CRT_SECURE_NO_WARNINGS /D_SCL_SECURE_NO_WARNINGS /D NOMINMAX)
    add_definitions(-D_USE_MATH_DEFINES)
    add_compile_options(/std:c++17)
endif()

add_library(SimpleColorKeyer SHARED SimpleColorKeyer.cpp)

# Add include directory so compiler finds DDImage headers
target_include_directories(SimpleColorKeyer PRIVATE "${NDKDIR}/include")

# Link libraries
target_link_libraries(SimpleColorKeyer PRIVATE "${NDKDIR}/DDImage.lib" OpenGL32)

set(CMAKE_SHARED_LIBRARY_PREFIX "")

set(CMAKE_INSTALL_PREFIX "$ENV{USERPROFILE}/.nuke")

install(TARGETS SimpleColorKeyer
        RUNTIME DESTINATION "${CMAKE_INSTALL_PREFIX}"
        LIBRARY DESTINATION "${CMAKE_INSTALL_PREFIX}"
)
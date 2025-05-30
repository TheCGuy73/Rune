cmake_minimum_required(VERSION 3.15)
project(voltage LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Optionally set output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Collect all source files (recursive, adjust pattern if needed)
file(GLOB_RECURSE VOLTAGE_SOURCES
    src/*.cpp
    src/*.cxx
    src/*.cc
)

# Add executable target
add_executable(voltage ${VOLTAGE_SOURCES})

# Include directories (add more if needed)
target_include_directories(voltage PRIVATE ${CMAKE_SOURCE_DIR}/src)

# Example: link libraries if necessary
# target_link_libraries(voltage PRIVATE some_library)

# Optionally enable warnings
if (MSVC)
    target_compile_options(voltage PRIVATE /W4 /permissive-)
else()
    target_compile_options(voltage PRIVATE -Wall -Wextra -pedantic)
endif()
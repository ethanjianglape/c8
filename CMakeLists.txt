cmake_minimum_required(VERSION 3.16)

project(c8
    VERSION 1.0.0
    DESCRIPTION "CHIP-8 Emulator with Time-Travel Debugging"
    LANGUAGES CXX
)

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Enable multithreaded builds
include(ProcessorCount)
ProcessorCount(N)
if(NOT N EQUAL 0)
    set(CMAKE_BUILD_PARALLEL_LEVEL ${N} CACHE STRING "Number of parallel build jobs")
    if(CMAKE_GENERATOR MATCHES "Make")
        set(CMAKE_MAKE_PROGRAM "${CMAKE_MAKE_PROGRAM} -j${N}")
    endif()
endif()

# Set default build type to Release if not specified
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build (Debug or Release)" FORCE)
endif()

# Find SFML (version 3.x)
find_package(SFML 3 REQUIRED COMPONENTS Graphics Window System)

# Collect all source files
set(SOURCES
    src/main.cpp
    src/cpu.cpp
    src/memory.cpp
    src/opcodes.cpp
    src/vga.cpp
    src/ui.cpp
)

# Collect all header files (for IDE support)
set(HEADERS
    src/cpu.hpp
    src/memory.hpp
    src/opcodes.hpp
    src/vga.hpp
    src/ui.hpp
    src/config.hpp
    src/quirks.hpp
    src/fonts.hpp
)

# Create executable
add_executable(c8 ${SOURCES} ${HEADERS})

# Create bin directory if it doesn't exist
file(MAKE_DIRECTORY "${CMAKE_SOURCE_DIR}/bin")

# Set output directory for the executable
set_target_properties(c8 PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin"
    RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_SOURCE_DIR}/bin"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_SOURCE_DIR}/bin"
)

# Link SFML libraries
target_link_libraries(c8 PRIVATE
    SFML::Graphics
    SFML::Window
    SFML::System
)

# Compiler options
target_compile_options(c8 PRIVATE
    # GCC/Clang warnings
    $<$<CXX_COMPILER_ID:GNU,Clang>:-Wall -Wextra>
    # MSVC warnings
    $<$<CXX_COMPILER_ID:MSVC>:/W4>
)

# Enable optimizations for Release builds
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    target_compile_options(c8 PRIVATE
        $<$<CXX_COMPILER_ID:GNU,Clang>:-O3>
    )
endif()

# Print build configuration
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "Compiler: ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS "SFML version: ${SFML_VERSION}")

cmake_minimum_required(VERSION 3.20)

project(mask)

set(CXX_STANDARD 20)

# Find LLVM package and configure include directories
find_package(LLVM REQUIRED CONFIG)
message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

# Include custom CMake modules
include(${PROJECT_SOURCE_DIR}/cmake/message.cmake)

# Set build type (default to Release if not specified)
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Configure compiler-specific flags
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")  # GCC
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        print_options(GNU Debug)
        add_compile_options(-std=c++20 -g -O0 -Wall -Wextra)
    else()
        print_options(GNU Release)
        add_compile_options(-std=c++20 -O3 -march=native -Wall -Wextra)
    endif()
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        print_options(Clang Debug)
        add_compile_options(-std=c++20 -g -O0 -Wall -Wextra)
    else()
        print_options(Clang Release)
        add_compile_options(-std=c++20 -O3 -march=native -Wall -Wextra)
    endif()
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")  # MSVC
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        print_options(MSVC Debug)
        add_compile_options(/Zi /Od /W4)
    else()
        print_options(MSVC Release)
        add_compile_options(/O2 /W4)
    endif()
endif()

# Collect source files recursively with CONFIGURE_DEPENDS
file(GLOB_RECURSE srcs CONFIGURE_DEPENDS
    ${PROJECT_SOURCE_DIR}/src/*.cpp
    ${PROJECT_SOURCE_DIR}/src/*.cxx
    ${PROJECT_SOURCE_DIR}/src/*.cc
)

if(NOT srcs)
    message(FATAL_ERROR "No source files found in ${PROJECT_SOURCE_DIR}/src. Please add at least one .cpp, .cxx, or .cc file.")
endif()

# Create the executable target
add_executable(mask ${srcs})

# Set target-specific include directories
target_include_directories(mask PUBLIC
    ${PROJECT_SOURCE_DIR}/inc
    ${LLVM_INCLUDE_DIRS}
)

# Set target-specific compile definitions
target_compile_definitions(mask PUBLIC ${LLVM_DEFINITIONS})

# Link LLVM libraries
llvm_map_components_to_libnames(llvm_libs 
    core 
    irreader 
    support
    # Add other LLVM components as needed
)

target_link_libraries(mask ${llvm_libs})

# Installation configuration
install(
    TARGETS mask
    DESTINATION bin
)

install(
    DIRECTORY ${PROJECT_SOURCE_DIR}/inc/
    DESTINATION include
    FILES_MATCHING PATTERN "*.h" PATTERN "*.hpp"
)

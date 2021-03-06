cmake_minimum_required(VERSION 3.13)
set(CMAKE_TOOLCHAIN_FILE ${CMAKE_CURRENT_SOURCE_DIR}/vendor/vcpkg/scripts/buildsystems/vcpkg.cmake CACHE STRING "Vcpkg toolchain file")
project(Overlay VERSION 0.1.0)

# Get git commit hash
execute_process(
    COMMAND git describe --always --dirty
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    OUTPUT_VARIABLE GIT_COMMIT_HASH
    OUTPUT_STRIP_TRAILING_WHITESPACE)

# Debug
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
	add_definitions(-DDEBUG)
endif()

# Set arch and build type name
string (TOLOWER ${CMAKE_BUILD_TYPE} BUILD_TYPE_NAME)

# Set C++17 standard
set (CMAKE_CXX_STANDARD 17)

# Fix setting options
cmake_policy(SET CMP0077 NEW)

# Ignore deprecations
add_definitions(-D_CRT_SECURE_NO_WARNINGS -D_SILENCE_ALL_CXX17_DEPRECATION_WARNINGS)
add_definitions(-DOVERLAY_VERSION="${CMAKE_PROJECT_VERSION}-${GIT_COMMIT_HASH}")

# Set runtime library
set(CMAKE_CXX_FLAGS_RELEASE "/O2 /MT")
set(CMAKE_CXX_FLAGS_DEBUG "/MTd")

# Set binary dir
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin/${BUILD_TYPE_NAME})

# Compile overlay shared object library
add_subdirectory(shared OverlayShared)
get_target_property(OVERLAY_SHARED_BINARY_DIR OverlayShared BINARY_DIR)
include_directories(${OVERLAY_SHARED_BINARY_DIR}/gens shared/src)

add_subdirectory(core OverlayCore)
add_subdirectory(injector OverlayInjector)
add_subdirectory(helper OverlayHelper)
add_subdirectory(demo OverlayDemo)
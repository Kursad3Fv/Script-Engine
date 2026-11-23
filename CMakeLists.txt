cmake_minimum_required(VERSION 3.21)

project(ScriptEngine LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# --------------------------------------------------
# Mono
# --------------------------------------------------

set(MONO_ROOT "" CACHE PATH "Path to Mono installation")

if(NOT MONO_ROOT AND DEFINED ENV{MONO_ROOT})
    set(MONO_ROOT "$ENV{MONO_ROOT}")
endif()

if(NOT MONO_ROOT)
    message(FATAL_ERROR
        "Mono not found. Set MONO_ROOT to your Mono installation directory."
    )
endif()

find_path(MONO_INCLUDE_DIR
    NAMES mono/jit/jit.h
    PATHS
        "${MONO_ROOT}/include/mono-2.0"
        "${MONO_ROOT}/include"
    NO_DEFAULT_PATH
)

find_library(MONO_LIBRARY
    NAMES mono-2.0-sgen mono-2.0
    PATHS
        "${MONO_ROOT}/lib"
        "${MONO_ROOT}/lib64"
    NO_DEFAULT_PATH
)

if(NOT MONO_INCLUDE_DIR)
    message(FATAL_ERROR "Mono headers not found in: ${MONO_ROOT}")
endif()

if(NOT MONO_LIBRARY)
    message(FATAL_ERROR "Mono library not found in: ${MONO_ROOT}")
endif()

message(STATUS "Mono Root: ${MONO_ROOT}")
message(STATUS "Mono Include: ${MONO_INCLUDE_DIR}")
message(STATUS "Mono Library: ${MONO_LIBRARY}")

# --------------------------------------------------
# Lib
# --------------------------------------------------

add_library(ScriptEngine STATIC
    Source/ScriptEngine.cpp
)

target_include_directories(ScriptEngine
    PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}/Include"
        "${MONO_INCLUDE_DIR}"
)

# --------------------------------------------------
# Include
# --------------------------------------------------

target_include_directories(ScriptEngine PRIVATE
    "${MONO_INCLUDE_DIR}"
)

# --------------------------------------------------
# Link
# --------------------------------------------------

target_link_libraries(ScriptEngine PRIVATE
    "${MONO_LIBRARY}"
)

if(WIN32)
    target_link_libraries(ScriptEngine PRIVATE
        ws2_32
        version
        psapi
    )
endif()

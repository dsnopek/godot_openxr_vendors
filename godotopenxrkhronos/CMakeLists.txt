cmake_minimum_required(VERSION 3.22.1)

## Project definition
project(godotopenxrvendors LANGUAGES CXX)

## Common dependencies
include(${PROJECT_SOURCE_DIR}/../common/common.cmake)

## khronos OpenXR loader library
find_package(OpenXR REQUIRED CONFIG)

## Setup the project sources
file(GLOB_RECURSE ANDROID_SOURCES ${PROJECT_SOURCE_DIR}/src/main/cpp/*.c**)
file(GLOB_RECURSE ANDROID_HEADERS ${PROJECT_SOURCE_DIR}/src/main/cpp/*.h**)

add_library(${PROJECT_NAME}
        SHARED
        ${ANDROID_SOURCES}
        ${ANDROID_HEADERS}
        ${COMMON_LIB_SOURCES}
        ${COMMON_LIB_HEADERS}
        )

target_include_directories(${PROJECT_NAME}
        SYSTEM PUBLIC
        ${GODOT_CPP_INCLUDE_DIRECTORIES}
        ${OPENXR_HEADERS_DIR}
        ${COMMON_LIB_HEADERS_DIR}
        )

target_link_libraries(${PROJECT_NAME}
        android
        log
        ${GODOT-CPP}
        OpenXR::openxr_loader
        )

# Add the compile flags
set_property(TARGET ${PROJECT_NAME} APPEND_STRING PROPERTY COMPILE_FLAGS ${GODOT_COMPILE_FLAGS})
set_property(TARGET ${PROJECT_NAME} APPEND_STRING PROPERTY LINK_FLAGS ${GODOT_LINKER_FLAGS})

add_definitions(-DKHRONOS_VENDOR_ENABLED)

cmake_minimum_required(VERSION 4.1.1)

# Project info
project("da-cubec" VERSION 1.0)

# Set the executable and library output to root
set (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR})

# Requiring C99
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED True)

# Export compile_commands.json for clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Defining compilations flags
set(GCC_COMPILE_FLAGS "-Wall -Wextra -g -O0")

# Adding all .c files in src
file(GLOB_RECURSE SOURCES "src/*.c")
add_executable(${PROJECT_NAME} ${SOURCES})

# Setting the compile flags just for da-cubec executable
set_target_properties(${PROJECT_NAME} PROPERTIES COMPILE_FLAGS ${GCC_COMPILE_FLAGS})

# Including our own header files to the project
target_include_directories(${PROJECT_NAME} PRIVATE include)

# Linking libraries
# glad
add_subdirectory(external/glad)
target_link_libraries(${PROJECT_NAME} PRIVATE glad)

# GLFW3
add_subdirectory(external/glfw)
find_package(OpenGL REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE glfw OpenGL::GL)

# stb_image.h
target_include_directories(${PROJECT_NAME} PUBLIC external/stb)

# cglm.h
target_link_libraries(${PROJECT_NAME} PRIVATE cglm)
add_subdirectory(external/cglm)

# From documentation
option(CGLM_SHARED "Shared build" ON)
option(CGLM_STATIC "Static build" OFF)
option(CGLM_USE_C99 "" ON)
option(CGLM_USE_TEST "Enable Tests" OFF) # for make check - make test

if(UNIX)
        target_link_libraries(${PROJECT_NAME} PRIVATE pthread dl X11)
endif()

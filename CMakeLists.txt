cmake_minimum_required(VERSION 4.1.1)

# Project info
set(PROJECT_TITLE "da-cubec")
project(${PROJECT_TITLE} VERSION 1.0)

# Requiring C99
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED True)

# Export compile_commands.json for clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Defining compilations flags
set(GCC_COMPILE_FLAGS "-Wall -O3")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${GCC_COMPILE_FLAGS}")

add_executable(${PROJECT_TITLE}
	src/main.c
	src/utils/io_utils.c
	src/utils/shader_utils.c
)

# Including our own header files to the project
target_include_directories(${PROJECT_TITLE} PRIVATE include)

# Linking libraries
# glad
add_subdirectory(external/glad)
target_link_libraries(${PROJECT_TITLE} PRIVATE glad)

# GLFW3
add_subdirectory(external/glfw)
find_package(OpenGL REQUIRED)
target_link_libraries(${PROJECT_TITLE} PRIVATE glfw OpenGL::GL)

# stb_image.h
target_include_directories(${PROJECT_TITLE} PUBLIC external/stb)

if(UNIX)
        target_link_libraries(${PROJECT_TITLE} PRIVATE pthread dl X11)
endif()

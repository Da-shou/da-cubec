cmake_minimum_required(VERSION 4.1.1)

# Project info
project("da-cubec" VERSION 1.0)

# Set the executable and library output to root
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR})
message(CMAKE_C_COMPILER=${CMAKE_C_COMPILER})

# Requiring C99
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED True)

# Export compile_commands.json for clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Defining compilations flags
set(COMPILATION_FLAGS "-Wall -Wextra -Wconversion -Wpedantic -O3")
set(BUILD_SHARED_LIBS OFF)

# Adding all .c files in src
file(GLOB_RECURSE SOURCES "src/*.c")
add_executable(${PROJECT_NAME} ${SOURCES})

# Setting the compile flags just for da-cubec executable
set_target_properties(${PROJECT_NAME}
        PROPERTIES COMPILE_FLAGS ${COMPILATION_FLAGS})

# Including our own header files to the project
target_include_directories(${PROJECT_NAME} PRIVATE include)

# Including math.h
target_link_libraries(${PROJECT_NAME} PRIVATE m)

# Linking libraries
# glad
add_subdirectory(external/glad)
target_link_libraries(${PROJECT_NAME} PRIVATE glad)

# GLFW3
add_subdirectory(external/glfw)
find_package(OpenGL REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE glfw OpenGL::GL)

# stb_image.h
target_include_directories(${PROJECT_NAME} SYSTEM PUBLIC external/stb)

# cglm.h
# From documentation
option(CGLM_SHARED "Shared build" OFF)
option(CGLM_STATIC "Static build" ON)
option(CGLM_USE_C99 "" ON)
option(CGLM_USE_TEST "Enable Tests" OFF)
add_subdirectory(external/cglm)
target_link_libraries(${PROJECT_NAME} PRIVATE cglm)

find_package(Doxygen)
if (DOXYGEN_FOUND)
    set(DOXYGEN_PROJECT_NAME ${PROJECT_NAME})
    set(DOXYGEN_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/docs")
    set(DOXYGEN_GENERATE_HTML YES)
    set(DOXYGEN_GENERATE_LATEX YES)
    set(DOXYGEN_EXTRACT_ALL YES)
    set(DOXYGEN_EXTRACT_STATIC YES)
    set(DOXYGEN_RECURSIVE YES)
    set(DOXYGEN_HAVE_DOT YES)
    set(DOXYGEN_CALL_GRAPH YES)
    set(DOXYGEN_CALLER_GRAPH YES)
    set(DOXYGEN_CLASS_GRAPH YES)
    set(DOXYGEN_INCLUDE_GRAPH YES)
    set(DOXYGEN_INCLUDED_BY_GRAPH YES)
    set(DOXYGEN_COLLABORATION_GRAPH YES)
    set(DOXYGEN_DOT_IMAGE_FORMAT svg)

    doxygen_add_docs(
            doc
            ${PROJECT_SOURCE_DIR}/src
            ${PROJECT_SOURCE_DIR}/include
            COMMENT "Generating API documentation with Doxygen"
    )

    add_custom_target(doc_pdf
            WORKING_DIRECTORY "${DOXYGEN_OUTPUT_DIRECTORY}/latex"
            COMMAND "make"
            DEPENDS doc
            COMMENT "Compiling LaTeX documentation to PDF"
            VERBATIM
    )
endif ()

if (UNIX)
    target_link_libraries(${PROJECT_NAME} PRIVATE pthread dl X11)
endif ()

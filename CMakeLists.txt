cmake_minimum_required(VERSION 3.16)
project(BreakoutGame LANGUAGES C CXX)

set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG 3.3.9
)
FetchContent_MakeAvailable(glfw)

find_package(OpenGL REQUIRED)

add_executable(Breakout
    src/main.cpp
    src/Shader.cpp
    src/glad.c
)

target_include_directories(Breakout PRIVATE
    include
)

target_link_libraries(Breakout PRIVATE
    glfw
    OpenGL::GL
)

cmake_minimum_required(VERSION 2.6)

include_directories("Lib/glfw-3.2.1/include")
include_directories("Lib/include")
include_directories("Glad/")

add_subdirectory("Glad/")
add_subdirectory("Lib/glfw-3.2.1/")
add_subdirectory("ObjGLF/")
add_subdirectory("TestProject/")
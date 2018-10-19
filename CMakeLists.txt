cmake_minimum_required(VERSION 2.6)

project (ObjGLUF)

include_directories("Lib/glfw-3.2.1/include")
include_directories("Lib/assimp-3.3.1/include")
include_directories("Lib/freetype-2.8.1/include")
include_directories("Lib/include")
include_directories("Glad/")

add_subdirectory("Glad/")
add_subdirectory("Lib/glfw-3.2.1/")
add_subdirectory("Lib/freetype-2.8.1/")
add_subdirectory("Lib/assimp-3.3.1/")
add_subdirectory("ObjGLUF/")
add_subdirectory("ObjGLUFGui/")
add_subdirectory("TestProject/")
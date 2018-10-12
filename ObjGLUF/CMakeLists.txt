cmake_minimum_required(VERSION 2.6)
project(objgluf)

# TODO: split ObjGLUF and GLUFGui into two projects
set(objgluf_SOURCES ObjGLUF.cpp GLUFGui.cpp)
set(objgluf_HEADERS ObjGLUF.h GLUFGui.h GLUFGuiTemplates.inl ObjGLUFTemplates.inl)

add_definitions(-DOBJGLUF_EXPORTS)

add_library(objgluf ${objgluf_SOURCES} ${objgluf_HEADERS})

# TODO: better support optional use of Assimp (b/c not everyone wants it)
target_link_libraries(objgluf glfw ${GLFW_LIBRARIES} freetype assimp glad)
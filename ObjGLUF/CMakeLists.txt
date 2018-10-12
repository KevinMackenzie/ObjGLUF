cmake_minimum_required(VERSION 2.6)
project(objglf)

# TODO: split ObjGLUF and GLUFGui into two projects
set(objglf_SOURCES ObjGLUF.cpp GLUFGui.cpp)
set(objglf_HEADERS ObjGLUF.h GLUFGui.h GLUFGuiTemplates.inl ObjGLUFTemplates.inl)

add_definitions(-DOBJGLUF_EXPORTS)

add_library(objglf ${objglf_SOURCES} ${objglf_HEADERS})

# TODO: better support optional use of Assimp (b/c not everyone wants it)
target_link_libraries(objglf glfw ${GLFW_LIBRARIES} freetype assimp glad)
cmake_minimum_required(VERSION 2.6)
project(objglf)

set(objglf_SOURCES ObjGLUF.cpp GLUFGui.cpp)

add_library(objglf ${objglf_SOURCES})
target_link_libraries(objglf glfw ${GLFW_LIBRARIES} glad)
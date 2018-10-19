cmake_minimum_required(VERSION 2.6)
project(objglufgui)

set(objglufgui_SOURCES GLUFGui.cpp)
set(objglufgui_HEADERS GLUFGui.h GLUFGuiTemplates.inl)

add_definitions(-DOBJGLUF_EXPORTS)

add_library(objglufgui ${objglufgui_SOURCES} ${objglufgui_HEADERS})

# TODO: better support optional use of Assimp (b/c not everyone wants it)
target_link_libraries(objglufgui glfw ${GLFW_LIBRARIES} objgluf freetype glad)
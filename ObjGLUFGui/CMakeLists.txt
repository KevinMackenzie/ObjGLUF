cmake_minimum_required(VERSION 2.6)
project(objglufgui)

set(objglufgui_SOURCES GLUFGui.cpp)
set(objglufgui_HEADERS GLUFGui.h GLUFGuiTemplates.inl Text.h BlendColor.h Element.h Dialog.h SpriteVertexStruct.h DialogResourceManager.h Control.h Controls/Static.h Controls/Button.h Controls/CheckBox.h Controls/RadioButton.h Controls/ScrollBar.h Controls/ListBox.h Controls/ComboBox.h Controls/Slider.h ModificationStack.h Controls/EditBox.h Exceptions.h)

add_definitions(-DOBJGLUF_EXPORTS)

add_library(objglufgui ${objglufgui_SOURCES} ${objglufgui_HEADERS} Text.cpp)

# TODO: better support optional use of Assimp (b/c not everyone wants it)
target_link_libraries(objglufgui glfw ${GLFW_LIBRARIES} objgluf freetype glad)
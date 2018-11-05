cmake_minimum_required(VERSION 2.6)
project(objglufgui)

set(objglufgui_SOURCES GLUFGui.cpp)
set(objglufgui_HEADERS GLUFGui.h GLUFGuiTemplates.inl Utils/Text.h Utils/BlendColor.h Element.h Dialog.h Utils/SpriteVertexStruct.h DialogResourceManager.h Control.h Controls/Static.h Controls/Button.h Controls/CheckBox.h Controls/RadioButton.h Controls/ScrollBar.h Controls/ListBox.h Controls/ComboBox.h Controls/Slider.h Utils/ModificationStack.h Controls/EditBox.h Exceptions.h Controls/Button.cpp Controls/Static.cpp Controls/Control.cpp DialogResourceManager.cpp Dialog.cpp Utils/BlendColor.cpp Element.cpp Controls/CheckBox.cpp Controls/RadioButton.cpp Controls/ScrollBar.cpp Controls/ListBox.cpp Controls/ComboBox.cpp Controls/Slider.cpp Utils/ModificationStack.cpp Controls/EditBox.cpp)

add_definitions(-DOBJGLUF_EXPORTS)

add_library(objglufgui ${objglufgui_SOURCES} ${objglufgui_HEADERS} Utils/Text.cpp)

# TODO: better support optional use of Assimp (b/c not everyone wants it)
target_link_libraries(objglufgui glfw ${GLFW_LIBRARIES} objgluf freetype glad)
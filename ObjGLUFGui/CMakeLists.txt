cmake_minimum_required(VERSION 2.6)
project(objglufgui)

set(objglufgui_SOURCES
        Control.cpp
        Dialog.cpp
        DialogResourceManager.cpp
        Element.cpp
        GLUFGui.cpp
        Controls/Button.cpp
        Controls/CheckBox.cpp
        Controls/ComboBox.cpp
        Controls/EditBox.cpp
        Controls/ListBox.cpp
        Controls/RadioButton.cpp
        Controls/ScrollBar.cpp
        Controls/Slider.cpp
        Controls/Static.cpp
        Utils/Text.cpp
        Utils/ModificationStack.cpp
        Utils/BlendColor.cpp
        )
set(objglufgui_HEADERS
        Control.h
        Dialog.h
        DialogResourceManager.h
        Element.h
        Exceptions.h
        Exports.h
        GLUFGui.h
        Types.h
        Controls/Button.h
        Controls/CheckBox.h
        Controls/ComboBox.h
        Controls/EditBox.h
        Controls/ListBox.h
        Controls/RadioButton.h
        Controls/ScrollBar.h
        Controls/Slider.h
        Controls/Static.h
        Utils/Text.h
        Utils/SpriteVertexStruct.h
        Utils/ModificationStack.h
        Utils/BlendColor.h
        )

add_definitions(-DOBJGLUF_GUI_EXPORTS)

add_library(objglufgui ${objglufgui_SOURCES} ${objglufgui_HEADERS})

target_link_libraries(objglufgui glfw ${GLFW_LIBRARIES} objgluf freetype glad)
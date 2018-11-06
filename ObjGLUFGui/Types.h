#ifndef OBJGLUF_TYPES_H
#define OBJGLUF_TYPES_H

#include "Exports.h"

namespace GLUF {
/*
======================================================================================================================================================================================================
Forward Declarations

*/

class OBJGLUF_GUI_API DialogResourceManager;

class OBJGLUF_GUI_API Control;

class OBJGLUF_GUI_API Button;

class OBJGLUF_GUI_API Static;

class OBJGLUF_GUI_API CheckBox;

class OBJGLUF_GUI_API RadioButton;

class OBJGLUF_GUI_API ComboBox;

class OBJGLUF_GUI_API Slider;

class OBJGLUF_GUI_API EditBox;

class OBJGLUF_GUI_API ListBox;

class OBJGLUF_GUI_API ScrollBar;

class OBJGLUF_GUI_API Element;

class OBJGLUF_GUI_API Font;

class OBJGLUF_GUI_API Dialog;

class OBJGLUF_GUI_API TextHelper;

struct ElementHolder;
struct TextureNode;
struct FontNode;

/*
======================================================================================================================================================================================================
Enumerations and Type Aliases

*/

enum MessageType {
    MB = 0,
    CURSOR_POS = 1,
    CURSOR_ENTER = 2,
    SCROLL = 3, //since i am using 32 bit integers as input, this value will be multiplied by 1000 to preserver any relevant decimal points
    KEY = 4,//don't' support joysticks yet
    UNICODE_CHAR = 5,
    RESIZE = 6,
    POS = 7,
    CLOSE = 8,
    REFRESH = 9,
    FOCUS = 10,
    ICONIFY = 11,
    FRAMEBUFFER_SIZE = 12
};


enum ControlType {
    CONTROL_BUTTON,
    CONTROL_STATIC,
    CONTROL_CHECKBOX,
    CONTROL_RADIOBUTTON,
    CONTROL_COMBOBOX,
    CONTROL_SLIDER,
    CONTROL_EDITBOX,
    CONTROL_IMEEDITBOX,
    CONTROL_LISTBOX,
    CONTROL_SCROLLBAR,
    CONTROL_CUSTOM
};

enum ControlState {
    STATE_NORMAL = 0,
    STATE_DISABLED = 1,
    STATE_FOCUS = 2,
    STATE_MOUSEOVER = 3,
    STATE_PRESSED = 4,
    STATE_HIDDEN = 5//THIS SHALL ALWAYS BE LAST
};

//WIP
enum Event {
    EVENT_BUTTON_CLICKED = 0,
    EVENT_COMBOBOX_SELECTION_CHANGED,
    EVENT_RADIOBUTTON_CHANGED,
    EVENT_CHECKBOXCHANGED,
    EVENT_SLIDER_VALUE_CHANGED,
    EVENT_SLIDER_VALUE_CHANGED_UP,
    EVENT_EDITBOX_STRING,
    EVENT_EDITBOX_CHANGE,//when the listbox contents change due to user input
    EVENT_LISTBOX_ITEM_DBLCLK,
    EVENT_LISTBOX_SELECTION,//when the selection changes in a single selection list box
    EVENT_LISTBOX_SELECTION_END,
};


enum Charset {
    ASCII = 0,
    Numeric = 2,
    Alphabetical = 3,
    AlphaNumeric = 4,
    Unicode = 5
};

/*

Alphabetized list of  GUI Pointer Aliases

*/
using ButtonPtr                 = std::shared_ptr<Button>;
using CheckBoxPtr               = std::shared_ptr<CheckBox>;
using ComboBoxPtr               = std::shared_ptr<ComboBox>;
using ControlPtr                = std::shared_ptr<Control>;
using DialogPtr                 = std::shared_ptr<Dialog>;
using DialogResourceManagerPtr  = std::shared_ptr<DialogResourceManager>;
using EditBoxPtr                = std::shared_ptr<EditBox>;
using ElementPtr                = std::shared_ptr<Element>;
using ListBoxPtr                = std::shared_ptr<ListBox>;
using RadioButtonPtr            = std::shared_ptr<RadioButton>;
using ScrollBarPtr              = std::shared_ptr<ScrollBar>;
using SliderPtr                 = std::shared_ptr<Slider>;
using StaticPtr                 = std::shared_ptr<Static>;
using TextHelperPtr             = std::shared_ptr<TextHelper>;
using ElementHolderPtr          = std::shared_ptr<ElementHolder>;
using FontNodePtr               = std::shared_ptr<FontNode>;
using TextureNodePtr            = std::shared_ptr<TextureNode>;

using TextureIndex      = uint32_t;
using TextureIndexResMan= uint32_t;
using FontIndex         = uint32_t;
using FontIndexResMan   = uint32_t;
using ElementIndex      = uint32_t;
using ControlIndex      = uint32_t;
using RadioButtonGroup  = uint32_t;
using Bitfield          = uint32_t;
using BitfieldL         = uint64_t;
using Size              = uint32_t;
using Value             = int32_t;
using UValue            = uint32_t;
using Index             = uint32_t;
using sIndex            = int32_t;
using KeyId             = uint32_t;

}
#endif //OBJGLUF_TYPES_H

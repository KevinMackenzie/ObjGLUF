/*

Copyright (C) 2015  Kevin Mackenzie

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License http://www.gnu.org/licenses/gpl-3.0.en.html 
for more details.

*/

#pragma once

#include "../ObjGLUF/ObjGLUF.h"


/*


TODO:

    -Support XML loading of dialogs


*/


/*

Note:
    -Any Function in the Library may throw and 'std::bad_alloc' without any express warning (even those marked 'noexcept'), 
        however the exception is probably the least of your problems in the event that it is thrown
*/

namespace GLUF
{
/*
======================================================================================================================================================================================================
Positioning and Controls Macros

*/

#define GT_CENTER    (1<<0)
#define GT_LEFT        (1<<1)
#define GT_RIGHT    (1<<2)

#define GT_VCENTER    (1<<3)//WARNING: this looks kind of funny with hard rect off
#define GT_TOP      (1<<4)
#define GT_BOTTOM   (1<<5)//WARNING: this looks kind of funny with hard rect off

#define GT_MULTI_LINE (1<<6) //NOTE: this is ONLY used by EditBox

/*
======================================================================================================================================================================================================
Forward Declarations

*/

class OBJGLUF_API DialogResourceManager;
class OBJGLUF_API Control;
class OBJGLUF_API Button;
class OBJGLUF_API Static;
class OBJGLUF_API CheckBox;
class OBJGLUF_API RadioButton;
class OBJGLUF_API ComboBox;
class OBJGLUF_API Slider;
class OBJGLUF_API EditBox;
class OBJGLUF_API ListBox;
class OBJGLUF_API ScrollBar;
class OBJGLUF_API Element;
class OBJGLUF_API Font;
class OBJGLUF_API Dialog;
class OBJGLUF_API TextHelper;
struct ElementHolder;
struct TextureNode;
struct FontNode;

/*
======================================================================================================================================================================================================
Enumerations and Type Aliases

*/

enum MessageType
{
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


enum ControlType
{
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

enum ControlState
{
    STATE_NORMAL = 0,
    STATE_DISABLED = 1,
    STATE_FOCUS = 2,
    STATE_MOUSEOVER = 3,
    STATE_PRESSED = 4,
    STATE_HIDDEN = 5//THIS SHALL ALWAYS BE LAST
};

//WIP
enum Event
{
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


enum Charset
{
    ASCII = 0,
    Numeric = 2,
    Alphabetical = 3,
    AlphaNumeric = 4,
    Unicode = 5
};

/*

Alphabetized list of  GUI Pointer Aliases

*/
using ButtonPtr                 = std::shared_ptr < Button > ;
using CheckBoxPtr               = std::shared_ptr < CheckBox > ;
using ComboBoxPtr               = std::shared_ptr < ComboBox > ;
using ControlPtr                = std::shared_ptr < Control > ;
using DialogPtr                 = std::shared_ptr < Dialog > ;
using DialogResourceManagerPtr  = std::shared_ptr < DialogResourceManager > ;
using EditBoxPtr                = std::shared_ptr < EditBox > ;
using ElementPtr                = std::shared_ptr < Element > ;
using ListBoxPtr                = std::shared_ptr < ListBox > ;
using RadioButtonPtr            = std::shared_ptr < RadioButton > ;
using ScrollBarPtr              = std::shared_ptr < ScrollBar > ;
using SliderPtr                 = std::shared_ptr < Slider > ;
using StaticPtr                 = std::shared_ptr < Static > ;
using TextHelperPtr             = std::shared_ptr < TextHelper > ;
using ElementHolderPtr          = std::shared_ptr < ElementHolder > ;
using FontNodePtr               = std::shared_ptr < FontNode > ;
using TextureNodePtr            = std::shared_ptr < TextureNode > ;

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

/*
======================================================================================================================================================================================================
Setup and Fundemental Utility Functions

*/

OBJGLUF_API GLuint GetWindowHeight();
OBJGLUF_API GLuint GetWindowWidth();

/*

CallbackFuncPtr

    Note:
        This may do whatever it pleases, however
        callbackFunc must explicitly call the callback methods of the 
        dialog manager and the dialog classes (and whatever else)
        to achieve purposeful use of this library.  
        For each 'MessageType', see the glfw documentation 
        for help.  If 'msg' requires non-integer data, 
        the floating point data will be multiplied 
        by 1000 and the rest will be truncated.  For
        more documentation see GLFW 'Input' documentation.

    Returns:
        'bool': true: the message is consumed; false: the message is not consumed

*/
using CallbackFuncPtr = bool(*)(MessageType, int, int, int, int);



/*
EventCallbackReceivable

A base class for all class which want to be able to have an event callback

*/
class EventCallbackReceivable
{
protected:
    virtual void EventCallback() = 0;

public:
    static void EventCallbackStatic(Event event, int ctrlId, Control);
};
using EventCallbackReceivablePtr = std::shared_ptr < EventCallbackReceivable > ;

/*
EventCallbackFuncPtr

    Parameters:
        'Event': what event occured
        'ControlPtr': the control which received the event
        'EventCallbackReceivablePtr': the if applicable, the instance of the class that will be called

    Note:
        the final parameter must be derived from 'EventCallbackReceivable'
        
*/
using EventCallbackFuncPtr = void(*)(Event, ControlPtr&, const EventCallbackReceivablePtr&);

/*

Shorthand Notation for Style

*/
#define _GUI_CALLBACK_PARAM MessageType msg, int32_t param1, int32_t param2, int32_t param3, int32_t param4
#define _PASS_CALLBACK_PARAM msg, param1, param2, param3, param4

/*
InitGui --Main Setup Function

    Note:

        This must be called AFTER 'InitOpenGLExtentions()'

    Parameters:
        'initializedWindow': an initialized GLFW window
        'callbackFunc': the Callback function for GLFW messages
        'controltex': the OpenGL id for the default control texture 

*/
OBJGLUF_API bool InitGui(GLFWwindow* initializedWindow, CallbackFuncPtr callbackFunc, GLuint controltex);

//swaps callback functions, returns old
OBJGLUF_API CallbackFuncPtr ChangeCallbackFunc(CallbackFuncPtr newCallback);


/*
TextureNode

    Note:
        A barebones texture node; plans for future expansion

    Data Members:
        'mTextureElement': the index of the texture within the dialog

*/
struct TextureNode
{
    TextureIndex mTextureElement;
};


/*
GenericData

    Note:
        This should be the base for all data which goes in 'ListBox' or 'ComboBox'

*/
struct GenericData
{};


/*
ListBoxItem

    Data Members:
        'mText': the text to represent the data
        'mData': the data this list box represents
        'mVisible': is visible?
        'mActiveRegion': the active region on the screen

*/
typedef struct ListBoxItem_t
{
    std::wstring mText;
    GenericData& mData;
    bool mVisible;
    Rect mTextRegion;

    ListBoxItem_t(GenericData& data) : mData(data){}
} ListBoxItem, ComboBoxItem;
using ListBoxItemPtr = std::shared_ptr < ListBoxItem > ;
using ComboBoxItemPtr = std::shared_ptr < ComboBoxItem > ;

}

/*

The Implementation for all template functions

*/
#include "GLUFGuiTemplates.inl"

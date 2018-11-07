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
#include "Types.h"
#include "Control.h"


/*


TODO:

    -Support XML loading of dialogs


*/


/*

Note:
    -Any Function in the Library may throw and 'std::bad_alloc' without any express warning (even those marked 'noexcept'), 
        however the exception is probably the least of your problems in the event that it is thrown
*/

namespace GLUF {
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
using CallbackFuncPtr = bool (*)(MessageType, int, int, int, int);


/*
EventCallbackReceivable

A base class for all class which want to be able to have an event callback

*/
class EventCallbackReceivable {
protected:
    virtual void EventCallback() = 0;

public:
    static void EventCallbackStatic(Event event, int ctrlId, Control&);
};

using EventCallbackReceivablePtr = std::shared_ptr<EventCallbackReceivable>;

/*
EventCallbackFuncPtr

    Parameters:
        'Event': what event occured
        'ControlPtr': the control which received the event
        'EventCallbackReceivablePtr': the if applicable, the instance of the class that will be called

    Note:
        the final parameter must be derived from 'EventCallbackReceivable'
        
*/
using EventCallbackFuncPtr = void (*)(Event, ControlPtr &, const EventCallbackReceivablePtr &);

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
OBJGLUF_API bool InitGui(GLFWwindow *initializedWindow, CallbackFuncPtr callbackFunc, GLuint controltex);

//swaps callback functions, returns old
OBJGLUF_API CallbackFuncPtr ChangeCallbackFunc(CallbackFuncPtr newCallback);


/*
TextureNode

    Note:
        A barebones texture node; plans for future expansion

    Data Members:
        'mTextureElement': the index of the texture within the dialog

*/
struct TextureNode {
    TextureIndex mTextureElement;
};


/*
GenericData

    Note:
        This should be the base for all data which goes in 'ListBox' or 'ComboBox'

*/
struct GenericData {
};


/*
ListBoxItem

    Data Members:
        'mText': the text to represent the data
        'mData': the data this list box represents
        'mVisible': is visible?
        'mActiveRegion': the active region on the screen

*/
typedef struct ListBoxItem_t {
    std::wstring mText;
    GenericData &mData;
    bool mVisible;
    Rect mTextRegion;

    ListBoxItem_t(GenericData &data) : mData(data) {}
} ListBoxItem, ComboBoxItem;
using ListBoxItemPtr = std::shared_ptr<ListBoxItem>;
using ComboBoxItemPtr = std::shared_ptr<ComboBoxItem>;

}

/*

The Implementation for all template functions

*/
#include "GLUFGuiTemplates.inl"

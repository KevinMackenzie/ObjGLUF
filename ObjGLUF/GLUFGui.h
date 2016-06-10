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

#include "ObjGLUF.h"


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

#define GT_CENTER	(1<<0)
#define GT_LEFT		(1<<1)
#define GT_RIGHT    (1<<2)

#define GT_VCENTER	(1<<3)//WARNING: this looks kind of funny with hard rect off
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
	CURSOR_POS,
	CURSOR_ENTER,
	SCROLL, //since i am using 32 bit integers as input, this value will be multiplied by 1000 to preserver any relevant decimal points
	KEY,//don't' support joysticks yet
	UNICODE_CHAR,
	RESIZE,
	POS,
	CLOSE,
	REFRESH,
	FOCUS,
	ICONIFY,
	FRAMEBUFFER_SIZE
};

//WIP
enum FontWeight
{
    FONT_WEIGHT_HAIRLINE = 0,
    FONT_WEIGHT_THIN,
    FONT_WEIGHT_ULTRA_LIGHT,
    FONT_WEIGHT_EXTRA_LIGHT,
    FONT_WEIGHT_LIGHT,
    FONT_WEIGHT_BOOK,
    FONT_WEIGHT_NORMAL,
    FONT_WEIGHT_MEDIUM,
    FONT_WEIGHT_SEMI_BOLD,
    FONT_WEIGHT_BOLD,
    FONT_WEIGHT_EXTRA_BOLD,
    FONT_WEIGHT_HEAVY,
    FONT_WEIGHT_BLACK,
    FONT_WEIGHT_EXTRA_BLACK,
    FONT_WEIGHT_ULTRA_BLACK
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
    ASCIIExtended = 1,
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
using EventCallbackFuncPtr = void(__cdecl*)(Event, ControlPtr&, const EventCallbackReceivablePtr&);

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
======================================================================================================================================================================================================
Everything Fonts

*/

using FontPtr = std::shared_ptr<Font>;
using FontSize = uint32_t;//in 'points'

//TODO: support dpi scaling
#define _POINTS_PER_PIXEL 1.333333f
#define _POINTS_TO_PIXELS(points) (FontSize)((float)points * _POINTS_PER_PIXEL)
#define _PICAS_TO_POINTERS(picas) ((picas) * 6.0)


OBJGLUF_API void SetDefaultFont(FontPtr& pDefFont);


class LoadFontException : public Exception
{
public:
    virtual const char* what() const
    {
        return "Error Loading Freetype Font";
    }

    EXCEPTION_CONSTRUCTOR(LoadFontException);
};

/*
LoadFont

    Parameters:
        'font': an uninitialized font
        'rawData': the raw data to load font from
        'fontHeight': how tall should the font be in points?

    Throws:
        'LoadFontException': if font loading failed

*/
OBJGLUF_API void LoadFont(FontPtr& font, const std::vector<char>& rawData, FontSize fontHeight);

OBJGLUF_API FontSize GetFontHeight(const FontPtr& font);




/*
======================================================================================================================================================================================================
Fundemental Utility Classes For Dialog Element Usage

*/


/*
BlendColor

    Note:
        Used to modulate colors for different control states to provide a responsive GUI experience
        Internally, Colors are represented as signed 16 bit integer vectors, because issues arise when using 
            'glm::mix' with unsigned values

    Data Members:
        'mStates': all of the different control states which can exist
        'mCurrentColor': the current color of the blend state

*/
using HighBitColor = glm::i16vec4;
using ColorStateMap = std::map < ControlState, HighBitColor >;

struct BlendColor
{
    float mPrevBlendTime = 0.0f;
    ColorStateMap       mStates;
    HighBitColor        mCurrentColor;
public:

    BlendColor();

    /*
    Init

        Parameters:
            'defaultColor': the default color state
            'disabledColor': the disabled color state
            'hiddenColor': the hidden color state
    
    */
    void        Init(const Color& defaultColor, const Color& disabledColor = { 128, 128, 128, 200 }, const Color& hiddenColor = { 255, 255, 255, 0 });

    /*
    Blend
        
        Parameters:
            'state': the state to blend from the current to
            'elapsedTime': the time elapsed since the begin of the blend period
            'rate': how quickly to blend between states

        Note:
            this is designed to be called every update cycle to provide a smooth blend animation
    
    */
	void        Blend(ControlState state, float elapsedTime, float rate = 5.0f);

    /*
    SetCurrent

        Parameters:
            'current': the color to set the current state
            'state': the state to set the current state to
    
    */
	void		SetCurrent(const Color& current);
	void		SetCurrent(ControlState state);

    /*
    SetAll

        Parameters:
            'color': the color to set all of the states to; used for static elements
    
    */
	void		SetAll(const Color& color);

    Color GetState(ControlState state) const noexcept;
    void        SetState(ControlState state, const Color& col) noexcept;
    Color GetCurrent() const noexcept;
};

/*
Element

    Note:
        Contains all the display tweakables for a sub-control

    Data Members:
        'mTextureIndex': the index of the texture for this element
        'mFontIndex': the index of the font for this element
        'mTextFormatFlags': a bitfield of formatting flags for the text for this element
        'mUVRect': Bounding rect for this element on the composite texture (Given in UV coords of the texture)
        'mTextureColor': the blend color for the texture for the element
        'mFontColor': the blend color for the font for this element
*/
class Element
{
public:

	TextureIndex mTextureIndex;
    FontIndex mFontIndex;
    Bitfield mTextFormatFlags;

    Rectf mUVRect;

    BlendColor mTextureColor;
    BlendColor mFontColor;

    /*
    SetTexture

        Parameters:
            'textureIndex': the index of the texture within the dialog resource manager to use
            'uvRect': the UV coordinates of the element within the texture
            'defaultTextureBlendColor': the default texture blend color
    
    */
    void    SetTexture(TextureIndex textureIndex, const Rectf& uvRect, const Color& defaultTextureColor = { 255, 255, 255, 255 });

    /*
    SetFont

        Parameters:
            'font': the font for this element to use
            'defaultFontColor': the default color for the font
            'textFormat': a bitfield of the horizontal and vertical text formatting
    
    */
    void    SetFont(FontIndex font, const Color& defaultFontColor = { 255, 255, 255, 255 }, Bitfield textFormat = GT_CENTER | GT_VCENTER);

    /*
    Refresh
        
        Note:
            This function sets the current blend states to the 'hidden' state
    */
	void    Refresh();
};



/*
======================================================================================================================================================================================================
Dialog Class

*/


class ControlCreationException : public Exception
{
public:
    virtual const char* what() const override
    {
        return "Control Falied to be Created!";
    }

    EXCEPTION_CONSTRUCTOR(ControlCreationException);
};

/*
Dialog

    Note:

        All controls must be assigned to a dialog, which handles
            input and rendering for the controls.

        Shared resource access. Fonts and textures are shared among
            all the controls.

    Data Members:
        'mFirstTime': true: 'MsgProc' has not been called yet; false: 'MsgProc' has been called at least once before
        'mLocked': whether or not the dialog is locked (i.e. position, maximized state, etc)
        'mAutoClamp': force the dialog rect to always be within the bounds of the window
        'mGrabAnywhere': whether or not to allow the dialog to be moved around the screen by being clicked anywhere instead of just the caption
        'mDragged': this keeps the dialog from minimizing/maximizing when dragging the dialog around
        'mDefaultControl': a pointer to the control which should receive focus when the dialog goes into focus the first time
        'sTimeRefresh': the minimum time before refreshing the dialog, usually 0
        'mTimePrevRefresh': the time of the previous refresh
        'mControlFocus': the control which has the focus
        'mControlPressed': the control currently pressed
        'mControlMouseOver': the control which is hovered over
        'mVisible': is the dialog visible?
        'mCaptionEnabled': does this dialog show a caption?
        'mMinimized': is the dialog minimized?
        'mDrag': does the dialog support dragging?
        'mCaptionText': the text of the caption
        'mRegion': the rect which contains the dialog
        'mCaptionHeight': the hight of the caption in pixels
        'mDialogManager': a pointer to the SINGLE dialog resource manager
        'mCallbackEvent': the callback function for this dialog's events
        'mCallbackContext': the class which the callback event will be called on; can be nullptr
        'mTextures': a list of texture indices which the dialog uses
        'mFonts': a list of font indices which the dialog uses
        'mDefaultElements': the list of default elements.  When a control is added, it sets gives it the default elements for this dialog
        'mCapElement': the element for the caption
        'mDlgElement': the element for the client area
        'mNextDialog': the dialog which gives this focus upon deletion
        'mPrevDialog': the dialog to give focus after this is deleted
        'mNonUserEvent': can events be triggered from sources other than the user
        'mKeyboardInput': does the dialog handle keyboard events?
        'mMouseInput': does the dialog handle mouse events?
        'mMousePosition': the mouse position of the window; cached at the beginning of the message procedure
        'mMousePositionDialogSpace': the mouse position relative to the dialog origin
        'mMousePositionOld': the mouse position cached when the mouse is pressed down to detect whether it is a drag or a minimize/maximize event

*/
class Dialog : public std::enable_shared_from_this<Dialog>
{
	friend class DialogResourceManager;

    bool mFirstTime = true;

    bool mLocked = true;
    bool mAutoClamp = false;
    bool mGrabAnywhere = false;
    bool mDragged = false;

    ControlPtr mDefaultControl = nullptr;

    static double sTimeRefresh;
    double mTimePrevRefresh = 0.0;

    //TODO: these might not want to be static
    static ControlPtr sControlFocus;
    static ControlPtr sControlPressed;
    ControlPtr mControlMouseOver = nullptr;

    bool mVisible = true;
    bool mCaptionEnabled = false;
    bool mMinimized = false;
    bool mDrag = false;

    std::wstring mCaptionText;

    //these are assumed to be based on the origin (bottom left)
    Rect mRegion;

    long mCaptionHeight = 0;

    DialogResourceManagerPtr mDialogManager = nullptr;

    EventCallbackFuncPtr mCallbackEvent = nullptr;
    EventCallbackReceivablePtr mCallbackContext = nullptr;

    std::map <TextureIndex, TextureIndexResMan> mTextures;
    std::map <FontIndex, FontIndexResMan> mFonts;

    std::map<ControlIndex, ControlPtr> mControls;
    std::vector<ElementHolderPtr> mDefaultElements;

    Element mCapElement;
    Element mDlgElement;

    DialogPtr mNextDialog = nullptr;
    DialogPtr mPrevDialog = nullptr;

    bool mNonUserEvents = true;
    bool mKeyboardInput = true;
    bool mMouseInput = true;

    Point mMousePosition;
    Point mMousePositionDialogSpace;
    Point mMousePositionOld;

    /*
    Constructor/Destructor

        Throws:
            no-throw guaratee

    */
    Dialog();
    friend std::shared_ptr<Dialog> CreateDialog();

public:
	~Dialog();

	
    /*
    Init

        Parameters:
            'manager': a pointer to the dialog resource manager
            'registerDialog': whether or not to register this dialog with the resource manager, usually true
            'textureIndex': the texture to use for this dialogs default controls

        Throws:
            'std::invalid_argument': if 'manager == nullptr'

    */
	void Init(DialogResourceManagerPtr& manager, bool registerDialog = true);
	void Init(DialogResourceManagerPtr& manager, bool registerDialog, TextureIndexResMan textureIndex);

    /*
    MsgProc

        Parameters:
            see CallbackFuncPtr for details

        Note:
            Handles message handling for this dialog and all sub controls

        Throws:
            no-throw guarantee
    
    */
	bool MsgProc(MessageType msg, int32_t param1, int32_t param2, int32_t param3, int32_t param4) noexcept;

    /*
    Add*
    
        Note:
            for adding controls to the dialog

        Parameters:
            'ID': the id to give the control
            'strText': the text the control has
            'region': the rect containing the region, relative to openGL origin (bottom left)
            'textFlags': a bitfield of flags for how to display the text
            'isDefault': is this control the default control to get focus
            'ctrlPtr': a pointer to get a pointer to the created control
            'hotKey': a keyboard button that will activate the control just like clicking it would
            'checked': should the check box default to the checked state
            'buttonGroup': the id of the button group for the radio button
            'min': the minimum internal value for the slider control to have
            'max': the maximum internal value for the slider control to have
            'value': the default position for the slider to have from 'min' to 'max'
            'charset': which character set should the edit box use
            'style': the list box style to use; a bitfield of styles; currently only 'MULTISELECTION' is supported

        Throws:
            'ControlCreationException': if control initialization failed

    
    */
    void AddStatic(ControlIndex ID, const std::wstring& strText, const Rect& region, Bitfield textFlags = GT_LEFT | GT_TOP, bool isDefault = false, std::shared_ptr<StaticPtr> ctrlPtr = nullptr);
    void AddButton(ControlIndex ID, const std::wstring& strText, const Rect& region, int hotkey = 0, bool isDefault = false, std::shared_ptr<ButtonPtr> ctrlPtr = nullptr);
    void AddCheckBox(ControlIndex ID, const std::wstring& strText, const Rect& region, bool checked = false, int hotkey = 0, bool isDefault = false, std::shared_ptr<CheckBoxPtr> ctrlPtr = nullptr);
    void AddRadioButton(ControlIndex ID, RadioButtonGroup buttonGroup, const std::wstring& strText, const Rect& region, bool checked = false, int hotkey = 0, bool isDefault = false, std::shared_ptr<RadioButtonPtr> ctrlPtr = nullptr);
    void AddComboBox(ControlIndex ID, const Rect& region, int hotKey = 0, bool isDefault = false, std::shared_ptr<ComboBoxPtr> ctrlPtr = nullptr);
    void AddSlider(ControlIndex ID, const Rect& region, long min, long max, long value, bool isDefault = false, std::shared_ptr<SliderPtr> ctrlPtr = nullptr);
    void AddEditBox(ControlIndex ID, const std::wstring& strText, const Rect& region, Charset charset = Unicode, GLbitfield textFlags = GT_LEFT | GT_TOP, bool isDefault = false, std::shared_ptr<EditBoxPtr> ctrlPtr = nullptr);
    void AddListBox(ControlIndex ID, const Rect& region, Bitfield style = 0, std::shared_ptr<ListBoxPtr> ctrlPtr = nullptr);

    /*
    AddControl
        
        Note:
            This is the generic function for adding custom or preconstructed controls

        Parameters:
            'control': a pointer to the control to add

        Throws:
            'ControlCreationException': if control initialization failed
    
    */
	void AddControl(ControlPtr& control);

    /*
    InitControl
        
        Note:
            Initializes a control with the default element, and calls 'OnInit' on it

        Throws:
            'ControlCreationException': if control initialization failed

    */
	void InitControl(ControlPtr& control);


    /*
    GetControl
    
        Parameters:
            'ID': the id of the control to retreive
            'controlType' the control type to filter

        Returns:
            a poitner to the control requested, in release mode, returns nullptr if 'ID' not found

        Throws:
            'std::bad_cast' if 'T' is not derived from 'Control'
            'std::invalid_argument': if 'ID' is not found, but only in _DEBUG mode
    
    */
    template<typename T>
    std::shared_ptr<T>  GetControl(ControlIndex ID) const;
    ControlPtr      GetControl(ControlIndex ID, ControlType controlType) const;


    /*
    GetControlAtPoint

        Parameters:
            'pt': the point in screen coordinates to look for the control

        Returns:
            a pointer to the control found, returns nullptr if none found

        Throws:
            no-throw guarantee
    
    */
	ControlPtr GetControlAtPoint(const Point& pt) const noexcept;

    /*
    Set/Get Control Enabled

        Parameters:
            'ID': id of the control
            'bEnabled': new enabled state

        Returns:
            the enabled state

        Throws:
            'std::invalid_argument': if 'ID' does not exist, only in _DEBUG
    
    */
    bool GetControlEnabled(ControlIndex ID) const;
    void SetControlEnabled(ControlIndex ID, bool bEnabled);

    /*
    ClearRadioButtonGroup
        
        Note:
            Resets the radio button group

        Parameters:
            'group': the id of the group
    
    */
    void ClearRadioButtonGroup(RadioButtonGroup group);

    /*
    GetRadioButtonGroup

        Parameters:
            'groupId': the id of the radio button group

        Returns:
            a vector of RadioButton's which have the given group
    
    */
    std::vector<RadioButtonPtr> GetRadioButtonGroup(RadioButtonGroup groupId);

    /*
    ClearComboBox

        Note:
            Removes all of the items from this combo box

        Parameters:
            'ID': the id of the combo box

        Throws:
            'std::invalid_argument': if 'ID' does not exist, or is not a combo box, in _DEBUG mode only
    
    */
	void ClearComboBox(ControlIndex ID);

    /*
    Set/Get DefaultElement

        Parameters:
            'controlType': the control type to set the default to
            'elementIndex': the element id within the control to set
            'element': the element to set

        Returns:
            the element of the control at 'elementIndex'

        Throws:
            'std::invalid_argument': if 'elementIndex' is not found within 'controlType', but only in _DEBUG; or if 'element' == nullptr
    
    */
    void            SetDefaultElement(ControlType controlType, ElementIndex elementIndex, const Element& element);
    Element     GetDefaultElement(ControlType controlType, ElementIndex elementIndex) const;


    /*
    SendEvent

        Note:
            Called by the controls for sending events

        Parameters:
            'ctrlEvent': the event to send
            'triggeredByUser': whether or not the event was triggered by the user
            'control': the control which sent the control

        Throws:
            no-throw guarantee
    
    */
	void SendEvent(Event ctrlEvent, bool triggeredByUser, ControlPtr control) noexcept;


    /*
    RequestFocus

        Parameters:
            'control': the control to request focus to

        Throws:
            'std::invalid_argument': if 'control' == nullptr, in _DEBUG mode
    
    */
	void RequestFocus(ControlPtr& control);

    /*
    Draw*
    
        Note:
            These functions are render helpers, for rendering things

        Parameters:
            'rect': the rect to draw
            'color': the color of the rect
            'element': the element to use for texture and style
            'depth': how far back to render the rect
            'textured': use texture or color?
            'text': the text to draw
            'shadow': should text have shadow on it
            'hardRect': should the text be bound completely to the rect
            'transform': should the rect be transformed openGL space from screen space

        Throws:
            'std::invalid_argument' if 'elemement' == nullptr in _DEBUG

    */
	void DrawRect(const Rect& rect, const Color& color, bool transform = true);
	//void DrawPolyLine(Point* apPoints, uint32_t nNumPoints, Color color);
	void DrawSprite(const Element& element, const Rect& rect, float depth, bool textured = true);
    void DrawText(const std::wstring& text, const Element& element, const Rect& rect, bool shadow = false, bool hardRect = false);

    /*
    CalcTextRect -- WIP --

        Note:
            This calculates a rect based on a set of text, and font size, formatting, etc

            This is a WIP Function!
        
        Parameters:
            'text': the text to use
            'element': the element to use for font/formatting/size information
            'rect the output rect containing a bounding box for the text

        Throws:
            'std::invalid_argument': if 'element' == nullptr, in _DEBUG
    
    */
	void CalcTextRect(const std::wstring& text, const Element& element, Rect& rect) const;


    /*
    Attribute Setters and Getters

        Note:
            For Documentation of what each of these do, see documentation for 'Data Members'

        Throws:
            no-throw guarantee
    
    */
	bool        GetVisible() const noexcept							    { return mVisible;						    }
    bool        GetMinimized() const noexcept                           { return mMinimized;                        }
    long        GetCaptionHeight() const noexcept                       { return mCaptionHeight;                    }
    Point   GetLocation() const noexcept                            { return{ mRegion.bottom, mRegion.left };   } 
    Rect    GetRegion() const noexcept                              { return mRegion;                           }
    long        GetWidth() const noexcept                               { return RectWidth(mRegion);            }
    long        GetHeight()	const noexcept                              { return RectHeight(mRegion);           } 

    Point   GetMousePositionDialogSpace() const noexcept            { return mMousePositionDialogSpace;         }

    DialogResourceManagerPtr GetManager() const noexcept            { return mDialogManager;                    }

    void        SetVisible(bool visible) noexcept                       { mVisible = visible;                       }
    void        SetMinimized(bool minimized) noexcept                   { mMinimized = minimized;                   }
    void        SetBackgroundColor(const Color& color) noexcept   { mDlgElement.mTextureColor.SetAll(color); }
    void        SetCaptionHeight(long height) noexcept                  { mCaptionHeight = height;                  }
    void        SetCaptionText(const std::wstring& text) noexcept       { mCaptionText = text;                      }
    void        SetLocation(long x, long y) noexcept                    { RepositionRect(mRegion, x, y);        }
    void        SetSize(long width, long height) noexcept               { ResizeRect(mRegion, width, height);   }
    static void	SetRefreshTime(float time) noexcept                     { sTimeRefresh = time;                      }

    void		Lock(bool lock = true) noexcept                         { mLocked = lock;                           }
    void		EnableGrabAnywhere(bool enable = true) noexcept         { mGrabAnywhere = enable;                   }
    void        EnableCaption(bool enable) noexcept                     { mCaptionEnabled = enable;                 }
    void		EnableAutoClamp(bool enable = true) noexcept            { mAutoClamp = enable;                      }
    void        EnableNonUserEvents(bool bEnable) noexcept              { mNonUserEvents = bEnable;                 }
    void        EnableKeyboardInput(bool bEnable) noexcept              { mKeyboardInput = bEnable;                 }
    void        EnableMouseInput(bool bEnable) noexcept                 { mMouseInput = bEnable;                    }
    bool        IsKeyboardInputEnabled() const noexcept                 { return mKeyboardInput;                    }

    /*    
    Set/Get Font/Texture
        
        Note:
            Fonts and textures must already be created from the dialog resource manager

        Parameters:
            'fontIndex': the font index within the dialog instance to set
            'resManFontIndex': the font index from the dialog resource manager to use
            'texIndex': the texture index within the dialog instance to set
            'resManTexIndex': the texture index from the dialog resource manager to use
            'index': the index within the dialog instance of the texture to get NOT within the DRM (dialog resource manager)

        Returns:
            The Font/Texture node at 'index'

        Throws:
            'std::out_of_range': if index/resManFontIndex/resManTexIndex do not exist in their respective locations

    */
    void                SetFont(FontIndex fontIndex, FontIndexResMan resManFontIndex);
    void                SetTexture(TextureIndex texIndex, TextureIndexResMan resManTexIndex);
    FontNodePtr	    GetFont(FontIndex index) const;
    TextureNodePtr	GetTexture(TextureIndex index) const;


    /*
    GetNext/PreviousControl

        Note:
            This is for cycling active controls using tab for example

        Parameters:
            'control': the control to cycle from
        
        Returns:
            the next or previous control to get focus

        Throws:
            TODO:
    
    */
	static ControlPtr GetNextControl(ControlPtr control);
	static ControlPtr GetPrevControl(ControlPtr control);

    /*
    ClampToScreen
        
        Note:
            This forces the dialog region to be within the window region

        Throws:
            no-throw guarantee
    */
	void ClampToScreen() noexcept;

    /*
    RemoveControl
        
        Parameters:
            'ID': the id of the control to remove

        Throws:
            'std::invalid_argument': in _DEBUG if no controls have id 'ID'
    
    */
	void RemoveControl(ControlIndex ID);

    /*
    RemoveAllControls

        Note:
            Removes all of the controls

        Throws:
            no-throw guarantee
    */
	void RemoveAllControls() noexcept;


    /*
    SetCallback

        Note:
            sets the callback function for your app to get message from control events

        Parameters:
            'callback': the callback function
            'userContext': the class to call the function on if applicable

        Throws:
            no-throw guarantee
    */
	void SetCallback(EventCallbackFuncPtr callback, EventCallbackReceivablePtr userContext = nullptr) noexcept;


    /*
    Refresh

        Note:
            this refreshes the dialog and all controls within it
            Called every frame by OnRender

        Throws:
            no-throw guarantee
    
    */
	void Refresh()  noexcept;

    /*
    OnRender

        Note:
            Call this every frame

        Parameters:
            'elapsedTime': the time since the previous call to 'OnRender()'

        Throws:
            no-throw guarantee
    
    */
	void OnRender(float elapsedTime) noexcept; 

    /*
    ClearFocus

        Note:
            Takes the focus from the control which has it, then sets it to nullptr

        Throws:
            no-throw guarantee

    */
	static void ClearFocus() noexcept;

    /*
    FocusDefaultControl

        Note:
            Sets the focus to the default control

        Throws:
            no-throw guarantee
    */
	void FocusDefaultControl() noexcept;

    /*
    ScreenSpaceToGLSpace
    
        Note:
            Converts screen space rect to a openGL space rect

        Parameters:
            'rc': the rect to convert
    
    */
    void ScreenSpaceToGLSpace(Rect& rc) noexcept;
    void ScreenSpaceToGLSpace(Point& pt) noexcept;


private:

    /*
    InitializeDefaultElements

        Note:
            pretty self explanitory;
            This is a monolithic function, so there is no support for XML loading of elements
    
    */
	void InitDefaultElements();

    /*
    Message Handlers
        
        Note:
            These are mainly to control which control has focus, and the state of the controls

        Parameters:
            'pt': the mouse position within the window

        Throws:
            no-throw guarantee
    
    */
	void OnMouseMove(const Point& pt) noexcept;
	void OnMouseUp(const Point& pt) noexcept;

    /*
    SetNextDialog

        Note:
            Sets the next dialog for cycling

        Parameters:
            'nextDialog': the dialog to switch to when this dialog closes            

        Throws:
            no-throw guarantee
    
    */
	void SetNextDialog(DialogPtr nextDialog) noexcept;

    /*
    OnCycleFocus

        Note: 
            Cycles focus between controls, or to the next/previous dialog

        Parameters:
            'forward': true: cycle forwards; false: cycle backwards

        Throws:
            no-throw guarantee

    */
	bool OnCycleFocus(bool forward) noexcept;

};


/*
======================================================================================================================================================================================================
Structs and Shared Resources

*/

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

//WIP, support more font options eg. stroke, italics, variable leading, etc.

/*
FontNode

    Note:
        Work in progress; support for more typographical options in the future, i.e.:
            stroke, italics, leading, etc.

    Data Members:
        'mWeight': WIP, doesn't do anything yet
        'mLeading': WIP, doesn't do anything yet
        'mFontType': an opaque pointer to the font to use, defaults to arial

*/
struct FontNode
{
	//FontSize mSize;
	FontWeight mWeight;
	FontSize mLeading;
	FontPtr mFontType;
};








/*
======================================================================================================================================================================================================
SpriteVertexStruct


*/



/*
SpriteVertexStruct

    Note:
        This is derived from the vertex struct class for use on AoS data packing

    Data Members:
        'mPos': a position
        'mColor': a color
        'mTexCoords': a uv coord

*/
struct SpriteVertexStruct : public VertexStruct
{
	glm::vec3 mPos;
	Color4f mColor;
    glm::vec2 mTexCoords;

    SpriteVertexStruct(){}
    SpriteVertexStruct(const glm::vec3& pos, const Color4f& color, const glm::vec2& texCoords) : 
        mPos(pos), mColor(color), mTexCoords(texCoords)
    {}

    virtual void* operator&() const override
    {
        char* ret = new char[size()];

        memcpy(ret, &mPos[0], 12);
        memcpy(ret + 12, &mColor[0], 16);
        memcpy(ret + 28, &mTexCoords[0], 8);

        return ret;
    }

    virtual size_t size() const override
    {
        return 36; // sizeof(mPos) + sizeof(mColor) + sizeof(mTexCoords);
    }

    virtual size_t n_elem_size(size_t element)
    {
        switch (element)
        {
        case 0:
            return 12;
        case 1:
            return 16;
        case 2:
            return 8;
        default:
            return 0;//if it is too big, just return 0; not worth an exception
        }
    }

    virtual void buffer_element(void* data, size_t element) override
    {
        switch (element)
        {
        case 0:
            mPos = static_cast<glm::vec3*>(data)[0];
        case 1:
            mColor = static_cast<Color4f*>(data)[0];
        case 2:
            mTexCoords = static_cast<glm::vec2*>(data)[0];
        default:
            break;
        }
    }

    static GLVector<SpriteVertexStruct> MakeMany(size_t howMany)
    {
        GLVector<SpriteVertexStruct> ret;
        ret.resize(howMany);

        return ret;
    }
};

/*
DialogResourceManager

    Note:
        Manages Resources Shared by the Dialog

    Data Members:
        'mWndSize': the size of the window
        'mSpriteBuffer': the vertex array for drawing sprites; use 'VertexArray'
            because it automatically handles OpenGL version restrictions
        'mDialogs': the list of registered dialogs
        'mTextureCache': a list of shared textures
        'mFontCache': a list of shared fonts

*/
class DialogResourceManager
{

    Point mWndSize;

    VertexArray mSpriteBuffer;
    std::vector<DialogPtr> mDialogs;
    std::vector<TextureNodePtr> mTextureCache;
    std::vector<FontNodePtr>    mFontCache;

    friend Dialog;
public:
	DialogResourceManager();
	~DialogResourceManager();

    /*
    MsgProg

        Note:
            This only cares about window resize messages in order to keep 'mWndSize' accurate

        Parameters:
            see 'EventCallbackFuncPtr' for details

        Throws:
            no-throw guarantee
    */
	bool MsgProc(MessageType msg, int32_t param1, int32_t param2, int32_t param3, int32_t param4) noexcept;
    
    /*
    ApplyRenderUI(Untex)
    
        Note:
            This sets up the global UI shader
            if 'Untex', it does not enable texture coordinates
        
        Throws:
            no-throw guarantee
    */
	void ApplyRenderUI() noexcept;
	void ApplyRenderUIUntex() noexcept;

    /*
    BeginSprites

        Note:
            Clears the sprite vertices; call before rendering anything

        Throws:
            no-throw guarantee
    */
	void BeginSprites() noexcept;

    /*
    EndSprites
        
        Note:
            This buffers and draws the data from the sprites
            This is generally called to render every element individually for flexibility

        Parameters:
            'element': an element whose formatting/texture data is used
            'textured': are the sprites textured

        Throws:
            'std::out_of_range': if element texture index is out of range
    */
	void EndSprites(const Element* element, bool textured);

    /*
    GetFont/TextureNode

        Parameters:
            'index': the index of the font node within the DRM
        
        Throws:
            'std::out_of_range': if 'index' is larger than the size of the font/texture cache
    */
	FontNodePtr    GetFontNode(FontIndex index) const		{ return mFontCache[index];		}
	TextureNodePtr GetTextureNode(TextureIndex index) const	{ return mTextureCache[index];	}


    /*
    GetTexture/FontCount

        Returns:
            the number of fonts/textures

        Throws:
            no-throw guarantee
    
    */
	Size GetTextureCount() const noexcept   { return static_cast<Size>(mTextureCache.size()); }
	Size GetFontCount() const noexcept      { return static_cast<Size>(mFontCache.size());    }

    /*
    AddFont

        Parameters:
            'font': a pointer to the typeface
            'leading': the vertical distance between lines of text
            'weight': the weight to use

        Returns:
            the index of the created font

        Throws:
            no-throw guarantee
        
    */
	FontIndex AddFont(const FontPtr& font, FontSize leading, FontWeight weight) noexcept;

    /*
    AddTexture
        
        Note:
            TODO: when  supports a more advanced texture system, pass one of those,
                but for now just require an openGL texture id

        Parameters:
            'texture': the OpenGL Texture Id to use

        Returns:
            the index of the created texture

        Throws:
            no-throw guarantee
    */
	TextureIndex AddTexture(GLuint texture) noexcept;

    /*
    RegisterDialog

        Note:
            This sets up the dialogs' focus order for tabbing

        Parameters:
            'dialog': the dialog to register

        Throws:
            no-throw guarantee
    
    */
	void RegisterDialog(const DialogPtr& pDialog) noexcept;

    /*
    UnregisterDialog

        Parameters:
            'dialog': the dialog to remove

        Throws:
            'std::invalid_argument': if 'dialog' == nullptr or if 'dialog' is not a registered dialog in _DEBUG mode
    
    */
	void UnregisterDialog(const DialogPtr& dialog);

    /*
    EnableKeyboardInputForAllDialogs
    
        Note:
            Pretty self explanitory

        Throws:
            no-throw guarantee
    */
	void EnableKeyboardInputForAllDialogs() noexcept;


    /*
    GetWindowSize()
        
        Note:
            This can also be used to update mWndSize; This function does not simply return the size, it actually 
                Gets the window size from GLFW

        Returns:
            The size of the window in pixels
    
    */
	Point GetWindowSize();
    
    /*
    GetOrthoMatrix

        Returns:
            the Orthographic projection matrix for the current window

        Throws:
            no-throw guarantee
    
    */
	glm::mat4 GetOrthoMatrix() noexcept;


    /*
    GetDialogPtrFromRef

        Returns:
            the std::shared_ptr version of the reference give, nullptr if not found

        Parameters:
            'ref': a reference to a dialog
    
    */
    DialogPtr GetDialogPtrFromRef(const Dialog& ref);
protected:
    /*
    ApplyOrtho

        Note:
            Gets the Ortho Matrix, then Applies it to the current draw call; used internally
    
        Throws:
            no-throw guarantee
    */
	void ApplyOrtho() noexcept;

};



/*
======================================================================================================================================================================================================
Control Base and Child Classes

Note:
    Child classes will only contain documentation for functions whose behavior may be ambiguous

*/

class ControlInitException
{
    virtual const char* what() const
    {
        return "Control or child failed to initialized correctly!";
    }

    EXCEPTION_CONSTRUCTOR(ControlInitException);
};

/*
Control


    Parents:
        'std::enable_shared_from_this': to allow passing 'this' as a shared pointer, however the 'this' pointer
            still needs to be on the heap for this to work, therefore the constructor is private, shared with 'std::make_shared'

    Data Members:
        'mID': the id of the control
        'mType': the control type
        'mHotkey': the virtual key code for this control's hotkey; represented by GLFW's keycodes
        'mEnabled': enable flag
        'mVisible': visibility flag
        'mMouseOver': is mouse over this control
        'mHasFocus': does control have input focus
        'mIsDefault': is this the default control
        'mRegion': the rect which this control occupies
        'mDialog': a pointer to the parent container
        'mIndex': the index of this control within the dialog
        'mElements': the elements making up this control

*/
class Control : public std::enable_shared_from_this<Control>
{
protected:
    /*
    Constructor
        
        Note:
            Constructor is private to only allow 'std::shared_ptr' to make instances of it, so 
                instances of this are only on the heap; this allows for 'std::enable_shared_from_this' to work

        Parameters:
            'dialog': the parent dialog

    */
    Control(Dialog& dialog);

    friend Dialog;

    ControlIndex mID;
    ControlType mType;
    KeyId  mHotkey;

    bool mEnabled;
    bool mVisible;
    bool mMouseOver;
    bool mHasFocus;
    bool mIsDefault;

    Rect mRegion;

    Dialog& mDialog;
    ControlIndex mIndex;

    //use a map, because there may be gaps in indices
    std::map<ElementIndex, Element> mElements;

    FontNodePtr GetElementFont(ElementIndex index);


public:
	virtual ~Control();


    /*
    OnInit

        Throws:
            'ControlInitException': if child class initialization fails
    
    */
    virtual void OnInit(){ UpdateRects(); }

    /*
    Refresh
        
        Note:
            Removes focus and mouse-over state, and refreshes all elements

        Throws:
            no-throw guarantee
    
    */
	virtual void Refresh() noexcept;

    /*
    Render

        Parameters:
            'elapsedTime': the time since the previous frame

        Throws:
            no-throw guarantee: child classes must NOT throw any critical exceptions
    
    */
	virtual void Render(float elapsedTime) noexcept{};

    /*
    MsgProc

        Parameters:
            see 'EventCallbackFuncPtr' for details

        Throws:
            no-throw guarantee
    */
	virtual bool MsgProc(MessageType msg, int32_t param1, int32_t param2, int32_t param3, int32_t param4) noexcept { return false; }

             
    /*
    Getters and Setters

        Note:
            Some of these may be overwritten by child classes, but
        Throws:
            GetElement: 'std::out_of_range'
    */
    virtual bool	GetEnabled() const noexcept		    { return mEnabled;              }
    virtual bool	GetVisible() const noexcept		    { return mVisible;              }
    int				GetID() const noexcept			    { return mID;                   }
    int				GetHotkey()	const noexcept		    { return mHotkey;               }
    ControlType     GetType() const	noexcept		    { return mType;                 }
	virtual bool	CanHaveFocus() const noexcept	    { return false;					}
    Rect            GetRegion() const noexcept          { return mRegion;               }
    Element&  	    GetElement(ElementIndex element);

    virtual void	SetEnabled(bool enabled) noexcept		    { mEnabled = enabled;                       }
    virtual void	SetVisible(bool visible) noexcept		    { mVisible = visible;                       }
    virtual void    SetRegion(const Rect& region) noexcept
                                                                { mRegion = region;                         }
    void			SetLocation(long x, long y) noexcept	    { RepositionRect(mRegion, x, y);        }
    void			SetSize(long width, long height) noexcept   { ResizeRect(mRegion, width, height);   }
    void			SetHotkey(KeyId hotkey) noexcept		{ mHotkey = hotkey;                         }
    void			SetID(int ID) noexcept					    { mID = ID;                                 }

    bool IsMouseOver()  const noexcept   { return mMouseOver; }
    bool IsFocused()    const noexcept   { return mHasFocus; }

    /*
    EventHandlers

        Note:
            These are barebones handlers, assuming the control does nothing at all upon these events

        Throws:
            no-throw guarantee; children may modify this behavior, but should not
    
    */
    virtual bool ContainsPoint(const Point& pt) const noexcept  
                                            { return PtInRect(mRegion, pt);                         }
	virtual void OnFocusIn() noexcept		{ mHasFocus = true;						    				}
	virtual void OnFocusOut() noexcept		{ mHasFocus = false;										}
	virtual void OnMouseEnter() noexcept	{ mMouseOver = true;										}
	virtual void OnMouseLeave() noexcept	{ mMouseOver = false;										}
	virtual void OnHotkey() noexcept		{ /*this function usually acts like the user 'clicked' it*/ }

    /*
    SetElement

        Note:
            'element' == nullptr, the resulted effect is that element location is deleted

        Parameters:
            'elementId': the id to give the element within this control
            'element': the element to add at that id

        Throws:
            no-throw guarantee

    */
	void SetElement(ElementIndex elementId, const Element& element) noexcept;

    /*
    SetTextColor

        Note:
            Sets the text color for all elements

        Parameters:
            'color': the color to set to

        Throws:
            no-throw guarantee
    
    */
	virtual void SetTextColor(const Color& color) noexcept;


protected:
    /*
    UpdateRects

        Note:
            This method is used to recreate the regions for different elements when the main rect changes

    */
    virtual void UpdateRects() noexcept{}
};



/*
ElementHolder

    Note:
        This class is used for default element data structures

    Data Members:
        'mControlType': which control type is this element for
        'mElementIndex': index of the element within the default elements
        'mElement': the element itself

*/
struct ElementHolder
{
	ControlType mControlType;
	ElementIndex mElementIndex;
	Element mElement;
};



/*
Static

    Note:
        A Static Text Control; Draws Text with no backgroud

    Data Members:
        'mText': the text to display
        'mTextFlags': the text formatting flats

*/
class Static : public Control
{
protected:
    std::wstring     mText;
    Bitfield     mTextFlags;

    Static(const Bitfield& textFlags, Dialog& dialog);
    friend std::shared_ptr<Static> CreateStatic(Bitfield textFlags, Dialog& dialog);

    Static() = delete;
    
public:


    /*
    ContainsPoint

        Note:
            This always returns false, because this control never should receive message
    
    */
	virtual bool ContainsPoint(const Point& pt) const override { return false; }

    /*
    Getters and Setters
    
        Throws:
            no-throw guarantee

    */
    void                  GetTextCopy(std::wstring& dest) const noexcept    { dest = mText;         }
	const std::wstring&   GetText() const noexcept                          { return mText;         }
    void                  SetText(const std::wstring& text) noexcept        { mText = text;         }
    void                  SetTextFlags(Bitfield flags) noexcept         { mTextFlags = flags;   }


    /*
    Overridden Unambiguous Member Functions

    */
    virtual void Render(float elapsedTime) noexcept override;
};


/*
Button

    DataMembers:
        'mPressed': boolean state of the button

*/
class Button : public Static
{

protected:

    Button() = delete;
    Button(Dialog& dialog);
    friend std::shared_ptr<Button> CreateButton(Dialog& dialog);

    bool mPressed;

public:

    /*
    Overridden Unambiguous Member Functions
    
    */
    virtual bool CanHaveFocus()	const noexcept override	{ return (mVisible && mEnabled); }
    virtual void Render(float elapsedTime) noexcept override;
    virtual bool ContainsPoint(const Point& pt) const noexcept override{ return PtInRect(mRegion, pt); }
    virtual bool MsgProc(MessageType msg, int32_t param1, int32_t param2, int32_t param3, int32_t param4) noexcept override;
    virtual void OnHotkey() noexcept override;

};


/*
CheckBox

    Parents:
        Button

    Data Members:
        'mChecked': is the box checked
        'mButtonRegion': the region the button covers
        'mTextRegion': the region the text covers

*/
class CheckBox : public Button
{

protected:

    CheckBox() = delete;
    CheckBox(bool checked, Dialog& dialog);
    friend std::shared_ptr<CheckBox> CreateCheckBox(bool checked, Dialog& dialog);

    bool mChecked;
    Rect mButtonRegion;
    Rect mTextRegion;

public:

    /*
    Setters and Getters

        Throws:
            no-throw guarantee
    
    */
	bool GetChecked() const noexcept         { return mChecked;                      }
	void SetChecked(bool checked) noexcept   { SetCheckedInternal(checked, false);   }

    /*
    Overridden Unambiguous Member Functions
    
    */
    virtual bool MsgProc(MessageType msg, int32_t param1, int32_t param2, int32_t param3, int32_t param4) noexcept override;
	virtual void Render(float elapsedTime) noexcept override;
    virtual void OnHotkey() noexcept override;
    virtual bool ContainsPoint(const Point& pt) const override;
    virtual void UpdateRects() noexcept override;

protected:

    /*
    SetCheckedInternal

        Note:
            sets the state of the box, but sends an event if it was from a user

        Parameters:
            'checked': state of the box
            'fromInput': was this a result of a user event

        Throws:
            no-throw guarantee
    
    */
	virtual void SetCheckedInternal(bool checked, bool fromInput);
};


/*
RadioButton

    Parents:
        CheckBox

    Data Members
        'mButtonGroup': the id of the button group this belongs to in a dialog
*/
class RadioButton : public CheckBox
{
public:
    
    RadioButton() = delete;
    RadioButton(Dialog& dialog);
    friend std::shared_ptr<RadioButton> CreateRadioButton(Dialog& dialog);

    RadioButtonGroup mButtonGroup;

public:


    /*
    Setters and Getters
    
        Throws:
            no-throw guarantee
    */
	void            SetChecked(bool checked, bool clearGroup = true) noexcept   { SetCheckedInternal(checked, clearGroup, false);   }
    void            SetButtonGroup(RadioButtonGroup buttonGroup) noexcept   { mButtonGroup = buttonGroup;                       }
	unsigned int    GetButtonGroup() const noexcept                             { return mButtonGroup;                              }


    /*
    Overridden Unambiguous Member Functions

    */
	virtual bool MsgProc(MessageType msg, int32_t param1, int32_t param2, int32_t param3, int32_t param4) noexcept override;
    virtual void OnHotkey() noexcept override;
    virtual void OnMouseEnter() noexcept override;
    virtual void OnMouseLeave() noexcept override;
    virtual void OnFocusIn() noexcept override;
    virtual void OnFocusOut() noexcept override;

protected:

    void OnMouseEnterNoRecurse() noexcept;
    void OnMouseLeaveNoRecurse() noexcept;
    void OnFocusInNoRecurse() noexcept;
    void OnFocusOutNoRecurse() noexcept;

    /*
    SetCheckedInternal
    
        Note:
            sets the state of the box, and may clear the group; sends an event if from the user

        Parameters:
            'checked': the new state of this box
            'clearGroup': whether or not to clear the button group (usually true)
            'fromInput': whether or not the user triggered this
    
    */
	virtual void SetCheckedInternal(bool checked, bool clearGroup, bool fromInput);
};


/*
ScrollBar

    Parents:
        Control
        
    Data Members:
        'mShowThumb': show the thumbnail (grabbable scroll piece)
        'mDrag': is the thumbail currently grabbed
        'mUpButtonRegion': the region of the up button
        'mDownButtonRegion': the region of the down button
        'mTrackRegion': the region of the track which the thumbail goes on
        'mThumbRegion': the region of the thumbnail
        'mPosition': the position along the scroll bar
        'mPageSize': how many items are displayable in one page
        'mStart': the first item
        'mEnd': the index after the last item
        'mPreviousMousePos': used to detect drag events
        'mArrow': the state of the arrows
        'mArrowTS': the timestamp of the previous arrow event
*/
class ScrollBar : public Control
{
public:

    /*
    ArrowState

        'CLEAR':            No arrow is down.
        'CLICKED_UP':       Up arrow is clicked.
        'CLICKED_DOWN':     Down arrow is clicked.
        'HELD_UP':          Up arrow is held down for sustained period.
        'HELD_DOWN':        Down arrow is held down for sustained period.
    */
    enum ArrowState
    {
        CLEAR,
        CLICKED_UP,
        CLICKED_DOWN,
        HELD_UP,
        HELD_DOWN
    };
protected:

    ScrollBar() = delete;
    ScrollBar(Dialog& dialog);
    friend std::shared_ptr<ScrollBar> CreateScrollBar(Dialog& dialog);


    bool mShowThumb;
    bool mDrag;
    Rect mUpButtonRegion;
    Rect mDownButtonRegion;
    Rect mTrackRegion;
    Rect mThumbRegion;
    Value mPosition;
    Value mPageSize;
    Value mStart;
    Value mEnd;
    Point mPreviousMousePos;
    ArrowState mArrow;
    double mArrowTS;

public:
	virtual         ~ScrollBar();


    /*
    Setters and Getters

        Throws:
            no-throw guarantee
    
    */
	void SetTrackRange(int nStart, int nEnd) noexcept;
	void SetTrackPos(int nPosition) noexcept	{ mPosition = nPosition; Cap(); UpdateThumbRect();  }
	void SetPageSize(int nPageSize)	noexcept    { mPageSize = nPageSize; Cap(); UpdateThumbRect();  }
	int  GetTrackPos() const noexcept	        { return mPosition;									}
	int  GetPageSize() const noexcept	        { return mPageSize;									}

    /*
    Scroll

        Parameters:
            'delta': the change in position
    */
	void Scroll(int delta);


    /*
    ShowItem

        Note:
            This makes sure 'index' is displayed, and will scroll if necessary
        
        Parameters:
            'index': the item to make sure is in the viewable region

    */
    void ShowItem(int index);


    /*
    Overridden Unambiguous Member Functions

    */
    virtual bool MsgProc(MessageType msg, int32_t param1, int32_t param2, int32_t param3, int32_t param4) noexcept override;
	virtual void Render(float elapsedTime) noexcept override;
	virtual void UpdateRects() noexcept override;

protected:
    
    /*
    UpdateThumbRect

        Note:
            This resizes the thumbnail based on how many items there are, and where the thumbnail is based on 'mPosition'
    
    */
    void UpdateThumbRect();


    /*
    Cap

        Note:
            This acts like a clamp function; it ensures 'mPosition' is within legal range
    
    */
    void Cap();
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

class NoItemSelectedException : Exception
{
public:
    virtual const char* what() const override
    {
        return "No Item Selected In List Box or Combo Box!";
    }

    EXCEPTION_CONSTRUCTOR(NoItemSelectedException);
};

/*
ListBox

    Parents:
        'Control'

    Data Members:
        'mTextRegion': the bounding box for the text
        'mSelectionRegion': the bounding box of the selected elements
        'mScrollBar': the scroll bar for the box
        'mSBWidth': the width of the scroll bar
        'mVerticalMargin': The added space above and below the list box when rendered
        'mHorizontalMargin': The added space to the left and right of the list box when rendered
        'mTextHeight': height of a single line of text
        'mStyle': style of list box (bitfield)
        'mSelected': the indices of the selected list box items
        'mDrag': whether the user is dragging the mouse to select multiple items

*/
class ListBox : public Control
{
protected:

    ListBox() = delete;
    ListBox(Dialog& dialog);
    friend std::shared_ptr<ListBox> CreateListBox(Dialog& dialog);


    Rect mTextRegion;
    Rect mSelectionRegion;
    ScrollBarPtr mScrollBar;
    Size mSBWidth;
    Size mVerticalMargin;
    Size mHorizontalMargin;
    FontSize  mTextHeight; 
    Bitfield mStyle; 
    std::vector<Index> mSelected;
    bool mDrag;
    std::vector<ListBoxItemPtr> mItems;

public:
	virtual ~ListBox();

    enum ListBoxStyle
    {
        MULTISELECTION = 0x01
    };


    /*
    Setters and Getters

        Note:
            everything is measured in pixels

        Throws:
            'GetSelectedData': 'NoItemSelectedException': if no item is selected
            'GetItemData': 'std::invalid_argument': if no item has string text 'text' or 'std::out_of_range' if 'index' is too big
            'GetItem': 'std::out_of_range': if 'index' is too big
            'SetSelectedByIndex': 'std::out_of_range': if 'index' is too big
            'SetSelectedByText': 'std::invalid_argument': if 'text' belongs to no item
            'SetSelectedByData': 'std::invalid_argument': if 'data' belongs to no item

    */
    
	GenericData&    GetItemData(const std::wstring& text, Index start) const;
    GenericData&    GetItemData(Index index) const;
	Size            GetNumItems() const	noexcept		{ return mItems.size();		}
    ListBoxItemPtr  GetItem(const std::wstring& text, Index start = 0) const;
    ListBoxItemPtr  GetItem(Index index) const		{ return mItems[index];     }
	Bitfield        GetStyle() const noexcept			{ return mStyle;			}
	Size            GetScrollBarWidth() const noexcept	{ return mSBWidth;          }

	void            SetStyle(Bitfield style)  noexcept				        { mStyle = style;						                        }
    void            SetScrollBarWidth(Size width) noexcept                  { mSBWidth = width; UpdateRects();                              }
    void            SetMargins(Size vertical, Size horizontal) noexcept     { mVerticalMargin = vertical; mHorizontalMargin = horizontal;   }

    /*
    AddItem

        Parameters:
            'text': the text for the item
            'data': the data for the item to represent

        Throws:
            no-throw guarantee
    */
    void AddItem(const std::wstring& text, GenericData& data = GenericData()) noexcept;

    /*
    InsertItem
           
        Note:
            Just like 'AddItem' except does not have to append
            If 'index' > size - 1, then the item will just be appended

        Parameters:
            'index': the index to insert at
            'text': the text for the item
            'data': the data for the item to represent

        Throws:
            no-throw guarantee
    
    */
    void InsertItem(Index index, const std::wstring& text, GenericData& data) noexcept;

    /*
    RemoveItem

        Parameters:
            'index': the index to remove, which then makes all elements after move back one
        
        Throws:
            'std::out_of_range': if 'index' is too big in _DEBUG

    */
	void RemoveItem(Index index);

    /*
    RemoveAllItems

        Note:
            This removes all of the items

        Throws:
            no-throw guarantee

    */
	void RemoveAllItems() noexcept;

    /*
    GetSelectedIndex

        Note:
            For single-selection listbox, returns the index of the selected item.
            For multi-selection, returns the first selected item after the 'previousSelected' position.

        Parameters:
            'previouslySelected': the previously selected index

        Returns:
            the selected index

        Throws:
            'NoItemSelectedException': if no item is selected

    */
	Index GetSelectedIndex(Index previousSelected) const;//for multi-line
    Index GetSelectedIndex() const;//for single-line (or finding the first selected item)

    /*
    GetSelectedItem
        
        Parameters:
            'previouslySelected': if multi-selection, the previously selected item

        Returns:
            the object representing of the selected item or next selected item in multi-selection

        Throws:
            'NoItemSelectedException': if no item is selected
    
    */
    ListBoxItemPtr GetSelectedItem(Index previousSelected) const{ return GetItem(GetSelectedIndex(previousSelected)); }
    ListBoxItemPtr GetSelectedItem() const { return GetItem(GetSelectedIndex()); }
    
    /*
    GetSelectedData
        
        Parameters:
            'previouslySelected': if multi-selection, the previously selected item

        Returns:
            the data of the selected item or next selected item in multi-selection

        Throws:
            'NoItemSelectedException': if no item is selected
    
    */
    GenericData& GetSelectedData(Index previousSelected) const { return GetItemData(GetSelectedIndex(previousSelected)); }
    GenericData& GetSelectedData() const { return GetItemData(GetSelectedIndex()); }

    /*
    SelectItem

        Note:
            for single-selection listbox, sets the currently selected
            for multi-selection listbox, adds 'newIndex' to the list of selected items, will only add the first item with 'text' found
    
        Parameters:
            'index': the index of the item to add to the selected list
            'text': the text of the item to add to the selected list
            'start': the starting point to look for 'text'

        Throws:
            'std::out_of_range': if index is too big in _DEBUG
            'std::invalid_argument': if 'text' is not found in _DEBUG
    */
	void SelectItem(Index index);
    void SelectItem(const std::wstring& text, Index start = 0);

    /*
    ClearSelected

        Note:
            clears all of the selected Items

        Throws:
            no-throw guarantee

    */
    void ClearSelected() noexcept;

    /*
    RemoveSelected
    
        Note:
            Removes 'index' from the selected list
    
        Parameters:
            'index': the index to remove from the selected list
            'text': the text of the item in the selected list to remove from said list
            'start': the starting index of the selected list to start looking for 'text'

        Throws:
            'std::invalid_argument': if 'index' does not exist, in _DEBUG
    */
    void RemoveSelected(Index index);
    void RemoveSelected(const std::wstring& text, Index start = 0);

    /*
    ContainsItem

        Parameters:
            'text': the text representing the potential item
            'start': the starting index of the search

        Returns:
            whether or not an item exists with the text 'text'

        Throws:
            no-throw guarantee
    
    */
    bool ContainsItem(const std::wstring& text, Index start = 0) const noexcept;

    /*
    FindItem(Index)
        
        Parameters:
            'text': the text representing the index of the item to find
            'start': the starting index of the search

        Returns:
            The index of the found item
            The first item found with text 'text'


        Throws:
            'std::invalid_argument': if no item is found
    
    */
    Index           FindItemIndex(const std::wstring& text, Index start = 0) const;
    ListBoxItemPtr  FindItem(const std::wstring& text, Index start = 0) const;


    /*
    Overrided Unambiguous Member Functions
    

    */
	virtual bool MsgProc(MessageType msg, int32_t param1, int32_t param2, int32_t param3, int32_t param4) noexcept override;
    virtual void OnInit() override                      { mDialog.InitControl(std::dynamic_pointer_cast<Control>(mScrollBar)); UpdateRects(); }
	virtual bool CanHaveFocus() const noexcept override	{ return (mVisible && mEnabled);			    }
	virtual void Render(float elapsedTime) noexcept override;
	virtual void UpdateRects() noexcept override;
	virtual bool ContainsPoint(const Point& pt) const noexcept override{ return Control::ContainsPoint(pt) || mScrollBar->ContainsPoint(pt); }

protected:

    /*
    UpdateItemRects

        Note:
            Updates internal item rects for various reasons, including scrolling causing some items to not be on the
                screen anymore, resizing, item addition/removal, etc.

        Throws:
            no-throw guarantee
    
    */
	virtual void UpdateItemRects() noexcept;
};



/*
ComboBox

    Parents:   
        'Button'

    Data Members:
        'mSelected': the index of the selected item
        'mFocused': the index of the focused item (i.e. when opened, the one the mouse is over)
        'mDropHeight': how far down the combo box list drops
        'mScrollBar': the scroll bar for scrolling through items
        'mSBWidth': the width of the scroll bar
        'mOpened': is the combo box opened?
        'mTextRegion': the rect which the text occupies
        'mButtonRegion': the rect which the button to drop down occupies
        'mDropdownRegion': the region of the drop-down occupies
        'mDropdownTextRegion': the region the drop-down text occupies
        'mItems': the items within the combo box

*/
class ComboBox : public Button
{
protected:

    ComboBox() = delete;
    ComboBox(Dialog& dialog);
    friend std::shared_ptr<ComboBox> CreateComboBox(Dialog& dialog);

    sIndex mSelected;
    sIndex mFocused;
    Size mDropHeight;
    ScrollBarPtr mScrollBar;
    Size mSBWidth;

    bool mOpened;

    Rect mTextRegion;
    Rect mButtonRegion;
    Rect mDropdownRegion;
    Rect mDropdownTextRegion;

    std::vector <ComboBoxItemPtr> mItems;

public:
	virtual         ~ComboBox();
    

    /*
    Setters and Getters

        Note:
            everything is measured in pixels

        Throws:
            'GetItemData': 'std::invalid_argument': if no item has string text 'text' or 'std::out_of_range' if 'index' is too big
            'GetItem': 'std::out_of_range': if 'index' is too big or if 'text' is not found
            'SetSelectedByIndex': 'std::out_of_range': if 'index' is too big
            'SetSelectedByText': 'std::invalid_argument': if 'text' belongs to no item
            'SetSelectedByData': 'std::invalid_argument': if 'data' belongs to no item

    */
    
	GenericData&    GetItemData(const std::wstring& text, Index start = 0) const;
    GenericData&    GetItemData(Index index) const;
	Size            GetScrollBarWidth() const noexcept	{ return mSBWidth;          }
	Size            GetNumItems() const	noexcept		{ return mItems.size();		}
    ComboBoxItemPtr GetItem(const std::wstring& text, Index start = 0) const;
    ComboBoxItemPtr GetItem(Index index) const		{ return mItems[index]; }

    void                SetDropHeight(Size nHeight)			    { mDropHeight = nHeight; UpdateRects(); }
    void                SetScrollBarWidth(Size width) noexcept  { mSBWidth = width; UpdateRects();      }

    /*
    AddItem

        Parameters:
            'text': the text for the item
            'data': the data for the item to represent

        Throws:
            no-throw guarantee
    */
    void AddItem(const std::wstring& text, GenericData& data = GenericData()) noexcept;

    /*
    InsertItem
           
        Note:
            Just like 'AddItem' except does not have to append
            If 'index' > size - 1, then the item will just be appended

        Parameters:
            'index': the index to insert at
            'text': the text for the item
            'data': the data for the item to represent

        Throws:
            no-throw guarantee
    
    */
    void InsertItem(Index index, const std::wstring& text, GenericData& data) noexcept;//IMPLEMENT

    /*
    RemoveItem

        Parameters:
            'index': the index to remove, which then makes all elements after move back one
        
        Throws:
            'std::out_of_range': if 'index' is too big in _DEBUG

    */
	void RemoveItem(Index index);

    /*
    RemoveAllItems

        Note:
            This removes all of the items

        Throws:
            no-throw guarantee

    */
	void RemoveAllItems() noexcept;

    /*
    GetSelectedIndex

        Returns:
            the selected index

        Throws:
            'NoItemSelectedException': if no item is selected

    */
    Index GetSelectedIndex() const;

    /*
    GetSelectedItem

        Returns:
            the object representing of the selected item

        Throws:
            'NoItemSelectedException': if no item is selected
    
    */
    ListBoxItemPtr GetSelectedItem() const;
    
    /*
    GetSelectedData

        Returns:
            the data of the selected item

        Throws:
            'NoItemSelectedException': if no item is selected
    
    */
    GenericData& GetSelectedData() const;

    /*
    SelectItem
    
        Parameters:
            'index': the index of the item to select
            'text': the text of the item to select
            'start': the starting point to look for 'text'
            'data': the data to find

        Throws:
            'std::out_of_range': if index is too big in _DEBUG
            'std::invalid_argument': if 'text' is not found in _DEBUG, or if 'data' is not found in _DEBUG
    */
	void SelectItem(Index index);
    void SelectItem(const std::wstring& text, Index start = 0);
    void SelectItem(const GenericData& data);
    
    /*
    ContainsItem

        Parameters:
            'text': the text representing the potential item
            'start': the starting index of the search

        Returns:
            whether or not an item exists with the text 'text'

        Throws:
            no-throw guarantee
    
    */
    bool ContainsItem(const std::wstring& text, Index start = 0) const noexcept;

    /*
    FindItem(Index)
        
        Parameters:
            'text': the text representing the index of the item to find
            'start': the starting index of the search

        Returns:
            The index of the found item
            The first item found with text 'text'


        Throws:
            'std::invalid_argument': if no item is found
    
    */
    Index           FindItemIndex(const std::wstring& text, Index start = 0) const;
    ComboBoxItemPtr FindItem(const std::wstring& text, Index start = 0) const;


    /*
    Overridden Unambiguous Member Functions
    
    
    */
	virtual bool MsgProc(MessageType msg, int32_t param1, int32_t param2, int32_t param3, int32_t param4) noexcept override;
	virtual void OnHotkey() noexcept override;
	virtual bool CanHaveFocus() const noexcept override{ return (mVisible && mEnabled); }
	virtual void OnFocusOut() noexcept override;
	virtual void Render(float elapsedTime) noexcept override;
	virtual void UpdateRects() noexcept override;
    virtual void OnInit() override;
	virtual void SetTextColor(const Color& Color) noexcept override;
    virtual bool ContainsPoint(const Point& pt) const noexcept override;

protected:
    
    /*
    UpdateItemRects

        Note:
            Updates internal item rects for various reasons, including scrolling causing some items to not be on the
                screen anymore, resizing, item addition/removal, etc.

        Throws:
            no-throw guarantee
    
    */
	void UpdateItemRects();
};


/*
Slider

    Parents:
        'Control'

    Data Members:
        'mValue': the value between 'mMin' and 'mMax' where the slider is at
        'mMin': the minimum value the slider can have
        'mMax': the maximum value the slider can have
        'mDragX': the mouse position at the start of the drag
        'mDragOffset': the difference between the 'mDragX' and the current mouse position; updated every frame
        'mButtonX': the X value of the button position in pixels
        'mPressed': is the button pressed to be dragged
        'mButtonRegion': the region the button occupies

*/
class Slider : public Control
{
protected:

    Slider() = delete;
    Slider(Dialog& dialog);
    friend std::shared_ptr<Slider> CreateSlider(Dialog& dialog);

    Value mValue;
    Value mMin;
    Value mMax;
    Value mDragX;
    Value mDragOffset;
    Value mButtonX;
    bool mPressed;
    Rect mButtonRegion;

public:

    /*
    Setters and Getters

        Throws:
            no-throw guarantee
    
    */
    Value   GetValue() const noexcept           { return mValue;                    }
    void        GetRange(Value& nMin, Value& nMax) const noexcept
                                                    { nMin = mMin; nMax = mMax;         }
    void        SetValue(Value nValue) noexcept { SetValueInternal(nValue, false);  }
    void        SetRange(Value  nMin, Value  nMax) noexcept;

    /*
    Overridden Unambiguous Member Functions
    
    
    */
    virtual bool MsgProc(MessageType msg, int32_t param1, int32_t param2, int32_t param3, int32_t param4) noexcept override;
    virtual bool ContainsPoint(const Point& pt) const noexcept override;
    virtual bool CanHaveFocus()const noexcept override{ return (mVisible && mEnabled); }
    virtual void UpdateRects() noexcept override;
    virtual void Render(float elapsedTime) noexcept override;

protected:
    /*
    SetValueInternal

        Note:
            sets the position of the slider, and throws an event if it is from the user

        Parameters:
            'value': the value to set the slider to
            'fromInput': whether the this value came from the user or not
    
        Throws:
            no-throw guarantee
    */
	void      SetValueInternal(int nValue, bool bFromInput) noexcept;

    /*
    ValueFromXPos
    
        Note:  
            Gets the value to set the slider given a pixel X coordinate

        Parameters:
            'xPos': the screen position in pixels 

        Returns:
            The value to set the slider from the given X coordinate

        Throws:
            no-throw guarantee

    */
	Value ValueFromXPos(Value xPos) const noexcept;

};



/*
ModificationStack

    Note:
        -This class is designed to keep track of modifications made to a set of data

    Data Members:
        'mUndoStack': the stack which can be undone
        'mRedoStack': the stack which can be redone
        'mText': the current text as a result to the stack changes
*/

namespace ModificationStackInternal
{
    class Modification
    {
    public:
        virtual void ApplyModificationToString(std::wstring& str) const = 0;
        virtual void RemoveModificationToString(std::wstring& str) const = 0;
    };
    class ModificationAddition : virtual public Modification
    {
    public:
        uint32_t mInsertLocation;
        std::wstring mNewText;

        virtual ModificationAddition& operator=(const ModificationAddition& str);

        virtual void ApplyModificationToString(std::wstring& str) const;
        virtual void RemoveModificationToString(std::wstring& str) const;
    };
    class ModificationRemoval : virtual public Modification
    {
    public:
        uint32_t mStartIndex;
        uint32_t mEndIndex;
        mutable std::wstring mRemovedText;

        virtual ModificationRemoval& operator=(const ModificationRemoval& str);

        virtual void ApplyModificationToString(std::wstring& str) const;
        virtual void RemoveModificationToString(std::wstring& str) const;
    };
    class ModificationRemovalAndAddition : public ModificationAddition, public ModificationRemoval
    {
    public:

        virtual ModificationRemovalAndAddition& operator=(const ModificationRemovalAndAddition& str);
        ModificationRemovalAndAddition(const ModificationRemovalAndAddition& other);
        ModificationRemovalAndAddition();

        virtual void ApplyModificationToString(std::wstring& str) const;
        virtual void RemoveModificationToString(std::wstring& str) const;
    };
    class GenericCompositeModification : public Modification
    {
    protected:
        std::vector<std::shared_ptr<Modification>> mParts;
    public:

        void ClearParts();

        template<typename modType>
        void PushPart(const modType& mod);

        bool Empty() const noexcept{ return mParts.empty(); }

        virtual void ApplyModificationToString(std::wstring& str) const;
        virtual void RemoveModificationToString(std::wstring& str) const;
    };
}
class ModificationStack
{
    ModificationStackInternal::GenericCompositeModification mPartialMod;
    std::stack <ModificationStackInternal::Modification*> mRedoStack;
    std::stack <ModificationStackInternal::Modification*> mUndoStack;
    std::wstring mText;

    void FlattenRedoStack() noexcept;
    void FlattenUndoStack() noexcept;
public:
    ModificationStack(const std::wstring& initialText);

    void PushAddition(const std::wstring& addition, uint32_t loc);
    void PushRemoval(uint32_t startIndex, uint32_t endIndex);
    //void PushRemovalAndAddition(uint32_t startIndex, uint32_t endIndex, const std::wstring& addition);

    template<typename modType>
    void PushPartialModification(const modType& m);
    void ApplyPartialModifications() noexcept;

    void PushPartialAddition(const std::wstring& text, uint32_t loc);
    void PushPartialRemoval(uint32_t startIndex, uint32_t endIndex);

    template<typename modType>
    void PushGeneric(const modType& m);

    void UndoNextItem() noexcept;
    void RedoNextItem() noexcept;

    void FlattenStack() noexcept;

    void SetText(const std::wstring& text) noexcept;

    std::wstring Top() const noexcept{ return mText; }
};

class StringContainsInvalidCharacters : public Exception
{
public:
    virtual const char* what() const override
    {
        return "String Contained Characters Not In Charset";
    }

    EXCEPTION_CONSTRUCTOR(StringContainsInvalidCharacters)
};


/*
EditBox

    Parents:
        'Control'

    Data Members:
        'mTextHistoryKeeper': the text in the edit box, including tools to allow undoing and redoing modifications
        'mTextRegion': the region where the text will be rendered
        'mCharset': the charset that is considered valid for this box
        'mBlinkPeriod': the time between blink state toggles if the caret
        'mPreviousBlinkTime': the previous time the blink state changed
        'mMouseDrag': if the mouse is currently pressed down and being moved
        'mMultiline': is this dialog multiple lines
        'mCaretOn': the state of the blinking caret
        'mHideCaret': is the caret enabled
        'mCaretPos': the caret's position in the original string buffer
        'mInsertMode': is the box in insert mode, or overwrite mode
        'mSelStart': the start of the selection, the caret signifies the end of the selection
        'mHorizontalMargin': the horizontal margin
        'mVerticalMargin': the vertical margin
        'mSelTextColor': the color of selected text
        'mSelBkColor': the color the the background of the selected region
        'mCaretColor': the color of the caret
        'mScrollBar': the scroll bar, visible in 'multiline', invisible otherwise
        'mSBWidth': the width of the scroll bar
        'mUpdateRequired': whether or not an update of the rendering rects needs to be made this frame
        'mCharacterBBs': the bounding boxes of each character
        'mRenderOffset': the starting character to render text
        'mRenderCount': the number of characters to render
        'mTextDataBuffer': the text box's openGL data buffer
*/
class EditBox : public Control
{
protected:

    EditBox() = delete;
    EditBox(Dialog& dialog, bool isMultiline);
    friend std::shared_ptr<EditBox> CreateEditBox(Dialog& dialog, bool isMultiline);

    /*
    
    Publically Available Attributes
    
    */
    ModificationStack mTextHistoryKeeper;

    Charset mCharset = Unicode;

    double mBlinkPeriod = 0.5;
    bool mHideCaret = false;
    Size mCaretSize = 2;
    Value mCaretPos = -1;//the space behind the first character
    bool mInsertMode = true;
    Value mSelStart = -2;//symbolizes nothing to be selected
    Size mHorizontalMargin;
    Size mVerticalMargin;

    //Color mTextColor; This is handeled by the elements
    BlendColor mSelTextColor;
    BlendColor mSelBkColor;
    BlendColor mCaretColor;


    /*
    
    Protected Attributes
    
    */
    const bool mMultiline = true;
    ScrollBarPtr mScrollBar;
    Size mSBWidth = 16;
    Rect mTextRegion;

    Rect mSubRegions[9];

    double mPreviousBlinkTime = 0;
    bool mMouseDrag = false;
    bool mCaretOn = true;
    bool mUpdateRequired;
    bool mIsEmpty = false;

    /*
    
    Render Data
    
    */
    std::vector<Rect> mCharacterBBs;
    std::vector<Rect> mCharacterRects;
    GLuint mRenderOffset = 0;
    GLuint mRenderCount = 0;
    VertexArrayPtr mTextDataBuffer;

    /*
    InvalidateRects
        
        Note:
            Sets 'mUpdateRequired' to 'true'

    */
    void InvalidateRects() noexcept;


    /*

    Text Control Methods

    */
    Value PointToCharPos(const Point& pt) noexcept;
    Rect CharPosToRect(Value charPos) noexcept;
    Value RenderTextToText(Value rndIndex);
    Value TextToRenderText(Value txtIndex);
    bool ShouldRenderCaret() noexcept;
    void ApplyCompositeModifications() noexcept;


    /*
    
    Text Formatting Methods
    
    */
    virtual void UpdateCharRects() noexcept;
    virtual void BufferCharRects() noexcept;
    virtual void RenderText(float elpasedTime) noexcept;
    void RemoveSelectedRegion() noexcept;


    /*
    
    Private Fields
    
    
    */
    using UpdateRectsPtr = void(EditBox::*)() noexcept;
    using UpdateCharRectsPtr = void(EditBox::*)() noexcept;
    using RenderPtr = void(EditBox::*)(float) noexcept;
    UpdateRectsPtr mUpdateRectsFunction;
    UpdateCharRectsPtr mUpdateCharRectsFunction;
    RenderPtr mRenderFunction;

    virtual void RenderMultiline(float elapsedTime) noexcept;
    virtual void RenderSingleline(float elapsedTime) noexcept;

    virtual void UpdateRectsMultiline() noexcept;
    virtual void UpdateRectsSingleline() noexcept;

    virtual void UpdateCharRectsMultiline() noexcept;
    virtual void UpdateCharRectsSingleline() noexcept;

public:
	virtual         ~EditBox();

    /*
    
    Setters and Getters

    
    */
#pragma region Setters and Getters
    std::wstring GetText() const noexcept{ return mTextHistoryKeeper.Top(); }
    Charset GetCharset() const noexcept{ return mCharset; }
    Rect GetTextRegion() const noexcept{ return mTextRegion; }
    double GetBlinkPeriod() const noexcept{ return mBlinkPeriod; }
    bool IsCaretHidden() const noexcept{ return mHideCaret; }
    Value GetCaretPos() const noexcept{ return mCaretPos; }
    bool GetInsertMode() const noexcept{ return mInsertMode; }
    Value GetSelectionStart() const noexcept{ return mSelStart; }
    Size GetHorizontalMargin() const noexcept{ return mHorizontalMargin; }
    Size GetVerticalMargin() const noexcept{ return mVerticalMargin; }
    BlendColor& GetSelectedTextBlendColor() noexcept{ return mSelTextColor; }
    BlendColor& GetSelectedBackgroundBlendColor() noexcept{ return mSelBkColor; }
    BlendColor& GetCaretBlendColor() noexcept{ return mCaretColor; }
    BlendColor& GetTextBlendColor() noexcept{ return mElements[0].mFontColor; }

    std::wstring GetSelectedText() noexcept;

    BlendColor GetSelectedTextCBlendColor() const noexcept{ return mSelTextColor; }
    BlendColor GetSelectedBackgroundCBlendColor() const noexcept{ return mSelBkColor; }
    BlendColor GetCaretCBlendColor() const noexcept{ return mCaretColor; }
    BlendColor GetTextCBlendColor() const noexcept{ return mElements.at(0).mFontColor; }
        


    void SetText(const std::wstring& text); //may throw 'StringContainsInvalidCharacters'
    void SetCharset(Charset chSet) noexcept;//this will automatically remove all characters not in this charset from the string
    void SetBlinkPeriod(double period) noexcept;
    void SetCaretState(bool state) noexcept;
    void SetCaretPosition(Value pos) noexcept;
    void SetInsertMode(bool insertMode) noexcept;
    void SetSelectionStart(Value pos) noexcept;
	void SetSelectionEmpty() noexcept;
    void SetVerticalMargin(Size marg) noexcept;
    void SetHorizontalMargin(Size marg) noexcept;
    void SetSelectedTextBlendColor(const BlendColor& col) noexcept;
    void SetSelectedBackgroundBlendColor(const BlendColor& col) noexcept;
    void SetCaretBlendColor(const BlendColor& col) noexcept;
    void SetTextBlendColor(const BlendColor& col) noexcept;

#pragma endregion


    /*
    
    Text Modification Methods
    
    */
    void InsertString(const std::wstring& str, Value pos) noexcept;
    void InsertChar(wchar_t ch, Value pos) noexcept;
    void DeleteChar(Value pos) noexcept;


    /*
    Overriden Unambiguous Member Functions

    */
    
	virtual bool MsgProc(MessageType msg, int32_t param1, int32_t param2, int32_t param3, int32_t param4) noexcept override;
	virtual void UpdateRects() noexcept override;
	virtual bool CanHaveFocus() const noexcept override { return (mVisible && mEnabled); }
	virtual void Render(float elapsedTime) noexcept override;
    virtual void OnFocusIn() noexcept override;
    virtual void OnFocusOut() noexcept override;
    virtual void OnMouseEnter() noexcept override;
    virtual void OnMouseLeave() noexcept override;
    virtual void OnInit() override;

};



/*
======================================================================================================================================================================================================
Text Controls Below

*/


/*
TextHelper

    Note:
        This class's purpose is to provide a way to draw text outside of a dialog

    Data Members:
        'mColor': the text color
        'mPoint': where to start drawing the text
        'mManager': a reference to the dialog resource manager
        'mFontIndex': the index of the font within the DRM
        'mFontSize': the font size in points
        'mLeading': the leading of the text
*/
class OBJGLUF_API TextHelper
{
protected:

    TextHelper() = delete;
    TextHelper(DialogResourceManagerPtr& drm);
    friend std::shared_ptr<TextHelper> CreateTextHelper(DialogResourceManagerPtr& drm);


    DialogResourceManagerPtr& mManager;

    /*
    Helper overloads for RenderString
    
    */

    template<typename T1, typename... Types>
    static void RenderText(std::wstringstream& formatStream, std::wstringstream& outString, const T1& arg1, const Types&... args);

    template<typename T1>
    static void RenderText(std::wstringstream& formatStream, std::wstringstream& outString, const T1& arg);

    FontIndex mFontIndex;
    FontSize mFontSize;
    FontSize mLeading;

public:
    Color mColor;
    Point mPoint;


	~TextHelper(){};

    /*
    Begin

        Note:
            Call this before calling any text drawing calls

        Parameters:
            'drmFont': the index of the font to use
            'size': the size of font to draw

        Throws:
            'std::out_of_range': if 'drmFont': is out of the range within the DRM
    */
	void Begin(FontIndex drmFont, FontSize leading, FontSize size);

    /*
    DrawFormattedTextLine

        Note:
            this replaces each % with a the next argument, more features will come in the future

        Parameters:
            'format': the format and text
            'args': the data to put in 'format'
            'rc': the rect to render the text within
            'flags': the flags for drawing

        Throws:
            no-throw guarantee
    */
    template<class... Types>
    void DrawFormattedTextLine(const std::wstring& format, const Types&... args) noexcept;

    template<class... Types>
    void DrawFormattedTextLineBase(const Rect& rc, Bitfield flags, const std::wstring& format, const Types&... args) noexcept;

    /*
    DrawTextLine

        Note:
            The first will automatically bring down the next line to draw, the second will not

        Parameters:
            'text': the text to draw
            'rc': the rect to render the text within
            'flags': the flags for drawing within the rect (i.e. GT_CENTER)

        Throws:
            no-throw guarantee
    */
	void DrawTextLine(const std::wstring& text) noexcept;
    void DrawTextLineBase(const Rect& rc, Bitfield flags, const std::wstring& text) noexcept;

    /*
    End

        Note:
            This actually renders the text; the previous functions just buffer it

        Throws:
            no-throw guarantee
    
    */
	void End() noexcept;

    /*
    RenderText

        Note:
            This is a very simple version of vsprintf with no formatting; each % will be replaced with the next value

        Parameters:
            'format': the format to take
            'outString': where the output will be stored
            'args': the parameters to insert into 'format'

        Throws:
            no-throw guarantee
    
    
    */
    template<class... Types>
    static void RenderText(const std::wstring& format, std::wstring& outString, const Types&... args) noexcept;
};


}

/*

The Implementation for all template functions

*/
#include "GLUFGuiTemplates.inl"

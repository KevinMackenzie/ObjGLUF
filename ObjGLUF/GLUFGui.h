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

class OBJGLUF_API GLUFDialogResourceManager;
class OBJGLUF_API GLUFControl;
class OBJGLUF_API GLUFButton;
class OBJGLUF_API GLUFStatic;
class OBJGLUF_API GLUFCheckBox;
class OBJGLUF_API GLUFRadioButton;
class OBJGLUF_API GLUFComboBox;
class OBJGLUF_API GLUFSlider;
class OBJGLUF_API GLUFEditBox;
class OBJGLUF_API GLUFListBox;
class OBJGLUF_API GLUFScrollBar;
class OBJGLUF_API GLUFElement;
class OBJGLUF_API GLUFFont;
class OBJGLUF_API GLUFDialog;
class OBJGLUF_API GLUFTextHelper;
struct GLUFElementHolder;
struct GLUFTextureNode;
struct GLUFFontNode;

/*
======================================================================================================================================================================================================
Enumerations and Type Aliases

*/

enum GLUFMessageType
{
	GM_MB = 0,
	GM_CURSOR_POS,
	GM_CURSOR_ENTER,
	GM_SCROLL, //since i am using 32 bit integers as input, this value will be multiplied by 1000 to preserver any relevant decimal points
	GM_KEY,//don't' support joysticks yet
	GM_UNICODE_CHAR,
	GM_RESIZE,
	GM_POS,
	GM_CLOSE,
	GM_REFRESH,
	GM_FOCUS,
	GM_ICONIFY,
	GM_FRAMEBUFFER_SIZE
};

//WIP
enum GLUFFontWeight
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


enum GLUFControlType
{
    GLUF_CONTROL_BUTTON,
    GLUF_CONTROL_STATIC,
    GLUF_CONTROL_CHECKBOX,
    GLUF_CONTROL_RADIOBUTTON,
    GLUF_CONTROL_COMBOBOX,
    GLUF_CONTROL_SLIDER,
    GLUF_CONTROL_EDITBOX,
    GLUF_CONTROL_IMEEDITBOX,
    GLUF_CONTROL_LISTBOX,
    GLUF_CONTROL_SCROLLBAR,
    GLUF_CONTROL_CUSTOM
};

enum GLUFControlState
{
    GLUF_STATE_NORMAL = 0,
    GLUF_STATE_DISABLED,
    GLUF_STATE_HIDDEN,
    GLUF_STATE_FOCUS,
    GLUF_STATE_MOUSEOVER,
    GLUF_STATE_PRESSED,
};

//WIP
enum GLUFEvent
{
    GLUF_EVENT_BUTTON_CLICKED = 0,
    GLUF_EVENT_COMBOBOX_SELECTION_CHANGED,
    GLUF_EVENT_RADIOBUTTON_CHANGED,
    GLUF_EVENT_CHECKBOXCHANGED,
    GLUF_EVENT_SLIDER_VALUE_CHANGED,
    GLUF_EVENT_SLIDER_VALUE_CHANGED_UP,
    GLUF_EVENT_EDITBOX_STRING,
    GLUF_EVENT_EDITBOX_CHANGE,//when the listbox contents change due to user input
    GLUF_EVENT_LISTBOX_ITEM_DBLCLK,
    GLUF_EVENT_LISTBOX_SELECTION,//when the selection changes in a single selection list box
    GLUF_EVENT_LISTBOX_SELECTION_END,
};


enum GLUFCharset
{
    ASCII = 0,
    ASCIIExtended = 1,
    Numeric = 2,
    Alphabetical = 3,
    AlphaNumeric = 4,
    Unicode = 5
};

/*

Alphabetized list of GLUF GUI Pointer Aliases

*/
using GLUFButtonPtr                 = std::shared_ptr < GLUFButton > ;
using GLUFCheckBoxPtr               = std::shared_ptr < GLUFCheckBox > ;
using GLUFComboBoxPtr               = std::shared_ptr < GLUFComboBox > ;
using GLUFControlPtr                = std::shared_ptr < GLUFControl > ;
using GLUFDialogPtr                 = std::shared_ptr < GLUFDialog > ;
using GLUFDialogResourceManagerPtr  = std::shared_ptr < GLUFDialogResourceManager > ;
using GLUFEditBoxPtr                = std::shared_ptr < GLUFEditBox > ;
using GLUFElementPtr                = std::shared_ptr < GLUFElement > ;
using GLUFListBoxPtr                = std::shared_ptr < GLUFListBox > ;
using GLUFRadioButtonPtr            = std::shared_ptr < GLUFRadioButton > ;
using GLUFScrollBarPtr              = std::shared_ptr < GLUFScrollBar > ;
using GLUFSliderPtr                 = std::shared_ptr < GLUFSlider > ;
using GLUFStaticPtr                 = std::shared_ptr < GLUFStatic > ;
using GLUFTextHelperPtr             = std::shared_ptr < GLUFTextHelper > ;
using GLUFElementHolderPtr          = std::shared_ptr < GLUFElementHolder > ;
using GLUFFontNodePtr               = std::shared_ptr < GLUFFontNode > ;
using GLUFTextureNodePtr            = std::shared_ptr < GLUFTextureNode > ;

using GLUFTextureIndex      = uint32_t;
using GLUFTextureIndexResMan= uint32_t;
using GLUFFontIndex         = uint32_t;
using GLUFFontIndexResMan   = uint32_t;
using GLUFElementIndex      = uint32_t;
using GLUFControlIndex      = uint32_t;
using GLUFRadioButtonGroup  = uint32_t;
using GLUFBitfield          = uint32_t;
using GLUFBitfieldL         = uint64_t;
using GLUFSize              = uint32_t;
using GLUFValue             = int32_t;
using GLUFUValue            = uint32_t;
using GLUFIndex             = uint32_t;   
using GLUFsIndex            = int32_t;
using GLUFKeyId             = uint32_t;

/*
======================================================================================================================================================================================================
Setup and Fundemental Utility Functions

*/

OBJGLUF_API GLuint GetWindowHeight();
OBJGLUF_API GLuint GetWindowWidth();

/*

GLUFCallbackFuncPtr

    Note:
        This may do whatever it pleases, however
        callbackFunc must explicitly call the callback methods of the 
        dialog manager and the dialog classes (and whatever else)
        to achieve purposeful use of this library.  
        For each 'GLUFMessageType', see the glfw documentation 
        for help.  If 'msg' requires non-integer data, 
        the floating point data will be multiplied 
        by 1000 and the rest will be truncated.  For
        more documentation see GLFW 'Input' documentation.

    Returns:
        'bool': true: the message is consumed; false: the message is not consumed

*/
using GLUFCallbackFuncPtr = bool(*)(GLUFMessageType, int, int, int, int);



/*
GLUFEventCallbackReceivable

A base class for all class which want to be able to have an event callback

*/
class GLUFEventCallbackReceivable
{
protected:
    virtual void GLUFEventCallback() = 0;

public:
    static void GLUFEventCallbackStatic(GLUFEvent event, int ctrlId, GLUFControl);
};
using GLUFEventCallbackReceivablePtr = std::shared_ptr < GLUFEventCallbackReceivable > ;

/*
GLUFEventCallbackFuncPtr

    Parameters:
        'GLUFEvent': what event occured
        'GLUFControlPtr': the control which received the event
        'GLUFEventCallbackReceivablePtr': the if applicable, the instance of the class that will be called

    Note:
        the final parameter must be derived from 'GLUFEventCallbackReceivable'
        
*/
using GLUFEventCallbackFuncPtr = void(__cdecl*)(GLUFEvent, GLUFControlPtr&, const GLUFEventCallbackReceivablePtr&);

/*

Shorthand Notation for Style

*/
#define GLUF_GUI_CALLBACK_PARAM GLUFMessageType msg, int32_t param1, int32_t param2, int32_t param3, int32_t param4
#define GLUF_PASS_CALLBACK_PARAM msg, param1, param2, param3, param4

/*
GLUFInitGui --Main Setup Function

    Note:

        This must be called AFTER 'GLUFInitOpenGLExtentions()'

    Parameters:
        'initializedGLUFWindow': an initialized GLFW window
        'callbackFunc': the Callback function for GLFW messages
        'controltex': the OpenGL id for the default control texture 

*/
OBJGLUF_API bool GLUFInitGui(GLFWwindow* initializedGLUFWindow, GLUFCallbackFuncPtr callbackFunc, GLuint controltex);

//swaps callback functions, returns old
OBJGLUF_API GLUFCallbackFuncPtr GLUFChangeCallbackFunc(GLUFCallbackFuncPtr newCallback);




/*
======================================================================================================================================================================================================
Everything Fonts

*/

using GLUFFontPtr = std::shared_ptr<GLUFFont>;
using GLUFFontSize = uint32_t;//in 'points'

//TODO: support dpi scaling
#define GLUF_POINTS_PER_PIXEL 1.333333f
#define GLUF_POINTS_TO_PIXELS(points) (GLUFFontSize)((float)points * GLUF_POINTS_PER_PIXEL)
#define GLUF_PICAS_TO_POINTERS(picas) ((picas) * 6.0)


OBJGLUF_API void GLUFSetDefaultFont(GLUFFontPtr& pDefFont);


class LoadFontException : public GLUFException
{
public:
    virtual const char* what() const
    {
        return "Error Loading Freetype Font";
    }

    EXCEPTION_CONSTRUCTOR(LoadFontException);
};

/*
GLUFLoadFont

    Parameters:
        'font': an uninitialized font
        'rawData': the raw data to load font from
        'fontHeight': how tall should the font be in points?

    Throws:
        'LoadFontException': if font loading failed

*/
OBJGLUF_API void GLUFLoadFont(GLUFFontPtr& font, const std::vector<char>& rawData, GLUFFontSize fontHeight);

OBJGLUF_API GLUFFontSize GLUFGetFontHeight(const GLUFFontPtr& font);




/*
======================================================================================================================================================================================================
Fundemental Utility Classes For Dialog Element Usage

*/


/*
GLUFBlendColor

    Note:
        Used to modulate colors for different control states to provide a responsive GUI experience

    Data Members:
        'mStates': all of the different control states which can exist
        'mCurrentColor': the current color of the blend state

*/
using ColorStateMap = std::map < GLUFControlState, GLUF::Color > ;

struct GLUFBlendColor
{
public:
    ColorStateMap       mStates;
    GLUF::Color         mCurrentColor;


    /*
    Init

        Parameters:
            'defaultColor': the default color state
            'disabledColor': the disabled color state
            'hiddenColor': the hidden color state
    
    */
    void        Init(const GLUF::Color& defaultColor, const GLUF::Color& disabledColor = { 200, 128, 128, 100 }, const GLUF::Color& hiddenColor = { 0, 0, 0, 0 });

    /*
    Blend
        
        Parameters:
            'state': the state to blend from the current to
            'elapsedTime': the time elapsed since the begin of the blend period
            'rate': how quickly to blend between states

        Note:
            this is designed to be called every update cycle to provide a smooth blend animation
    
    */
	void        Blend(GLUFControlState state, float elapsedTime, float rate = 0.7f);

    /*
    SetCurrent

        Parameters:
            'current': the color to set the current state
            'state': the state to set the current state to
    
    */
	void		SetCurrent(const GLUF::Color& current);
	void		SetCurrent(GLUFControlState state);

    /*
    SetAll

        Parameters:
            'color': the color to set all of the states to; used for static elements
    
    */
	void		SetAll(const GLUF::Color& color);
};

/*
GLUFElement

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
class GLUFElement
{
public:

	GLUFTextureIndex mTextureIndex;
    GLUFFontIndex mFontIndex;
    GLUFBitfield mTextFormatFlags;

    GLUF::GLUFRectf mUVRect;

    GLUFBlendColor mTextureColor;
    GLUFBlendColor mFontColor;

    /*
    SetTexture

        Parameters:
            'textureIndex': the index of the texture within the dialog resource manager to use
            'uvRect': the UV coordinates of the element within the texture
            'defaultTextureBlendColor': the default texture blend color
    
    */
    void    SetTexture(GLUFTextureIndex textureIndex, const GLUFRectf& uvRect, const GLUF::Color& defaultTextureColor = { 255, 255, 255, 0 });

    /*
    SetFont

        Parameters:
            'font': the font for this element to use
            'defaultFontColor': the default color for the font
            'textFormat': a bitfield of the horizontal and vertical text formatting
    
    */
    void    SetFont(GLUFFontIndex font, const GLUF::Color& defaultFontColor = { 0, 0, 0, 255 }, GLUFBitfield textFormat = GT_CENTER | GT_VCENTER);

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


class GLUFControlCreationException : public GLUFException
{
public:
    virtual const char* what() const override
    {
        return "GLUFControl Falied to be Created!";
    }

    EXCEPTION_CONSTRUCTOR(GLUFControlCreationException);
};

/*
GLUFDialog

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
class GLUFDialog : public std::enable_shared_from_this<GLUFDialog>
{
	friend class GLUFDialogResourceManager;

    bool mFirstTime = true;

    bool mLocked = true;
    bool mAutoClamp = false;
    bool mGrabAnywhere = false;
    bool mDragged = false;

    GLUFControlPtr mDefaultControl = nullptr;

    static double sTimeRefresh;
    double mTimePrevRefresh = 0.0;

    //TODO: these might not want to be static
    static GLUFControlPtr sControlFocus;
    static GLUFControlPtr sControlPressed;
    GLUFControlPtr mControlMouseOver = nullptr;

    bool mVisible = true;
    bool mCaptionEnabled = false;
    bool mMinimized = false;
    bool mDrag = false;

    std::wstring mCaptionText;

    //these are assumed to be based on the origin (bottom left)
    GLUFRect mRegion;

    long mCaptionHeight = 0;

    GLUFDialogResourceManagerPtr mDialogManager = nullptr;

    GLUFEventCallbackFuncPtr mCallbackEvent = nullptr;
    GLUFEventCallbackReceivablePtr mCallbackContext = nullptr;

    std::map <GLUFTextureIndex, GLUFTextureIndexResMan> mTextures;
    std::map <GLUFFontIndex, GLUFFontIndexResMan> mFonts;

    std::map<GLUFControlIndex, GLUFControlPtr> mControls;
    std::vector<GLUFElementHolderPtr> mDefaultElements;

    GLUFElementPtr mCapElement;
    GLUFElementPtr mDlgElement;

    GLUFDialogPtr mNextDialog = nullptr;
    GLUFDialogPtr mPrevDialog = nullptr;

    bool mNonUserEvents = true;
    bool mKeyboardInput = true;
    bool mMouseInput = true;

    GLUF::GLUFPoint mMousePosition;
    GLUF::GLUFPoint mMousePositionDialogSpace;
    GLUF::GLUFPoint mMousePositionOld;

    /*
    Constructor/Destructor

        Throws:
            no-throw guaratee

    */
    GLUFDialog();
    friend std::shared_ptr<GLUFDialog> CreateDialog();

public:
	~GLUFDialog();

	
    /*
    Init

        Parameters:
            'manager': a pointer to the dialog resource manager
            'registerDialog': whether or not to register this dialog with the resource manager, usually true
            'textureIndex': the texture to use for this dialogs default controls

        Throws:
            'std::invalid_argument': if 'manager == nullptr'

    */
	void Init(GLUFDialogResourceManagerPtr& manager, bool registerDialog = true);
	void Init(GLUFDialogResourceManagerPtr& manager, bool registerDialog, GLUFTextureIndex textureIndex);

    /*
    MsgProc

        Parameters:
            see GLUFCallbackFuncPtr for details

        Note:
            Handles message handling for this dialog and all sub controls

        Throws:
            no-throw guarantee
    
    */
	bool MsgProc(GLUFMessageType msg, int32_t param1, int32_t param2, int32_t param3, int32_t param4) noexcept;

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
            'GLUFControlCreationException': if control initialization failed

    
    */
    void AddStatic(GLUFControlIndex ID, const std::wstring& strText, const GLUFRect& region, GLUFBitfield textFlags = GT_LEFT | GT_TOP, bool isDefault = false, std::shared_ptr<GLUFStaticPtr> ctrlPtr = nullptr);
    void AddButton(GLUFControlIndex ID, const std::wstring& strText, const GLUFRect& region, int hotkey = 0, bool isDefault = false, std::shared_ptr<GLUFButtonPtr> ctrlPtr = nullptr);
    void AddCheckBox(GLUFControlIndex ID, const std::wstring& strText, const GLUFRect& region, bool checked = false, int hotkey = 0, bool isDefault = false, std::shared_ptr<GLUFCheckBoxPtr> ctrlPtr = nullptr);
    void AddRadioButton(GLUFControlIndex ID, GLUFRadioButtonGroup buttonGroup, const std::wstring& strText, const GLUFRect& region, bool checked = false, int hotkey = 0, bool isDefault = false, std::shared_ptr<GLUFRadioButtonPtr> ctrlPtr = nullptr);
    void AddComboBox(GLUFControlIndex ID, const GLUFRect& region, int hotKey = 0, bool isDefault = false, std::shared_ptr<GLUFComboBoxPtr> ctrlPtr = nullptr);
    void AddSlider(GLUFControlIndex ID, const GLUFRect& region, long min, long max, long value, bool isDefault = false, std::shared_ptr<GLUFSliderPtr> ctrlPtr = nullptr);
    //void AddEditBox(GLUFControlIndex ID, const std::wstring& strText, const GLUFRect& region, GLUFCharset charset = Unicode, GLbitfield textFlags = GT_LEFT | GT_TOP, bool isDefault = false, std::shared_ptr<GLUFEditBoxPtr> ctrlPtr = nullptr);
    void AddListBox(GLUFControlIndex ID, const GLUFRect& region, GLUFBitfield style = 0, std::shared_ptr<GLUFListBoxPtr> ctrlPtr = nullptr);

    /*
    AddControl
        
        Note:
            This is the generic function for adding custom or preconstructed controls

        Parameters:
            'control': a pointer to the control to add

        Throws:
            'GLUFControlCreationException': if control initialization failed
    
    */
	void AddControl(GLUFControlPtr& control);

    /*
    InitControl
        
        Note:
            Initializes a control with the default element, and calls 'OnInit' on it

        Throws:
            'GLUFControlCreationException': if control initialization failed

    */
	void InitControl(GLUFControlPtr& control);


    /*
    GetControl
    
        Parameters:
            'ID': the id of the control to retreive
            'controlType' the control type to filter

        Returns:
            a poitner to the control requested, in release mode, returns nullptr if 'ID' not found

        Throws:
            'std::bad_cast' if 'T' is not derived from 'GLUFControl'
            'std::invalid_argument': if 'ID' is not found, but only in GLUF_DEBUG mode
    
    */
    template<typename T>
    std::shared_ptr<T>  GetControl(GLUFControlIndex ID) const;
    GLUFControlPtr      GetControl(GLUFControlIndex ID, GLUFControlType controlType) const;


    /*
    GetControlAtPoint

        Parameters:
            'pt': the point in screen coordinates to look for the control

        Returns:
            a pointer to the control found, in release mode, returns nullptr if none found

        Throws:
            'std::invalid_argument': if no control exists at 'pt', but only in GLUF_DEBUG mode
    
    */
	GLUFControlPtr GetControlAtPoint(const GLUF::GLUFPoint& pt) const;

    /*
    Set/Get Control Enabled

        Parameters:
            'ID': id of the control
            'bEnabled': new enabled state

        Returns:
            the enabled state

        Throws:
            'std::invalid_argument': if 'ID' does not exist, only in GLUF_DEBUG
    
    */
    bool GetControlEnabled(GLUFControlIndex ID) const;
    void SetControlEnabled(GLUFControlIndex ID, bool bEnabled);

    /*
    ClearRadioButtonGroup
        
        Note:
            Resets the radio button group

        Parameters:
            'group': the id of the group
    
    */
    void ClearRadioButtonGroup(GLUFRadioButtonGroup group);

    /*
    ClearComboBox

        Note:
            Removes all of the items from this combo box

        Parameters:
            'ID': the id of the combo box

        Throws:
            'std::invalid_argument': if 'ID' does not exist, or is not a combo box, in GLUF_DEBUG mode only
    
    */
	void ClearComboBox(GLUFControlIndex ID);

    /*
    Set/Get DefaultElement

        Parameters:
            'controlType': the control type to set the default to
            'elementIndex': the element id within the control to set
            'element': the element to set

        Returns:
            the element of the control at 'elementIndex'

        Throws:
            'std::invalid_argument': if 'elementIndex' is not found within 'controlType', but only in GLUF_DEBUG; or if 'element' == nullptr
    
    */
    void            SetDefaultElement(GLUFControlType controlType, GLUFElementIndex elementIndex, const GLUFElementPtr& element);
    GLUFElementPtr  GetDefaultElement(GLUFControlType controlType, GLUFElementIndex elementIndex) const;


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
	void SendEvent(GLUFEvent ctrlEvent, bool triggeredByUser, GLUFControlPtr control) noexcept;


    /*
    RequestFocus

        Parameters:
            'control': the control to request focus to

        Throws:
            'std::invalid_argument': if 'control' == nullptr, in GLUF_DEBUG mode
    
    */
	void RequestFocus(GLUFControlPtr& control);

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

        Throws:
            'std::invalid_argument' if 'elemement' == nullptr in GLUF_DEBUG

    */
	void DrawRect(const GLUF::GLUFRect& rect, const GLUF::Color& color);
	//void DrawPolyLine(GLUF::GLUFPoint* apPoints, uint32_t nNumPoints, GLUF::Color color);
	void DrawSprite(const GLUFElementPtr& element, const GLUF::GLUFRect& rect, float depth, bool textured = true);
    void DrawText(const std::wstring& text, const GLUFElementPtr& element, const GLUF::GLUFRect& rect, bool shadow = false, bool hardRect = false);

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
            'std::invalid_argument': if 'element' == nullptr, in GLUF_DEBUG
    
    */
	void CalcTextRect(const std::wstring& text, const GLUFElementPtr& element, GLUF::GLUFRect& rect) const;


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
    GLUFPoint   GetLocation() const noexcept                            { return{ mRegion.bottom, mRegion.left };   } 
    GLUFRect    GetRegion() const noexcept                              { return mRegion;                           }
    long        GetWidth() const noexcept                               { return GLUFRectWidth(mRegion);            }
    long        GetHeight()	const noexcept                              { return GLUFRectHeight(mRegion);           } 

    GLUFPoint   GetMousePositionDialogSpace() const noexcept            { return mMousePositionDialogSpace;        }

    GLUFDialogResourceManagerPtr GetManager() const noexcept            { return mDialogManager;                    }

    void        SetVisible(bool visible) noexcept                       { mVisible = visible;                       }
    void        SetMinimized(bool minimized) noexcept                   { mMinimized = minimized;                   }
    void        SetBackgroundColor(const GLUF::Color& color) noexcept   { mDlgElement->mTextureColor.SetAll(color); }
    void        SetCaptionHeight(long height) noexcept                  { mCaptionHeight = height;                  }
    void        SetCaptionText(const std::wstring& text) noexcept       { mCaptionText = text;                      }
    void        SetLocation(long x, long y) noexcept                    { GLUFRepositionRect(mRegion, x, y);        }
    void        SetSize(long width, long height) noexcept               { GLUFResizeRect(mRegion, width, height);   }
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
    void                SetFont(GLUFFontIndex fontIndex, GLUFFontIndexResMan resManFontIndex);
    void                SetTexture(GLUFTextureIndex texIndex, GLUFTextureIndexResMan resManTexIndex);
    GLUFFontNodePtr	    GetFont(GLUFFontIndex index) const;
    GLUFTextureNodePtr	GetTexture(GLUFTextureIndex index) const;


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
	static GLUFControlPtr GetNextControl(GLUFControlPtr control);
	static GLUFControlPtr GetPrevControl(GLUFControlPtr control);

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
            'std::invalid_argument': in GLUF_DEBUG if no controls have id 'ID'
    
    */
	void RemoveControl(GLUFControlIndex ID);

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
	void SetCallback(GLUFEventCallbackFuncPtr callback, GLUFEventCallbackReceivablePtr userContext = nullptr) noexcept;


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
	void OnMouseMove(const GLUF::GLUFPoint& pt) noexcept;
	void OnMouseUp(const GLUF::GLUFPoint& pt) noexcept;

    /*
    SetNextDialog

        Note:
            Sets the next dialog for cycling

        Parameters:
            'nextDialog': the dialog to switch to when this dialog closes            

        Throws:
            no-throw guarantee
    
    */
	void SetNextDialog(GLUFDialogPtr nextDialog) noexcept;

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
GLUFTextureNode

    Note:
        A barebones texture node; plans for future expansion

    Data Members:
        'mTextureElement': the index of the texture within the dialog

*/
struct GLUFTextureNode
{
	GLUFTextureIndex mTextureElement;
};

//WIP, support more font options eg. stroke, italics, variable leading, etc.

/*
GLUFFontNode

    Note:
        Work in progress; support for more typographical options in the future, i.e.:
            stroke, italics, leading, etc.

    Data Members:
        'mWeight': WIP, doesn't do anything yet
        'mLeading': WIP, doesn't do anything yet
        'mFontType': an opaque pointer to the font to use, defaults to arial

*/
struct GLUFFontNode
{
	//GLUFFontSize mSize;
	GLUFFontWeight mWeight;
	GLUFFontSize mLeading;
	GLUFFontPtr mFontType;
};








/*
======================================================================================================================================================================================================
GLUFSpriteVertexStruct


*/



/*
GLUFSpriteVertexStruct

    Note:
        This is derived from the vertex struct class for use on AoS data packing

    Data Members:
        'mPos': a position
        'mColor': a color
        'mTexCoords': a uv coord

*/
struct GLUFSpriteVertexStruct : public GLUFVertexStruct
{
	glm::vec3 mPos;
	GLUF::Color4f mColor;
    glm::vec2 mTexCoords;

    GLUFSpriteVertexStruct(){}
    GLUFSpriteVertexStruct(const glm::vec3& pos, const GLUF::Color4f& color, const glm::vec2& texCoords) : 
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

    static GLUFGLVector<GLUFSpriteVertexStruct> MakeMany(size_t howMany)
    {
        GLUFGLVector<GLUFSpriteVertexStruct> ret;
        ret.resize(howMany);

        return ret;
    }
};

/*
GLUFDialogResourceManager

    Note:
        Manages Resources Shared by the Dialog

    Data Members:
        'mWndSize': the size of the window
        'mSpriteBuffer': the vertex array for drawing sprites; use 'GLUFVertexArray'
            because it automatically handles OpenGL version restrictions
        'mDialogs': the list of registered dialogs
        'mTextureCache': a list of shared textures
        'mFontCache': a list of shared fonts

*/
class GLUFDialogResourceManager
{

    GLUF::GLUFPoint mWndSize;

    GLUFVertexArray mSpriteBuffer;
    std::vector<GLUFDialogPtr> mDialogs;
    std::vector<GLUFTextureNodePtr> mTextureCache;
    std::vector<GLUFFontNodePtr>    mFontCache;

    friend GLUFDialog;
public:
	GLUFDialogResourceManager();
	~GLUFDialogResourceManager();

    /*
    MsgProg

        Note:
            This only cares about window resize messages in order to keep 'mWndSize' accurate

        Parameters:
            see 'GLUFEventCallbackFuncPtr' for details

        Throws:
            no-throw guarantee
    */
	bool MsgProc(GLUFMessageType msg, int32_t param1, int32_t param2, int32_t param3, int32_t param4) noexcept;
    
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
	void EndSprites(GLUFElementPtr element, bool textured);

    /*
    GetFont/TextureNode

        Parameters:
            'index': the index of the font node within the DRM
        
        Throws:
            'std::out_of_range': if 'index' is larger than the size of the font/texture cache
    */
	GLUFFontNodePtr    GetFontNode(GLUFFontIndex index) const		{ return mFontCache[index];		}
	GLUFTextureNodePtr GetTextureNode(GLUFTextureIndex index) const	{ return mTextureCache[index];	}


    /*
    GetTexture/FontCount

        Returns:
            the number of fonts/textures

        Throws:
            no-throw guarantee
    
    */
	GLUFSize GetTextureCount() const noexcept   { return static_cast<GLUFSize>(mTextureCache.size()); }
	GLUFSize GetFontCount() const noexcept      { return static_cast<GLUFSize>(mFontCache.size());    }

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
	GLUFFontIndex AddFont(const GLUFFontPtr& font, GLUFFontSize leading, GLUFFontWeight weight) noexcept;

    /*
    AddTexture
        
        Note:
            TODO: when GLUF supports a more advanced texture system, pass one of those,
                but for now just require an openGL texture id

        Parameters:
            'texture': the OpenGL Texture Id to use

        Returns:
            the index of the created texture

        Throws:
            no-throw guarantee
    */
	GLUFTextureIndex AddTexture(GLuint texture) noexcept;

    /*
    RegisterDialog

        Note:
            This sets up the dialogs' focus order for tabbing

        Parameters:
            'dialog': the dialog to register

        Throws:
            no-throw guarantee
    
    */
	void RegisterDialog(const GLUFDialogPtr& pDialog) noexcept;

    /*
    UnregisterDialog

        Parameters:
            'dialog': the dialog to remove

        Throws:
            'std::invalid_argument': if 'dialog' == nullptr or if 'dialog' is not a registered dialog in GLUF_DEBUG mode
    
    */
	void UnregisterDialog(const GLUFDialogPtr& dialog);

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
	GLUF::GLUFPoint GetWindowSize();
    
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
    GLUFDialogPtr GetDialogPtrFromRef(const GLUFDialog& ref);
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

class GLUFControlInitException
{
    virtual const char* what() const
    {
        return "GLUFControl or child failed to initialized correctly!";
    }

    EXCEPTION_CONSTRUCTOR(GLUFControlInitException);
};

/*
GLUFControl


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
class GLUFControl : public std::enable_shared_from_this<GLUFControl>
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
    GLUFControl(GLUFDialog& dialog);

    friend GLUFDialog;

    GLUFControlIndex mID;
    GLUFControlType mType;
    GLUFKeyId  mHotkey;

    bool mEnabled;
    bool mVisible;
    bool mMouseOver;
    bool mHasFocus;
    bool mIsDefault;

    GLUFRect mRegion;

    GLUFDialog& mDialog;
    GLUFControlIndex mIndex;

    //use a map, because there may be gaps in indices
    std::map<GLUFElementIndex, GLUFElementPtr> mElements;


public:
	virtual ~GLUFControl();


    /*
    OnInit

        Throws:
            'GLUFControlInitException': if child class initialization fails
    
    */
	virtual void OnInit(){ }

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
            see 'GLUFEventCallbackFuncPtr' for details

        Throws:
            no-throw guarantee
    */
	virtual bool MsgProc(GLUFMessageType msg, int32_t param1, int32_t param2, int32_t param3, int32_t param4) noexcept { return false; }

             
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
    GLUFControlType GetType() const	noexcept		    { return mType;                 }
	virtual bool	CanHaveFocus() const noexcept	    { return false;					}
    GLUFRect GetRegion() const noexcept                 { return mRegion;               }
    GLUFElementPtr	GetElement(GLUFElementIndex element) const;

    virtual void	SetEnabled(bool enabled) noexcept		    { mEnabled = enabled;                       }
    virtual void	SetVisible(bool visible) noexcept		    { mVisible = visible;                       }
    virtual void    SetRegion(const GLUF::GLUFRect& region) noexcept
                                                                { mRegion = region;                         }
    void			SetLocation(long x, long y) noexcept	    { GLUFRepositionRect(mRegion, x, y);        }
    void			SetSize(long width, long height) noexcept   { GLUFResizeRect(mRegion, width, height);   }
    void			SetHotkey(GLUFKeyId hotkey) noexcept		{ mHotkey = hotkey;                         }
    void			SetID(int ID) noexcept					    { mID = ID;                                 }


    /*
    EventHandlers

        Note:
            These are barebones handlers, assuming the control does nothing at all upon these events

        Throws:
            no-throw guarantee; children may modify this behavior, but should not
    
    */
    virtual bool ContainsPoint(const GLUF::GLUFPoint& pt) const noexcept  
                                            { return GLUFPtInRect(mRegion, pt);                         }
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
	void SetElement(GLUFElementIndex elementId, const GLUFElementPtr& element) noexcept;

    /*
    SetTextColor

        Note:
            Sets the text color for all elements

        Parameters:
            'color': the color to set to

        Throws:
            no-throw guarantee
    
    */
	virtual void SetTextColor(const GLUF::Color& color) noexcept;


protected:
    /*
    UpdateRects

        Note:
            This method is used to recreate the regions for different elements when the main rect changes

    */
    virtual void UpdateRects() noexcept{}
};



/*
GLUFElementHolder

    Note:
        This class is used for default element data structures

    Data Members:
        'mControlType': which control type is this element for
        'mElementIndex': index of the element within the default elements
        'mElement': the element itself

*/
struct GLUFElementHolder
{
	GLUFControlType mControlType;
	GLUFElementIndex mElementIndex;
	GLUFElementPtr mElement;
};



/*
GLUFStatic

    Note:
        A Static Text Control; Draws Text with no backgroud

    Data Members:
        'mText': the text to display
        'mTextFlags': the text formatting flats

*/
class GLUFStatic : public GLUFControl
{
protected:
    std::wstring     mText;
    GLUFBitfield     mTextFlags;

    GLUFStatic(const GLUFBitfield& textFlags, GLUFDialog& dialog);
    friend std::shared_ptr<GLUFStatic> CreateStatic(GLUFBitfield textFlags, GLUFDialog& dialog);

    GLUFStatic() = delete;
    
public:


    /*
    ContainsPoint

        Note:
            This always returns false, because this control never should receive message
    
    */
	virtual bool ContainsPoint(const GLUF::GLUFPoint& pt) const override { return false; }

    /*
    Getters and Setters
    
        Throws:
            no-throw guarantee

    */
    void                  GetTextCopy(std::wstring& dest) const noexcept    { dest = mText;         }
	const std::wstring&   GetText() const noexcept                          { return mText;         }
    void                  SetText(const std::wstring& text) noexcept        { mText = text;         }
    void                  SetTextFlags(GLUFBitfield flags) noexcept         { mTextFlags = flags;   }


    /*
    Overridden Unambiguous Member Functions

    */
    virtual void Render(float elapsedTime) noexcept override;
};


/*
GLUFButton

    DataMembers:
        'mPressed': boolean state of the button

*/
class GLUFButton : public GLUFStatic
{

protected:

    GLUFButton() = delete;
    GLUFButton(GLUFDialog& dialog);
    friend std::shared_ptr<GLUFButton> CreateButton(GLUFDialog& dialog);

    bool mPressed;

public:

    /*
    Overridden Unambiguous Member Functions
    
    */
    virtual bool CanHaveFocus()	const noexcept override	{ return (mVisible && mEnabled); }
    virtual void Render(float elapsedTime) noexcept override;
    virtual bool MsgProc(GLUFMessageType msg, int32_t param1, int32_t param2, int32_t param3, int32_t param4) noexcept override;
    virtual void OnHotkey() noexcept override;

};


/*
GLUFCheckBox

    Parents:
        GLUFButton

    Data Members:
        'mChecked': is the box checked
        'mButtonRegion': the region the button covers
        'mTextRegion': the region the text covers

*/
class GLUFCheckBox : public GLUFButton
{

protected:

    GLUFCheckBox() = delete;
    GLUFCheckBox(bool checked, GLUFDialog& dialog);
    friend std::shared_ptr<GLUFCheckBox> CreateCheckBox(bool checked, GLUFDialog& dialog);

    bool mChecked;
    GLUF::GLUFRect mButtonRegion;
    GLUF::GLUFRect mTextRegion;

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
    virtual bool MsgProc(GLUFMessageType msg, int32_t param1, int32_t param2, int32_t param3, int32_t param4) noexcept override;
	virtual void Render(float elapsedTime) noexcept override;
    virtual void OnHotkey() noexcept override;
    virtual bool ContainsPoint(const GLUF::GLUFPoint& pt) const override;
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
GLUFRadioButton

    Parents:
        GLUFCheckBox

    Data Members
        'mButtonGroup': the id of the button group this belongs to in a dialog
*/
class GLUFRadioButton : public GLUFCheckBox
{
public:
    
    GLUFRadioButton() = delete;
    GLUFRadioButton(GLUFDialog& dialog);
    friend std::shared_ptr<GLUFRadioButton> CreateRadioButton(GLUFDialog& dialog);

    GLUFRadioButtonGroup mButtonGroup;

public:


    /*
    Setters and Getters
    
        Throws:
            no-throw guarantee
    */
	void            SetChecked(bool checked, bool clearGroup = true) noexcept   { SetCheckedInternal(checked, clearGroup, false);   }
    void            SetButtonGroup(GLUFRadioButtonGroup buttonGroup) noexcept   { mButtonGroup = buttonGroup;                       }
	unsigned int    GetButtonGroup() const noexcept                             { return mButtonGroup;                              }


    /*
    Overridden Unambiguous Member Functions

    */
	virtual bool MsgProc(GLUFMessageType msg, int32_t param1, int32_t param2, int32_t param3, int32_t param4) noexcept override;
    virtual void OnHotkey() noexcept override;

protected:

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
GLUFScrollBar

    Parents:
        GLUFControl
        
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
class GLUFScrollBar : public GLUFControl
{
public:

    /*
    GLUFArrowState

        'CLEAR':            No arrow is down.
        'CLICKED_UP':       Up arrow is clicked.
        'CLICKED_DOWN':     Down arrow is clicked.
        'HELD_UP':          Up arrow is held down for sustained period.
        'HELD_DOWN':        Down arrow is held down for sustained period.
    */
    enum GLUFArrowState
    {
        CLEAR,
        CLICKED_UP,
        CLICKED_DOWN,
        HELD_UP,
        HELD_DOWN
    };
protected:

    GLUFScrollBar() = delete;
    GLUFScrollBar(GLUFDialog& dialog);
    friend std::shared_ptr<GLUFScrollBar> CreateScrollBar(GLUFDialog& dialog);


    bool mShowThumb;
    bool mDrag;
    GLUF::GLUFRect mUpButtonRegion;
    GLUF::GLUFRect mDownButtonRegion;
    GLUF::GLUFRect mTrackRegion;
    GLUF::GLUFRect mThumbRegion;
    GLUFValue mPosition;
    GLUFValue mPageSize;
    GLUFValue mStart;
    GLUFValue mEnd;
    GLUF::GLUFPoint mPreviousMousePos;
    GLUFArrowState mArrow;
    double mArrowTS;

public:
	virtual         ~GLUFScrollBar();


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
    virtual bool MsgProc(GLUFMessageType msg, int32_t param1, int32_t param2, int32_t param3, int32_t param4) noexcept override;
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
GLUFGenericData

    Note:
        This should be the base for all data which goes in 'GLUFListBox' or 'GLUFComboBox'

*/
struct GLUFGenericData
{};

/*
GLUFListBoxItem

    Data Members:
        'mText': the text to represent the data
        'mData': the data this list box represents
        'mVisible': is visible?
        'mActiveRegion': the active region on the screen

*/
typedef struct GLUFListBoxItem_t
{
	std::wstring mText;
    GLUFGenericData& mData;
	bool mVisible;
	GLUF::GLUFRect mActiveRegion;

    GLUFListBoxItem_t(GLUFGenericData& data) : mData(data){}
} GLUFListBoxItem, GLUFComboBoxItem;

using GLUFListBoxItemPtr = std::shared_ptr < GLUFListBoxItem > ;
using GLUFComboBoxItemPtr = std::shared_ptr < GLUFComboBoxItem > ;

class NoItemSelectedException : GLUFException
{
public:
    virtual const char* what() const override
    {
        return "No Item Selected In List Box or Combo Box!";
    }

    EXCEPTION_CONSTRUCTOR(NoItemSelectedException);
};

/*
GLUFListBox

    Parents:
        'GLUFControl'

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
class GLUFListBox : public GLUFControl
{
protected:

    GLUFListBox() = delete;
    GLUFListBox(GLUFDialog& dialog);
    friend std::shared_ptr<GLUFListBox> CreateListBox(GLUFDialog& dialog);


    GLUF::GLUFRect mTextRegion;
    GLUF::GLUFRect mSelectionRegion;
    GLUFScrollBarPtr mScrollBar;
    GLUFSize mSBWidth;
    GLUFSize mVerticalMargin;
    GLUFSize mHorizontalMargin;
    GLUFFontSize  mTextHeight; 
    GLUFBitfield mStyle; 
    std::vector<GLUFIndex> mSelected;
    bool mDrag;
    std::vector<GLUFListBoxItemPtr> mItems;

public:
	virtual ~GLUFListBox();

    enum GLUFListBoxStyle
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
    
	GLUFGenericData&    GetItemData(const std::wstring& text, GLUFIndex start) const;
    GLUFGenericData&    GetItemData(GLUFIndex index) const;
	GLUFSize            GetNumItems() const	noexcept		{ return mItems.size();		}
    GLUFListBoxItemPtr  GetItem(const std::wstring& text, GLUFIndex start = 0) const;
    GLUFListBoxItemPtr  GetItem(GLUFIndex index) const		{ return mItems[index];     }
	GLUFBitfield        GetStyle() const noexcept			{ return mStyle;			}
	GLUFSize            GetScrollBarWidth() const noexcept	{ return mSBWidth;          }

	void                SetStyle(GLUFBitfield style)  noexcept				       { mStyle = style;						                       }
    void                SetScrollBarWidth(GLUFSize width) noexcept                 { mSBWidth = width; UpdateRects();                              }
    void                SetMargins(GLUFSize vertical, GLUFSize horizontal) noexcept{ mVerticalMargin = vertical; mHorizontalMargin = horizontal;   }

    /*
    AddItem

        Parameters:
            'text': the text for the item
            'data': the data for the item to represent

        Throws:
            no-throw guarantee
    */
    void AddItem(const std::wstring& text, GLUFGenericData& data) noexcept;

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
    void InsertItem(GLUFIndex index, const std::wstring& text, GLUFGenericData& data) noexcept;

    /*
    RemoveItem

        Parameters:
            'index': the index to remove, which then makes all elements after move back one
        
        Throws:
            'std::out_of_range': if 'index' is too big in GLUF_DEBUG

    */
	void RemoveItem(GLUFIndex index);

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
	GLUFIndex GetSelectedIndex(GLUFIndex previousSelected) const;//for multi-line
    GLUFIndex GetSelectedIndex() const;//for single-line (or finding the first selected item)

    /*
    GetSelectedItem
        
        Parameters:
            'previouslySelected': if multi-selection, the previously selected item

        Returns:
            the object representing of the selected item or next selected item in multi-selection

        Throws:
            'NoItemSelectedException': if no item is selected
    
    */
    GLUFListBoxItemPtr GetSelectedItem(GLUFIndex previousSelected) const{ return GetItem(GetSelectedIndex(previousSelected)); }
    GLUFListBoxItemPtr GetSelectedItem() const { return GetItem(GetSelectedIndex()); }
    
    /*
    GetSelectedData
        
        Parameters:
            'previouslySelected': if multi-selection, the previously selected item

        Returns:
            the data of the selected item or next selected item in multi-selection

        Throws:
            'NoItemSelectedException': if no item is selected
    
    */
    GLUFGenericData& GetSelectedData(GLUFIndex previousSelected) const { return GetItemData(GetSelectedIndex(previousSelected)); }
    GLUFGenericData& GetSelectedData() const { return GetItemData(GetSelectedIndex()); }

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
            'std::out_of_range': if index is too big in GLUF_DEBUG
            'std::invalid_argument': if 'text' is not found in GLUF_DEBUG
    */
	void SelectItem(GLUFIndex index);
    void SelectItem(const std::wstring& text, GLUFIndex start = 0);

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
            'std::invalid_argument': if 'index' does not exist, in GLUF_DEBUG
    */
    void RemoveSelected(GLUFIndex index);
    void RemoveSelected(const std::wstring& text, GLUFIndex start = 0);

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
    bool ContainsItem(const std::wstring& text, GLUFIndex start = 0) const noexcept;

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
    GLUFIndex           FindItemIndex(const std::wstring& text, GLUFIndex start = 0) const;
    GLUFListBoxItemPtr  FindItem(const std::wstring& text, GLUFIndex start = 0) const;


    /*
    Overrided Unambiguous Member Functions
    

    */
	virtual bool MsgProc(GLUFMessageType msg, int32_t param1, int32_t param2, int32_t param3, int32_t param4) noexcept override;
	virtual void OnInit() override                      { mDialog.InitControl(std::dynamic_pointer_cast<GLUFControl>(mScrollBar));	}
	virtual bool CanHaveFocus() const noexcept override	{ return (mVisible && mEnabled);			    }
	virtual void Render(float elapsedTime) noexcept override;
	virtual void UpdateRects() noexcept override;
	virtual bool ContainsPoint(const GLUF::GLUFPoint& pt) const noexcept override{ return GLUFControl::ContainsPoint(pt) || mScrollBar->ContainsPoint(pt); }

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
GLUFComboBox

    Parents:   
        'GLUFButton'

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
class GLUFComboBox : public GLUFButton
{
protected:

    GLUFComboBox() = delete;
    GLUFComboBox(GLUFDialog& dialog);
    friend std::shared_ptr<GLUFComboBox> CreateComboBox(GLUFDialog& dialog);

    GLUFsIndex mSelected;
    GLUFsIndex mFocused;
    GLUFSize mDropHeight;
    GLUFScrollBarPtr mScrollBar;
    GLUFSize mSBWidth;

    bool mOpened;

    GLUF::GLUFRect mTextRegion;
    GLUF::GLUFRect mButtonRegion;
    GLUF::GLUFRect mDropdownRegion;
    GLUF::GLUFRect mDropdownTextRegion;

    std::vector <GLUFComboBoxItemPtr> mItems;

public:
	virtual         ~GLUFComboBox();
    

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
    
	GLUFGenericData&    GetItemData(const std::wstring& text, GLUFIndex start = 0) const;
    GLUFGenericData&    GetItemData(GLUFIndex index) const;
	GLUFSize            GetScrollBarWidth() const noexcept	{ return mSBWidth;          }
	GLUFSize            GetNumItems() const	noexcept		{ return mItems.size();		}
    GLUFComboBoxItemPtr GetItem(const std::wstring& text, GLUFIndex start = 0) const;
    GLUFComboBoxItemPtr GetItem(GLUFIndex index) const		{ return mItems[index]; }

    void                SetDropHeight(GLUFSize nHeight)			    { mDropHeight = nHeight; UpdateRects(); }
    void                SetScrollBarWidth(GLUFSize width) noexcept  { mSBWidth = width; UpdateRects();      }

    /*
    AddItem

        Parameters:
            'text': the text for the item
            'data': the data for the item to represent

        Throws:
            no-throw guarantee
    */
    void AddItem(const std::wstring& text, GLUFGenericData& data) noexcept;

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
    void InsertItem(GLUFIndex index, const std::wstring& text, GLUFGenericData& data) noexcept;//IMPLEMENT

    /*
    RemoveItem

        Parameters:
            'index': the index to remove, which then makes all elements after move back one
        
        Throws:
            'std::out_of_range': if 'index' is too big in GLUF_DEBUG

    */
	void RemoveItem(GLUFIndex index);

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
    GLUFIndex GetSelectedIndex() const;

    /*
    GetSelectedItem

        Returns:
            the object representing of the selected item

        Throws:
            'NoItemSelectedException': if no item is selected
    
    */
    GLUFListBoxItemPtr GetSelectedItem() const;
    
    /*
    GetSelectedData

        Returns:
            the data of the selected item

        Throws:
            'NoItemSelectedException': if no item is selected
    
    */
    GLUFGenericData& GetSelectedData() const;

    /*
    SelectItem
    
        Parameters:
            'index': the index of the item to select
            'text': the text of the item to select
            'start': the starting point to look for 'text'
            'data': the data to find

        Throws:
            'std::out_of_range': if index is too big in GLUF_DEBUG
            'std::invalid_argument': if 'text' is not found in GLUF_DEBUG, or if 'data' is not found in GLUF_DEBUG
    */
	void SelectItem(GLUFIndex index);
    void SelectItem(const std::wstring& text, GLUFIndex start = 0);
    void SelectItem(const GLUFGenericData& data);
    
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
    bool ContainsItem(const std::wstring& text, GLUFIndex start = 0) const noexcept;

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
    GLUFIndex           FindItemIndex(const std::wstring& text, GLUFIndex start = 0) const;
    GLUFComboBoxItemPtr FindItem(const std::wstring& text, GLUFIndex start = 0) const;


    /*
    Overridden Unambiguous Member Functions
    
    
    */
	virtual bool MsgProc(GLUFMessageType msg, int32_t param1, int32_t param2, int32_t param3, int32_t param4) noexcept override;
	virtual void OnHotkey() noexcept override;
	virtual bool CanHaveFocus() const noexcept override{ return (mVisible && mEnabled); }
	virtual void OnFocusOut() noexcept override;
	virtual void Render(float elapsedTime) noexcept override;
	virtual void UpdateRects() noexcept override;
	virtual void OnInit() override{ return mDialog.InitControl(std::dynamic_pointer_cast<GLUFControl>(mScrollBar)); }
	virtual void SetTextColor(const GLUF::Color& Color) noexcept override;

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
GLUFSlider

    Parents:
        'GLUFControl'

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
class GLUFSlider : public GLUFControl
{
protected:

    GLUFSlider() = delete;
    GLUFSlider(GLUFDialog& dialog);
    friend std::shared_ptr<GLUFSlider> CreateSlider(GLUFDialog& dialog);

    GLUFValue mValue;
    GLUFValue mMin;
    GLUFValue mMax;
    GLUFValue mDragX;
    GLUFValue mDragOffset;
    GLUFValue mButtonX;
    bool mPressed;
    GLUF::GLUFRect mButtonRegion;

public:

    /*
    Setters and Getters

        Throws:
            no-throw guarantee
    
    */
    GLUFValue   GetValue() const noexcept           { return mValue;                    }
    void        GetRange(GLUFValue& nMin, GLUFValue& nMax) const noexcept
                                                    { nMin = mMin; nMax = mMax;         }
    void        SetValue(GLUFValue nValue) noexcept { SetValueInternal(nValue, false);  }
    void        SetRange(GLUFValue  nMin, GLUFValue  nMax) noexcept;

    /*
    Overridden Unambiguous Member Functions
    
    
    */
    virtual bool MsgProc(GLUFMessageType msg, int32_t param1, int32_t param2, int32_t param3, int32_t param4) noexcept override;
    virtual bool ContainsPoint(const GLUF::GLUFPoint& pt) const noexcept override;
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
	GLUFValue ValueFromXPos(GLUFUValue xPos) const noexcept;

};



/*


Stopped Here June 4 2015


*/


/*
GLUFEditBox

    Temp:
        This is a total rebuild of the edit box class

    Parents:
        'GLUFControl'

    Data Members:
        'mText': the source text buffer
        'mFormattedText': the text which will be rendered (not guaranteed to contain all of the text)
        'mCharset': the available character set for user input (denies anything not within the charset)
        'mAnalyseRequired': has the string changed since the previous analysis
        'mBorder': the boarder on every edge of the edit box
        'mSpacing': the spacing between the text and the edge of the boarder
        'mTextRegion': the region the text occupies
        'mBlinkPeriod': the time between caret blinks in seconds
        'mPreviousBlink': the previous blink time of the caret
        'mCaretOn': whether the caret is currently visible due to blinking periods
        'mCaretPos': the caret position in characters of the original string buffer
        'mInsertMode': true: insert mode; false: overwrite mode
        'mSelectStart': the starting position of the selection; the caret marks the end
        'mTextColor': the color of the text
        'mSelectTextColor': the color of the selected text
        'mSelectBkgrndColor': the color of the selected region's highlight
        'mCaretColor': the color of the caret
        'mScrollBar': the internal scrollbar used in single-line, and one which is displayed in multi-line
        'mSBWidth': the width of the scrollbar
        'mMouseDrag': indicates if a drag is in progress
        'mMultiline': whether or not the box is multiline or not
        'mHideCaret': whether or not to hide the caret completely

*/
/*class GLUFEditBox : public GLUFControl
{
    GLUF_FORCE_SMART_POINTERS(GLUFEditBox, GLUFCharset& charset, bool& isMultiline, GLUFDialog& dialog);

protected:

    std::wstring mText;
    std::wstring mFormattedText;

    GLUFCharset mCharset;
    bool mAnalyseRequired;

    GLUFSize mBorder;
    GLUFSize mSpacing;
    GLUF::GLUFRect mTextRegion;

    double mBlinkPeriod;
    double mPreviousBlink;
    bool mMouseDrag;
    bool mMultiline = true;
    bool mCaretOn;
    bool mHideCaret;
    GLUFValue mCaretPos;
    bool mInsertMode;
    GLUFValue mSelStart;

    GLUF::Color mTextColor;
    GLUF::Color mSelTextColor;
    GLUF::Color mSelBkColor;
    GLUF::Color mCaretColor;

    GLUFScrollBar mScrollBar;
    GLUFSize mSBWidth;

public:
	virtual         ~GLUFEditBox();

    //ended updating here!

	void            SetText(std::wstring wszText, bool bSelected = false);
	std::wstring     GetText(){ return m_strBuffer; }
	int             GetTextLength(){ return (int)m_strBuffer.length(); }  // Returns text length in chars excluding nullptr.
	std::wstring     GetTextClamped();//this gets the text, but clamped to the bounding box, (NOTE: this will overflow off the bottom);
	void            ClearText();
	virtual void    SetTextColor(GLUF::Color Color){ m_TextColor = Color; }  // Text color
	void            SetSelectedTextColor(GLUF::Color Color){ m_SelTextColor = Color; }  // Selected text color
	void            SetSelectedBackColor(GLUF::Color Color){ m_SelBkColor = Color; }  // Selected background color
	void            SetCaretColor(GLUF::Color Color){ m_CaretColor = Color; }  // Caret color
	void            SetBorderWidth(long fBorder){ m_fBorder = fBorder; UpdateRects(); m_bAnalyseRequired = true; }  // Border of the window
	void            SetSpacing(long fSpacing){ m_fSpacing = fSpacing; UpdateRects(); m_bAnalyseRequired = true; }
	//void            ParseFloatArray(float* pNumbers, int nCount);
	//void            SetTextFloatArray(const float* pNumbers, int nCount);


    /*
    Overriden Unambiguous Member Functions

    
    
	virtual bool MsgProc(GLUFMessageType msg, int32_t param1, int32_t param2, int32_t param3, int32_t param4) noexcept override;
	virtual void UpdateRects() noexcept override;
	virtual bool CanHaveFocus() const noexcept override { return (mVisible && mEnabled); }
	virtual void Render(float elapsedTime) noexcept override;
	virtual void OnFocusIn() noexcept override;
	virtual void OnInit() override { return mDialog.InitControl(mScrollBar); }


protected:
	void            PlaceCaret(int nCP);//input m_strBuffer
	void			PlaceCaretRndBuffer(int nRndCp);//input m_strRenderBuffer
	void            DeleteSelectionText();
	void            ResetCaretBlink();
	void            CopyToClipboard();
	void            PasteFromClipboard();

	//NOTE: nCP must be the index from m_strRenderBuffer;
	int             GetLineNumberFromCharPos(unsigned int nCP);//the value returned is the line number within the box, so even if it is scrolled, the top line is still 0

	int				GetStrIndexFromStrRenderIndex(int strRenderIndex);//this is used to convert an index of an object that was clicked on the screen to the index of the real string
	int             GetStrRenderIndexFromStrIndex(int strIndex);//just the opposite

	//former CUniBuffer Methods
	void Analyse();
	
	//NOTE: input the cursor position in m_strRenderBuffer space
	bool CPtoRC(int nCP, GLUF::GLUFRect *pPt);

	//NOTE: outputs the cursor position in m_strRenderBuffer space
	bool PttoCP(GLUF::GLUFPoint pt, int* pCP, bool* bTrail);
	
	//NOTE: all methods referencing a position within the edit box will be done IN STRING  SPACE and will be converted appropriately to make it so

	void InsertString(int pos, std::wstring str);
	void InsertChar(int pos, wchar_t ch);

	void RemoveString(int pos, int len);
	void RemoveChar(int pos);

	void GetNextItemPos(int pos, int& next);
	void GetPriorItemPos(int pos, int& prior);

	int GetNumNewlines();
};*/



/*
======================================================================================================================================================================================================
Text Controls Below

*/


/*
GLUFTextHelper

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
class OBJGLUF_API GLUFTextHelper
{
protected:

    GLUFTextHelper() = delete;
    GLUFTextHelper(GLUFDialogResourceManagerPtr& drm);
    friend std::shared_ptr<GLUFTextHelper> CreateTextHelper(GLUFDialogResourceManagerPtr& drm);


    GLUFDialogResourceManagerPtr& mManager;

    /*
    Helper overloads for RenderString
    
    */

    template<typename T1, typename... Types>
    void RenderText(std::wstringstream& formatStream, std::wstringstream& outString, T1 arg1, Types... args);

    template<typename T1>
    void RenderText(std::wstringstream& formatStream, std::wstringstream& outString, T1 arg);

    GLUFFontIndex mFontIndex;
    GLUFFontSize mFontSize;
    GLUFFontSize mLeading;

public:
    GLUF::Color mColor;
    GLUF::GLUFPoint mPoint;


	~GLUFTextHelper(){};

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
	void Begin(GLUFFontIndex drmFont, GLUFFontSize leading, GLUFFontSize size);

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
    void DrawFormattedTextLine(const std::wstring& format, Types&... args) noexcept;

    template<class... Types>
    void DrawFormattedTextLine(const GLUF::GLUFRect& rc, GLUFBitfield flags, const std::wstring& format, Types&... args) noexcept;

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
	void DrawTextLine(const GLUF::GLUFRect& rc, GLUFBitfield flags, const std::wstring& text) noexcept;

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
    static void RenderText(const std::wstring& format, std::wstring& outString, Types&... args) noexcept;
};


}

/*

The Implementation for all template functions

*/
#include "GLUFGuiTemplates.inl"

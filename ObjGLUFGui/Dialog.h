#ifndef OBJGLUF_DIALOG_H
#define OBJGLUF_DIALOG_H

namespace GLUF {

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
class Dialog : public std::enable_shared_from_this<Dialog> {
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

    std::map <ControlIndex, ControlPtr> mControls;
    std::vector <ElementHolderPtr> mDefaultElements;

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
    friend std::shared_ptr <Dialog> CreateDialog_();

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
    void Init(DialogResourceManagerPtr &manager, bool registerDialog = true);
    void Init(DialogResourceManagerPtr &manager, bool registerDialog, TextureIndexResMan textureIndex);

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
    void AddStatic(ControlIndex ID, const std::wstring &strText, const Rect &region, Bitfield textFlags = GT_LEFT |
                                                                                                          GT_TOP, bool isDefault = false, std::shared_ptr <StaticPtr> ctrlPtr = nullptr);
    void AddButton(ControlIndex ID, const std::wstring &strText, const Rect &region, int hotkey = 0, bool isDefault = false, std::shared_ptr <ButtonPtr> ctrlPtr = nullptr);
    void AddCheckBox(ControlIndex ID, const std::wstring &strText, const Rect &region, bool checked = false, int hotkey = 0, bool isDefault = false, std::shared_ptr <CheckBoxPtr> ctrlPtr = nullptr);
    void AddRadioButton(ControlIndex ID, RadioButtonGroup buttonGroup, const std::wstring &strText, const Rect &region, bool checked = false, int hotkey = 0, bool isDefault = false, std::shared_ptr <RadioButtonPtr> ctrlPtr = nullptr);
    void AddComboBox(ControlIndex ID, const Rect &region, int hotKey = 0, bool isDefault = false, std::shared_ptr <ComboBoxPtr> ctrlPtr = nullptr);
    void AddSlider(ControlIndex ID, const Rect &region, long min, long max, long value, bool isDefault = false, std::shared_ptr <SliderPtr> ctrlPtr = nullptr);
    void AddEditBox(ControlIndex ID, const std::wstring &strText, const Rect &region, Charset charset = Unicode, GLbitfield textFlags =
    GT_LEFT | GT_TOP, bool isDefault = false, std::shared_ptr <EditBoxPtr> ctrlPtr = nullptr);
    void AddListBox(ControlIndex ID, const Rect &region, Bitfield style = 0, std::shared_ptr <ListBoxPtr> ctrlPtr = nullptr);

    /*
    AddControl

        Note:
            This is the generic function for adding custom or preconstructed controls

        Parameters:
            'control': a pointer to the control to add

        Throws:
            'ControlCreationException': if control initialization failed

    */
    void AddControl(ControlPtr control);

    /*
    InitControl

        Note:
            Initializes a control with the default element, and calls 'OnInit' on it

        Throws:
            'ControlCreationException': if control initialization failed

    */
    void InitControl(ControlPtr control);


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
    std::shared_ptr <T> GetControl(ControlIndex ID) const;
    ControlPtr GetControl(ControlIndex ID, ControlType controlType) const;


    /*
    GetControlAtPoint

        Parameters:
            'pt': the point in screen coordinates to look for the control

        Returns:
            a pointer to the control found, returns nullptr if none found

        Throws:
            no-throw guarantee

    */
    ControlPtr GetControlAtPoint(const Point &pt) const noexcept;

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
    std::vector <RadioButtonPtr> GetRadioButtonGroup(RadioButtonGroup groupId);

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
    void SetDefaultElement(ControlType controlType, ElementIndex elementIndex, const Element &element);
    Element GetDefaultElement(ControlType controlType, ElementIndex elementIndex) const;


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
    void RequestFocus(ControlPtr control);

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
    void DrawRect(const Rect &rect, const Color &color, bool transform = true);
    //void DrawPolyLine(Point* apPoints, uint32_t nNumPoints, Color color);
    void DrawSprite(const Element &element, const Rect &rect, float depth, bool textured = true);
    void DrawText(const std::wstring &text, const Element &element, const Rect &rect, bool shadow = false, bool hardRect = false);

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
    void CalcTextRect(const std::wstring &text, const Element &element, Rect &rect) const;


    /*
    Attribute Setters and Getters

        Note:
            For Documentation of what each of these do, see documentation for 'Data Members'

        Throws:
            no-throw guarantee

    */
    bool GetVisible() const noexcept { return mVisible; }

    bool GetMinimized() const noexcept { return mMinimized; }

    long GetCaptionHeight() const noexcept { return mCaptionHeight; }

    Point GetLocation() const noexcept { return {mRegion.bottom, mRegion.left}; }

    Rect GetRegion() const noexcept { return mRegion; }

    long GetWidth() const noexcept { return RectWidth(mRegion); }

    long GetHeight() const noexcept { return RectHeight(mRegion); }

    Point GetMousePositionDialogSpace() const noexcept { return mMousePositionDialogSpace; }

    DialogResourceManagerPtr GetManager() const noexcept { return mDialogManager; }

    void SetVisible(bool visible) noexcept { mVisible = visible; }

    void SetMinimized(bool minimized) noexcept { mMinimized = minimized; }

    void SetBackgroundColor(const Color &color) noexcept { mDlgElement.mTextureColor.SetAll(color); }

    void SetCaptionHeight(long height) noexcept { mCaptionHeight = height; }

    void SetCaptionText(const std::wstring &text) noexcept { mCaptionText = text; }

    void SetLocation(long x, long y) noexcept { RepositionRect(mRegion, x, y); }

    void SetSize(long width, long height) noexcept { ResizeRect(mRegion, width, height); }

    static void SetRefreshTime(float time) noexcept { sTimeRefresh = time; }

    void Lock(bool lock = true) noexcept { mLocked = lock; }

    void EnableGrabAnywhere(bool enable = true) noexcept { mGrabAnywhere = enable; }

    void EnableCaption(bool enable) noexcept { mCaptionEnabled = enable; }

    void EnableAutoClamp(bool enable = true) noexcept { mAutoClamp = enable; }

    void EnableNonUserEvents(bool bEnable) noexcept { mNonUserEvents = bEnable; }

    void EnableKeyboardInput(bool bEnable) noexcept { mKeyboardInput = bEnable; }

    void EnableMouseInput(bool bEnable) noexcept { mMouseInput = bEnable; }

    bool IsKeyboardInputEnabled() const noexcept { return mKeyboardInput; }

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
    void SetFont(FontIndex fontIndex, FontIndexResMan resManFontIndex);
    void SetTexture(TextureIndex texIndex, TextureIndexResMan resManTexIndex);
    FontNodePtr GetFont(FontIndex index) const;
    TextureNodePtr GetTexture(TextureIndex index) const;


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
    void Refresh() noexcept;

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
    void ScreenSpaceToGLSpace(Rect &rc) noexcept;
    void ScreenSpaceToGLSpace(Point &pt) noexcept;


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
    void OnMouseMove(const Point &pt) noexcept;
    void OnMouseUp(const Point &pt) noexcept;

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

}
#include "Dialog.inl"

#endif //OBJGLUF_DIALOG_H

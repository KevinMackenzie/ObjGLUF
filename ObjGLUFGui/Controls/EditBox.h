#ifndef OBJGLUF_EDITBOX_H
#define OBJGLUF_EDITBOX_H

namespace GLUF {

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
class EditBox : public Control {
protected:

    EditBox() = delete;
    EditBox(Dialog &dialog, bool isMultiline);
    friend std::shared_ptr <EditBox> CreateEditBox(Dialog &dialog, bool isMultiline);

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
    std::vector <Rect> mCharacterBBs;
    std::vector <Rect> mCharacterRects;
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
    Value PointToCharPos(const Point &pt) noexcept;
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
    using UpdateRectsPtr = void (EditBox::*)() noexcept;
    using UpdateCharRectsPtr = void (EditBox::*)() noexcept;
    using RenderPtr = void (EditBox::*)(float) noexcept;
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

    std::wstring GetText() const noexcept { return mTextHistoryKeeper.Top(); }

    Charset GetCharset() const noexcept { return mCharset; }

    Rect GetTextRegion() const noexcept { return mTextRegion; }

    double GetBlinkPeriod() const noexcept { return mBlinkPeriod; }

    bool IsCaretHidden() const noexcept { return mHideCaret; }

    Value GetCaretPos() const noexcept { return mCaretPos; }

    bool GetInsertMode() const noexcept { return mInsertMode; }

    Value GetSelectionStart() const noexcept { return mSelStart; }

    Size GetHorizontalMargin() const noexcept { return mHorizontalMargin; }

    Size GetVerticalMargin() const noexcept { return mVerticalMargin; }

    BlendColor &GetSelectedTextBlendColor() noexcept { return mSelTextColor; }

    BlendColor &GetSelectedBackgroundBlendColor() noexcept { return mSelBkColor; }

    BlendColor &GetCaretBlendColor() noexcept { return mCaretColor; }

    BlendColor &GetTextBlendColor() noexcept { return mElements[0].mFontColor; }

    std::wstring GetSelectedText() noexcept;

    BlendColor GetSelectedTextCBlendColor() const noexcept { return mSelTextColor; }

    BlendColor GetSelectedBackgroundCBlendColor() const noexcept { return mSelBkColor; }

    BlendColor GetCaretCBlendColor() const noexcept { return mCaretColor; }

    BlendColor GetTextCBlendColor() const noexcept { return mElements.at(0).mFontColor; }


    void SetText(const std::wstring &text); //may throw 'StringContainsInvalidCharacters'
    void SetCharset(Charset chSet) noexcept;//this will automatically remove all characters not in this charset from the string
    void SetBlinkPeriod(double period) noexcept;
    void SetCaretState(bool state) noexcept;
    void SetCaretPosition(Value pos) noexcept;
    void SetInsertMode(bool insertMode) noexcept;
    void SetSelectionStart(Value pos) noexcept;
    void SetSelectionEmpty() noexcept;
    void SetVerticalMargin(Size marg) noexcept;
    void SetHorizontalMargin(Size marg) noexcept;
    void SetSelectedTextBlendColor(const BlendColor &col) noexcept;
    void SetSelectedBackgroundBlendColor(const BlendColor &col) noexcept;
    void SetCaretBlendColor(const BlendColor &col) noexcept;
    void SetTextBlendColor(const BlendColor &col) noexcept;

#pragma endregion


    /*

    Text Modification Methods

    */
    void InsertString(const std::wstring &str, Value pos) noexcept;
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
}
#endif //OBJGLUF_EDITBOX_H

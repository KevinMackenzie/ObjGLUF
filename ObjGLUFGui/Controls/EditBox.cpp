#include "EditBox.h"

//--------------------------------------------------------------------------------------
EditBox::EditBox(Dialog& dialog, bool isMultiline) : Control(dialog), mTextDataBuffer(std::make_shared<VertexArray>(GL_TRIANGLES, GL_STREAM_DRAW, true)), mMultiline(isMultiline), mTextHistoryKeeper(L"")
{
    mType = CONTROL_EDITBOX;

    mScrollBar = CreateScrollBar(dialog);

    mDialog.InitControl(std::dynamic_pointer_cast<Control>(mScrollBar));

    mTextDataBuffer->AddVertexAttrib({ 4, 3, g_TextShaderLocations.position, GL_FLOAT, 0 });
    mTextDataBuffer->AddVertexAttrib({ 4, 2, g_TextShaderLocations.uv, GL_FLOAT, 12 });

    mCaretColor.SetAll({ 0, 0, 0, 255 });
    mCaretColor.SetState(STATE_DISABLED, { 0, 0, 0, 0 });
    mCaretColor.SetState(STATE_HIDDEN, { 0, 0, 0, 0 });

    mSelBkColor.SetAll({ 0, 0, 200, 128 });
    mSelBkColor.SetState(STATE_HIDDEN, { 0, 0, 0, 0 });

    mSelTextColor.SetAll({ 0, 0, 0, 255 });
    mSelTextColor.SetState(STATE_HIDDEN, { 0, 0, 0, 0 });

    if (isMultiline)
    {
        mUpdateRectsFunction = &EditBox::UpdateRectsMultiline;
        mUpdateCharRectsFunction = &EditBox::UpdateCharRectsMultiline;
        mRenderFunction = &EditBox::RenderMultiline;
    }
    else
    {
        mUpdateRectsFunction = &EditBox::UpdateRectsSingleline;
        mUpdateCharRectsFunction = &EditBox::UpdateCharRectsSingleline;
        mRenderFunction = &EditBox::RenderSingleline;
    }
}

//--------------------------------------------------------------------------------------
EditBox::~EditBox()
{
}

//--------------------------------------------------------------------------------------
void EditBox::InvalidateRects() noexcept
{
    mUpdateRequired = true;
}

#pragma region Setters and Getters

//--------------------------------------------------------------------------------------
std::wstring EditBox::GetSelectedText() noexcept
{
    if (mSelStart == -2)
        return L"";

    std::wstring text = GetText();
    std::wstring::iterator begin, end;
    long len = 0;
    if (mSelStart < mCaretPos)
    {
        begin = std::begin(text) + (mSelStart + 1);
        end = std::begin(text) + (mCaretPos + 1);
        len = mCaretPos + 1 - (mSelStart + 1);
    }
    else
    {
        begin = std::begin(text) + (mCaretPos + 1);
        end = std::begin(text) + (mSelStart + 1);
        len = mSelStart + 1 - (mCaretPos + 1);
    }

    std::wstring ret(len, L' ');
    std::copy(begin, end, ret.begin());

    return ret;
}

//--------------------------------------------------------------------------------------
void EditBox::SetText(const std::wstring& text)
{
    if (!CharsetContains(text, mCharset))
        GLUF_CRITICAL_EXCEPTION(StringContainsInvalidCharacters());

    mTextHistoryKeeper.SetText(text);
    InvalidateRects();
}

//--------------------------------------------------------------------------------------
void EditBox::SetCharset(Charset chSet) noexcept
{
    mCharset = chSet;
    std::wstring text = GetText();

    //remove all characters from the string that are not in 'chSet'
    for (auto it = text.begin(); it != text.end(); ++it)
    {
        if (!CharsetContains(*it, chSet))
            text.erase(it);
    }

    mTextHistoryKeeper.SetText(text);
}

//--------------------------------------------------------------------------------------
void EditBox::SetBlinkPeriod(double period) noexcept
{
    mBlinkPeriod = period;
}

//--------------------------------------------------------------------------------------
void EditBox::SetCaretState(bool state) noexcept
{
    mHideCaret = !state;
}

//--------------------------------------------------------------------------------------
void EditBox::SetCaretPosition(Value pos) noexcept
{
    if (mIsEmpty)
        mCaretPos = -1;

    mCaretPos = glm::clamp(pos, -1, (int32_t)GetText().length());
}

//--------------------------------------------------------------------------------------
void EditBox::SetInsertMode(bool insertMode) noexcept
{
    mInsertMode = insertMode;
}

//--------------------------------------------------------------------------------------
void EditBox::SetSelectionStart(Value pos) noexcept
{
    mSelStart = pos;
}

//--------------------------------------------------------------------------------------
void EditBox::SetSelectionEmpty() noexcept
{
    SetSelectionStart(-2);
    SetCaretPosition(-1);
}

//--------------------------------------------------------------------------------------
void EditBox::SetHorizontalMargin(Size marg) noexcept
{
    mHorizontalMargin = marg;

    InvalidateRects();
}

//--------------------------------------------------------------------------------------
void EditBox::SetVerticalMargin(Size marg) noexcept
{
    mVerticalMargin = marg;

    InvalidateRects();
}

//--------------------------------------------------------------------------------------
void EditBox::SetSelectedTextBlendColor(const BlendColor& col) noexcept
{
    mSelTextColor = col;
}

//--------------------------------------------------------------------------------------
void EditBox::SetSelectedBackgroundBlendColor(const BlendColor& col) noexcept
{
    mSelBkColor = col;
}

//--------------------------------------------------------------------------------------
void EditBox::SetCaretBlendColor(const BlendColor& col) noexcept
{
    mCaretColor = col;
}

//--------------------------------------------------------------------------------------
void EditBox::SetTextBlendColor(const BlendColor& col) noexcept
{
    mElements[0].mFontColor = col;
}


#pragma endregion

//--------------------------------------------------------------------------------------
void EditBox::InsertString(const std::wstring& str, Value pos) noexcept
{

    //start by removing ALL newlines
    std::wstring newStr = str;
    {
        std::wstring::iterator it = std::find(newStr.begin(), newStr.end(), '\n');

        while (it != newStr.end())
        {
            newStr.erase(it);
            it = std::find(newStr.begin(), newStr.end(), '\n');
        }
    }

    pos = glm::clamp(pos, -1, static_cast<Value>(GetText().size()));

    //since a space is used as a placeholder when there is no text, make sure to clear when we get text
    if (mIsEmpty)
    {
        mTextHistoryKeeper.PushPartialRemoval(0, 1);
        mIsEmpty = false;
    }
    mTextHistoryKeeper.PushPartialAddition(newStr, pos);

    if (pos <= mCaretPos)
        mCaretPos += newStr.size();

    mSelStart = -2;

    InvalidateRects();
}

//--------------------------------------------------------------------------------------
void EditBox::InsertChar(wchar_t ch, Value pos) noexcept
{
    pos = glm::clamp(pos, -1, static_cast<Value>(GetText().size()));

    //since a space is used as a placeholder when there is no text, make sure to clear when we get text
    if (mIsEmpty)
    {
        mTextHistoryKeeper.PushPartialRemoval(0, 1);
        mIsEmpty = false;
    }
    mTextHistoryKeeper.PushPartialAddition(std::wstring() + ch, pos + 1);

    if (pos <= mCaretPos)
        ++mCaretPos;

    mSelStart = -2;

    InvalidateRects();
}

void EditBox::DeleteChar(Value pos) noexcept
{
    if (pos < 0 || pos >= GetText().size())
        return;

    if (GetText().size() == 1)
    {
        if (!mIsEmpty)
        {
            mTextHistoryKeeper.PushPartialRemoval(0, 1);
            mTextHistoryKeeper.PushPartialAddition(L" ", 0);
            mIsEmpty = true;
        }
    }
    else
    {
        mTextHistoryKeeper.PushPartialRemoval(pos, pos + 1);
    }

    InvalidateRects();
}

//--------------------------------------------------------------------------------------
bool EditBox::MsgProc(MessageType msg, int32_t param1, int32_t param2, int32_t param3, int32_t param4) noexcept
{
    auto mousePos = mDialog.GetMousePositionDialogSpace();

    if (mScrollBar->MsgProc(_PASS_CALLBACK_PARAM)/* || mScrollBar->ContainsPoint(mousePos)*/)
    {
        InvalidateRects();
        return true;
    }

    switch (msg)
    {
        case MB:
        {
            if (param1 == GLFW_MOUSE_BUTTON_LEFT)
            {
                if (param2 == GLFW_PRESS)
                {
                    if (PtInRect(mRegion, mousePos))
                    {
                        if (!mHasFocus)
                            mDialog.RequestFocus(shared_from_this());

                        mSelStart = -2;
                        SetCaretPosition(PointToCharPos(mousePos));
                        mMouseDrag = true;
                    }
                }
                else if (param2 == GLFW_RELEASE)
                {
                    //is the scroll bar pressed
                    /*if (mScrollBar->IsPressed())
                    {
                        break;
                    }*/

                    //if they occupy the same index, no sense in rendering a blank selection region
                    if (mSelStart == mCaretPos)
                        mSelStart = -2;

                    mMouseDrag = false;
                }
            }

            break;
        }
        case CURSOR_POS:
        {
            //if there is a selection, make sure to update which parts are selected
            if (mMouseDrag)
            {
                if (mSelStart == -2)
                {
                    mSelStart = PointToCharPos(mousePos);
                    SetCaretPosition(mSelStart);
                }
                else
                {
                    SetCaretPosition(PointToCharPos(mousePos));
                }

            }
            break;
        }
        case SCROLL:
        {
            mScrollBar->Scroll(-(param2 / _WHEEL_DELTA));
            InvalidateRects();
            MsgProc(MessageType::CURSOR_POS, 0, 0, 0, 0);//this doesn't 100% work
            /*if (mMultiline)
            {
                mScrollBar->Scroll(-(param2 / WHEEL_DELTA));
            }
            else
            {
                mScrollBar->Scroll(param2 > 0 ? 1 : -1);
            }*/
            break;
        }
        case UNICODE_CHAR:
        {

            //is there a selection?
            if (mSelStart != -2)
            {
                //if so, delete it
                RemoveSelectedRegion();
            }

            InsertChar(static_cast<wchar_t>(param1), mCaretPos);

            break;
        }
        case KEY:
        {
            switch(param1)
            {
                case GLFW_KEY_BACKSPACE:
                    if (param3 != GLFW_RELEASE)
                    {
                        if (mSelStart != -2)
                        {
                            RemoveSelectedRegion();
                        }
                        else
                        {
                            DeleteChar(mCaretPos);
                            SetCaretPosition(mCaretPos - 1);
                        }
                    }

                    break;
                case GLFW_KEY_DELETE:
                    if (param3 != GLFW_RELEASE)
                    {
                        if (mSelStart != -2)
                        {
                            RemoveSelectedRegion();
                        }
                        else
                        {
                            DeleteChar(mCaretPos + 1);
                        }
                    }

                    break;
                case GLFW_KEY_ENTER:
                    if (param3 != GLFW_RELEASE)
                    {
                        if (mSelStart != -2)
                        {
                            RemoveSelectedRegion();
                        }

                        InsertChar(L'\n', mCaretPos);

                    }
                    break;
                case GLFW_KEY_C:
                    if (param4 == GLFW_MOD_CONTROL && param3 == GLFW_PRESS)
                    {
                        //get selected text
                        auto selText = GetSelectedText();

                        glfwSetClipboardString(g_pGLFWWindow, WStringToString(selText).c_str());
                    }
                    break;
                case GLFW_KEY_V:
                    if (param4 == GLFW_MOD_CONTROL && param3 != GLFW_RELEASE)
                    {
                        //insert clipboard
                        std::string clipboardString = glfwGetClipboardString(g_pGLFWWindow);

                        InsertString(StringToWString(clipboardString), mCaretPos + 1);
                    }
                    break;
                case GLFW_KEY_X:
                    if (param4 == GLFW_MOD_CONTROL && param3 == GLFW_PRESS)
                    {
                        auto selText = GetSelectedText();

                        glfwSetClipboardString(g_pGLFWWindow, WStringToString(selText).c_str());

                        RemoveSelectedRegion();
                    }
                    break;
                case GLFW_KEY_A:
                    if (param4 == GLFW_MOD_CONTROL && param3 == GLFW_PRESS)
                    {
                        mSelStart = -1;
                        mCaretPos = GetText().size() - 1;
                    }
                    break;
                case GLFW_KEY_Z:
                    if (param4 == GLFW_MOD_CONTROL && param3 != GLFW_RELEASE)
                    {
                        mTextHistoryKeeper.UndoNextItem();
                        InvalidateRects();
                    }
                    break;
                case GLFW_KEY_Y:
                    if (param4 == GLFW_MOD_CONTROL && param3 != GLFW_RELEASE)
                    {
                        mTextHistoryKeeper.RedoNextItem();
                        InvalidateRects();
                    }
                    break;
            }
            break;
        }
        case FRAMEBUFFER_SIZE:
            //if the window resizes, we must reset our text
            InvalidateRects();
            break;
        case FOCUS:
            if (param1 == GL_TRUE)
            {
                SetSelectionEmpty();
            }
            break;
        default:
            break;
    }

    ApplyCompositeModifications();

    return false;
}

//--------------------------------------------------------------------------------------
void EditBox::UpdateRectsMultiline() noexcept
{

    mScrollBar->SetRegion({ { mTextRegion.right - static_cast<long>(mSBWidth) }, mSubRegions[0].top, mSubRegions[0].right, { mSubRegions[0].bottom } });
    mScrollBar->SetPageSize(static_cast<int>(RectHeight(mTextRegion) / mDialog.GetFont(mElements[0].mFontIndex)->mLeading));

    mTextRegion.right -= static_cast<long>(2 * mSBWidth);

    //TODO: finish setting up the scroll bar page size/everything else for the scroll bar update
    mScrollBar->UpdateRects();
}

//--------------------------------------------------------------------------------------
void EditBox::UpdateRectsSingleline() noexcept
{
}

//--------------------------------------------------------------------------------------
void EditBox::UpdateRects() noexcept
{
    if (GetText().size() == 0)
    {
        mIsEmpty = true;
    }

    Control::UpdateRects();

    mTextRegion = mRegion;
    InflateRect(mTextRegion, -static_cast<int32_t>(2 * mHorizontalMargin), -static_cast<int32_t>(2 * mVerticalMargin));

    mSubRegions[0] = mRegion;
    InflateRect(mSubRegions[0], -14, -14);
    SetRect(mSubRegions[1], mRegion.left, mRegion.top, mSubRegions[0].left, mSubRegions[0].top);
    SetRect(mSubRegions[2], mSubRegions[0].left, mRegion.top, mSubRegions[0].right, mSubRegions[0].top);
    SetRect(mSubRegions[3], mSubRegions[0].right, mRegion.top, mRegion.right, mSubRegions[0].top);
    SetRect(mSubRegions[4], mRegion.left, mSubRegions[0].top, mSubRegions[0].left, mSubRegions[0].bottom);
    SetRect(mSubRegions[5], mSubRegions[0].right, mSubRegions[0].top, mRegion.right, mSubRegions[0].bottom);
    SetRect(mSubRegions[6], mRegion.left, mSubRegions[0].bottom, mSubRegions[0].left, mRegion.bottom);
    SetRect(mSubRegions[7], mSubRegions[0].left, mSubRegions[0].bottom, mSubRegions[0].right, mRegion.bottom);
    SetRect(mSubRegions[8], mSubRegions[0].right, mSubRegions[0].bottom, mRegion.right, mRegion.bottom);

    (this->*mUpdateRectsFunction)();

    UpdateCharRects();

    mUpdateRequired = false;
}

//--------------------------------------------------------------------------------------
void EditBox::RenderMultiline(float elapsedTime) noexcept
{
    //debug rendering
    /*unsigned int color = 0x80000000;
    for (auto it : mCharacterBBs)
    {
        mDialog.DrawRect(it, *reinterpret_cast<Color*>(&color), false);

        color += 0x0000000F;
    }*/

    mScrollBar->Render(elapsedTime);
}

//--------------------------------------------------------------------------------------
void EditBox::RenderSingleline(float elapsedTime) noexcept
{
}

//--------------------------------------------------------------------------------------
void EditBox::Render(float elapsedTime) noexcept
{
    if (mUpdateRequired)
        UpdateRects();

    auto state = STATE_NORMAL;
    if (!mVisible)
        state = STATE_HIDDEN;
    else if (!mEnabled)
        state = STATE_DISABLED;
    else if (mMouseOver)
        state = STATE_MOUSEOVER;
    else if (mHasFocus)
        state = STATE_FOCUS;

    unsigned int i = 0;
    for (auto& it : mElements)
    {
        it.second.mTextureColor.Blend(state, elapsedTime);

        if (i == 0)
        {
            it.second.mFontColor.Blend(state, elapsedTime);
        }

        mDialog.DrawSprite(it.second, mSubRegions[i], _NEAR_BUTTON_DEPTH);

        ++i;
    }

    mSelBkColor.Blend(state, elapsedTime);
    mSelTextColor.Blend(state, elapsedTime);
    mCaretColor.Blend(state, elapsedTime);

    //draw the text
    RenderText(elapsedTime);

    //draw the selected region
    if (mSelStart != -2 && mCharacterBBs.size() > 0 && mSelStart != mCaretPos)
    {

        Value begin = 0;
        Value end = 0;
        if (mCaretPos > mSelStart)
        {
            begin = mSelStart;
            end = mCaretPos;
        }
        else if (mCaretPos < mSelStart)
        {
            begin = mCaretPos;
            end = mSelStart;
        }

        begin++;

        begin = glm::clamp(begin, static_cast<Value>(mRenderOffset), static_cast<Value>(mRenderOffset + mRenderCount));
        end = glm::clamp(end, static_cast<Value>(mRenderOffset), static_cast<Value>(mRenderOffset + mRenderCount));

        //if caret position and selection start are equal, it won't render anything

        for (int32_t i = begin; i <= end; ++i)
        {
            mDialog.DrawRect(mCharacterBBs[i], mSelBkColor.GetCurrent(), false);
        }
    }

    //draw the caret
    if (!mHideCaret)
    {

        if (mCaretOn && ShouldRenderCaret())
        {
            //get the caret rect
            Rect caretRect = CharPosToRect(mCaretPos);
            mDialog.DrawRect(caretRect, mCaretColor.GetCurrent(), false);
        }

        if (GetTime() - mPreviousBlinkTime >= mBlinkPeriod)
        {
            bFlip(mCaretOn);
            mPreviousBlinkTime = GetTime();
        }
    }

    (this->*mRenderFunction)(elapsedTime);
}

//--------------------------------------------------------------------------------------
void EditBox::RenderText(float elapsedTime) noexcept
{
    auto& element = mElements[0];

    SHADERMANAGER.UseProgram(g_TextProgram);

    //first uniform: model-view matrix
    SHADERMANAGER.GLUniformMatrix4f(g_TextShaderLocations.ortho, Text::g_TextOrtho);

    //second, the sampler
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, mDialog.GetFont(element.mFontIndex)->mFontType->mTexId);
    SHADERMANAGER.GLUniform1i(g_TextShaderLocations.sampler, 0);


    //third, the color
    SHADERMANAGER.GLUniform4f(g_TextShaderLocations.color, ColorToFloat(element.mFontColor.GetCurrent()));

    //make sure to enable this with text
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    if (mSelStart == -2 || mSelStart == mCaretPos)
    {
        mTextDataBuffer->DrawRange(mRenderOffset * 6, mRenderCount * 6);
    }
    else
    {
        //get the begin and end of the selected text region
        GLuint begin = 6;
        GLuint end = 6;

        if (mSelStart > mCaretPos)
        {
            begin *= mCaretPos;
            end *= mSelStart;
        }
        else if (mSelStart < mCaretPos)
        {
            begin *= mSelStart;
            end *= mCaretPos;
        }

        //account for the one character offset to the left of the 'beginning' of the selection
        begin += 6;

        //get the values for the start and end of the preselection, the selection, and the postselection regions

        GLuint rndOffsetIndex = mRenderOffset * 6;
        GLuint rndEndIndex = rndOffsetIndex + mRenderCount * 6;

        GLuint selectionStartIndex = glm::clamp(begin, rndOffsetIndex, rndEndIndex);
        GLuint selectionSize = glm::clamp(end, rndOffsetIndex, rndEndIndex) - selectionStartIndex;

        GLuint preSelectionStartIndex = rndOffsetIndex;
        GLuint preSelectionSize = selectionStartIndex - preSelectionStartIndex;

        GLuint postSelectionStartIndex = glm::clamp(selectionStartIndex + selectionSize, rndOffsetIndex, rndEndIndex);
        GLuint postSelectionSize = rndEndIndex - postSelectionStartIndex;


        /*

        The Pre-selection region

        */
        //SHADERMANAGER.GLUniform4f(g_TextShaderLocations.color, ColorToFloat(elemenet.mFontColor.GetCurrent()));
        mTextDataBuffer->DrawRange(preSelectionStartIndex, preSelectionSize);

        /*

        The Selection Region

        */
        SHADERMANAGER.GLUniform4f(g_TextShaderLocations.color, ColorToFloat(mSelTextColor.GetCurrent()));
        mTextDataBuffer->DrawRange(selectionStartIndex, selectionSize);

        /*

        The Post-Selection Region

        */
        SHADERMANAGER.GLUniform4f(g_TextShaderLocations.color, ColorToFloat(element.mFontColor.GetCurrent()));
        mTextDataBuffer->DrawRange(postSelectionStartIndex, postSelectionSize);
    }
}

//--------------------------------------------------------------------------------------
void EditBox::RemoveSelectedRegion() noexcept
{
    if (mSelStart != -2)
    {
        if (mSelStart > mCaretPos)
            mTextHistoryKeeper.PushPartialRemoval(mCaretPos + 1, mSelStart + 1);
        else
            mTextHistoryKeeper.PushPartialRemoval(mSelStart + 1, mCaretPos + 1);

        if (mSelStart == -1 && mCaretPos == GetText().size() - 1)
            mTextHistoryKeeper.PushPartialAddition(L" ", 0);

        mCaretPos = glm::min(mCaretPos, mSelStart);
        mSelStart = -2;
    }

    InvalidateRects();
}

//--------------------------------------------------------------------------------------
Value EditBox::PointToCharPos(const Point& pt) noexcept
{
    //convert the point into the right space
    Point newPt = pt;
    mDialog.ScreenSpaceToGLSpace(newPt);

    //first see if it intersects a character
    for (unsigned int i = 0; i < mCharacterBBs.size(); ++i)
    {
        auto thisRect = mCharacterBBs[i];
        if (PtInRect(thisRect, newPt))
        {
            //see which side of the rect it is on
            if (newPt.x > thisRect.left + RectWidth(thisRect) / 2 || !mInsertMode)
            {
                //left side:
                return i;
            }
            else
            {
                //right side:
                return i - 1;
            }
        }
    }

    //if it didn't hit a character, get the nearest character (TODO:)

    //which line is the cursor on?
    {
        bool foundLine = false;
        bool onCurrentLine = false;
        Rect regionRndSpace = mRegion;
        mDialog.ScreenSpaceToGLSpace(regionRndSpace);
        Rect textRegionRndSpace = mTextRegion;
        mDialog.ScreenSpaceToGLSpace(textRegionRndSpace);
        for (int i = 0; i < mCharacterBBs.size(); ++i)
        {
            if (!onCurrentLine && foundLine)
                return i - 2;

            auto rightOf = mCharacterBBs[i];
            auto leftOf = mCharacterBBs[i];

            leftOf.left = regionRndSpace.left;
            leftOf.right = textRegionRndSpace.left;

            //use the left as the left of the text region, because if the cursor is to the left of the text region, we want the FIRST character on the line, not the last
            rightOf.left = textRegionRndSpace.left;
            rightOf.right = regionRndSpace.right;
            if (PtInRect(rightOf, newPt))
            {
                foundLine = true;
                onCurrentLine = true;
            }
            else if (PtInRect(leftOf, newPt))
            {
                //this can only possibly occur the first time on the line
                return i;
            }
            else
            {
                //the first character on the next line is hit, so activate that flag
                onCurrentLine = false;
            }
        }
    }

    return -1;
}

//--------------------------------------------------------------------------------------
Value EditBox::RenderTextToText(Value rndIndex)
{
    return -1;
}

//--------------------------------------------------------------------------------------
Rect EditBox::CharPosToRect(Value charPos) noexcept
{
    charPos = glm::clamp(charPos, -1, static_cast<Value>(mCharacterBBs.size() - 1));

    //if it is the first position
    if (charPos == -1)
    {
        auto firstRect = mCharacterBBs[0];

        if (mInsertMode)
        {
            firstRect.right = firstRect.left;
            firstRect.left -= mCaretSize;
        }

        return firstRect;
    }

    auto thisRect = mCharacterBBs[charPos];

    if (mInsertMode)
    {
        thisRect.right = mCharacterRects[charPos].right + mCaretSize;
        thisRect.left = thisRect.right - mCaretSize;
    }

    return thisRect;
}

//--------------------------------------------------------------------------------------
bool EditBox::ShouldRenderCaret() noexcept
{
    auto caretPosRect = CharPosToRect(mCaretPos);
    auto textRenderRect = mTextRegion;
    mDialog.ScreenSpaceToGLSpace(textRenderRect);


    //is the caret within the visible window
    if (PtInRect(textRenderRect, { caretPosRect.x, caretPosRect.y }) ||
        PtInRect(textRenderRect, { caretPosRect.right, caretPosRect.y }))
        return true;


    return false;
}

void EditBox::ApplyCompositeModifications() noexcept
{
    mTextHistoryKeeper.ApplyPartialModifications();
}

//--------------------------------------------------------------------------------------
Value EditBox::TextToRenderText(Value txtIndex)
{
    return -1;
}

//--------------------------------------------------------------------------------------
void EditBox::OnFocusIn() noexcept
{
    Control::OnFocusIn();
    mScrollBar->OnFocusIn();

    MsgProc(FOCUS, GL_TRUE, 0, 0, 0);
}

//--------------------------------------------------------------------------------------
void EditBox::OnFocusOut() noexcept
{
    Control::OnFocusOut();
    mScrollBar->OnFocusOut();

    MsgProc(FOCUS, GL_FALSE, 0, 0, 0);
}

//--------------------------------------------------------------------------------------
void EditBox::OnMouseEnter() noexcept
{
    Control::OnMouseEnter();
    mScrollBar->OnMouseEnter();
}

//--------------------------------------------------------------------------------------
void EditBox::OnMouseLeave() noexcept
{
    Control::OnMouseLeave();
    mScrollBar->OnMouseLeave();
}

//--------------------------------------------------------------------------------------
void EditBox::OnInit()
{
    Control::OnInit();

    mScrollBar->OnInit();
}

//--------------------------------------------------------------------------------------
void EditBox::UpdateCharRects() noexcept
{
    if (mIsEmpty)
    {
        //mText = L" ";
    }

    (this->*mUpdateCharRectsFunction)();

    BufferCharRects();
}

//--------------------------------------------------------------------------------------
void EditBox::UpdateCharRectsMultiline() noexcept
{
    std::wstring text = GetText();

    Rect rcScreen = mTextRegion;
    auto dlgRect = mDialog.GetRegion();
    mDialog.ScreenSpaceToGLSpace(rcScreen);

    auto &element = mElements[0];
    auto textFlags = element.mTextFormatFlags;
    auto font = mDialog.GetFont(element.mFontIndex);
    auto fontHeight = font->mFontType->mHeight;
    auto leading = font->mLeading;

    std::vector<std::wstring> textLines = { L"" };
    std::vector<bool> whichLinesCausedByNewlines = { 0 };

    mCharacterRects.resize(text.size());
    mCharacterBBs.resize(text.size());

    {
        auto textRegionWidth = RectWidth(rcScreen);
        int lineWidth = 0;

        /*

        Get Each Line of Text

        */
        for (unsigned int i = 0; i < text.size(); ++i)
        {
            auto thisChar = text[i];
            auto thisCharWidth = 0;

            int potentialNewLineWidth = 0;

            /*

            this bool is a workaround for newlines not being in the character atlas

            */
            bool execNewlineCode = thisChar == '\n';
            if (!execNewlineCode)
            {
                thisCharWidth = font->mFontType->GetCharAdvance(thisChar);
                potentialNewLineWidth = lineWidth + thisCharWidth;
                lineWidth += thisCharWidth;

                execNewlineCode = potentialNewLineWidth > textRegionWidth;
            }

            if (execNewlineCode)
            {
                textLines.push_back(L"");

                lineWidth = 0;

                if (thisChar == '\n')
                {
                    whichLinesCausedByNewlines.push_back(true);
                    continue;
                }
                whichLinesCausedByNewlines.push_back(false);
            }


            textLines[textLines.size() - 1] += thisChar;
        }

        mScrollBar->SetTrackRange(0, textLines.size() - 1);

        /*

        Get the Rects for each line

        */
        unsigned charIndex = 0;
        long currY = rcScreen.top + mScrollBar->GetTrackPos() * leading;
        long lineXOffset = 0;
        for (unsigned int i = 0; i < textLines.size() + 1; ++i)
        {
            //do this at the beginning of the loop to put the newline character in the right place for the caret
            if (i != 0 && i < whichLinesCausedByNewlines.size())
            {
                if (whichLinesCausedByNewlines[i])
                {
                    //make the bounding box invalid so the newline character can never truly be 'clicked on'
                    SetRect(mCharacterRects[charIndex], rcScreen.left, currY, rcScreen.left - 1, currY - leading);
                    mCharacterBBs[charIndex] = mCharacterRects[charIndex];
                    charIndex++;//add one at the end of each line
                }
            }

            //there is ONLY a newline on the next line, OR text is over
            if (i == textLines.size())
                break;

            auto thisLine = textLines[i];
            auto lineWidth = font->mFontType->GetStringWidth(thisLine);
            lineXOffset = rcScreen.left;



            if (textFlags & GT_CENTER)
                lineXOffset += static_cast<long>((static_cast<float>(textRegionWidth) / 2.0f) - (static_cast<float>(lineWidth) / 2.0f));
            else if (textFlags & GT_RIGHT)
                lineXOffset += textRegionWidth - lineWidth;
            //else if(textFlags & GT_LEFT)
            //  lineXOffset = 0;


            //get the rects for this line
            for (unsigned int j = 0; j < thisLine.size(); ++j)
            {
                auto thisCharRect = font->mFontType->GetCharRect(thisLine[j]);
                OffsetRect(thisCharRect, lineXOffset, currY - fontHeight);

                mCharacterRects[charIndex] = thisCharRect;

                long left = j != 0 ? mCharacterBBs[charIndex - 1].right : thisCharRect.left;
                SetRect(mCharacterBBs[charIndex], left, currY, left + font->mFontType->GetCharAdvance(thisLine[j]), currY - leading);

                lineXOffset += font->mFontType->GetCharAdvance(thisLine[j]);
                charIndex++;
            }



            currY -= leading;
        }
    }



    /*

    Get Rendering offset and count based on scroll bar position

    */
    auto sbPos = mScrollBar->GetTrackPos();
    auto lastLine = mScrollBar->GetPageSize() + sbPos;
    mRenderOffset = 0;
    mRenderCount = 0;
    for (auto it = 0; it < textLines.size(); ++it)
    {
        if (it < sbPos)
            mRenderOffset += textLines[it].size();
        else if (it < lastLine)
            mRenderCount += textLines[it].size();
    }
}

//--------------------------------------------------------------------------------------
void EditBox::UpdateCharRectsSingleline() noexcept
{
    Rect rcScreen = mTextRegion;
    auto dlgRect = mDialog.GetRegion();
    mDialog.ScreenSpaceToGLSpace(rcScreen);

    auto &element = mElements[0];
    auto textFlags = element.mTextFormatFlags;
    auto font = mDialog.GetFont(element.mFontIndex);
    auto fontHeight = font->mFontType->mHeight;
    auto leading = font->mLeading;

    std::wstring str = GetText();

    mCharacterRects.resize(str.size());
    mCharacterBBs.resize(str.size());

    //get the vertical alignment
    long yPos = rcScreen.top;
    if (textFlags | GT_VCENTER)
    {
        yPos = rcScreen.top - (RectHeight(rcScreen) - fontHeight) / 2;
    }
    else if (textFlags | GT_BOTTOM)
    {
        yPos = rcScreen.bottom + fontHeight;
    }

    //update the scroll bar
    mScrollBar->SetTrackRange(0, str.size());

    //get the horizontal offset based on the scroll bar position
    long currX = rcScreen.left;
    {
        long offset = 0;
        for (Value i = 0; i < mScrollBar->GetTrackPos(); ++i)
        {
            offset -= font->mFontType->GetCharAdvance(str[i]);
        }

        currX += offset;
    }

    //no horizontal alignment on single-line edit boxes (TODO)

    //get the character rects
    unsigned int i = 0;
    for (auto it : str)
    {
        auto thisCharRect = font->mFontType->GetCharRect(it);
        OffsetRect(thisCharRect, currX, yPos - fontHeight);

        mCharacterRects[i] = thisCharRect;

        long left = i != 0 ? mCharacterBBs[i - 1].right : thisCharRect.left;
        SetRect(mCharacterBBs[i], left, yPos, left + font->mFontType->GetCharAdvance(it), yPos - leading);

        ++i;
        currX += font->mFontType->GetCharAdvance(it);
    }

    //get the rendering offset based on the scroll bar position
    mRenderOffset = mScrollBar->GetTrackPos();
    mRenderCount = 0;
    {
        long tmpWidth = 0;
        long textRectWidth = RectWidth(mTextRegion);
        for (i = mScrollBar->GetTrackPos(); i < str.size(); ++i)
        {
            tmpWidth += font->mFontType->GetCharAdvance(str[i]);
            if (tmpWidth > textRectWidth)
                break;

            ++mRenderCount;
        }
    }
}

//--------------------------------------------------------------------------------------
void EditBox::BufferCharRects() noexcept
{
    auto str = GetText();

    //make sure there are characters to load
    if (str.size() == 0)
        return;

    auto font = mDialog.GetFont(GetElement(0).mFontIndex);
    /*

    Buffer data into OpenGL

    */
    GLVector<TextVertexStruct> textVertices = TextVertexStruct::MakeMany(str.size() * 4);
    std::vector<glm::u32vec3> indices;
    indices.resize(str.size() * 2);

    float z = _NEAR_BUTTON_DEPTH;
    for (unsigned int i = 0; i < str.size(); ++i)
    {
        auto &glyph = mCharacterRects[i];
        auto ch = str[i];


        auto UV = font->mFontType->GetCharTexRect(ch);

        unsigned int vertI = i * 4;

        textVertices[vertI] =
                {
                        glm::vec3(GetVec2FromRect(glyph, false, false), z),
                        GetVec2FromRect(UV, false, false)
                };

        textVertices[vertI + 1] =
                {
                        glm::vec3(GetVec2FromRect(glyph, true, false), z),
                        GetVec2FromRect(UV, true, false)
                };

        textVertices[vertI + 2] =
                {
                        glm::vec3(GetVec2FromRect(glyph, true, true), z),
                        GetVec2FromRect(UV, true, true)
                };

        textVertices[vertI + 3] =
                {
                        glm::vec3(GetVec2FromRect(glyph, false, true), z),
                        GetVec2FromRect(UV, false, true)
                };

        unsigned int indexI = i * 2;
        indices[indexI] =
                {
                        vertI + 3, vertI, vertI + 2
                };

        indices[indexI + 1] =
                {
                        vertI + 2, vertI, vertI + 1
                };
    }

    mTextDataBuffer->BufferData(textVertices);
    mTextDataBuffer->BufferIndices(indices);
}



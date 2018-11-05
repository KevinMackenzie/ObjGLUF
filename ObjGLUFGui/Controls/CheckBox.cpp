#include "CheckBox.h"

//--------------------------------------------------------------------------------------
CheckBox::CheckBox(bool checked, Dialog& dialog) : Button(dialog)
{
    mType = CONTROL_CHECKBOX;

    mChecked = checked;
}

//--------------------------------------------------------------------------------------
bool CheckBox::MsgProc(MessageType msg, int32_t param1, int32_t param2, int32_t param3, int32_t param4) noexcept
{
    if (!mEnabled || !mVisible)
        return false;

    Point mousePos = mDialog.GetMousePositionDialogSpace();

    switch (msg)
    {

    case CURSOR_POS:

        if (mPressed)
        {
            //if the button is pressed and the mouse is moved off, then unpress it
            if (!ContainsPoint(mousePos))
            {
                mPressed = false;

                //ContainsPoint(mousePos);

                if (!mDialog.IsKeyboardInputEnabled())
                    mDialog.ClearFocus();
            }
        }

        break;
    case MB:
    {
        if (param1 == GLFW_MOUSE_BUTTON_LEFT)
        {
            if (param2 == GLFW_PRESS)
            {
                if (ContainsPoint(mousePos))
                {
                    // Pressed while inside the control
                    mPressed = true;
                    //SetCapture(GetHWND());

                    if (!mHasFocus)
                        mDialog.RequestFocus(shared_from_this());

                    return true;

                }
            }
            else if (param2 == GLFW_RELEASE)
            {
                if (mPressed && ContainsPoint(mousePos))
                {
                    mPressed = false;
                    //ReleaseCapture();

                    if (!mDialog.IsKeyboardInputEnabled())
                        mDialog.ClearFocus();

                    // Button click
                    if (ContainsPoint(mousePos))
                        SetCheckedInternal(!mChecked, true);

                    return true;
                }

            }
        }
        break;
    }

    /*case KEY:
    {

        if (param1 = GLFW_KEY_SPACE)
        {
            if (param3 == GLFW_PRESS)
            {
                mPressed = true;
            }
            if (param3 == GLFW_RELEASE)
            {
                mPressed = false;

                SetCheckedInternal(!mChecked, true);
            }

            return true;
        }


        return true;

        break;
    }*/
    };

    return false;
}


//--------------------------------------------------------------------------------------
void CheckBox::SetCheckedInternal(bool checked, bool fromInput)
{
    mChecked = checked;

    mDialog.SendEvent(EVENT_CHECKBOXCHANGED, fromInput, shared_from_this());
}


//--------------------------------------------------------------------------------------
bool CheckBox::ContainsPoint(const Point& pt) const noexcept
{
    return (PtInRect(mRegion, pt) ||
    PtInRect(mButtonRegion, pt) ||
    PtInRect(mTextRegion, pt));
}


//--------------------------------------------------------------------------------------
void CheckBox::UpdateRects() noexcept
{
    Button::UpdateRects();

    mButtonRegion = mRegion;
    mButtonRegion.right = mButtonRegion.left + RectHeight(mButtonRegion);

    mTextRegion = mRegion;
    OffsetRect(mTextRegion, RectWidth(mButtonRegion)/*1.25f*/, 0L);

    //resize the text rect based on the length of the string
    mTextRegion.right = mTextRegion.left + mDialog.GetFont(mElements[0].mFontIndex)->mFontType->GetStringWidth(mText);
}


//--------------------------------------------------------------------------------------
void CheckBox::Render(float elapsedTime) noexcept
{
    ControlState iState = STATE_NORMAL;

    if (mVisible == false)
        iState = STATE_HIDDEN;
    else if (mEnabled == false)
        iState = STATE_DISABLED;
    else if (mPressed)
        iState = STATE_PRESSED;
    else if (mMouseOver)
        iState = STATE_MOUSEOVER;
    else if (mHasFocus)
        iState = STATE_FOCUS;

    Element* pElement = &mElements[0];

    float fBlendRate = 5.0f;

    pElement->mTextureColor.Blend(iState, elapsedTime, fBlendRate);
    pElement->mFontColor.Blend(iState, elapsedTime, fBlendRate);

    mDialog.DrawSprite(*pElement, mButtonRegion, _FAR_BUTTON_DEPTH);
    mDialog.DrawText(mText, *pElement, mTextRegion, false, false);

    if (mChecked)
    {
        pElement = &mElements[1];

        pElement->mTextureColor.Blend(iState, elapsedTime, fBlendRate);
        mDialog.DrawSprite(*pElement, mButtonRegion, _NEAR_BUTTON_DEPTH);
    }
}


//--------------------------------------------------------------------------------------
void CheckBox::OnHotkey() noexcept
{
    if (mDialog.IsKeyboardInputEnabled())
        mDialog.RequestFocus(shared_from_this());
    SetCheckedInternal(!mChecked, true);
}


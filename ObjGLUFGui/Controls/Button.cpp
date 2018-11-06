#include "Button.h"

namespace GLUF {

Button::Button(Dialog &dialog) : Static(GT_CENTER | GT_VCENTER, dialog) {
    mType = CONTROL_BUTTON;

    mPressed = false;
}

//--------------------------------------------------------------------------------------
void Button::OnHotkey() noexcept {
    if (mDialog.IsKeyboardInputEnabled()) {
        mDialog.RequestFocus(shared_from_this());
        mDialog.SendEvent(EVENT_BUTTON_CLICKED, true, shared_from_this());
    }
}

//--------------------------------------------------------------------------------------
bool Button::MsgProc(MessageType msg, int32_t param1, int32_t param2, int32_t param3, int32_t param4) noexcept {
    if (!mEnabled || !mVisible)
        return false;

    Point mousePos = mDialog.GetMousePositionDialogSpace();

    switch (msg) {

        case CURSOR_POS:

            if (mPressed) {
                //if the button is pressed and the mouse is moved off, then unpress it
                if (!ContainsPoint(mousePos)) {
                    mPressed = false;

                    ContainsPoint(mousePos);

                    if (!mDialog.IsKeyboardInputEnabled())
                        mDialog.ClearFocus();
                }
            }

            break;
        case MB:
            if (param1 == GLFW_MOUSE_BUTTON_LEFT) {
                if (param2 == GLFW_PRESS) {
                    if (ContainsPoint(mousePos)) {
                        // Pressed while inside the control
                        mPressed = true;
                        //SetCapture(GetHWND());

                        if (!mHasFocus)
                            mDialog.RequestFocus(shared_from_this());

                        return true;

                    }
                } else if (param2 == GLFW_RELEASE) {
                    if (mPressed) {
                        mPressed = false;

                        if (!mDialog.IsKeyboardInputEnabled())
                            mDialog.ClearFocus();

                        // Button click
                        if (ContainsPoint(mousePos))
                            mDialog.SendEvent(EVENT_BUTTON_CLICKED, true, shared_from_this());

                        return true;
                    }

                }
            }
            break;

        case KEY: {

            if (param1 == mHotkey) {
                if (param3 == GLFW_PRESS) {
                    mPressed = true;
                }
                if (param3 == GLFW_RELEASE) {
                    mPressed = false;

                    mDialog.SendEvent(EVENT_BUTTON_CLICKED, true, shared_from_this());
                }

                return true;
            }

            return true;
        }
    };

    return false;

    return false;
}

//--------------------------------------------------------------------------------------
void Button::Render(float elapsedTime) noexcept {
    int nOffsetX = 0;
    int nOffsetY = 0;

    Point wndSize = mDialog.GetManager()->GetWindowSize();

    ControlState iState = STATE_NORMAL;

    if (mVisible == false) {
        iState = STATE_HIDDEN;
    } else if (mEnabled == false) {
        iState = STATE_DISABLED;
    } else if (mPressed) {
        iState = STATE_PRESSED;

        nOffsetX = 1;
        nOffsetY = 2;
    } else if (mMouseOver) {
        iState = STATE_MOUSEOVER;

        nOffsetX = -1;
        nOffsetY = -2;
    } else if (mHasFocus) {
        iState = STATE_FOCUS;
    }

    float fBlendRate = 5.0f;//(iState == STATE_PRESSED) ? 0.0f : 0.8f;

    Rect rcWindow = mRegion;
    OffsetRect(rcWindow, nOffsetX, nOffsetY);


    // Background fill layer
    Element *pElement = &mElements[0];

    // Blend current color
    pElement->mTextureColor.Blend(iState, elapsedTime, fBlendRate);
    pElement->mFontColor.Blend(iState, elapsedTime, fBlendRate);

    mDialog.DrawSprite(*pElement, rcWindow, _FAR_BUTTON_DEPTH);
    //mDialog.DrawText(m_strText, pElement, rcWindow, false, true);

    // Main button
    pElement = &mElements[1];

    // Blend current color
    pElement->mTextureColor.Blend(iState, elapsedTime, fBlendRate);
    pElement->mFontColor.Blend(iState, elapsedTime, fBlendRate);

    mDialog.DrawSprite(*pElement, rcWindow, _NEAR_BUTTON_DEPTH);
    mDialog.DrawText(mText, *pElement, rcWindow, false, true);
}
}

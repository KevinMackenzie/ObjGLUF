#include "RadioButton.h"

namespace GLUF {

//--------------------------------------------------------------------------------------
RadioButton::RadioButton(Dialog &dialog) : CheckBox(false, dialog) {
    mType = CONTROL_RADIOBUTTON;
}

//--------------------------------------------------------------------------------------
bool RadioButton::MsgProc(MessageType msg, int32_t param1, int32_t param2, int32_t param3, int32_t param4) noexcept {
    if (!mEnabled || !mVisible)
        return false;

    Point mousePos = mDialog.GetMousePositionDialogSpace();

    switch (msg) {

        case CURSOR_POS:

            if (mPressed) {
                //if the button is pressed and the mouse is moved off, then unpress it
                if (!ContainsPoint(mousePos)) {
                    mPressed = false;

                    if (!mDialog.IsKeyboardInputEnabled())
                        mDialog.ClearFocus();
                }
            }

            break;
        case MB: {
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
                    if (mPressed || !mChecked) {
                        if (ContainsPoint(mousePos)) {
                            mPressed = false;

                            SetCheckedInternal(true, true, true);
                            return true;
                        }
                    }

                }
            }
        }
            break;

        case KEY: {

            if (param1 = GLFW_KEY_SPACE) {
                if (param3 == GLFW_PRESS) {
                    mPressed = true;
                } else if (param3 == GLFW_RELEASE) {
                    if (mChecked)
                        return false;

                    mPressed = false;

                    SetCheckedInternal(true, true, true);
                }
            }


            return true;

            break;
        }
    };

    return false;
}

//--------------------------------------------------------------------------------------
void RadioButton::SetCheckedInternal(bool checked, bool clearGroup, bool fromInput) {
    if (checked && clearGroup)
        mDialog.ClearRadioButtonGroup(mButtonGroup);

    mChecked = checked;
    mDialog.SendEvent(EVENT_RADIOBUTTON_CHANGED, fromInput, shared_from_this());
}

//--------------------------------------------------------------------------------------
void RadioButton::OnHotkey() noexcept {
    if (mDialog.IsKeyboardInputEnabled())
        mDialog.RequestFocus(shared_from_this());

    SetCheckedInternal(true, true, true);
}

//--------------------------------------------------------------------------------------
void RadioButton::OnMouseEnter() noexcept {
    mMouseOver = true;

    auto thisGroup = mDialog.GetRadioButtonGroup(mButtonGroup);
    for (auto it : thisGroup) {
        it->OnMouseEnterNoRecurse();
    }
}

//--------------------------------------------------------------------------------------
void RadioButton::OnMouseLeave() noexcept {
    mMouseOver = false;

    auto thisGroup = mDialog.GetRadioButtonGroup(mButtonGroup);
    for (auto it : thisGroup) {
        it->OnMouseLeaveNoRecurse();
    }
}

//--------------------------------------------------------------------------------------
void RadioButton::OnMouseEnterNoRecurse() noexcept {
    mMouseOver = true;
}

//--------------------------------------------------------------------------------------
void RadioButton::OnMouseLeaveNoRecurse() noexcept {
    mMouseOver = false;
}

//--------------------------------------------------------------------------------------
void RadioButton::OnFocusIn() noexcept {
    mHasFocus = true;

    auto thisGroup = mDialog.GetRadioButtonGroup(mButtonGroup);
    for (auto it : thisGroup) {
        it->OnFocusInNoRecurse();
    }
}

//--------------------------------------------------------------------------------------
void RadioButton::OnFocusOut() noexcept {
    mHasFocus = false;

    auto thisGroup = mDialog.GetRadioButtonGroup(mButtonGroup);
    for (auto it : thisGroup) {
        it->OnFocusOutNoRecurse();
    }
}

//--------------------------------------------------------------------------------------
void RadioButton::OnFocusInNoRecurse() noexcept {
    mHasFocus = true;
}

//--------------------------------------------------------------------------------------
void RadioButton::OnFocusOutNoRecurse() noexcept {
    mHasFocus = false;
}

}

#include "Slider.h"

namespace GLUF {

//--------------------------------------------------------------------------------------
Slider::Slider(Dialog &dialog) : Control(dialog) {
    mType = CONTROL_SLIDER;

    mMin = 0;
    mMax = 100;
    mValue = 0;
    mPressed = false;
}


//--------------------------------------------------------------------------------------
bool Slider::ContainsPoint(const Point &pt) const noexcept {
    return (PtInRect(mRegion, pt) ||
            PtInRect(mButtonRegion, pt));
}


//--------------------------------------------------------------------------------------
void Slider::UpdateRects() noexcept {
    Control::UpdateRects();

    mButtonRegion = mRegion;
    mButtonRegion.right = mButtonRegion.left + RectHeight(mButtonRegion);
    OffsetRect(mButtonRegion, -RectWidth(mButtonRegion) / 2, 0);

    mButtonX = (int) ((float(mValue - mMin) / float(mMax - mMin)) * RectWidth(mRegion));
    OffsetRect(mButtonRegion, mButtonX, 0);
}


//--------------------------------------------------------------------------------------
Value Slider::ValueFromXPos(Value x) const noexcept {
    float fValuePerPixel = (float) (mMax - mMin) / RectWidth(mRegion);
    float fPixelPerValue2 = 1.0f / (2.0f *
                                    fValuePerPixel);//use this to get it to change locations at the half way mark instead of using truncate int methods
    return int(((x - mRegion.x + fPixelPerValue2) * fValuePerPixel) + mMin);
}


//--------------------------------------------------------------------------------------
bool Slider::MsgProc(MessageType msg, int32_t param1, int32_t param2, int32_t param3, int32_t param4) noexcept {
    if (!mEnabled || !mVisible)
        return false;

    Point pt = mDialog.GetMousePositionDialogSpace();

    switch (msg) {
        case MB:
            if (param1 == GLFW_MOUSE_BUTTON_LEFT) {
                if (param2 == GLFW_PRESS) {
                    if (PtInRect(mButtonRegion, pt)) {
                        // Pressed while inside the control
                        mPressed = true;
                        //SetCapture(GetHWND());

                        mDragX = pt.x;
                        //m_nDragY = pt.y;
                        mDragOffset = mButtonX - mDragX;

                        //m_nDragValue = mValue;

                        if (!mHasFocus)
                            mDialog.RequestFocus(shared_from_this());

                        return true;
                    }

                    if (PtInRect(mRegion, pt)) {

                        if (!mHasFocus)
                            mDialog.RequestFocus(shared_from_this());

                        SetValueInternal(ValueFromXPos(pt.x), true);

                        return true;
                    }
                } else if (param2 == GLFW_RELEASE) {
                    if (mPressed) {
                        mPressed = false;
                        //ReleaseCapture();
                        mDialog.SendEvent(EVENT_SLIDER_VALUE_CHANGED_UP, true, shared_from_this());

                        return true;
                    }

                    break;

                }
                break;
            }

        case CURSOR_POS: {

            if (mPressed) {
                SetValueInternal(ValueFromXPos(mRegion.x + pt.x + mDragOffset), true);
                return true;
            }

            break;
        }

        case SCROLL: {
            int nScrollAmount = param2 / _WHEEL_DELTA;
            SetValueInternal(mValue - nScrollAmount, true);
            return true;
        }
        case KEY: {
            if (param3 == GLFW_RELEASE)
                break;

            switch (param1) {
                case GLFW_KEY_HOME:
                    SetValueInternal(mMin, true);
                    return true;

                case GLFW_KEY_END:
                    SetValueInternal(mMax, true);
                    return true;

                case GLFW_KEY_LEFT:
                case GLFW_KEY_DOWN:
                    SetValueInternal(mValue - 3, true);
                    return true;

                case GLFW_KEY_RIGHT:
                case GLFW_KEY_UP:
                    SetValueInternal(mValue + 3, true);
                    return true;

                case GLFW_KEY_PAGE_DOWN:
                    SetValueInternal(mValue - (10 > (mMax - mMin) / 10 ? 10 : (mMax - mMin) / 10),
                                     true);
                    return true;

                case GLFW_KEY_PAGE_UP:
                    SetValueInternal(mValue + (10 > (mMax - mMin) / 10 ? 10 : (mMax - mMin) / 10),
                                     true);
                    return true;
            }
            break;
        }
    };

    return false;

    return false;
}

//--------------------------------------------------------------------------------------
void Slider::SetRange(int nMin, int nMax) noexcept {
    mMin = nMin;
    mMax = nMax;

    SetValueInternal(mValue, false);
}


//--------------------------------------------------------------------------------------
void Slider::SetValueInternal(int nValue, bool bFromInput) noexcept {
    // Clamp to range
    nValue = std::clamp(nValue, mMin, mMax);


    if (nValue == mValue)
        return;

    mValue = nValue;

    UpdateRects();

    mDialog.SendEvent(EVENT_SLIDER_VALUE_CHANGED, bFromInput, shared_from_this());
}


//--------------------------------------------------------------------------------------
void Slider::Render(float elapsedTime) noexcept {
    if (mVisible == false)
        return;

    int OffsetX = 0;
    int OffsetY = 0;

    ControlState iState = STATE_NORMAL;

    if (mVisible == false) {
        iState = STATE_HIDDEN;
    } else if (mEnabled == false) {
        iState = STATE_DISABLED;
    } else if (mPressed) {
        iState = STATE_PRESSED;

        OffsetX = 1;
        OffsetY = 2;
    } else if (mMouseOver) {
        iState = STATE_MOUSEOVER;

        OffsetX = -1;
        OffsetY = -2;
    } else if (mHasFocus) {
        iState = STATE_FOCUS;
    }

    float fBlendRate = 5.0f;

    Element *pElement = &mElements[0];

    // Blend current color
    pElement->mTextureColor.Blend(iState, elapsedTime, fBlendRate);
    mDialog.DrawSprite(*pElement, mRegion, _FAR_BUTTON_DEPTH);

    pElement = &mElements[1];

    // Blend current color
    pElement->mTextureColor.Blend(iState, elapsedTime, fBlendRate);
    mDialog.DrawSprite(*pElement, mButtonRegion, _NEAR_BUTTON_DEPTH);
}
}

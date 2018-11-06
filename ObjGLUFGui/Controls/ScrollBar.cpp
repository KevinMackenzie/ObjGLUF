#include "ScrollBar.h"

namespace GLUF {

//--------------------------------------------------------------------------------------
ScrollBar::ScrollBar(Dialog &dialog) : Control(dialog) {
    mType = CONTROL_SCROLLBAR;

    mShowThumb = true;
    mDrag = false;

    SetRect(mUpButtonRegion, 0, 0, 0, 0);
    SetRect(mDownButtonRegion, 0, 0, 0, 0);
    SetRect(mTrackRegion, 0, 0, 0, 0);
    SetRect(mThumbRegion, 0, 0, 0, 0);
    mPosition = 0;
    mPageSize = 1;
    mStart = 0;
    mEnd = 1;
    mArrow = CLEAR;
    mArrowTS = 0.0;
}


//--------------------------------------------------------------------------------------
ScrollBar::~ScrollBar() {
}


//--------------------------------------------------------------------------------------
void ScrollBar::UpdateRects() noexcept {
    Control::UpdateRects();

    // Make the buttons square

    SetRect(mUpButtonRegion, mRegion.left, mRegion.top,
            mRegion.right, mRegion.top - RectWidth(mRegion));
    //OffsetRect(mDownButtonRegion, 0.0f, RectHeight(mDownButtonRegion));

    SetRect(mDownButtonRegion, mRegion.left, mRegion.bottom + RectWidth(mRegion),
            mRegion.right, mRegion.bottom);
    //OffsetRect(mDownButtonRegion, 0.0f, RectHeight(mDownButtonRegion));

    SetRect(mTrackRegion, mUpButtonRegion.left, mUpButtonRegion.bottom,
            mDownButtonRegion.right, mDownButtonRegion.top);
    //OffsetRect(mDownButtonRegion, 0.0f, RectHeight(mDownButtonRegion));

    mThumbRegion.left = mUpButtonRegion.left;
    mThumbRegion.right = mUpButtonRegion.right;

    UpdateThumbRect();
}


//--------------------------------------------------------------------------------------
// Compute the dimension of the scroll thumb
void ScrollBar::UpdateThumbRect() {
    //TODO: fix bug where the icon can go just below the max it should
    if (mEnd - mStart > mPageSize) {
        int nThumbHeight = std::clamp((int) RectHeight(mTrackRegion) * mPageSize / (mEnd - mStart),
                                      SCROLLBAR_MINTHUMBSIZE, (int) RectHeight(mTrackRegion));
        int nMaxPosition = mEnd - mStart - mPageSize + 1;
        mThumbRegion.top = mTrackRegion.top - (mPosition - mStart) * (RectHeight(mTrackRegion) - nThumbHeight)
                                              / nMaxPosition;
        mThumbRegion.bottom = mThumbRegion.top - nThumbHeight;
        mShowThumb = true;

    } else {
        // No content to scroll
        mThumbRegion.bottom = mThumbRegion.top;
        mShowThumb = false;
    }
}


//--------------------------------------------------------------------------------------
// Scroll() scrolls by nDelta items.  A positive value scrolls down, while a negative
// value scrolls up.
void ScrollBar::Scroll(int nDelta) {
    // Perform scroll
    mPosition += nDelta;

    // Cap position
    Cap();

    // Update thumb position
    UpdateThumbRect();
}


//--------------------------------------------------------------------------------------
void ScrollBar::ShowItem(int nIndex) {
    // Cap the index

    nIndex = std::clamp(nIndex, 0, mEnd - 1);

    // Adjust position

    if (mPosition > nIndex)
        mPosition = nIndex;
    else if (mPosition + mPageSize <= nIndex)
        mPosition = nIndex - mPageSize + 1;

    UpdateThumbRect();
}

//--------------------------------------------------------------------------------------

bool ScrollBar::MsgProc(MessageType msg, int32_t param1, int32_t param2, int32_t param3, int32_t param4) noexcept {
    //UNREFERENCED_PARAMETER(wParam);

    if (FOCUS == msg && param1 == GL_FALSE) {
        // The application just lost mouse capture. We may not have gotten
        // the WM_MOUSEUP message, so reset mDrag here.
        //if ((HWND)lParam != GetHWND())
        mDrag = false;
    }
    static int nThumbYOffset;

    Point pt = mDialog.GetMousePositionDialogSpace();
    mPreviousMousePos = pt;


    //if the mousebutton is NOT PRESSED, the stop scrolling(since this does not get the mouse release button if it is not over the control)
    //int mbPressed = glfwGetMouseButton(g_pGLFWWindow, GLFW_MOUSE_BUTTON_LEFT);
    //if (mbPressed == GLFW_RELEASE)
    //    mDrag = false;

    switch (msg) {
        case MB:
            if (param1 == GLFW_MOUSE_BUTTON_LEFT) {
                if (param2 == GLFW_PRESS) {
                    // Check for click on up button

                    if (PtInRect(mUpButtonRegion, pt)) {
                        //SetCapture(GetHWND());
                        if (mPosition > mStart)
                            --mPosition;

                        UpdateThumbRect();
                        mArrow = CLICKED_UP;
                        mArrowTS = GetTime();
                        return true;
                    }

                    // Check for click on down button

                    if (PtInRect(mDownButtonRegion, pt)) {
                        //SetCapture(GetHWND());
                        if (mPosition + mPageSize <= mEnd)
                            ++mPosition;

                        UpdateThumbRect();
                        mArrow = CLICKED_DOWN;
                        mArrowTS = GetTime();
                        return true;
                    }

                    // Check for click on thumb

                    if (PtInRect(mThumbRegion, pt)) {
                        //SetCapture(GetHWND());
                        mDrag = true;
                        nThumbYOffset = mThumbRegion.top - pt.y;
                        return true;
                    }

                    // Check for click on track

                    if (mThumbRegion.left <= pt.x &&
                        mThumbRegion.right > pt.x) {
                        //SetCapture(GetHWND());
                        if (mThumbRegion.top > pt.y &&
                            mTrackRegion.top <= pt.y) {
                            Scroll(-(mPageSize - 1));
                            return true;
                        } else if (mThumbRegion.bottom <= pt.y &&
                                   mTrackRegion.bottom > pt.y) {
                            Scroll(mPageSize - 1);
                            return true;
                        }
                    }

                    break;
                } else if (param2 == GLFW_RELEASE) {
                    mDrag = false;
                    //ReleaseCapture();
                    UpdateThumbRect();
                    mArrow = CLEAR;
                    break;
                }
            }

        case CURSOR_POS: {
            if (mDrag) {
                static int nThumbHeight;
                nThumbHeight = RectHeight(mThumbRegion);

                mThumbRegion.top = pt.y + nThumbYOffset;
                mThumbRegion.bottom = mThumbRegion.top - nThumbHeight;
                if (mThumbRegion.top > mTrackRegion.top)
                    OffsetRect(mThumbRegion, 0, -(mThumbRegion.top - mTrackRegion.top));
                else if (mThumbRegion.bottom < mTrackRegion.bottom)
                    OffsetRect(mThumbRegion, 0, -(mThumbRegion.bottom - mTrackRegion.bottom));

                // Compute first item index based on thumb position

                int nMaxFirstItem = mEnd - mStart - mPageSize + 1;  // Largest possible index for first item
                int nMaxThumb = RectHeight(mTrackRegion) -
                                RectHeight(mThumbRegion);  // Largest possible thumb position from the top

                mPosition = mStart + int(((mTrackRegion.top - mThumbRegion.top) * (float) nMaxFirstItem) / nMaxThumb);
                /*(mTrackRegion.top - mThumbRegion.top +
                nMaxThumb / (nMaxFirstItem * 2)) * // Shift by half a row to avoid last row covered by only one pixel
                nMaxFirstItem / nMaxThumb;*/

                return true;
            }

            break;
        }
    }

    return false;
}


//--------------------------------------------------------------------------------------
void ScrollBar::Render(float elapsedTime) noexcept {
    if (mVisible == false)
        return;

    // Check if the arrow button has been held for a while.
    // If so, update the thumb position to simulate repeated
    // scroll.
    if (mArrow != CLEAR) {
        double dCurrTime = GetTime();
        if (PtInRect(mUpButtonRegion, mPreviousMousePos)) {
            switch (mArrow) {
                case CLICKED_UP:
                    if (SCROLLBAR_ARROWCLICK_DELAY < dCurrTime - mArrowTS) {
                        Scroll(-1);
                        mArrow = HELD_UP;
                        mArrowTS = dCurrTime;
                    }
                    break;
                case HELD_UP:
                    if (SCROLLBAR_ARROWCLICK_REPEAT < dCurrTime - mArrowTS) {
                        Scroll(-1);
                        mArrowTS = dCurrTime;
                    }
                    break;
            }
        } else if (PtInRect(mDownButtonRegion, mPreviousMousePos)) {
            switch (mArrow) {
                case CLICKED_DOWN:
                    if (SCROLLBAR_ARROWCLICK_DELAY < dCurrTime - mArrowTS) {
                        Scroll(1);
                        mArrow = HELD_DOWN;
                        mArrowTS = dCurrTime;
                    }
                    break;
                case HELD_DOWN:
                    if (SCROLLBAR_ARROWCLICK_REPEAT < dCurrTime - mArrowTS) {
                        Scroll(1);
                        mArrowTS = dCurrTime;
                    }
                    break;
            }
        }
    }

    ControlState iState = STATE_NORMAL;

    if (mVisible == false)
        iState = STATE_HIDDEN;
    else if (mEnabled == false || mShowThumb == false)
        iState = STATE_DISABLED;
    else if (mMouseOver)
        iState = STATE_MOUSEOVER;
    else if (mHasFocus)
        iState = STATE_FOCUS;


    float fBlendRate = 5.0f;

    // Background track layer
    Element *pElement = &mElements[0];

    // Blend current color
    pElement->mTextureColor.Blend(iState, elapsedTime, fBlendRate);
    mDialog.DrawSprite(*pElement, mTrackRegion, _FAR_BUTTON_DEPTH);


    ControlState iArrowState = iState;
    //if it is all the way at the top, then disable
    if (mPosition == 0 && iState != STATE_HIDDEN)
        iArrowState = STATE_DISABLED;

    // Up Arrow
    pElement = &mElements[1];

    // Blend current color
    pElement->mTextureColor.Blend(iArrowState, elapsedTime, fBlendRate);
    mDialog.DrawSprite(*pElement, mUpButtonRegion, _NEAR_BUTTON_DEPTH);


    //similar with the bottom
    iArrowState = iState;
    if ((mPosition + mPageSize - 1 == mEnd && iState != STATE_HIDDEN) || mEnd == 1/*when no scrolling is necesary*/)
        iArrowState = STATE_DISABLED;

    // Down Arrow
    pElement = &mElements[2];

    // Blend current color
    pElement->mTextureColor.Blend(iArrowState, elapsedTime, fBlendRate);
    mDialog.DrawSprite(*pElement, mDownButtonRegion, _NEAR_BUTTON_DEPTH);

    // Thumb button
    pElement = &mElements[3];

    // Blend current color
    pElement->mTextureColor.Blend(iState, elapsedTime, fBlendRate);
    mDialog.DrawSprite(*pElement, mThumbRegion, _NEAR_BUTTON_DEPTH);
}


//--------------------------------------------------------------------------------------
void ScrollBar::SetTrackRange(int nStart, int nEnd) noexcept {
    mStart = nStart;
    mEnd = nEnd;
    Cap();
    UpdateThumbRect();
}


//--------------------------------------------------------------------------------------
void ScrollBar::Cap()  // Clips position at boundaries. Ensures it stays within legal range.
{
    if (mPosition < mStart ||
        mEnd - mStart <= mPageSize) {
        mPosition = mStart;
    } else if (mPosition + mPageSize > mEnd)
        mPosition = mEnd - mPageSize + 1;
}

}

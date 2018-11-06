#include "ComboBox.h"

namespace GLUF {

//--------------------------------------------------------------------------------------
ComboBox::ComboBox(Dialog &dialog) : mScrollBar(CreateScrollBar(dialog)), Button(dialog) {
    mType = CONTROL_COMBOBOX;

    mDropHeight = 100L;

    mSBWidth = 16L;
    mOpened = false;
    mSelected = -1;
    mFocused = -1;
}


//--------------------------------------------------------------------------------------
ComboBox::~ComboBox() {
    RemoveAllItems();
}


//--------------------------------------------------------------------------------------
void ComboBox::SetTextColor(const Color &Color) noexcept {
    Element *pElement = &mElements[0];

    pElement->mFontColor.mStates[STATE_NORMAL] = Color;

    pElement = &mElements[2];

    if (pElement)
        pElement->mFontColor.mStates[STATE_NORMAL] = Color;
}


//--------------------------------------------------------------------------------------
void ComboBox::UpdateRects() noexcept {

    Button::UpdateRects();

    mButtonRegion = mRegion;
    mButtonRegion.left = mButtonRegion.right - RectHeight(mButtonRegion);

    mTextRegion = mRegion;
    mTextRegion.right = mButtonRegion.left;
    mTextRegion.left += 5;//so it isn't falling off the edges

    mDropdownRegion.left = long(mRegion.left/* * 1.019f*/);
    mDropdownRegion.top = long(/*1.02f * */mTextRegion.bottom);
    mDropdownRegion.right = mTextRegion.right;
    mDropdownRegion.bottom = mDropdownRegion.top - mDropHeight;
    //OffsetRect(mDropdownRegion, 0, -RectHeight(mTextRegion));

    mDropdownTextRegion = mDropdownRegion;
    mDropdownTextRegion.left += long(0.05f * RectWidth(mDropdownRegion));
    mDropdownTextRegion.right -= long(0.05f * RectWidth(mDropdownRegion));
    mDropdownTextRegion.top += long(0.05f * RectHeight(mDropdownRegion));
    mDropdownTextRegion.bottom -= long(0.05f * RectHeight(mDropdownRegion));

    // Update the scrollbar's rects
    mScrollBar->SetLocation(mDropdownRegion.right, mDropdownRegion.bottom);
    mScrollBar->SetSize(mSBWidth, abs(mButtonRegion.bottom - mDropdownRegion.bottom));
    //Rect tmpRect = mScrollBar->GetRegion();
    //tmpRect.y = mTextRegion.top;
    //mScrollBar->SetRegion(tmpRect);
    FontNodePtr pFontNode = mDialog.GetFont(mElements[2].mFontIndex);
    if (pFontNode/* && pFontNode->mSize*/) {
        mScrollBar->SetPageSize(int(RectHeight(mDropdownTextRegion) / pFontNode->mFontType->mHeight));

        // The selected item may have been scrolled off the page.
        // Ensure that it is in page again.
        mScrollBar->ShowItem(mSelected);
    }

    mScrollBar->UpdateRects();
    mRegion.right = mButtonRegion.left;

}


//--------------------------------------------------------------------------------------
void ComboBox::OnInit() {
    UpdateRects();

    mDialog.InitControl(std::dynamic_pointer_cast<Control>(mScrollBar));
}

//--------------------------------------------------------------------------------------
void ComboBox::UpdateItemRects() noexcept {
    FontNodePtr pFont = mDialog.GetFont(GetElement(2).mFontIndex);
    if (pFont) {
        int curY = mTextRegion.bottom - 4;// +((mScrollBar->GetTrackPos() - 1) * pFont->mSize);
        int fRemainingHeight = RectHeight(mDropdownTextRegion) -
                               pFont->mLeading;//subtract the font size initially too, because we do not want it hanging off the edge


        for (size_t i = mScrollBar->GetTrackPos(); i < mItems.size(); i++) {
            ComboBoxItemPtr pItem = mItems[i];

            // Make sure there's room left in the dropdown
            fRemainingHeight -= pFont->mLeading;
            if (fRemainingHeight <= 0.0f) {
                pItem->mVisible = false;
                continue;
            }

            pItem->mVisible = true;

            SetRect(pItem->mTextRegion, mDropdownTextRegion.left, curY, mDropdownTextRegion.right,
                    curY - pFont->mFontType->mHeight);
            curY -= pFont->mLeading;
        }
    }
}

//--------------------------------------------------------------------------------------
void ComboBox::OnFocusOut() noexcept {
    Button::OnFocusOut();

    mOpened = false;
}


//--------------------------------------------------------------------------------------
bool ComboBox::MsgProc(MessageType msg, int32_t param1, int32_t param2, int32_t param3, int32_t param4) noexcept {
    if (!mEnabled || !mVisible)
        return false;

    // Let the scroll bar handle it first.
    if (mScrollBar->MsgProc(msg, param1, param2, param3, param4))
        return true;

    Point pt = mDialog.GetMousePositionDialogSpace();

    switch (msg) {
        case CURSOR_POS: {
            /*if (mPressed)
            {
            //if the button is pressed and the mouse is moved off, then unpress it
            if (!ContainsPoint(pt))
            {
            mPressed = false;

            ContainsPoint(pt);

            if (!mDialog.IsKeyboardInputEnabled())
            mDialog.ClearFocus();
            }
            }*/

            if (mOpened && PtInRect(mDropdownRegion, pt)) {
                // Determine which item has been selected
                for (size_t i = 0; i < mItems.size(); i++) {
                    ComboBoxItemPtr pItem = mItems[i];
                    if (pItem->mVisible &&
                        PtInRect(pItem->mTextRegion, pt)) {
                        mFocused = static_cast<int>(i);
                    }
                }
                return true;
            }
            break;
        }

        case MB:
            if (param1 == GLFW_MOUSE_BUTTON_LEFT) {
                if (param2 == GLFW_PRESS) {
                    if (ContainsPoint(pt)) {
                        // Pressed while inside the control
                        mPressed = true;
                        //SetCapture(GetHWND());

                        if (!mHasFocus)
                            mDialog.RequestFocus(shared_from_this());

                        return true;
                    }

                    // Perhaps this click is within the dropdown
                    if (mOpened && PtInRect(mDropdownRegion, pt)) {
                        // Determine which item has been selected
                        for (size_t i = mScrollBar->GetTrackPos(); i < mItems.size(); i++) {
                            ComboBoxItemPtr pItem = mItems[i];
                            if (pItem->mVisible &&
                                PtInRect(pItem->mTextRegion, pt)) {
                                mFocused = mSelected = static_cast<int>(i);
                                mDialog.SendEvent(EVENT_COMBOBOX_SELECTION_CHANGED, true, shared_from_this());
                                mOpened = false;

                                if (!mDialog.IsKeyboardInputEnabled())
                                    mDialog.ClearFocus();

                                break;
                            }
                        }

                        return true;
                    }

                    // Mouse click not on main control or in dropdown, fire an event if needed
                    if (mOpened) {
                        mFocused = mSelected;

                        mDialog.SendEvent(EVENT_COMBOBOX_SELECTION_CHANGED, true, shared_from_this());
                        mOpened = false;
                    }


                    break;
                } else if (param2 == GLFW_RELEASE) {
                    if (mPressed && ContainsPoint(pt)) {
                        // Button click
                        mPressed = false;

                        // Toggle dropdown
                        if (mHasFocus) {
                            mOpened = !mOpened;

                            if (!mOpened) {
                                if (!mDialog.IsKeyboardInputEnabled())
                                    mDialog.ClearFocus();

                                mFocused = mSelected;
                            }

                            //setup the scroll bar to the correct position (if it is still within the range, it looks better to keep its old position)
                            int pageMin, pageMax;
                            pageMin = mScrollBar->GetTrackPos();
                            pageMax = mScrollBar->GetTrackPos() + mScrollBar->GetPageSize() - 2;
                            if (mFocused > pageMax || mFocused < pageMin)
                                mScrollBar->SetTrackPos(mFocused);
                        }

                        //ReleaseCapture();
                        return true;
                    }

                    break;
                }
            }

        case SCROLL: {
            int zDelta = (param2) / _WHEEL_DELTA;
            if (mOpened) {
                //UINT uLines = 0;
                //if (!SystemParametersInfo(SPI_GETWHEELSCROLLLINES, 0, &uLines, 0))
                //    uLines = 0;
                mScrollBar->Scroll(-zDelta/* * uLines*/);

                //if it is scroll, then make sure to also send a mouse move event to select the newly hovered item
                UpdateItemRects();
                this->MsgProc(CURSOR_POS, 0, 0, 0,
                              0);//all blank params may be sent because it retrieves the mouse position from the old message
                //TODO: make this work, but for now:

                /*if (PtInRect(mDropdownRegion, pt))
                {
                // Determine which item has been selected
                for (size_t i = 0; i < mItems.size(); i++)
                {
                ComboBoxItemPtr pItem = mItems[i];
                Rect oldRect = pItem->mActiveRegion;

                OffsetRect(oldRect, 0, float(mScrollBar->GetTrackPos() - oldValue) * RectHeight(pItem->mActiveRegion));
                if (pItem->mVisible &&
                PtInRect(oldRect, pt))
                {
                mFocused = static_cast<int>(i);
                }
                }
                }*/

            } else {
                if (zDelta > 0) {
                    if (mFocused > 0) {
                        mFocused--;
                        mSelected = mFocused;

                        if (!mOpened)
                            mDialog.SendEvent(EVENT_COMBOBOX_SELECTION_CHANGED, true, shared_from_this());
                    }
                } else {
                    if (mFocused + 1 < (int) GetNumItems()) {
                        mFocused++;
                        mSelected = mFocused;

                        if (!mOpened)
                            mDialog.SendEvent(EVENT_COMBOBOX_SELECTION_CHANGED, true, shared_from_this());
                    }
                }

            }
            return true;
        }
        case KEY: {
            if (param3 != GLFW_RELEASE)
                return true;

            switch (param1) {
                case GLFW_KEY_ENTER:
                    if (mOpened) {
                        if (mSelected != mFocused) {
                            mSelected = mFocused;
                            mDialog.SendEvent(EVENT_COMBOBOX_SELECTION_CHANGED, true, shared_from_this());
                        }
                        mOpened = false;

                        if (!mDialog.IsKeyboardInputEnabled())
                            mDialog.ClearFocus();

                        return true;
                    }
                    break;

                case GLFW_KEY_F4:
                    // Filter out auto-repeats
                    /*if (param3 == GLFW_REPEAT)
                        return true;*/

                    mOpened = !mOpened;

                    if (!mOpened) {
                        mDialog.SendEvent(EVENT_COMBOBOX_SELECTION_CHANGED, true, shared_from_this());

                        if (!mDialog.IsKeyboardInputEnabled())
                            mDialog.ClearFocus();
                    }

                    return true;

                case GLFW_KEY_UP:
                case GLFW_KEY_LEFT:
                    if (mFocused > 0) {
                        mFocused--;
                        mSelected = mFocused;

                        if (!mOpened)
                            mDialog.SendEvent(EVENT_COMBOBOX_SELECTION_CHANGED, true, shared_from_this());
                    }

                    return true;

                case GLFW_KEY_RIGHT:
                case GLFW_KEY_DOWN:
                    if (mFocused + 1 < (int) GetNumItems()) {
                        mFocused++;
                        mSelected = mFocused;

                        if (!mOpened)
                            mDialog.SendEvent(EVENT_COMBOBOX_SELECTION_CHANGED, true, shared_from_this());
                    }

                    return true;
            }
            break;
        }
    };

    return false;
}

//--------------------------------------------------------------------------------------
void ComboBox::OnHotkey() noexcept {
    if (mOpened)
        return;

    if (mSelected == -1)
        return;

    if (mDialog.IsKeyboardInputEnabled())
        mDialog.RequestFocus(shared_from_this());

    mSelected++;

    if (mSelected >= (int) mItems.size())
        mSelected = 0;

    mFocused = mSelected;
    mDialog.SendEvent(EVENT_COMBOBOX_SELECTION_CHANGED, true, shared_from_this());
}

bool ComboBox::ContainsPoint(const Point &pt) const noexcept {
    return (PtInRect(mRegion, pt) || PtInRect(mButtonRegion, pt));// || (PtInRect(mDropdownRegion, pt) && mOpened));
}


//--------------------------------------------------------------------------------------
void ComboBox::Render(float elapsedTime) noexcept {
    if (mVisible == false)
        return;
    ControlState iState = STATE_NORMAL;

    //if (!mOpened)
    //    iState = STATE_HIDDEN;

    // Dropdown box
    Element *pElement = &mElements[2];

    // If we have not initialized the scroll bar page size,
    // do that now.
    static bool bSBInit;
    if (!bSBInit) {
        // Update the page size of the scroll bar
        auto fontNode = mDialog.GetFont(pElement->mFontIndex);
        if (fontNode->mFontType->mHeight) {
            mScrollBar->SetPageSize(
                    static_cast<int>(glm::round(
                            static_cast<float>(RectHeight(mDropdownTextRegion)) /
                            static_cast<float>(fontNode->mLeading))));
        } else
            mScrollBar->SetPageSize(0);
        bSBInit = true;
    }

    // Scroll bar --EDITED, only render any of this stuff if OPENED
    if (mOpened) {
        mScrollBar->Render(elapsedTime);

        // Blend current color
        pElement->mTextureColor.Blend(iState, elapsedTime);
        pElement->mFontColor.Blend(iState, elapsedTime);

        mDialog.DrawSprite(*pElement, mDropdownRegion, _NEAR_BUTTON_DEPTH);


        // Selection outline
        Element *pSelectionElement = &mElements[3];
        pSelectionElement->mTextureColor.GetCurrent() = pElement->mTextureColor.GetCurrent();
        pSelectionElement->mFontColor.SetCurrent(/*pSelectionElement->mFontColor.mStates[STATE_NORMAL]*/{0, 0, 0, 255});

        FontNodePtr pFont = mDialog.GetFont(pElement->mFontIndex);
        if (pFont) {
            //float curY = mDropdownTextRegion.top - 0.02f;
            //float fRemainingHeight = RectHeight(mDropdownTextRegion) - pFont->mSize;//subtract the font size initially too, because we do not want it hanging off the edge
            //WCHAR strDropdown[4096] = {0};

            UpdateItemRects();

            for (size_t i = mScrollBar->GetTrackPos(); i < mItems.size(); i++) {
                ComboBoxItemPtr pItem = mItems[i];
                Rect active = pItem->mTextRegion;

                active.top = active.bottom + pFont->mLeading;

                // Make sure there's room left in the dropdown

                if (!pItem->mVisible)
                    continue;
                //SetRect(pItem->mActiveRegion, mDropdownTextRegion.left, curY, mDropdownTextRegion.right, curY - pFont->mSize);
                //curY -= pFont->mSize;

                //debug
                //int blue = 50 * i;
                //mDialog.DrawRect( &pItem->mActiveRegion, 0xFFFF0000 | blue );

                //pItem->mVisible = true;

                //SetRect(rc, mDropdownRegion.left, pItem->mActiveRegion.top - (2 / mDialog.GetManager()->GetWindowSize().y), mDropdownRegion.right,
                //    pItem->mActiveRegion.bottom + (2 / mDialog.GetManager()->GetWindowSize().y));
                //SetRect(rc, mDropdownRegion.left + RectWidth(mDropdownRegion) / 12.0f, mDropdownRegion.top - (RectHeight(pItem->mActiveRegion) * i), mDropdownRegion.right,
                //    mDropdownRegion.top - (RectHeight(pItem->mActiveRegion) * (i + 1)));

                if ((int) i == mFocused) {
                    //SetRect(rc, mDropdownRegion.left, pItem->mActiveRegion.top - (2 / mDialog.GetManager()->GetWindowSize().y), mDropdownRegion.right,
                    //    pItem->mActiveRegion.bottom + (2 / mDialog.GetManager()->GetWindowSize().y));
                    /*SetRect(rc, mDropdownRegion.left, mDropdownRegion.top - (RectHeight(pItem->mActiveRegion) * i), mDropdownRegion.right,
                        mDropdownRegion.top - (RectHeight(pItem->mActiveRegion) * (i + 1)));*/
                    //mDialog.DrawText(pItem->mText, pSelectionElement, rc);
                    mDialog.DrawSprite(*pSelectionElement, active, _NEAR_BUTTON_DEPTH);
                    mDialog.DrawText(pItem->mText, *pSelectionElement, pItem->mTextRegion);
                } else {
                    mDialog.DrawText(pItem->mText, *pElement, pItem->mTextRegion);
                }
            }
        }
    }

    int OffsetX = 0;
    int OffsetY = 0;

    iState = STATE_NORMAL;

    if (mVisible == false)
        iState = STATE_HIDDEN;
    else if (mEnabled == false)
        iState = STATE_DISABLED;
    else if (mPressed) {
        iState = STATE_PRESSED;

        OffsetX = 1;
        OffsetY = 2;
    } else if (mMouseOver) {
        iState = STATE_MOUSEOVER;

        OffsetX = -1;
        OffsetY = -2;
    } else if (mHasFocus)
        iState = STATE_FOCUS;

    float fBlendRate = 5.0f;

    // Button
    pElement = &mElements[1];

    // Blend current color
    pElement->mTextureColor.Blend(iState, elapsedTime, fBlendRate);

    Rect rcWindow = mButtonRegion;
    OffsetRect(rcWindow, OffsetX, OffsetY);
    mDialog.DrawSprite(*pElement, rcWindow, _FAR_BUTTON_DEPTH);

    if (mOpened)
        iState = STATE_PRESSED;


    // Main text box
    pElement = &mElements[0];

    // Blend current color
    pElement->mTextureColor.Blend(iState, elapsedTime, fBlendRate);
    pElement->mFontColor.Blend(iState, elapsedTime, fBlendRate);


    mDialog.DrawSprite(*pElement, mRegion, _NEAR_BUTTON_DEPTH);

    if (mSelected >= 0 && mSelected < (int) mItems.size()) {
        ComboBoxItemPtr pItem = mItems[mSelected];
        if (pItem) {
            mDialog.DrawText(pItem->mText, *pElement, mTextRegion, false, true);

        }
    }

}


//--------------------------------------------------------------------------------------

void ComboBox::AddItem(const std::wstring &text, GenericData &data) noexcept {
    // Create a new item and set the data
    auto pItem = std::make_shared<ComboBoxItem>(data);

    pItem->mText = text;
    //pItem->mData = data;

    mItems.push_back(pItem);

    // Update the scroll bar with new range
    mScrollBar->SetTrackRange(0, (int) mItems.size());

    // If this is the only item in the list, it's selected
    if (GetNumItems() == 1) {
        mSelected = 0;
        mFocused = 0;
        mDialog.SendEvent(EVENT_COMBOBOX_SELECTION_CHANGED, false, shared_from_this());
    }
}


//--------------------------------------------------------------------------------------
void ComboBox::RemoveItem(Index index) {
    if (index >= mItems.size()) {
        GLUF_NON_CRITICAL_EXCEPTION(std::out_of_range("Error Removing Item From Combo Box"));
        return;
    }

    //erase the item (kinda sloppy)
    std::vector <ComboBoxItemPtr> newItemList;
    newItemList.resize(mItems.size() - 1);
    for (Index i = 0; i < mItems.size(); ++i) {
        if (i == index)
            continue;

        newItemList[i] = mItems[i];
    }
    mItems = newItemList;

    mScrollBar->SetTrackRange(0, (int) mItems.size());
    if (mSelected >= (int) mItems.size())
        mSelected = (int) mItems.size() - 1;
}


//--------------------------------------------------------------------------------------
void ComboBox::RemoveAllItems() noexcept {
    mItems.clear();
    mScrollBar->SetTrackRange(0, 1);
    mFocused = mSelected = -1;
}


//--------------------------------------------------------------------------------------
bool ComboBox::ContainsItem(const std::wstring &text, Index start) const noexcept {
    return (-1 != FindItemIndex(text, start));
}


//--------------------------------------------------------------------------------------
Index ComboBox::FindItemIndex(const std::wstring &text, Index start) const {
    for (Index i = start; i < mItems.size(); ++i) {
        ComboBoxItemPtr pItem = mItems[i];

        if (pItem->mText == text)//REMEMBER if this returns 0, they are the same
        {
            return static_cast<int>(i);
        }
    }

    throw std::invalid_argument("\"text\" was not found in combo box");
}


//--------------------------------------------------------------------------------------
ComboBoxItemPtr ComboBox::FindItem(const std::wstring &text, Index start) const {
    for (auto it : mItems) {
        if (it->mText == text)//REMEMBER if this returns 0, they are the same
        {
            return it;
        }
    }

    throw std::invalid_argument("\"text\" was not found in combo box");
}


//--------------------------------------------------------------------------------------
GenericData &ComboBox::GetSelectedData() const {
    if (mSelected < 0)
        throw NoItemSelectedException();

    ComboBoxItemPtr pItem = mItems[mSelected];
    return pItem->mData;
}


//--------------------------------------------------------------------------------------
ComboBoxItemPtr ComboBox::GetSelectedItem() const {
    if (mSelected < 0)
        throw NoItemSelectedException();

    return mItems[mSelected];
}


//--------------------------------------------------------------------------------------
GenericData &ComboBox::GetItemData(const std::wstring &text, Index start) const {
    return FindItem(text, start)->mData;
}


//--------------------------------------------------------------------------------------
GenericData &ComboBox::GetItemData(Index index) const {
    return mItems[index]->mData;
}


//--------------------------------------------------------------------------------------
void ComboBox::SelectItem(Index index) {
    if (index >= mItems.size()) {
        GLUF_NON_CRITICAL_EXCEPTION(std::out_of_range("Index Too Large"));
        return;
    }

    mFocused = mSelected = index;
    mDialog.SendEvent(EVENT_COMBOBOX_SELECTION_CHANGED, false, shared_from_this());
}


//--------------------------------------------------------------------------------------
void ComboBox::SelectItem(const std::wstring &text, Index start) {
    Index itemIndex = 0;
    try {
        itemIndex = FindItemIndex(text, start);
    }
    catch (...) {
        GLUF_NON_CRITICAL_EXCEPTION(std::invalid_argument("\"text\" not found in combo box"));
        return;
    }

    mFocused = mSelected = itemIndex;
    mDialog.SendEvent(EVENT_COMBOBOX_SELECTION_CHANGED, false, shared_from_this());
}


//--------------------------------------------------------------------------------------
void ComboBox::SelectItem(const GenericData &data) {
    sIndex itemIndex = -1;
    for (unsigned int i = 0; i < mItems.size(); ++i) {
        if (&mItems[i]->mData == &data) {
            itemIndex = i;
            break;
        }
    }

    if (itemIndex == -1) {
        GLUF_NON_CRITICAL_EXCEPTION(std::invalid_argument("\"data\" not found"));
        return;
    }

    SelectItem(itemIndex);
}

}

#include "ListBox.h"

//--------------------------------------------------------------------------------------
ListBox::ListBox(Dialog& dialog) : mScrollBar(CreateScrollBar(dialog)), Control(dialog)
{
    mType = CONTROL_LISTBOX;

    Point pt = mDialog.GetManager()->GetWindowSize();

    mStyle = 0;
    mSBWidth = 16;
    mSelected.push_back(-1);
    mDrag = false;
    mVerticalMargin = 6;
    mHorizontalMargin = 5;
    mTextHeight = 20;
}


//--------------------------------------------------------------------------------------
ListBox::~ListBox()
{
    RemoveAllItems();
}

//--------------------------------------------------------------------------------------
GenericData& ListBox::GetItemData(const std::wstring& text, Index start) const
{
    return FindItem(text, start)->mData;
}

//--------------------------------------------------------------------------------------
GenericData& ListBox::GetItemData(Index index) const
{
    return mItems[index]->mData;
}


//--------------------------------------------------------------------------------------
void ListBox::UpdateRects() noexcept
{
    Control::UpdateRects();

    FontNodePtr pFont = mDialog.GetFont(GetElement(0).mFontIndex);
    mTextHeight = pFont->mLeading;

    mSelectionRegion = mRegion;
    mSelectionRegion.right -= mSBWidth;
    InflateRect(mSelectionRegion, -(int32_t)mHorizontalMargin, -(int32_t)mVerticalMargin);
    mTextRegion = mSelectionRegion;
    InflateRect(mTextRegion, -(int32_t)mHorizontalMargin, -(int32_t)mVerticalMargin);

    // Update the scrollbar's rects
    //mScrollBar->SetLocation(mRegion.right - mSBWidth, mRegion.top);
    //mScrollBar->SetSize(mSBWidth, m_height);

    mScrollBar->SetLocation(mRegion.right, mRegion.bottom);
    mScrollBar->SetSize(mSBWidth, RectHeight(mRegion));
    FontNodePtr pFontNode = mDialog.GetFont(mElements[0].mFontIndex);
    if (pFontNode && pFontNode->mFontType->mHeight)
    {
        mScrollBar->SetPageSize(int(RectHeight(mTextRegion) / pFontNode->mLeading));

        // The selected item may have been scrolled off the page.
        // Ensure that it is in page again.
        mScrollBar->ShowItem(mSelected[mSelected.size() - 1]);
    }

    mScrollBar->UpdateRects();
    UpdateItemRects();
}


//--------------------------------------------------------------------------------------

void ListBox::AddItem(const std::wstring& text, GenericData& data) noexcept
{
    InsertItem(mItems.size(), text, data);
}


//--------------------------------------------------------------------------------------

void ListBox::InsertItem(Index index, const std::wstring& text, GenericData& data) noexcept
{
    auto newItem = std::make_shared<ListBoxItem>(data);

    //clear the selection vector
    mSelected.clear();//this makes it so we do not have to offset the selection
    mSelected.push_back(-1);

    //wcscpy_s(pNewItem->mText, 256, wszText);
    newItem->mText = text;
    SetRect(newItem->mTextRegion, 0, 0, 0, 0);
    //pNewItem->bSelected = false;

    if (index >= mItems.size())
    {
        mItems.push_back(nullptr);
        index = mItems.size() - 1;
    }

    mItems[index] = newItem;
    mScrollBar->SetTrackRange(0, (int)mItems.size());
}


//--------------------------------------------------------------------------------------
void ListBox::RemoveItem(Index index)
{
    if (index >= (int)mItems.size())
    {
        GLUF_NON_CRITICAL_EXCEPTION(std::out_of_range("Attempt To Remove List Box Item Out Of Range"));
        return;
    }

    auto it = mItems.begin() + index;
    mItems.erase(it);
    mScrollBar->SetTrackRange(0, (int)mItems.size());
    if (mSelected[0] >= (int)mItems.size())
        mSelected[0] = int(mItems.size()) - 1;

    mDialog.SendEvent(EVENT_LISTBOX_SELECTION, true, shared_from_this());
}


//--------------------------------------------------------------------------------------
void ListBox::RemoveAllItems() noexcept
{
    mItems.clear();
    mScrollBar->SetTrackRange(0, 1);
    mSelected.clear();
    //mSelected.push_back(-1);
}


//--------------------------------------------------------------------------------------
ListBoxItemPtr ListBox::GetItem(const std::wstring& text, Index start) const
{
    for (auto it : mItems)
    {
        if (it->mText == text)
            return it;
    }

    return nullptr;
}

//--------------------------------------------------------------------------------------
Index ListBox::GetSelectedIndex(Index previousSelected) const
{
    if (mStyle & MULTISELECTION)
    {
        // Multiple selection enabled. Search for the next item with the selected flag.
        /*for (int i = nPreviousSelected + 1; i < (int)mItems.size(); ++i)
        {
        ListBoxItemPtr pItem = mItems[i];

        if (pItem->bSelected)
        return i;
        }*/

        auto in = std::find(mSelected.begin(), mSelected.end(), previousSelected) + 1;
        if (in == mSelected.end())
            throw NoItemSelectedException();

        return *in;
    }
    else
    {
        // Single selection
        return GetSelectedIndex();
    }
}

//--------------------------------------------------------------------------------------
Index ListBox::GetSelectedIndex() const
{
    if (mSelected.size() == 0)
        throw NoItemSelectedException();

    return mSelected[0];
}

//--------------------------------------------------------------------------------------
void ListBox::SelectItem(Index index)
{
    // If no item exists, do nothing.
    if (index >= mItems.size())
    {
        GLUF_NON_CRITICAL_EXCEPTION(std::out_of_range("Item To Select Does Not Exist"));
        return;
    }

    int nOldSelected = mSelected[0];

    // Adjust m_nSelected
    mSelected[0] = index;

    // Perform capping
    if (mSelected[0] < 0)
        mSelected[0] = 0;
    if (mSelected[0] >= (int)mItems.size())
        mSelected[0] = int(mItems.size()) - 1;

    if (nOldSelected != mSelected[0])
    {
        index = mSelected[0];
        mSelected.clear();
        mSelected.push_back(index);

        // Adjust scroll bar
        mScrollBar->ShowItem(mSelected[mSelected.size() - 1]);
    }

    mDialog.SendEvent(EVENT_LISTBOX_SELECTION, true, shared_from_this());
}

//--------------------------------------------------------------------------------------
void ListBox::SelectItem(const std::wstring& text, Index start)
{
    for (unsigned int i = start; i < mItems.size(); ++i)
    {
        if (mItems[i]->mText == text)
        {
            SelectItem(i);
            return;
        }
    }

    //get to this point, then text is not found
    GLUF_NON_CRITICAL_EXCEPTION(std::invalid_argument("\"text\" not found in mSelected in ListBox"));
}

//--------------------------------------------------------------------------------------
void ListBox::ClearSelected() noexcept
{
    mSelected.clear();
}


//--------------------------------------------------------------------------------------
bool ListBox::ContainsItem(const std::wstring& text, Index start) const noexcept
{
    for (auto it : mItems)
    {
        if (it->mText == text)
            return true;
    }

    return false;
}


//--------------------------------------------------------------------------------------
Index ListBox::FindItemIndex(const std::wstring& text, Index start) const
{
    for (Index i = start; i < mItems.size(); ++i)
    {
        ListBoxItemPtr pItem = mItems[i];

        if (pItem->mText == text)//REMEMBER if this returns 0, they are the same
        {
            return static_cast<int>(i);
        }
    }

    throw std::invalid_argument("\"text\" was not found in combo box");
}


//--------------------------------------------------------------------------------------
ListBoxItemPtr ListBox::FindItem(const std::wstring& text, Index start) const
{
    for (auto it : mItems)
    {
        if (it->mText == text)//REMEMBER if this returns 0, they are the same
        {
            return it;
        }
    }

    throw std::invalid_argument("\"text\" was not found in combo box");
}

//--------------------------------------------------------------------------------------
bool ListBox::MsgProc(MessageType msg, int32_t param1, int32_t param2, int32_t param3, int32_t param4) noexcept
{

    if (FOCUS == msg && param1 == GL_FALSE)
    {
        // The application just lost mouse capture. We may not have gotten
        // the WM_MOUSEUP message, so reset mDrag here.
        mDrag = false;
    }

    Point pt = mDialog.GetMousePositionDialogSpace();

    if (!mEnabled || !mVisible)
        return false;

    // First acquire focus
    if (MB == msg && param1 == GLFW_MOUSE_BUTTON_LEFT && param2 == GLFW_PRESS)
        if (!mHasFocus)
            mDialog.RequestFocus(shared_from_this());

    // Let the scroll bar have a chance to handle it first
    if (mScrollBar->MsgProc(_PASS_CALLBACK_PARAM))
        return true;

    switch (msg)
    {
        case KEY:
            if (param3 == GLFW_RELEASE)
            {
                switch (param1)
                {
                    case GLFW_KEY_UP:
                    case GLFW_KEY_DOWN:
                    case GLFW_KEY_PAGE_DOWN:
                    case GLFW_KEY_PAGE_UP:
                    case GLFW_KEY_HOME:
                    case GLFW_KEY_END:
                    {
                        // If no item exists, do nothing.
                        if (mItems.size() == 0)
                            return true;

                        int nOldSelected = mSelected[0];
                        mSelected.clear();
                        mSelected.push_back(nOldSelected);

                        // Adjust m_nSelected
                        switch (param1)
                        {
                            case GLFW_KEY_UP:
                                --mSelected[0]; break;
                            case GLFW_KEY_DOWN:
                                ++mSelected[0]; break;
                            case GLFW_KEY_PAGE_DOWN:
                                mSelected[0] += mScrollBar->GetPageSize() - 1; break;
                            case GLFW_KEY_PAGE_UP:
                                mSelected[0] -= mScrollBar->GetPageSize() - 1; break;
                            case GLFW_KEY_HOME:
                                mSelected[0] = 0; break;
                            case GLFW_KEY_END:
                                mSelected[0] = int(mItems.size()) - 1; break;
                        }

                        // Perform capping
                        if (mSelected[0] < 0)
                            mSelected[0] = 0;
                        if (mSelected[0] >= (int)mItems.size())
                            mSelected[0] = int(mItems.size()) - 1;

                        if (nOldSelected != mSelected[0])
                        {
                            if (mStyle & MULTISELECTION)
                            {
                                // Multiple selection

                                // Clear all selection
                                /*for (int i = 0; i < (int)mItems.size(); ++i)
                                {
                                ListBoxItemPtr pItem = mItems[i];
                                pItem->bSelected = false;
                                }

                                mSelected.clear();

                                if (param4 & GLFW_MOD_SHIFT)
                                {
                                // Select all items from m_nSelStart to
                                // m_nSelected
                                int nEnd = mSelected[mSelected.size() - 1];//std::max(m_nSelStart, m_nSelected);

                                for (int n = mSelected[0]; n <= nEnd; ++n)
                                mItems[n]->bSelected = true;
                                }
                                else
                                {
                                mItems[mSelected[0]]->bSelected = true;

                                // Update selection start
                                //m_nSelStart = m_nSelected;
                                }*/

                                //TODO: key callback
                            }
                            else;
                            //m_nSelStart = m_nSelected;

                            // Adjust scroll bar

                            mScrollBar->ShowItem(mSelected[0]);

                            // Send notification

                            mDialog.SendEvent(EVENT_LISTBOX_SELECTION, true, shared_from_this());
                        }
                        return true;
                    }

                        // Space is the hotkey for double-clicking an item.
                        //
                    case GLFW_KEY_SPACE:
                        mDialog.SendEvent(EVENT_LISTBOX_ITEM_DBLCLK, true, shared_from_this());
                        return true;
                }
            }
            break;
            //case WM_LBUTTONDOWN:
            //case WM_LBUTTONDBLCLK:
        case MB:
            if (param2 == GLFW_PRESS)
            {
                // Check for clicks in the text area
                if (!mItems.empty() && PtInRect(mSelectionRegion, pt))
                {
                    // Compute the index of the clicked item

                    //int nClicked;
                    //mDialog.GetManager()->GetFontNode(p
                    //nClicked = int(mScrollBar->GetTrackPos() + (pt.y - mTextRegion.top) / m_fTextHeight);

                    // Only proceed if the click falls on top of an item.

                    /*if (nClicked >= mScrollBar->GetTrackPos() &&
                    nClicked < (int)mItems.size() &&
                    nClicked < mScrollBar->GetTrackPos() + mScrollBar->GetPageSize())
                    {
                    //SetCapture(GetHWND());
                    mDrag = true;

                    // If this is a double click, fire off an event and exit
                    // since the first click would have taken care of the selection
                    // updating.
                    //TODO: handle doubleclicking
                    if (uMsg == WM_LBUTTONDBLCLK)
                    {
                    mDialog.SendEvent(EVENT_LISTBOX_ITEM_DBLCLK, true, this);
                    return true;
                    }

                    m_nSelected = nClicked;
                    if (!(param3 & GLFW_MOD_SHIFT))
                    m_nSelStart = m_nSelected;*/

                    // If this is a multi-selection listbox, update per-item
                    // selection data.

                    Index currSelectedIndex = 0;

                    //the easy way
                    for (unsigned int it = 0; it < mItems.size(); ++it)
                    {
                        if (it != 0 && mItems[it]->mTextRegion.top <= pt.y && mItems[it - 1]->mTextRegion.bottom >= pt.y)
                        {
                            long halfDistance = (mItems[it - 1]->mTextRegion.bottom - mItems[it]->mTextRegion.top) / 2;
                            long relative = pt.y - mItems[it - 1]->mTextRegion.bottom;
                            currSelectedIndex = (relative > halfDistance) ? it : it - 1;
                        }
                        if (PtInRect(mItems[it]->mTextRegion, pt))
                        {
                            currSelectedIndex = it;
                            break;
                        }
                    }

                    if (mStyle & MULTISELECTION)
                    {
                        // Determine behavior based on the state of Shift and Ctrl

                        //ListBoxItemPtr pSelItem = mItems[currSelectedIndex];
                        if (param3 & GLFW_MOD_CONTROL)
                        {
                            // Control click. Reverse the selection of this item.

                            //pSelItem->bSelected = !pSelItem->bSelected;
                            auto it = std::find(mSelected.begin(), mSelected.end(), currSelectedIndex);
                            if (it == mSelected.end())
                            {
                                mSelected.push_back(currSelectedIndex);
                            }
                            else
                            {
                                mSelected.erase(it);//this should never fail

                                //make sure that if it is the last one, then add the -1
                                /*if (mSelected.size() == 0)
                                {
                                mSelected.push_back(-1);
                                }*/
                            }
                        }
                        else if (param3 & GLFW_MOD_SHIFT)
                        {
                            // Shift click. Set the selection for all items
                            // from last selected item to the current item.
                            // Clear everything else.

                            //if (mSelected[0] == -1)
                            //    mSelected[0] = 0;//this just fixes any issues with accidently keeping -1 in here

                            Index nBegin = mSelected[0];
                            Index nEnd = currSelectedIndex;

                            mSelected.clear();

                            /*for (int i = 0; i < nBegin; ++i)
                            {
                            ListBoxItemPtr pItem = mItems[i];
                            pItem->bSelected = false;
                            }

                            for (int i = nEnd + 1; i < (int)mItems.size(); ++i)
                            {
                            ListBoxItemPtr pItem = mItems[i];
                            pItem->bSelected = false;
                            }*/

                            if (nBegin < nEnd)
                            {
                                for (uint32_t i = nBegin; i <= nEnd; ++i)
                                {
                                    /*ListBoxItemPtr pItem = mItems[i];
                                    pItem->bSelected = true;
                                    */
                                    mSelected.push_back(i);
                                }
                            }
                            else if (nBegin > nEnd)
                            {
                                for (uint32_t i = nBegin; i >= nEnd; --i)
                                {
                                    /*ListBoxItemPtr pItem = mItems[i];
                                    pItem->bSelected = true;
                                    */
                                    mSelected.push_back(i);

                                    if (i == 0)
                                        break;
                                }
                            }
                            /*else
                            {
                            mSelected.push_back(-1);
                            */
                        }
                        else if (param3 & (GLFW_MOD_SHIFT | GLFW_MOD_CONTROL))
                        {
                            //No one uses shift control anyway (i see no use in it

                            // Control-Shift-click.

                            // The behavior is:
                            //   Set all items from m_nSelStart to m_nSelected to
                            //     the same state as m_nSelStart, not including m_nSelected.
                            //   Set m_nSelected to selected.

                            /*int nBegin = mSelected[0];
                            int nEnd = currSelectedIndex;
                            mSelected.clear();

                            // The two ends do not need to be set here.

                            bool bLastSelected = mItems[nBegin]->bSelected;
                            for (int i = nBegin + 1; i < nEnd; ++i)
                            {
                            ListBoxItemPtr pItem = mItems[i];
                            pItem->bSelected = bLastSelected;

                            mSelected.push_back(i);
                            }

                            pSelItem->bSelected = true;*/

                            // Restore m_nSelected to the previous value
                            // This matches the Windows behavior

                            //m_nSelected = m_nSelStart;
                        }
                        else
                        {
                            // Simple click.  Clear all items and select the clicked
                            // item.


                            /*for (int i = 0; i < (int)mItems.size(); ++i)
                            {
                            ListBoxItemPtr pItem = mItems[i];
                            pItem->bSelected = false;
                            }

                            pSelItem->bSelected = true;
                            */

                            mSelected.clear();
                            mSelected.push_back(currSelectedIndex);

                            //NOTE: clicking not on an item WILL lead to a clearing of the selection
                        }
                    }  // End of multi-selection case
                    else
                    {
                        mSelected[0] = currSelectedIndex;
                    }

                    //always make sure we have one
                    //if (mSelected.size() == 0)
                    //    mSelected.push_back(-1);

                    //sort it for proper functionality when using shift-clicking (NOT HELPFUL)
                    //std::sort(mSelected.begin(), mSelected.end());

                    mDialog.SendEvent(EVENT_LISTBOX_SELECTION, true, shared_from_this());


                    return true;
                }
                break;
            }
            else
            {
                //TODO: drag click
                //ReleaseCapture();
                /*mDrag = false;

                if (mSelected[0] != -1)
                {
                // Set all items between m_nSelStart and m_nSelected to
                // the same state as m_nSelStart
                int nEnd = mSelected[mSelected.size() - 1];

                for (int n = mSelected[0] + 1; n < nEnd; ++n)
                mItems[n]->bSelected = mItems[mSelected[0]]->bSelected;
                mItems[mSelected[0]]->bSelected = mItems[mSelected[0]]->bSelected;

                // If m_nSelStart and m_nSelected are not the same,
                // the user has dragged the mouse to make a selection.
                // Notify the application of this.
                if (mSelected[0] != mSelected[mSelected.size() - 1])
                mDialog.SendEvent(EVENT_LISTBOX_SELECTION, true, this);

                mDialog.SendEvent(EVENT_LISTBOX_SELECTION_END, true, this);
                }*/
                return false;
            }

        case CURSOR_POS:
            /*if (mDrag)
            {
            // Compute the index of the item below cursor

            int nItem = -1;
            for (unsigned int it = 0; it < mItems.size(); ++it)
            {
            if (PtInRect(mItems[it]->mActiveRegion, pt))
            {
            nItem = it;
            break;
            }
            }

            // Only proceed if the cursor is on top of an item.

            if (nItem >= (int)mScrollBar->GetTrackPos() &&
            nItem < (int)mItems.size() &&
            nItem < mScrollBar->GetTrackPos() + mScrollBar->GetPageSize())
            {
            mSelected[0] = nItem;
            mDialog.SendEvent(EVENT_LISTBOX_SELECTION, true, this);
            }
            else if (nItem < (int)mScrollBar->GetTrackPos())
            {
            // User drags the mouse above window top
            mScrollBar->Scroll(-1);
            mSelected[0] = mScrollBar->GetTrackPos();
            mDialog.SendEvent(EVENT_LISTBOX_SELECTION, true, this);
            }
            else if (nItem >= mScrollBar->GetTrackPos() + mScrollBar->GetPageSize())
            {
            // User drags the mouse below window bottom
            mScrollBar->Scroll(1);
            mSelected[0] = std::min((int)mItems.size(), mScrollBar->GetTrackPos() +
            mScrollBar->GetPageSize()) - 1;
            mDialog.SendEvent(EVENT_LISTBOX_SELECTION, true, this);
            }
            }*/
            break;

        case SCROLL:
            //UINT uLines = 0;
            //if (!SystemParametersInfo(SPI_GETWHEELSCROLLLINES, 0, &uLines, 0))
            //    uLines = 0;
            //int nScrollAmount = int((short)HIWORD(wParam)) / WHEEL_DELTA * uLines;
            mScrollBar->Scroll(-(param2 / _WHEEL_DELTA));
            return true;
    }

    return false;
}

//--------------------------------------------------------------------------------------
void ListBox::UpdateItemRects() noexcept
{
    FontNodePtr pFont = mDialog.GetFont(GetElement(0).mFontIndex);
    if (pFont)
    {
        int curY = mTextRegion.top - mVerticalMargin;
        int nRemainingHeight = RectHeight(mRegion) - 2 * mVerticalMargin;


        //int nRemainingHeight = RectHeight(mRegion) - pFont->mLeading;

        //for all of the ones before the displayed, just set them to something impossible
        for (size_t i = 0; i < (size_t)mScrollBar->GetTrackPos(); ++i)
        {
            SetRect(mItems[i]->mTextRegion, 0, 0, 0, 0);
        }
        for (size_t i = mScrollBar->GetTrackPos(); i < mItems.size(); i++)
        {
            ListBoxItemPtr pItem = mItems[i];

            // Make sure there's room left in the box
            nRemainingHeight -= pFont->mLeading;
            if (nRemainingHeight - (int)mVerticalMargin < 0)
            {
                pItem->mVisible = false;
                continue;
            }

            pItem->mVisible = true;

            SetRect(pItem->mTextRegion, mRegion.left + mHorizontalMargin, curY, mRegion.right - mHorizontalMargin, curY - pFont->mFontType->mHeight);
            curY -= pFont->mLeading;
        }
    }
}

//--------------------------------------------------------------------------------------
void ListBox::Render(float elapsedTime) noexcept
{
    if (mVisible == false)
        return;

    Element* pElement = &mElements[0];
    pElement->mTextureColor.Blend(STATE_NORMAL, elapsedTime);
    pElement->mFontColor.Blend(STATE_NORMAL, elapsedTime);

    Element& pSelElement = mElements[1];
    pSelElement.mTextureColor.Blend(STATE_NORMAL, elapsedTime);
    pSelElement.mFontColor.Blend(STATE_NORMAL, elapsedTime);

    mDialog.DrawSprite(*pElement, mRegion, _FAR_BUTTON_DEPTH);

    FontNodePtr pFont = mDialog.GetFont(pElement->mFontIndex);
    // Render the text
    if (!mItems.empty() && pFont)
    {

        UpdateItemRects();

        static bool bSBInit;
        if (!bSBInit)
        {
            // Update the page size of the scroll bar
            if (mTextHeight > 0)
                mScrollBar->SetPageSize(int((RectHeight(mRegion) - (2 * mVerticalMargin)) / mTextHeight) + 1);
            else
                mScrollBar->SetPageSize(0);
            bSBInit = true;
        }


        for (int i = mScrollBar->GetTrackPos(); i < (int)mItems.size(); ++i)
        {

            ListBoxItemPtr pItem = mItems[i];

            if (!pItem->mVisible)
                continue;

            // Determine if we need to render this item with the
            // selected element.
            bool bSelectedStyle = false;

            //mItems[i]->bSelected = false;

            if (!(mStyle & MULTISELECTION) && i == mSelected[0])
                bSelectedStyle = true;
            else if (mStyle & MULTISELECTION)
            {
                for (auto it : mSelected)
                    if (i == it)
                        bSelectedStyle = true;


                /*if (mDrag &&
                ((i >= mSelected[0] && i < mSelected[0]) ||
                (i <= mSelected[0] && i > mSelected[0])))
                bSelectedStyle = mItems[mSelected[0]]->bSelected;
                else if (pItem->bSelected)
                bSelectedStyle = true;*/
            }

            //bSelectedStyle = mItems[i]->bSelected;
            if (bSelectedStyle)
            {
                //rcSel.top = rc.top; rcSel.bottom = rc.bottom;
                Rect activeRect = pItem->mTextRegion;

                //add 1 here in order to make SURE the selection will be seamless
                InflateRect(activeRect, 0, pFont->mLeading / 4 + 1);
                mDialog.DrawSprite(pSelElement, activeRect, _NEAR_BUTTON_DEPTH);
                mDialog.DrawText(pItem->mText, pSelElement, pItem->mTextRegion);
            }
            else
                mDialog.DrawText(pItem->mText, *pElement, pItem->mTextRegion);

            //OffsetRect(rc, 0, m_fTextHeight);
        }

    }

    // Render the scroll bar

    mScrollBar->Render(elapsedTime);
}


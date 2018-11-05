#ifndef OBJGLUF_COMBOBOX_H
#define OBJGLUF_COMBOBOX_H

/*
ComboBox

    Parents:
        'Button'

    Data Members:
        'mSelected': the index of the selected item
        'mFocused': the index of the focused item (i.e. when opened, the one the mouse is over)
        'mDropHeight': how far down the combo box list drops
        'mScrollBar': the scroll bar for scrolling through items
        'mSBWidth': the width of the scroll bar
        'mOpened': is the combo box opened?
        'mTextRegion': the rect which the text occupies
        'mButtonRegion': the rect which the button to drop down occupies
        'mDropdownRegion': the region of the drop-down occupies
        'mDropdownTextRegion': the region the drop-down text occupies
        'mItems': the items within the combo box

*/
class ComboBox : public Button
{
protected:

    ComboBox() = delete;
    ComboBox(Dialog& dialog);
    friend std::shared_ptr<ComboBox> CreateComboBox(Dialog& dialog);

    sIndex mSelected;
    sIndex mFocused;
    Size mDropHeight;
    ScrollBarPtr mScrollBar;
    Size mSBWidth;

    bool mOpened;

    Rect mTextRegion;
    Rect mButtonRegion;
    Rect mDropdownRegion;
    Rect mDropdownTextRegion;

    std::vector <ComboBoxItemPtr> mItems;

public:
    virtual         ~ComboBox();


    /*
    Setters and Getters

        Note:
            everything is measured in pixels

        Throws:
            'GetItemData': 'std::invalid_argument': if no item has string text 'text' or 'std::out_of_range' if 'index' is too big
            'GetItem': 'std::out_of_range': if 'index' is too big or if 'text' is not found
            'SetSelectedByIndex': 'std::out_of_range': if 'index' is too big
            'SetSelectedByText': 'std::invalid_argument': if 'text' belongs to no item
            'SetSelectedByData': 'std::invalid_argument': if 'data' belongs to no item

    */

    GenericData&    GetItemData(const std::wstring& text, Index start = 0) const;
    GenericData&    GetItemData(Index index) const;
    Size            GetScrollBarWidth() const noexcept    { return mSBWidth;          }
    Size            GetNumItems() const    noexcept        { return mItems.size();        }
    ComboBoxItemPtr GetItem(const std::wstring& text, Index start = 0) const;
    ComboBoxItemPtr GetItem(Index index) const        { return mItems[index]; }

    void                SetDropHeight(Size nHeight)                { mDropHeight = nHeight; UpdateRects(); }
    void                SetScrollBarWidth(Size width) noexcept  { mSBWidth = width; UpdateRects();      }

    /*
    AddItem

        Parameters:
            'text': the text for the item
            'data': the data for the item to represent

        Throws:
            no-throw guarantee
    */
    void AddItem(const std::wstring& text, GenericData& data) noexcept;

    /*
    InsertItem

        Note:
            Just like 'AddItem' except does not have to append
            If 'index' > size - 1, then the item will just be appended

        Parameters:
            'index': the index to insert at
            'text': the text for the item
            'data': the data for the item to represent

        Throws:
            no-throw guarantee

    */
    void InsertItem(Index index, const std::wstring& text, GenericData& data) noexcept;//IMPLEMENT

    /*
    RemoveItem

        Parameters:
            'index': the index to remove, which then makes all elements after move back one

        Throws:
            'std::out_of_range': if 'index' is too big in _DEBUG

    */
    void RemoveItem(Index index);

    /*
    RemoveAllItems

        Note:
            This removes all of the items

        Throws:
            no-throw guarantee

    */
    void RemoveAllItems() noexcept;

    /*
    GetSelectedIndex

        Returns:
            the selected index

        Throws:
            'NoItemSelectedException': if no item is selected

    */
    Index GetSelectedIndex() const;

    /*
    GetSelectedItem

        Returns:
            the object representing of the selected item

        Throws:
            'NoItemSelectedException': if no item is selected

    */
    ListBoxItemPtr GetSelectedItem() const;

    /*
    GetSelectedData

        Returns:
            the data of the selected item

        Throws:
            'NoItemSelectedException': if no item is selected

    */
    GenericData& GetSelectedData() const;

    /*
    SelectItem

        Parameters:
            'index': the index of the item to select
            'text': the text of the item to select
            'start': the starting point to look for 'text'
            'data': the data to find

        Throws:
            'std::out_of_range': if index is too big in _DEBUG
            'std::invalid_argument': if 'text' is not found in _DEBUG, or if 'data' is not found in _DEBUG
    */
    void SelectItem(Index index);
    void SelectItem(const std::wstring& text, Index start = 0);
    void SelectItem(const GenericData& data);

    /*
    ContainsItem

        Parameters:
            'text': the text representing the potential item
            'start': the starting index of the search

        Returns:
            whether or not an item exists with the text 'text'

        Throws:
            no-throw guarantee

    */
    bool ContainsItem(const std::wstring& text, Index start = 0) const noexcept;

    /*
    FindItem(Index)

        Parameters:
            'text': the text representing the index of the item to find
            'start': the starting index of the search

        Returns:
            The index of the found item
            The first item found with text 'text'


        Throws:
            'std::invalid_argument': if no item is found

    */
    Index           FindItemIndex(const std::wstring& text, Index start = 0) const;
    ComboBoxItemPtr FindItem(const std::wstring& text, Index start = 0) const;


    /*
    Overridden Unambiguous Member Functions


    */
    virtual bool MsgProc(MessageType msg, int32_t param1, int32_t param2, int32_t param3, int32_t param4) noexcept override;
    virtual void OnHotkey() noexcept override;
    virtual bool CanHaveFocus() const noexcept override{ return (mVisible && mEnabled); }
    virtual void OnFocusOut() noexcept override;
    virtual void Render(float elapsedTime) noexcept override;
    virtual void UpdateRects() noexcept override;
    virtual void OnInit() override;
    virtual void SetTextColor(const Color& Color) noexcept override;
    virtual bool ContainsPoint(const Point& pt) const noexcept override;

protected:

    /*
    UpdateItemRects

        Note:
            Updates internal item rects for various reasons, including scrolling causing some items to not be on the
                screen anymore, resizing, item addition/removal, etc.

        Throws:
            no-throw guarantee

    */
    void UpdateItemRects() noexcept;
};
#endif //OBJGLUF_COMBOBOX_H

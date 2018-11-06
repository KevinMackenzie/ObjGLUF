#ifndef OBJGLUF_LISTBOX_H
#define OBJGLUF_LISTBOX_H

namespace GLUF {

/*
ListBox

    Parents:
        'Control'

    Data Members:
        'mTextRegion': the bounding box for the text
        'mSelectionRegion': the bounding box of the selected elements
        'mScrollBar': the scroll bar for the box
        'mSBWidth': the width of the scroll bar
        'mVerticalMargin': The added space above and below the list box when rendered
        'mHorizontalMargin': The added space to the left and right of the list box when rendered
        'mTextHeight': height of a single line of text
        'mStyle': style of list box (bitfield)
        'mSelected': the indices of the selected list box items
        'mDrag': whether the user is dragging the mouse to select multiple items

*/
class ListBox : public Control {
protected:

    ListBox() = delete;
    ListBox(Dialog &dialog);
    friend std::shared_ptr <ListBox> CreateListBox(Dialog &dialog);


    Rect mTextRegion;
    Rect mSelectionRegion;
    ScrollBarPtr mScrollBar;
    Size mSBWidth;
    Size mVerticalMargin;
    Size mHorizontalMargin;
    FontSize mTextHeight;
    Bitfield mStyle;
    std::vector <Index> mSelected;
    bool mDrag;
    std::vector <ListBoxItemPtr> mItems;

public:
    virtual ~ListBox();

    enum ListBoxStyle {
        MULTISELECTION = 0x01
    };


    /*
    Setters and Getters

        Note:
            everything is measured in pixels

        Throws:
            'GetSelectedData': 'NoItemSelectedException': if no item is selected
            'GetItemData': 'std::invalid_argument': if no item has string text 'text' or 'std::out_of_range' if 'index' is too big
            'GetItem': 'std::out_of_range': if 'index' is too big
            'SetSelectedByIndex': 'std::out_of_range': if 'index' is too big
            'SetSelectedByText': 'std::invalid_argument': if 'text' belongs to no item
            'SetSelectedByData': 'std::invalid_argument': if 'data' belongs to no item

    */

    GenericData &GetItemData(const std::wstring &text, Index start) const;
    GenericData &GetItemData(Index index) const;

    Size GetNumItems() const noexcept { return mItems.size(); }

    ListBoxItemPtr GetItem(const std::wstring &text, Index start = 0) const;

    ListBoxItemPtr GetItem(Index index) const { return mItems[index]; }

    Bitfield GetStyle() const noexcept { return mStyle; }

    Size GetScrollBarWidth() const noexcept { return mSBWidth; }

    void SetStyle(Bitfield style) noexcept { mStyle = style; }

    void SetScrollBarWidth(Size width) noexcept {
        mSBWidth = width;
        UpdateRects();
    }

    void SetMargins(Size vertical, Size horizontal) noexcept {
        mVerticalMargin = vertical;
        mHorizontalMargin = horizontal;
    }

    /*
    AddItem

        Parameters:
            'text': the text for the item
            'data': the data for the item to represent

        Throws:
            no-throw guarantee
    */
    void AddItem(const std::wstring &text, GenericData &data) noexcept;

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
    void InsertItem(Index index, const std::wstring &text, GenericData &data) noexcept;

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

        Note:
            For single-selection listbox, returns the index of the selected item.
            For multi-selection, returns the first selected item after the 'previousSelected' position.

        Parameters:
            'previouslySelected': the previously selected index

        Returns:
            the selected index

        Throws:
            'NoItemSelectedException': if no item is selected

    */
    Index GetSelectedIndex(Index previousSelected) const;//for multi-line
    Index GetSelectedIndex() const;//for single-line (or finding the first selected item)

    /*
    GetSelectedItem

        Parameters:
            'previouslySelected': if multi-selection, the previously selected item

        Returns:
            the object representing of the selected item or next selected item in multi-selection

        Throws:
            'NoItemSelectedException': if no item is selected

    */
    ListBoxItemPtr GetSelectedItem(Index previousSelected) const { return GetItem(GetSelectedIndex(previousSelected)); }

    ListBoxItemPtr GetSelectedItem() const { return GetItem(GetSelectedIndex()); }

    /*
    GetSelectedData

        Parameters:
            'previouslySelected': if multi-selection, the previously selected item

        Returns:
            the data of the selected item or next selected item in multi-selection

        Throws:
            'NoItemSelectedException': if no item is selected

    */
    GenericData &GetSelectedData(Index previousSelected) const {
        return GetItemData(GetSelectedIndex(previousSelected));
    }

    GenericData &GetSelectedData() const { return GetItemData(GetSelectedIndex()); }

    /*
    SelectItem

        Note:
            for single-selection listbox, sets the currently selected
            for multi-selection listbox, adds 'newIndex' to the list of selected items, will only add the first item with 'text' found

        Parameters:
            'index': the index of the item to add to the selected list
            'text': the text of the item to add to the selected list
            'start': the starting point to look for 'text'

        Throws:
            'std::out_of_range': if index is too big in _DEBUG
            'std::invalid_argument': if 'text' is not found in _DEBUG
    */
    void SelectItem(Index index);
    void SelectItem(const std::wstring &text, Index start = 0);

    /*
    ClearSelected

        Note:
            clears all of the selected Items

        Throws:
            no-throw guarantee

    */
    void ClearSelected() noexcept;

    /*
    RemoveSelected

        Note:
            Removes 'index' from the selected list

        Parameters:
            'index': the index to remove from the selected list
            'text': the text of the item in the selected list to remove from said list
            'start': the starting index of the selected list to start looking for 'text'

        Throws:
            'std::invalid_argument': if 'index' does not exist, in _DEBUG
    */
    void RemoveSelected(Index index);
    void RemoveSelected(const std::wstring &text, Index start = 0);

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
    bool ContainsItem(const std::wstring &text, Index start = 0) const noexcept;

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
    Index FindItemIndex(const std::wstring &text, Index start = 0) const;
    ListBoxItemPtr FindItem(const std::wstring &text, Index start = 0) const;


    /*
    Overrided Unambiguous Member Functions


    */
    virtual bool MsgProc(MessageType msg, int32_t param1, int32_t param2, int32_t param3, int32_t param4) noexcept override;

    virtual void OnInit() override {
        mDialog.InitControl(std::dynamic_pointer_cast<Control>(mScrollBar));
        UpdateRects();
    }

    virtual bool CanHaveFocus() const noexcept override { return (mVisible && mEnabled); }

    virtual void Render(float elapsedTime) noexcept override;
    virtual void UpdateRects() noexcept override;

    virtual bool ContainsPoint(const Point &pt) const noexcept override {
        return Control::ContainsPoint(pt) || mScrollBar->ContainsPoint(pt);
    }

protected:

    /*
    UpdateItemRects

        Note:
            Updates internal item rects for various reasons, including scrolling causing some items to not be on the
                screen anymore, resizing, item addition/removal, etc.

        Throws:
            no-throw guarantee

    */
    virtual void UpdateItemRects() noexcept;
};
}
#endif //OBJGLUF_LISTBOX_H

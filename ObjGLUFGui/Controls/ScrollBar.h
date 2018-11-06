#ifndef OBJGLUF_SCROLLBAR_H
#define OBJGLUF_SCROLLBAR_H

namespace GLUF {

/*
ScrollBar

    Parents:
        Control

    Data Members:
        'mShowThumb': show the thumbnail (grabbable scroll piece)
        'mDrag': is the thumbail currently grabbed
        'mUpButtonRegion': the region of the up button
        'mDownButtonRegion': the region of the down button
        'mTrackRegion': the region of the track which the thumbail goes on
        'mThumbRegion': the region of the thumbnail
        'mPosition': the position along the scroll bar
        'mPageSize': how many items are displayable in one page
        'mStart': the first item
        'mEnd': the index after the last item
        'mPreviousMousePos': used to detect drag events
        'mArrow': the state of the arrows
        'mArrowTS': the timestamp of the previous arrow event
*/
class ScrollBar : public Control {
public:

    /*
    ArrowState

        'CLEAR':            No arrow is down.
        'CLICKED_UP':       Up arrow is clicked.
        'CLICKED_DOWN':     Down arrow is clicked.
        'HELD_UP':          Up arrow is held down for sustained period.
        'HELD_DOWN':        Down arrow is held down for sustained period.
    */
    enum ArrowState {
        CLEAR,
        CLICKED_UP,
        CLICKED_DOWN,
        HELD_UP,
        HELD_DOWN
    };
protected:

    ScrollBar() = delete;
    ScrollBar(Dialog &dialog);
    friend std::shared_ptr <ScrollBar> CreateScrollBar(Dialog &dialog);


    bool mShowThumb;
    bool mDrag;
    Rect mUpButtonRegion;
    Rect mDownButtonRegion;
    Rect mTrackRegion;
    Rect mThumbRegion;
    Value mPosition;
    Value mPageSize;
    Value mStart;
    Value mEnd;
    Point mPreviousMousePos;
    ArrowState mArrow;
    double mArrowTS;

public:
    virtual         ~ScrollBar();


    /*
    Setters and Getters

        Throws:
            no-throw guarantee

    */
    void SetTrackRange(int nStart, int nEnd) noexcept;

    void SetTrackPos(int nPosition) noexcept {
        mPosition = nPosition;
        Cap();
        UpdateThumbRect();
    }

    void SetPageSize(int nPageSize) noexcept {
        mPageSize = nPageSize;
        Cap();
        UpdateThumbRect();
    }

    int GetTrackPos() const noexcept { return mPosition; }

    int GetPageSize() const noexcept { return mPageSize; }

    /*
    Scroll

        Parameters:
            'delta': the change in position
    */
    void Scroll(int delta);


    /*
    ShowItem

        Note:
            This makes sure 'index' is displayed, and will scroll if necessary

        Parameters:
            'index': the item to make sure is in the viewable region

    */
    void ShowItem(int index);


    /*
    Overridden Unambiguous Member Functions

    */
    virtual bool MsgProc(MessageType msg, int32_t param1, int32_t param2, int32_t param3, int32_t param4) noexcept override;
    virtual void Render(float elapsedTime) noexcept override;
    virtual void UpdateRects() noexcept override;

protected:

    /*
    UpdateThumbRect

        Note:
            This resizes the thumbnail based on how many items there are, and where the thumbnail is based on 'mPosition'

    */
    void UpdateThumbRect();


    /*
    Cap

        Note:
            This acts like a clamp function; it ensures 'mPosition' is within legal range

    */
    void Cap();
};
}
#endif //OBJGLUF_SCROLLBAR_H

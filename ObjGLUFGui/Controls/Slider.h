#ifndef OBJGLUF_SLIDER_H
#define OBJGLUF_SLIDER_H

/*
Slider

    Parents:
        'Control'

    Data Members:
        'mValue': the value between 'mMin' and 'mMax' where the slider is at
        'mMin': the minimum value the slider can have
        'mMax': the maximum value the slider can have
        'mDragX': the mouse position at the start of the drag
        'mDragOffset': the difference between the 'mDragX' and the current mouse position; updated every frame
        'mButtonX': the X value of the button position in pixels
        'mPressed': is the button pressed to be dragged
        'mButtonRegion': the region the button occupies

*/
class Slider : public Control
{
protected:

    Slider() = delete;
    Slider(Dialog& dialog);
    friend std::shared_ptr<Slider> CreateSlider(Dialog& dialog);

    Value mValue;
    Value mMin;
    Value mMax;
    Value mDragX;
    Value mDragOffset;
    Value mButtonX;
    bool mPressed;
    Rect mButtonRegion;

public:

    /*
    Setters and Getters

        Throws:
            no-throw guarantee

    */
    Value   GetValue() const noexcept           { return mValue;                    }
    void        GetRange(Value& nMin, Value& nMax) const noexcept
    { nMin = mMin; nMax = mMax;         }
    void        SetValue(Value nValue) noexcept { SetValueInternal(nValue, false);  }
    void        SetRange(Value  nMin, Value  nMax) noexcept;

    /*
    Overridden Unambiguous Member Functions


    */
    virtual bool MsgProc(MessageType msg, int32_t param1, int32_t param2, int32_t param3, int32_t param4) noexcept override;
    virtual bool ContainsPoint(const Point& pt) const noexcept override;
    virtual bool CanHaveFocus()const noexcept override{ return (mVisible && mEnabled); }
    virtual void UpdateRects() noexcept override;
    virtual void Render(float elapsedTime) noexcept override;

protected:
    /*
    SetValueInternal

        Note:
            sets the position of the slider, and throws an event if it is from the user

        Parameters:
            'value': the value to set the slider to
            'fromInput': whether the this value came from the user or not

        Throws:
            no-throw guarantee
    */
    void      SetValueInternal(int nValue, bool bFromInput) noexcept;

    /*
    ValueFromXPos

        Note:
            Gets the value to set the slider given a pixel X coordinate

        Parameters:
            'xPos': the screen position in pixels

        Returns:
            The value to set the slider from the given X coordinate

        Throws:
            no-throw guarantee

    */
    Value ValueFromXPos(Value xPos) const noexcept;

};
#endif //OBJGLUF_SLIDER_H

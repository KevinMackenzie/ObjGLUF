#ifndef OBJGLUF_CHECKBOX_H
#define OBJGLUF_CHECKBOX_H

/*
CheckBox

    Parents:
        Button

    Data Members:
        'mChecked': is the box checked
        'mButtonRegion': the region the button covers
        'mTextRegion': the region the text covers

*/
class CheckBox : public Button
{

protected:

    CheckBox() = delete;
    CheckBox(bool checked, Dialog& dialog);
    friend std::shared_ptr<CheckBox> CreateCheckBox(bool checked, Dialog& dialog);

    bool mChecked;
    Rect mButtonRegion;
    Rect mTextRegion;

public:

    /*
    Setters and Getters

        Throws:
            no-throw guarantee

    */
    bool GetChecked() const noexcept         { return mChecked;                      }
    void SetChecked(bool checked) noexcept   { SetCheckedInternal(checked, false);   }

    /*
    Overridden Unambiguous Member Functions

    */
    virtual bool MsgProc(MessageType msg, int32_t param1, int32_t param2, int32_t param3, int32_t param4) noexcept override;
    virtual void Render(float elapsedTime) noexcept override;
    virtual void OnHotkey() noexcept override;
    virtual bool ContainsPoint(const Point& pt) const noexcept override;
    virtual void UpdateRects() noexcept override;

protected:

    /*
    SetCheckedInternal

        Note:
            sets the state of the box, but sends an event if it was from a user

        Parameters:
            'checked': state of the box
            'fromInput': was this a result of a user event

        Throws:
            no-throw guarantee

    */
    virtual void SetCheckedInternal(bool checked, bool fromInput);
};

#endif //OBJGLUF_CHECKBOX_H

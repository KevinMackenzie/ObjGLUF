#ifndef OBJGLUF_RADIOBUTTON_H
#define OBJGLUF_RADIOBUTTON_H

/*
RadioButton

    Parents:
        CheckBox

    Data Members
        'mButtonGroup': the id of the button group this belongs to in a dialog
*/
class RadioButton : public CheckBox
{
public:

    RadioButton() = delete;
    RadioButton(Dialog& dialog);
    friend std::shared_ptr<RadioButton> CreateRadioButton(Dialog& dialog);

    RadioButtonGroup mButtonGroup;

public:


    /*
    Setters and Getters

        Throws:
            no-throw guarantee
    */
    void            SetChecked(bool checked, bool clearGroup = true) noexcept   { SetCheckedInternal(checked, clearGroup, false);   }
    void            SetButtonGroup(RadioButtonGroup buttonGroup) noexcept   { mButtonGroup = buttonGroup;                       }
    unsigned int    GetButtonGroup() const noexcept                             { return mButtonGroup;                              }


    /*
    Overridden Unambiguous Member Functions

    */
    virtual bool MsgProc(MessageType msg, int32_t param1, int32_t param2, int32_t param3, int32_t param4) noexcept override;
    virtual void OnHotkey() noexcept override;
    virtual void OnMouseEnter() noexcept override;
    virtual void OnMouseLeave() noexcept override;
    virtual void OnFocusIn() noexcept override;
    virtual void OnFocusOut() noexcept override;

protected:

    void OnMouseEnterNoRecurse() noexcept;
    void OnMouseLeaveNoRecurse() noexcept;
    void OnFocusInNoRecurse() noexcept;
    void OnFocusOutNoRecurse() noexcept;

    /*
    SetCheckedInternal

        Note:
            sets the state of the box, and may clear the group; sends an event if from the user

        Parameters:
            'checked': the new state of this box
            'clearGroup': whether or not to clear the button group (usually true)
            'fromInput': whether or not the user triggered this

    */
    virtual void SetCheckedInternal(bool checked, bool clearGroup, bool fromInput);
};
#endif //OBJGLUF_RADIOBUTTON_H

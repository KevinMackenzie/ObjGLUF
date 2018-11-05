#ifndef OBJGLUF_BUTTON_H
#define OBJGLUF_BUTTON_H

/*
Button

    DataMembers:
        'mPressed': boolean state of the button

*/
class Button : public Static
{

protected:

    Button() = delete;
    Button(Dialog& dialog);
    friend std::shared_ptr<Button> CreateButton(Dialog& dialog);

    bool mPressed;

public:

    /*
    Overridden Unambiguous Member Functions

    */
    virtual bool CanHaveFocus()    const noexcept override    { return (mVisible && mEnabled); }
    virtual void Render(float elapsedTime) noexcept override;
    virtual bool ContainsPoint(const Point& pt) const noexcept override{ return PtInRect(mRegion, pt); }
    virtual bool MsgProc(MessageType msg, int32_t param1, int32_t param2, int32_t param3, int32_t param4) noexcept override;
    virtual void OnHotkey() noexcept override;

};

#endif //OBJGLUF_BUTTON_H

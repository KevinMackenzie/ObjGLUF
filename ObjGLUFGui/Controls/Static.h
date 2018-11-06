#ifndef OBJGLUF_STATIC_H
#define OBJGLUF_STATIC_H

namespace GLUF {
/*
Static

    Note:
        A Static Text Control; Draws Text with no backgroud

    Data Members:
        'mText': the text to display
        'mTextFlags': the text formatting flats

*/
class Static : public Control
{
protected:
    std::wstring     mText;
    Bitfield     mTextFlags;

    Static(const Bitfield& textFlags, Dialog& dialog);
    friend std::shared_ptr<Static> CreateStatic(Bitfield textFlags, Dialog& dialog);

    Static() = delete;

public:


    /*
    ContainsPoint

        Note:
            This always returns false, because this control never should receive message

    */
    virtual bool ContainsPoint(const Point& pt) const noexcept override { return false; }

    /*
    Getters and Setters

        Throws:
            no-throw guarantee

    */
    void                  GetTextCopy(std::wstring& dest) const noexcept    { dest = mText;         }
    const std::wstring&   GetText() const noexcept                          { return mText;         }
    void                  SetText(const std::wstring& text) noexcept        { mText = text;         }
    void                  SetTextFlags(Bitfield flags) noexcept         { mTextFlags = flags;   }


    /*
    Overridden Unambiguous Member Functions

    */
    virtual void Render(float elapsedTime) noexcept override;
};
}
#endif //OBJGLUF_STATIC_H

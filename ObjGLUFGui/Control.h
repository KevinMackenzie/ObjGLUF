#ifndef OBJGLUF_CONTROL_H
#define OBJGLUF_CONTROL_H

/*
Control


    Parents:
        'std::enable_shared_from_this': to allow passing 'this' as a shared pointer, however the 'this' pointer
            still needs to be on the heap for this to work, therefore the constructor is private, shared with 'std::make_shared'

    Data Members:
        'mID': the id of the control
        'mType': the control type
        'mHotkey': the virtual key code for this control's hotkey; represented by GLFW's keycodes
        'mEnabled': enable flag
        'mVisible': visibility flag
        'mMouseOver': is mouse over this control
        'mHasFocus': does control have input focus
        'mIsDefault': is this the default control
        'mRegion': the rect which this control occupies
        'mDialog': a pointer to the parent container
        'mIndex': the index of this control within the dialog
        'mElements': the elements making up this control

*/
class Control : public std::enable_shared_from_this<Control>
{
protected:
    /*
    Constructor

        Note:
            Constructor is private to only allow 'std::shared_ptr' to make instances of it, so
                instances of this are only on the heap; this allows for 'std::enable_shared_from_this' to work

        Parameters:
            'dialog': the parent dialog

    */
    Control(Dialog& dialog);

    friend Dialog;

    ControlIndex mID;
    ControlType mType;
    KeyId  mHotkey;

    bool mEnabled;
    bool mVisible;
    bool mMouseOver;
    bool mHasFocus;
    bool mIsDefault;

    Rect mRegion;

    Dialog& mDialog;
    ControlIndex mIndex;

    //use a map, because there may be gaps in indices
    std::map<ElementIndex, Element> mElements;

    FontNodePtr GetElementFont(ElementIndex index);


public:
    virtual ~Control();


    /*
    OnInit

        Throws:
            'ControlInitException': if child class initialization fails

    */
    virtual void OnInit(){ UpdateRects(); }

    /*
    Refresh

        Note:
            Removes focus and mouse-over state, and refreshes all elements

        Throws:
            no-throw guarantee

    */
    virtual void Refresh() noexcept;

    /*
    Render

        Parameters:
            'elapsedTime': the time since the previous frame

        Throws:
            no-throw guarantee: child classes must NOT throw any critical exceptions

    */
    virtual void Render(float elapsedTime) noexcept{};

    /*
    MsgProc

        Parameters:
            see 'EventCallbackFuncPtr' for details

        Throws:
            no-throw guarantee
    */
    virtual bool MsgProc(MessageType msg, int32_t param1, int32_t param2, int32_t param3, int32_t param4) noexcept { return false; }


    /*
    Getters and Setters

        Note:
            Some of these may be overwritten by child classes, but
        Throws:
            GetElement: 'std::out_of_range'
    */
    virtual bool    GetEnabled() const noexcept            { return mEnabled;              }
    virtual bool    GetVisible() const noexcept            { return mVisible;              }
    int                GetID() const noexcept                { return mID;                   }
    int                GetHotkey()    const noexcept            { return mHotkey;               }
    ControlType     GetType() const    noexcept            { return mType;                 }
    virtual bool    CanHaveFocus() const noexcept        { return false;                    }
    Rect            GetRegion() const noexcept          { return mRegion;               }
    Element&          GetElement(ElementIndex element);

    virtual void    SetEnabled(bool enabled) noexcept            { mEnabled = enabled;                       }
    virtual void    SetVisible(bool visible) noexcept            { mVisible = visible;                       }
    virtual void    SetRegion(const Rect& region) noexcept
    { mRegion = region;                         }
    void            SetLocation(long x, long y) noexcept        { RepositionRect(mRegion, x, y);        }
    void            SetSize(long width, long height) noexcept   { ResizeRect(mRegion, width, height);   }
    void            SetHotkey(KeyId hotkey) noexcept        { mHotkey = hotkey;                         }
    void            SetID(int ID) noexcept                        { mID = ID;                                 }

    bool IsMouseOver()  const noexcept   { return mMouseOver; }
    bool IsFocused()    const noexcept   { return mHasFocus; }

    /*
    EventHandlers

        Note:
            These are barebones handlers, assuming the control does nothing at all upon these events

        Throws:
            no-throw guarantee; children may modify this behavior, but should not

    */
    virtual bool ContainsPoint(const Point& pt) const noexcept
    { return PtInRect(mRegion, pt);                         }
    virtual void OnFocusIn() noexcept        { mHasFocus = true;                                            }
    virtual void OnFocusOut() noexcept        { mHasFocus = false;                                        }
    virtual void OnMouseEnter() noexcept    { mMouseOver = true;                                        }
    virtual void OnMouseLeave() noexcept    { mMouseOver = false;                                        }
    virtual void OnHotkey() noexcept        { /*this function usually acts like the user 'clicked' it*/ }

    /*
    SetElement

        Note:
            'element' == nullptr, the resulted effect is that element location is deleted

        Parameters:
            'elementId': the id to give the element within this control
            'element': the element to add at that id

        Throws:
            no-throw guarantee

    */
    void SetElement(ElementIndex elementId, const Element& element) noexcept;

    /*
    SetTextColor

        Note:
            Sets the text color for all elements

        Parameters:
            'color': the color to set to

        Throws:
            no-throw guarantee

    */
    virtual void SetTextColor(const Color& color) noexcept;


protected:
    /*
    UpdateRects

        Note:
            This method is used to recreate the regions for different elements when the main rect changes

    */
    virtual void UpdateRects() noexcept{}
};

#endif //OBJGLUF_CONTROL_H

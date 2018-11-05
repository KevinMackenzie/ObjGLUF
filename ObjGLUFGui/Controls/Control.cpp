#include "Control.h"

Control::Control(Dialog& dialog) : mDialog(dialog)
{
    mType = CONTROL_BUTTON;
    mID = 0;
    mHotkey = 0;
    mIndex = 0;

    mEnabled = true;
    mVisible = true;
    mMouseOver = false;
    mHasFocus = false;
    mIsDefault = false;

    mRegion = { { 0 }, 0, 0, { 0 } };
}

FontNodePtr Control::GetElementFont(ElementIndex index)
{
    return mDialog.GetFont(mElements[index].mFontIndex);
}

//--------------------------------------------------------------------------------------
Control::~Control()
{
}


//--------------------------------------------------------------------------------------
Element& Control::GetElement(ElementIndex element)
{
    return mElements[element];
}

//--------------------------------------------------------------------------------------
void Control::SetTextColor(const Color& color) noexcept
{
    Element& element = mElements[0];

    element.mFontColor.mStates[STATE_NORMAL] = color;
}


//--------------------------------------------------------------------------------------
void Control::SetElement(ElementIndex elementId, const Element& element) noexcept
{
    mElements[elementId] = element;
}


//--------------------------------------------------------------------------------------
void Control::Refresh() noexcept
{
    mMouseOver = false;
    mHasFocus = false;

    for (auto it : mElements)
    {
        it.second.Refresh();
    }
}


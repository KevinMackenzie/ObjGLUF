#include "Static.h"

namespace  GLUF {

//--------------------------------------------------------------------------------------
Static::Static(const Bitfield &textFlags, Dialog &dialog) : Control(dialog), mTextFlags(textFlags) {
    mType = CONTROL_STATIC;
}


//--------------------------------------------------------------------------------------
void Static::Render(float elapsedTime) noexcept {
    if (!mVisible)
        return;

    ControlState state = STATE_NORMAL;

    if (mEnabled == false)
        state = STATE_DISABLED;

    Element &element = mElements[0];
    element.mTextFormatFlags = mTextFlags;

    element.mFontColor.Blend(state, elapsedTime);

    mDialog.DrawText(mText, element, mRegion, false, false);
}

}

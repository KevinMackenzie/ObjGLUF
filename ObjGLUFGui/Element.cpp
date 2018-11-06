#include "Element.h"

namespace GLUF {

//--------------------------------------------------------------------------------------
void Element::SetTexture(TextureIndex textureIndex, const Rectf &uvRect, const Color &defaultTextureColor) {
    mTextureIndex = textureIndex;
    mUVRect = uvRect;
    mTextureColor.Init(defaultTextureColor);
}


//--------------------------------------------------------------------------------------
void Element::SetFont(FontIndex font, const Color &defaultFontColor, Bitfield textFormat) {
    mFontIndex = font;
    mFontColor.Init(defaultFontColor);
    mTextFormatFlags = textFormat;
}


//--------------------------------------------------------------------------------------
void Element::Refresh() {
    //mTextureColor.SetCurrent(STATE_HIDDEN);
    //mFontColor.SetCurrent(STATE_HIDDEN);
}

}

#ifndef OBJGLUF_ELEMENT_H
#define OBJGLUF_ELEMENT_H




/*
Element

    Note:
        Contains all the display tweakables for a sub-control

    Data Members:
        'mTextureIndex': the index of the texture for this element
        'mFontIndex': the index of the font for this element
        'mTextFormatFlags': a bitfield of formatting flags for the text for this element
        'mUVRect': Bounding rect for this element on the composite texture (Given in UV coords of the texture)
        'mTextureColor': the blend color for the texture for the element
        'mFontColor': the blend color for the font for this element
*/
class Element
{
public:

    TextureIndex mTextureIndex;
    FontIndex mFontIndex;
    Bitfield mTextFormatFlags;

    Rectf mUVRect;

    BlendColor mTextureColor;
    BlendColor mFontColor;

    /*
    SetTexture

        Parameters:
            'textureIndex': the index of the texture within the dialog resource manager to use
            'uvRect': the UV coordinates of the element within the texture
            'defaultTextureBlendColor': the default texture blend color

    */
    void    SetTexture(TextureIndex textureIndex, const Rectf& uvRect, const Color& defaultTextureColor = { 255, 255, 255, 255 });

    /*
    SetFont

        Parameters:
            'font': the font for this element to use
            'defaultFontColor': the default color for the font
            'textFormat': a bitfield of the horizontal and vertical text formatting

    */
    void    SetFont(FontIndex font, const Color& defaultFontColor = { 255, 255, 255, 255 }, Bitfield textFormat = GT_CENTER | GT_VCENTER);

    /*
    Refresh

        Note:
            This function sets the current blend states to the 'hidden' state
    */
    void    Refresh();
};


/*
ElementHolder

    Note:
        This class is used for default element data structures

    Data Members:
        'mControlType': which control type is this element for
        'mElementIndex': index of the element within the default elements
        'mElement': the element itself

*/
struct ElementHolder
{
    ControlType mControlType;
    ElementIndex mElementIndex;
    Element mElement;
};
#endif //OBJGLUF_ELEMENT_H

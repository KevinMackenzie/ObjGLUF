#include "Text.h"

#include <ft2build.h>
#include FT_FREETYPE_H"freetype/freetype.h"


struct TextVertexStruct : public VertexStruct
{
    glm::vec3 mPos;
    glm::vec2 mTexCoords;

    TextVertexStruct(){}
    TextVertexStruct(const glm::vec3& pos, const glm::vec2& texCoords) :
            mPos(pos), mTexCoords(texCoords)
    {}

    virtual char* get_data() const override
    {
        char* ret = new char[size()];

        memcpy(ret, &mPos[0], 12);
        memcpy(ret + 12, &mTexCoords[0], 8);

        return ret;
    }

    virtual size_t size() const override
    {
        return 20; // sizeof(mPos) + sizeof(mTexCoords);
    }

    virtual size_t n_elem_size(size_t element)
    {
        switch (element)
        {
            case 0:
                return 12;
            case 1:
                return 8;
            default:
                return 0;//if it is too big, just return 0; not worth an exception
        }
    }

    virtual void buffer_element(void* data, size_t element) override
    {
        switch (element)
        {
            case 0:
                mPos = static_cast<glm::vec3*>(data)[0];
            case 1:
                mTexCoords = static_cast<glm::vec2*>(data)[0];
            default:
                break;
        }
    }

    static GLVector<TextVertexStruct> MakeMany(size_t howMany)
    {
        GLVector<TextVertexStruct> ret;
        ret.resize(howMany);

        return ret;
    }
};

FT_Library g_FtLib;
ProgramPtr g_TextProgram = nullptr;
VertexArrayPtr g_TextVertexArray = nullptr;
struct TextShaderLocations_t
{
    GLuint position = 0;
    GLuint uv = 0;
    GLuint ortho = 0;
    GLuint color = 0;
    GLuint sampler = 0;

}g_TextShaderLocations;

std::string g_TextShaderVert =
        "#version 120                                                        \n"\
"attribute vec3 _Position;                                            \n"\
"attribute vec2 _UV;                                                \n"\
"uniform mat4 _Ortho;                                                \n"\
"varying vec2 uvCoord;                                                \n"\
"void main(void)                                                    \n"\
"{                                                                    \n"\
"   uvCoord = /*abs(vec2(0.0f, 1.0f) - */_UV/*)*/;                    \n"\
"    gl_Position = vec4(_Position, 1.0f) * _Ortho;                    \n"\
"}                                                                    \n";

std::string g_TextShaderFrag =
        "#version 120                                                        \n"\
"uniform vec4 _Color;                                                \n"\
"uniform sampler2D _TS;                                                \n"\
"varying vec2 uvCoord;                                                \n"\
"void main(void)                                                    \n"\
"{                                                                    \n"\
"    vec4 Color;                                                        \n"\
"    Color.a = texture2D(_TS, uvCoord).r;                            \n"\
"    Color.rgb = _Color.rgb;                                            \n"\
"    Color.a *= _Color.a;                                            \n"\
"    gl_FragColor = Color;                                            \n"\
"}                                                                    \n";

bool InitText() {

    ShaderSourceList sources;
    sources.insert({ SH_VERTEX_SHADER, g_TextShaderVert });
    sources.insert({ SH_FRAGMENT_SHADER, g_TextShaderFrag });
    SHADERMANAGER.CreateProgram(g_TextProgram, sources);

    g_TextShaderLocations.position        = SHADERMANAGER.GetShaderVariableLocation(g_TextProgram, GLT_ATTRIB, "_Position");
    g_TextShaderLocations.uv            = SHADERMANAGER.GetShaderVariableLocation(g_TextProgram, GLT_ATTRIB, "_UV");
    g_TextShaderLocations.color            = SHADERMANAGER.GetShaderVariableLocation(g_TextProgram, GLT_UNIFORM, "_Color");
    g_TextShaderLocations.ortho            = SHADERMANAGER.GetShaderVariableLocation(g_TextProgram, GLT_UNIFORM, "_Ortho");
    g_TextShaderLocations.sampler        = SHADERMANAGER.GetShaderVariableLocation(g_TextProgram, GLT_UNIFORM, "_TS");

    g_TextVertexArray = std::make_shared<VertexArray>(GL_TRIANGLES, GL_STREAM_DRAW, true);
    g_TextVertexArray->AddVertexAttrib({ 4, 3, g_TextShaderLocations.position, GL_FLOAT, 0 });
    g_TextVertexArray->AddVertexAttrib({ 4, 2, g_TextShaderLocations.uv, GL_FLOAT, 12 });

    //initialize the freetype library.
    FT_Error err = FT_Init_FreeType(&g_FtLib);
    if (err)
    {
        GLUF_ERROR("Failed to Initialize the Freetype Library!");
        return false;
    }

    return true;
}


/*
======================================================================================================================================================================================================
Font Stuff


*/


//--------------------------------------------------------------------------------------
void SetDefaultFont(FontPtr& pDefFont)
{
    g_DefaultFont = pDefFont;
}

/*
CharacterInfo

    Data Members:
        'mAdvance': advance of glyph
        'mBitSize': {bitmap.width, bitmap.rows}
        'mBitLoc': {bitmap_left, bitmap_top}; location of bitmap within texture
        'mTexXOffset': the x offset of glyph in texture coordinates

*/
struct CharacterInfo
{
    glm::u32vec2 mAdvance;
    glm::u32vec2 mBitSize;
    glm::u32vec2 mBitLoc;
    float mTexXOffset;
    float mTexYOffset;
};


/*
Font

    Todo:
        Setup with languages

    Data Members:
        'mFtFont': the freetype font instance
        'mHeight': the height of the font
        'mAtlasSize': the width and height of the atlas
        'mCharAtlas': the character atlas
        'mCharacterOffset': the offset within the charset to start creating the texture, default to 32, because that is where ascii characters start
        'mCharacterEnd': the end of the writable characters
        'mTexLineBreakIndices': the character codes which caused line breaks within the texture
*/
class Font
{
    std::vector<uint32_t> mTexLineBreakIndices;
public:

    FT_Face mFtFont;
    FontSize mHeight;
    glm::u32vec2 mAtlasSize;
    GLuint mTexId = 0;
    std::vector<CharacterInfo> mCharAtlas;
    glm::uint32 mCharacterOffset = 32;
    glm::uint32 mCharacterEnd = 0xFFF;


    /*
    GetCharWidth

        Note:
            Gets the character width in pixles, which is usually wider than the character, usually use 'GetCharAdvance' instead

        Parameters:
            'ch': the character to get from

        Throws:
            'std::out_of_range': if ch is not within mCharacterOffset and mCaracterEnd

    */
    FontSize GetCharWidth(wchar_t ch);

    /*
    GetCharHeight

        Parameters:
            'ch': the character to get from

        Throws:
            'std::out_of_range': if ch is not within mCharacterOffset and mCaracterEnd

    */
    FontSize GetCharHeight(wchar_t ch);

    /*
    GetCharAdvance

        Note:
            Gets the distance between this character and the next character when drawing, i.e. the 't' character will have a smaller advance than width

        Parameters:
            'ch': the character to get from

        Throws:
            'std::out_of_range': if ch is not within mCharacterOffset and mCaracterEnd

    */
    FontSize GetCharAdvance(wchar_t ch);

    /*
    GetStringWidth

        Note:
            This is a gets how wide the string is.  This adds the advance of each character together for the width


        Throws:
            'std::out_of_range': if any characters within the string are not within mCharacterOffset and mCharacterEnd

    */
    FontSize GetStringWidth(const std::wstring& str);

    /*
    Init

        Note:
            Creates a font from a set of data

        Parameters:
            'data': the data to be created from
            'fontHeight': the height of the font to load

        Throws:
            'LoadFontException': if loading failed
    */
    void Init(const std::vector<char>& data, FontSize fontHeight);

    /*
    Refresh

        Note:
            This should be called when the window is resized

        Throws:
            no-throw guarantee

    */
    void Refresh() noexcept;

    /*
    GetChar*Rect

        Note:
            Gets the window pixel rect or normalized rect within the texture respectively

        Parameters:
            'ch': the character to lookup

        Throws:
            'std::out_of_range': if ch is not within mCharacterOffset and mCaracterEnd

    */
    Rect GetCharRect(wchar_t ch);
    Rectf GetCharTexRect(wchar_t ch);

};

FontSize GetFontHeight(FontPtr font)
{
    return font->mHeight;
}

void Font::Refresh() noexcept
{

    //reset variables
    mAtlasSize = { 0, 0 };


    //int mult = (g_WndHeight >= g_WndWidth) ? g_WndWidth : g_WndHeight;

    int pxlHeight = mHeight;

    if (FT_Set_Char_Size(mFtFont, pxlHeight << 6, pxlHeight << 6, 96, 96))
    {
        GLUF_ERROR("Size setting Failed");
    }

    FT_GlyphSlot g = mFtFont->glyph;

    //get maximum texture size, so we don't get opengl complaining
    GLint maxTexSize = 1024;
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxTexSize);

    //add one line of character texture to start
    uint32_t lineHeightWithSpacing = static_cast<uint32_t>((4.f / 3.f) * static_cast<float>(mHeight));
    mAtlasSize.y += lineHeightWithSpacing;
    uint32_t atlasWidthTmp = 0;

    //load the characters' dimmensions
    for (unsigned int i = mCharacterOffset; i < mCharacterEnd; i++)
    {
        if (FT_Load_Char(mFtFont, i, FT_LOAD_RENDER))
        {
            GLUF_ERROR("Loading character failed!\n");
            continue;
        }

        //on each line, make sure we check if the next texture will go past the max x position
        glm::uint32 potentialAtlasX = atlasWidthTmp + g->bitmap.width + GLYPH_PADDING;
        if (potentialAtlasX > maxTexSize)
        {
            mAtlasSize.y += lineHeightWithSpacing;
            if (mAtlasSize.y > maxTexSize)
            {
                //if it is too big, actually clamp the characters which can not be loaded (not good, but not catestrophic either)
                mCharacterEnd = i - 1;
                break;
            }

            mTexLineBreakIndices.push_back(i);
            atlasWidthTmp = 0;
        }
        else
        {
            atlasWidthTmp = potentialAtlasX;
            mAtlasSize.x = glm::max(atlasWidthTmp, mAtlasSize.x);
        }

        //mAtlasSize.y = std::max(mAtlasSize.y, static_cast<glm::uint32>(g->bitmap.rows));

    }

    //resize after getting the texture size, so if characters were omitted due to insufficient texture size, the atlas is of appropriate size
    mCharAtlas.resize(mCharacterEnd - mCharacterOffset);

    //generate textures
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, mTexId);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    // Fonts should be rendered at native resolution so no need for texture filtering
    //float fLargest = 0.0f;
    //glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &fLargest);
    //    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, fLargest);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    // Stop chararcters from bleeding over edges
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

    GLfloat transparent[] = { 0.0f, 0.0f, 0.0f, 0.0f };
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, transparent);//any overflow will be transparent

    glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, mAtlasSize.x, mAtlasSize.y, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, 0);

    //load texture data

    //the data for the glyph padding
    std::vector<char> paddingData;
    paddingData.resize(GLYPH_PADDING * lineHeightWithSpacing, 0);

    int x = 0;
    int y = 0;
    unsigned int lineIndex = 0;
    for (unsigned int i = mCharacterOffset; i < mCharacterEnd; ++i)
    {
        if (std::find(std::begin(mTexLineBreakIndices), std::end(mTexLineBreakIndices), i) != std::end(mTexLineBreakIndices))
        {
            y += lineHeightWithSpacing;
            x = 0;
            lineIndex = 0;
        }

        unsigned int p = i - mCharacterOffset;

        if (FT_Load_Char(mFtFont, i, FT_LOAD_RENDER))
            continue;

        if (i == 32/*space*/)
        {
            FT_UInt spId = FT_Get_Char_Index(mFtFont, 32);
            if (FT_Load_Glyph(mFtFont, spId, FT_LOAD_RENDER))
                continue;

            mCharAtlas[p].mAdvance.x = (mFtFont->glyph->advance.x >> 6);
            mCharAtlas[p].mAdvance.y = 0L;

            mCharAtlas[p].mBitSize.x = mCharAtlas[p].mAdvance.x;//this is useful in the edit box.
            mCharAtlas[p].mBitSize.y = mAtlasSize.y;

            mCharAtlas[p].mBitLoc.x = 0L;
            mCharAtlas[p].mBitLoc.y = 0L;

            mCharAtlas[p].mTexXOffset = static_cast<float>(mAtlasSize.x - GLYPH_PADDING);
            mCharAtlas[p].mTexYOffset = static_cast<float>(y) / static_cast<float>(mAtlasSize.y);

            continue;
        }

        glTexSubImage2D(GL_TEXTURE_2D, 0, x + (lineIndex * GLYPH_PADDING), y, g->bitmap.width, g->bitmap.rows, GL_LUMINANCE, GL_UNSIGNED_BYTE, g->bitmap.buffer);

        //this adds padding to keep the characters looking clean
        glTexSubImage2D(GL_TEXTURE_2D, 0, x + (lineIndex * GLYPH_PADDING) + g->bitmap.width, y, GLYPH_PADDING, mAtlasSize.y, GL_LUMINANCE, GL_UNSIGNED_BYTE, paddingData.data());

        mCharAtlas[p].mTexXOffset = static_cast<float>(x + (lineIndex * GLYPH_PADDING)) / static_cast<float>(mAtlasSize.x);
        mCharAtlas[p].mTexYOffset = (static_cast<float>(y)+0.5f) / static_cast<float>(mAtlasSize.y);

        x += g->bitmap.width;

        mCharAtlas[p].mAdvance.x = (g->advance.x >> 6);
        mCharAtlas[p].mAdvance.y = (g->advance.y >> 6);

        mCharAtlas[p].mBitSize.x = g->bitmap.width;
        mCharAtlas[p].mBitSize.y = g->bitmap.rows;

        mCharAtlas[p].mBitLoc.x = g->bitmap_left;
        mCharAtlas[p].mBitLoc.y = g->bitmap_top;

        ++lineIndex;
    }

    //FT_Done_Face(mFtFont);
    g = 0;
}

void Font::Init(const std::vector<char>& data, FontSize fontHeight)
{
    mHeight = fontHeight;

    if (FT_New_Memory_Face(g_FtLib, (const FT_Byte*)data.data(), (FT_Long)data.size(), 0, &mFtFont))
        throw LoadFontException();

    glGenTextures(1, &mTexId);

    if (mTexId == 0)
    {
        throw LoadFontException();
    }

    Refresh();
}

Rect Font::GetCharRect(wchar_t ch)
{
    if (ch < mCharacterOffset || ch >= mCharacterEnd)
    {
        return{ { 0 }, 0, 0, { 0 } };
    }

    Rect rc = { 0, GetCharHeight(ch), GetCharWidth(ch), 0 };

    if (ch < mCharacterOffset)
        return rc;
    //OffsetRect(rc, 0.0f, mCharAtlas[ch - mCharacterOffset].ay);

    //if there is a dropdown, make sure it is accounted for
    //float dy = ((mCharAtlas[ch - mCharacterOffset].bh - mCharAtlas[ch - mCharacterOffset].bt) / mAtlasHeight) * mHeight;
    //OffsetRect(rc, (mCharAtlas[ch - mCharacterOffset].bl * GetCharHeight(ch)) / mAtlasHeight, dy);
    OffsetRect(rc, mCharAtlas[ch - mCharacterOffset].mBitLoc.x, -(long)(mCharAtlas[ch - mCharacterOffset].mBitSize.y - mCharAtlas[ch - mCharacterOffset].mBitLoc.y));
    return rc;
}

/*Rect Font::GetCharRectNDC(wchar_t ch)
{
    Rect rc = { 0.0f, 2.0f * GetCharHeight(ch), 2.0f * GetCharWidth(ch), 0.0f };
    //OffsetRect(rc, 0.0f, mCharAtlas[ch - mCharacterOffset].ay);

    //if there is a dropdown, make sure it is accounted for
    float dy = ((mCharAtlas[ch - mCharacterOffset].bh - mCharAtlas[ch - mCharacterOffset].bt) / mAtlasHeight) * mHeight;
    OffsetRect(rc, (mCharAtlas[ch - mCharacterOffset].bl * GetCharHeight(ch)) / mAtlasHeight, -2 * dy);
    return rc;
}*/

Rectf Font::GetCharTexRect(wchar_t ch)
{
    if (ch < mCharacterOffset || ch >= mCharacterEnd)
    {
        return{ 0, 0, 0, 0 };
    }

    float l = 0, t = 0, r = 0, b = 0;

    l = mCharAtlas[ch - mCharacterOffset].mTexXOffset;
    t = mCharAtlas[ch - mCharacterOffset].mTexYOffset;
    r = mCharAtlas[ch - mCharacterOffset].mTexXOffset + static_cast<float>(GetCharWidth(ch)) / static_cast<float>(mAtlasSize.x);
    b = static_cast<float>(mCharAtlas[ch - mCharacterOffset].mBitSize.y) / static_cast<float>(mAtlasSize.y) + mCharAtlas[ch - mCharacterOffset].mTexYOffset;

    return{ l, t, r, b };
};

FontSize Font::GetCharAdvance(wchar_t ch)
{
    if (ch < mCharacterOffset || ch >= mCharacterEnd)
    {
        return 0;
    }

    return mCharAtlas[ch - mCharacterOffset].mAdvance.x;
}

FontSize Font::GetCharWidth(wchar_t ch)
{
    if (ch < mCharacterOffset || ch >= mCharacterEnd)
    {
        return 0;
    }

    return mCharAtlas[ch - mCharacterOffset].mBitSize.x;
}

FontSize Font::GetCharHeight(wchar_t ch)
{
    if (ch < mCharacterOffset || ch >= mCharacterEnd)
    {
        return 0;
    }

    return mCharAtlas[ch - mCharacterOffset].mBitSize.y;
}

FontSize Font::GetStringWidth(const std::wstring& str)
{
    /*for (auto ch : str)
    {
        if (ch < mCharacterOffset || ch >= mCharacterEnd)
        {
            GLUF_NON_CRITICAL_EXCEPTION(std::out_of_range("Character Not In Atlas!"));
        }
    }
    */
    FontSize tmp = 0;
    for (auto it : str)
    {
        tmp += GetCharAdvance(it);
    }
    return tmp;
}


void LoadFont(FontPtr& font, const std::vector<char>& rawData, FontSize fontHeight)
{
    font = std::make_shared<Font>();

    font->Init(rawData, fontHeight);
}


//glm::mat4 g_TextModelMatrix;
//GLVector<TextVertexStruct> g_TextVertices;
//Color4f g_TextColor;

//--------------------------------------------------------------------------------------
void BeginText(const glm::mat4& orthoMatrix)
{
    g_TextOrtho = orthoMatrix;
    //g_TextVertices.clear();
}

//--------------------------------------------------------------------------------------
void DrawText(const FontNodePtr& font, const std::wstring& text, const Rect& rect, const Color& color, Bitfield textFlags, bool hardRect)
{

    if ((long)font->mFontType->mHeight > RectHeight(rect) && hardRect)
        return;//no sense rendering if it is too big

    //rcScreen = ScreenToClipspace(rcScreen);

    FontSize tmpSize = font->mFontType->mHeight; // _FONT_HEIGHT_NDC(font->mFontType->mHeight);

    Rectf UV;

    Rect rcScreen = rect;
    OffsetRect(rcScreen, -long(g_WndWidth / 2), -long(g_WndHeight / 2));

    long CurX = rcScreen.left;
    long CurY = rcScreen.top;

    //calc widths
    long strWidth = font->mFontType->GetStringWidth(text);
    int centerOffset = (RectWidth(rcScreen) - strWidth) / 2;
    if (textFlags & GT_CENTER)
    {
        CurX = rcScreen.left + centerOffset;
    }
    else if (textFlags & GT_RIGHT)
    {
        CurX = rcScreen.left + centerOffset * 2;
    }

    int numLines = 1;//always have one to get the GT_VCENTER correct
    for (auto it : text)
    {
        if (it == L'\n')
            numLines++;
    }

    if (textFlags & GT_VCENTER)
    {
        long value = RectHeight(rcScreen);
        value = value - numLines * font->mFontType->mHeight;
        value /= 2;
        CurY -= (RectHeight(rcScreen) - (long)numLines * (long)font->mFontType->mHeight) / 2;
    }
    else if (textFlags & GT_BOTTOM)
    {
        CurY -= RectHeight(rcScreen) - numLines * font->mFontType->mHeight;
    }


    //get the number of vertices
    GLsizei textBuffLen = text.size() * 4;

    auto textVertices = TextVertexStruct::MakeMany(textBuffLen);
    std::vector<glm::u32vec3> indices;
    indices.resize(text.size() * 2);

    float z = _NEAR_BUTTON_DEPTH;
    unsigned int i = 0;
    for (auto ch : text)
    {
        int widthConverted = font->mFontType->GetCharAdvance(ch);//(font->mFontType->CellX * tmpSize) / font->mFontType->mAtlasWidth;

        //lets support newlines :) (or if the next char will go outside the rect)
        if (ch == '\n' || (CurX + widthConverted > rcScreen.right && hardRect))
        {
            if (textFlags & GT_CENTER)
                CurX = rcScreen.left + centerOffset;
            else if (textFlags & GT_LEFT)
                CurX = rcScreen.left;
            else if (textFlags & GT_RIGHT)
                CurX = rcScreen.left + centerOffset * 2;

            CurY -= font->mLeading;// *1.1f;//assume a reasonible leding

            //if the next line will go off of the page, then don't draw it
            if ((CurY - (long)font->mLeading < rcScreen.bottom) && hardRect)
                break;

            if (ch == '\n')
            {
                continue;
            }
        }

        //Row = (ch - font->mFontType->Base) / font->mFontType->RowPitch;
        //Col = (ch - font->mFontType->Base) - Row*font->mFontType->RowPitch;

        //U = Col*font->mFontType->ColFactor;
        //V = Row*font->mFontType->RowFactor;
        //U1 = U + font->mFontType->ColFactor;
        //V1 = V + font->mFontType->RowFactor;

        //U = font->mFontType->GetTextureXOffset(ch);
        //V = 0.0f;
        //U1 = U + font->mFontType->GetCharWidth(ch);
        //V1 = 1.0f;
        UV = font->mFontType->GetCharTexRect(ch);

        //glTexCoord2f(U, V1);  glVertex2i(CurX, CurY);
        //glTexCoord2f(U1, V1);  glVertex2i(CurX + font->mFontType->CellX, CurY);
        //glTexCoord2f(U1, V); glVertex2i(CurX + font->mFontType->CellX, CurY + font->mFontType->CellY);
        //glTexCoord2f(U, V); glVertex2i(CurX, CurY + font->mFontType->CellY);

        Rect glyph = font->mFontType->GetCharRect(ch);

        //remember to expand for this
        //glyph.right = _FONT_HEIGHT_NDC(glyph.right);
        //glyph.top   = _FONT_HEIGHT_NDC(glyph.right);

        OffsetRect(glyph, CurX, CurY - (tmpSize));

        //glyph.left = CurX;
        //glyph.right = CurX + widthConverted;
        //glyph.top = CurY;
        //glyph.bottom = CurY - tmpSize;

        unsigned int vertI = i * 4;

        textVertices[vertI] =
                {
                        glm::vec3(GetVec2FromRect(glyph, false, false), z),
                        GetVec2FromRect(UV, false, false)
                };

        textVertices[vertI + 1] =
                {
                        glm::vec3(GetVec2FromRect(glyph, true, false), z),
                        GetVec2FromRect(UV, true, false)
                };

        textVertices[vertI + 2] =
                {
                        glm::vec3(GetVec2FromRect(glyph, true, true), z),
                        GetVec2FromRect(UV, true, true)
                };

        textVertices[vertI + 3] =
                {
                        glm::vec3(GetVec2FromRect(glyph, false, true), z),
                        GetVec2FromRect(UV, false, true)
                };

        unsigned int indexI = i * 2;
        indices[indexI] =
                {
                        vertI + 3, vertI, vertI + 2
                };

        indices[indexI + 1] =
                {
                        vertI + 2, vertI, vertI + 1
                };

        CurX += widthConverted;

        ++i;
    }
    //glEnd();

    //g_TextColor = ColorToFloat(color);

    //g_TextModelMatrix = glm::translate(glm::mat4(), glm::vec3(0.5f, 1.5f, 0.0f));//glm::mat4(1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.3f, 0.3f, 0.0f, 1.0f);

    //buffer the data
    g_TextVertexArray->BufferData(textVertices);

    //buffer the indices
    g_TextVertexArray->BufferIndices(indices);

    EndText(font->mFontType, g_TextVertexArray, color, g_TextOrtho);

}


void EndText(const FontPtr& font, const VertexArrayPtr& data, const Color& textColor, const glm::mat4& projMatrix)
{
    SHADERMANAGER.UseProgram(g_TextProgram);

    //first uniform: model-view matrix
    SHADERMANAGER.GLUniformMatrix4f(g_TextShaderLocations.ortho, projMatrix);

    //second, the sampler
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, font->mTexId);
    SHADERMANAGER.GLUniform1i(g_TextShaderLocations.sampler, 0);


    //third, the color
    SHADERMANAGER.GLUniform4f(g_TextShaderLocations.color, ColorToFloat(textColor));

    //make sure to enable this with text
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    data->Draw();

    //g_TextVertices.clear();
}


//TextHelper
TextHelper::TextHelper(DialogResourceManagerPtr& manager) :
        mManager(manager), mColor(0, 0, 0, 255), mPoint(0L, 0L),
        mFontIndex(0), mFontSize(15L)
{}

void TextHelper::Begin(FontIndex drmFont, FontSize leading, FontSize size)
{
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_CLAMP);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    mManager->GetFontNode(drmFont);

    mFontIndex = drmFont;
    mFontSize = size;
    mLeading = leading;

    Text::BeginText(mManager->GetOrthoMatrix());
}

void TextHelper::DrawTextLine(const std::wstring& text) noexcept
{
    DrawTextLineBase({ { mPoint.x }, mPoint.y, mPoint.x + 50L, { mPoint.y - 50L } }, GT_LEFT | GT_TOP, text);

    //set the point down however many lines were drawn
    for (auto it : text)
    {
        if (it == '\n')
            mPoint.y += mLeading;
    }
    mPoint.y -= mLeading;//once no matter what because we are drawing a LINE of text
}

void TextHelper::DrawTextLineBase(const Rect& rc, Bitfield flags, const std::wstring& text) noexcept
{
    mManager->GetFontNode(mFontIndex)->mLeading = mLeading;
    Text::DrawText(mManager->GetFontNode(mFontIndex), text, rc, mColor, flags, true);
}

void TextHelper::End() noexcept
{
    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_CLAMP);//set this back because it is the default
}

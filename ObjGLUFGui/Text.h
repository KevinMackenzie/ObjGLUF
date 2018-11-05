#ifndef OBJGLUF_TEXT_H
#define OBJGLUF_TEXT_H


//WIP, support more font options eg. stroke, italics, variable leading, etc.

/*
FontNode

    Note:
        Work in progress; support for more typographical options in the future, i.e.:
            stroke, italics, leading, etc.

    Data Members:
        'mWeight': WIP, doesn't do anything yet
        'mLeading': WIP, doesn't do anything yet
        'mFontType': an opaque pointer to the font to use, defaults to arial

*/
struct FontNode
{
    //FontSize mSize;
    FontWeight mWeight;
    FontSize mLeading;
    FontPtr mFontType;
};

//WIP
enum FontWeight
{
    FONT_WEIGHT_HAIRLINE = 0,
    FONT_WEIGHT_THIN,
    FONT_WEIGHT_ULTRA_LIGHT,
    FONT_WEIGHT_EXTRA_LIGHT,
    FONT_WEIGHT_LIGHT,
    FONT_WEIGHT_BOOK,
    FONT_WEIGHT_NORMAL,
    FONT_WEIGHT_MEDIUM,
    FONT_WEIGHT_SEMI_BOLD,
    FONT_WEIGHT_BOLD,
    FONT_WEIGHT_EXTRA_BOLD,
    FONT_WEIGHT_HEAVY,
    FONT_WEIGHT_BLACK,
    FONT_WEIGHT_EXTRA_BLACK,
    FONT_WEIGHT_ULTRA_BLACK
};

/*
======================================================================================================================================================================================================
Everything Fonts

*/

using FontPtr = std::shared_ptr<Font>;
using FontSize = uint32_t;//in 'points'

//TODO: support dpi scaling
#define _POINTS_PER_PIXEL 1.333333f
#define _POINTS_TO_PIXELS(points) (FontSize)((float)points * _POINTS_PER_PIXEL)
#define _PICAS_TO_POINTERS(picas) ((picas) * 6.0)


OBJGLUF_API void SetDefaultFont(FontPtr& pDefFont);


class LoadFontException : public Exception
{
public:
    virtual const char* what() const noexcept
    {
        return "Error Loading Freetype Font";
    }

    EXCEPTION_CONSTRUCTOR(LoadFontException);
};

/*
LoadFont

    Parameters:
        'font': an uninitialized font
        'rawData': the raw data to load font from
        'fontHeight': how tall should the font be in points?

    Throws:
        'LoadFontException': if font loading failed

*/
OBJGLUF_API void LoadFont(FontPtr& font, const std::vector<char>& rawData, FontSize fontHeight);

OBJGLUF_API FontSize GetFontHeight(const FontPtr& font);

/*
TextHelper

    Note:
        This class's purpose is to provide a way to draw text outside of a dialog

    Data Members:
        'mColor': the text color
        'mPoint': where to start drawing the text
        'mManager': a reference to the dialog resource manager
        'mFontIndex': the index of the font within the DRM
        'mFontSize': the font size in points
        'mLeading': the leading of the text
*/
class OBJGLUF_API TextHelper
{
protected:

    TextHelper() = delete;
    TextHelper(DialogResourceManagerPtr& drm);
    friend std::shared_ptr<TextHelper> CreateTextHelper(DialogResourceManagerPtr& drm);


    DialogResourceManagerPtr& mManager;

    /*
    Helper overloads for RenderString

    */

    template<typename T1, typename... Types>
    static void RenderText(std::wstringstream& formatStream, std::wstringstream& outString, const T1& arg1, const Types&... args);

    template<typename T1>
    static void RenderText(std::wstringstream& formatStream, std::wstringstream& outString, const T1& arg);

    FontIndex mFontIndex;
    FontSize mFontSize;
    FontSize mLeading;

    public:
    Color mColor;
    Point mPoint;


    ~TextHelper(){};

    /*
    Begin

        Note:
            Call this before calling any text drawing calls

        Parameters:
            'drmFont': the index of the font to use
            'size': the size of font to draw

        Throws:
            'std::out_of_range': if 'drmFont': is out of the range within the DRM
    */
    void Begin(FontIndex drmFont, FontSize leading, FontSize size);

    /*
    DrawFormattedTextLine

        Note:
            this replaces each % with a the next argument, more features will come in the future

        Parameters:
            'format': the format and text
            'args': the data to put in 'format'
            'rc': the rect to render the text within
            'flags': the flags for drawing

        Throws:
            no-throw guarantee
    */
    template<class... Types>
    void DrawFormattedTextLine(const std::wstring& format, const Types&... args) noexcept;

    template<class... Types>
    void DrawFormattedTextLineBase(const Rect& rc, Bitfield flags, const std::wstring& format, const Types&... args) noexcept;

    /*
    DrawTextLine

        Note:
            The first will automatically bring down the next line to draw, the second will not

        Parameters:
            'text': the text to draw
            'rc': the rect to render the text within
            'flags': the flags for drawing within the rect (i.e. GT_CENTER)

        Throws:
            no-throw guarantee
    */
    void DrawTextLine(const std::wstring& text) noexcept;
    void DrawTextLineBase(const Rect& rc, Bitfield flags, const std::wstring& text) noexcept;

    /*
    End

        Note:
            This actually renders the text; the previous functions just buffer it

        Throws:
            no-throw guarantee

    */
    void End() noexcept;

    /*
    RenderText

        Note:
            This is a very simple version of vsprintf with no formatting; each % will be replaced with the next value

        Parameters:
            'format': the format to take
            'outString': where the output will be stored
            'args': the parameters to insert into 'format'

        Throws:
            no-throw guarantee


    */
    template<class... Types>
    static void RenderText(const std::wstring& format, std::wstring& outString, const Types&... args) noexcept;
};

#endif //OBJGLUF_TEXT_H

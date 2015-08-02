#include "stdafx.h"
#include "GLUFGui.h"
//#include "CBFG/BitmapFontClass.h"
#include <ft2build.h> 
#include FT_FREETYPE_H

#include <algorithm>

#ifdef WIN32
#pragma warning( disable : 4715 )
#endif

namespace GLUF
{

/*

Random Text Helping functions

================================================= TODO: ==================================================
    Relocate these methods into a more sensible spot/container/namespace/whatever
        Probably in a source file instead of a header file
*/
namespace Text
{
    void BeginText(const glm::mat4& orthoMatrix);


    void DrawText(const FontNodePtr& font, const std::wstring& text, const Rect& rect, const Color& color, Bitfield textFlags, bool hardRect = false);
    void EndText(const FontPtr& font);

}

//this defines the space in pixels between glyphs in the font
#define GLYPH_PADDING 5

// Minimum scroll bar thumb size
#define SCROLLBAR_MINTHUMBSIZE 8

// Delay and repeat period when clicking on the scroll bar arrows
#define SCROLLBAR_ARROWCLICK_DELAY  0.33f
#define SCROLLBAR_ARROWCLICK_REPEAT 0.05f

#define _NEAR_BUTTON_DEPTH -0.6f
#define _FAR_BUTTON_DEPTH -0.8f

#define _MAX_GUI_SPRITES 500
#define WHEEL_DELTA 400//TODO:

//this is just a constant to be a little bit less windows api dependent (TODO: make this a setting)
unsigned int GetCaretBlinkTime()
{
	return 400;
}


/*
======================================================================================================================================================================================================
GLFW Callbacks


*/

void MessageProcedure(MessageType, int, int, int, int);

/*
===================================================================================================
GLFW Window Callbacks

*/

//--------------------------------------------------------------------------------------
void GLFWWindowPosCallback(GLFWwindow*, int x, int y)
{
	MessageProcedure(POS, x, y, 0, 0);
}

//--------------------------------------------------------------------------------------
void GLFWWindowSizeCallback(GLFWwindow*, int width, int height)
{
	MessageProcedure(RESIZE, width, height, 0, 0);
}

//--------------------------------------------------------------------------------------
void GLFWWindowCloseCallback(GLFWwindow*)
{
	MessageProcedure(CLOSE, 0, 0, 0, 0);
}
/*
//--------------------------------------------------------------------------------------
void GLFWWindowRefreshCallback(GLFWwindow*)
{
	MessageProcedure(REFRESH, 0, 0, 0, 0);
}*/

//--------------------------------------------------------------------------------------
void GLFWWindowFocusCallback(GLFWwindow*, int focused)
{
	MessageProcedure(FOCUS, focused, 0, 0, 0);
}

//--------------------------------------------------------------------------------------
void GLFWWindowIconifyCallback(GLFWwindow*, int iconified)
{
	MessageProcedure(ICONIFY, iconified, 0, 0, 0);
}

//--------------------------------------------------------------------------------------
void GLFWFrameBufferSizeCallback(GLFWwindow*, int width, int height)
{
	MessageProcedure(FRAMEBUFFER_SIZE, width, height, 0, 0);
}

/*
===================================================================================================
GLFW Input Callback

*/

//--------------------------------------------------------------------------------------
void GLFWMouseButtonCallback(GLFWwindow*, int button, int action, int mods)
{
	MessageProcedure(MB, button, action, mods, 0);
}

//--------------------------------------------------------------------------------------
void GLFWCursorPosCallback(GLFWwindow*, double xPos, double yPos)
{
	MessageProcedure(CURSOR_POS, (int)xPos, (int)yPos, 0, 0);
}

//--------------------------------------------------------------------------------------
void GLFWCursorEnterCallback(GLFWwindow*, int entered)
{
	MessageProcedure(CURSOR_ENTER, entered, 0, 0, 0);
}

//--------------------------------------------------------------------------------------
void GLFWScrollCallback(GLFWwindow*, double xoffset, double yoffset)
{
	MessageProcedure(SCROLL, (int)(xoffset * 1000.0), (int)(yoffset * 1000.0), 0, 0);
}

//--------------------------------------------------------------------------------------
void GLFWKeyCallback(GLFWwindow*, int key, int scancode, int action, int mods)
{
	MessageProcedure(KEY, key, scancode, action, mods);
}

//--------------------------------------------------------------------------------------
void GLFWCharCallback(GLFWwindow*, unsigned int codepoint)
{
	MessageProcedure(UNICODE_CHAR, (int)codepoint, 0, 0, 0);
}


CallbackFuncPtr g_pCallback;

//--------------------------------------------------------------------------------------
void MessageProcedure(MessageType msg, int param1, int param2, int param3, int param4)
{
	if (g_pCallback(msg, param1, param2, param3, param4))
	{

	}

	//todo: anything else to do?
}




/*
======================================================================================================================================================================================================
Various Structs Used For UI


*/
struct ScreenVertex
{
	glm::vec3 pos;
	Color     color;
	glm::vec2 uv;
};

struct ScreenVertexUntex
{
	glm::vec3 pos;
	Color     color;
};

struct TextVertexStruct : public VertexStruct
{
    glm::vec3 mPos;
    glm::vec2 mTexCoords;

    TextVertexStruct(){}
    TextVertexStruct(const glm::vec3& pos, const glm::vec2& texCoords) :
        mPos(pos), mTexCoords(texCoords)
    {}

    virtual void* operator&() const override
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


/*
======================================================================================================================================================================================================
Initialization and Globals


*/


FT_Library g_FtLib;

unsigned short g_WndWidth = 0;
unsigned short g_WndHeight = 0;

FontPtr g_DefaultFont = nullptr;
ProgramPtr g_UIProgram = nullptr;
ProgramPtr g_UIProgramUntex = nullptr;
ProgramPtr g_TextProgram = nullptr;
VertexArrayPtr g_TextVertexArray = nullptr;

GLFWwindow* g_pGLFWWindow;
GLuint g_pControlTexturePtr;
int g_ControlTextureResourceManLocation = -1;



//uniform locations
struct UIShaderLocations_t
{
    GLuint position = 0;
    GLuint color = 0;
    GLuint uv = 0;
    GLuint ortho = 0;
    GLuint sampler = 0;

}g_UIShaderLocations;

struct UIShaderLocationsUntex_t
{
    GLuint position = 0;
    GLuint color = 0;
    GLuint ortho = 0;

}g_UIShaderLocationsUntex;

struct TextShaderLocations_t
{
    GLuint position = 0;
    GLuint uv = 0;
    GLuint ortho = 0;
    GLuint color = 0;
    GLuint sampler = 0;

}g_TextShaderLocations;


/*
======================================================================================================================================================================================================
Shaders for UI Elements


*/

std::string g_UIShaderVert =
"#version 120														\n"\
"attribute vec3 _Position;											\n"\
"attribute vec2 _UV;												\n"\
"attribute vec4 _Color;												\n"\
"uniform   mat4 _Ortho;												\n"\
"varying vec4 Color;												\n"\
"varying vec2 uvCoord;												\n"\
"void main(void)													\n"\
"{																	\n"\
"	gl_Position = vec4(_Position, 1.0f) * _Ortho;					\n"\
"	Color = _Color;		        									\n"\
"   uvCoord = abs(vec2(0.0f, 1.0f) - _UV);							\n"\
"}																	\n";
/*the V's are inverted because the texture is loaded bottom to top*/


std::string g_UIShaderFrag =
"#version 120														\n"\
"varying vec4 Color;												\n"\
"varying vec2 uvCoord;												\n"\
"uniform sampler2D _TS;												\n"\
"void main(void)													\n"\
"{																	\n"\
"	//Color = vec4(1.0f, 0.0, 0.0f, 1.0f);							\n"\
"	//Color = fs_in.Color;											\n"\
"   vec4 oColor = texture2D(_TS, uvCoord);							\n"\
"	oColor = vec4(                                                  \n"\
"       oColor.r * Color.r,                                         \n"\
"       oColor.g * Color.g,                                         \n"\
"       oColor.b * Color.b,                                         \n"\
"       oColor.a * Color.a);	                                    \n"\
"	gl_FragColor = oColor;											\n"\
"}																	\n"; 

std::string g_UIShaderFragUntex =
"#version 120														\n"\
"varying vec4 Color;												\n"\
"varying vec2 uvCoord;												\n"\
"void main(void)													\n"\
"{																	\n"\
"	gl_FragColor = Color;											\n"\
"}																	\n";

std::string g_TextShaderVert =
"#version 120														\n"\
"attribute vec3 _Position;											\n"\
"attribute vec2 _UV;												\n"\
"uniform mat4 _Ortho;												\n"\
"varying vec2 uvCoord;												\n"\
"void main(void)													\n"\
"{																	\n"\
"   uvCoord = /*abs(vec2(0.0f, 1.0f) - */_UV/*)*/;					\n"\
"	gl_Position = vec4(_Position, 1.0f) * _Ortho;					\n"\
"}																	\n";

std::string g_TextShaderFrag =
"#version 120														\n"\
"uniform vec4 _Color;												\n"\
"uniform sampler2D _TS;												\n"\
"varying vec2 uvCoord;												\n"\
"void main(void)													\n"\
"{																	\n"\
"	vec4 Color;														\n"\
"	Color.a = texture2D(_TS, uvCoord).r;							\n"\
"	Color.rgb = _Color.rgb;											\n"\
"	Color.a *= _Color.a;											\n"\
"	gl_FragColor = Color;											\n"\
"}																	\n";


/*
======================================================================================================================================================================================================
Initialization Functions


*/

//--------------------------------------------------------------------------------------
bool InitGui(GLFWwindow* pInitializedGLFWWindow, CallbackFuncPtr callback, GLuint controltex)
{
	g_pGLFWWindow = pInitializedGLFWWindow;
	g_pCallback = callback;
	
	//register glfw event handlers
	glfwSetMouseButtonCallback(g_pGLFWWindow, GLFWMouseButtonCallback);
	glfwSetCursorPosCallback(g_pGLFWWindow, GLFWCursorPosCallback);
	glfwSetCursorEnterCallback(g_pGLFWWindow, GLFWCursorEnterCallback);
	glfwSetScrollCallback(g_pGLFWWindow, GLFWScrollCallback);
	glfwSetKeyCallback(g_pGLFWWindow, GLFWKeyCallback);
	glfwSetCharCallback(g_pGLFWWindow, GLFWCharCallback);

	glfwSetWindowPosCallback(g_pGLFWWindow, GLFWWindowPosCallback);
	glfwSetWindowSizeCallback(g_pGLFWWindow, GLFWWindowSizeCallback);
	glfwSetWindowCloseCallback(g_pGLFWWindow, GLFWWindowCloseCallback);
	//glfwSetWindowRefreshCallback(g_pGLFWWindow, GLFWWindowRefreshCallback); huge performace overhead
	glfwSetWindowFocusCallback(g_pGLFWWindow, GLFWWindowFocusCallback);
	glfwSetWindowIconifyCallback(g_pGLFWWindow, GLFWWindowIconifyCallback);
	glfwSetFramebufferSizeCallback(g_pGLFWWindow, GLFWFrameBufferSizeCallback);

	//load the ui shaders
    ShaderSourceList sources;
    sources.insert({ SH_VERTEX_SHADER, g_UIShaderVert });
    sources.insert({ SH_FRAGMENT_SHADER, g_UIShaderFrag });
	SHADERMANAGER.CreateProgram(g_UIProgram, sources);
	sources.clear();

    sources.insert({ SH_VERTEX_SHADER, g_UIShaderVert });
    sources.insert({ SH_FRAGMENT_SHADER, g_UIShaderFragUntex });
	SHADERMANAGER.CreateProgram(g_UIProgramUntex, sources);
	sources.clear();

    sources.insert({ SH_VERTEX_SHADER, g_TextShaderVert });
    sources.insert({ SH_FRAGMENT_SHADER, g_TextShaderFrag });
	SHADERMANAGER.CreateProgram(g_TextProgram, sources);


	//load the locations
	g_UIShaderLocations.position		= SHADERMANAGER.GetShaderVariableLocation(g_UIProgram, GLT_ATTRIB, "_Position");
	g_UIShaderLocations.uv				= SHADERMANAGER.GetShaderVariableLocation(g_UIProgram, GLT_ATTRIB, "_UV");
	g_UIShaderLocations.color			= SHADERMANAGER.GetShaderVariableLocation(g_UIProgram, GLT_ATTRIB, "_Color");
	g_UIShaderLocations.ortho			= SHADERMANAGER.GetShaderVariableLocation(g_UIProgram, GLT_UNIFORM, "_Ortho");
	g_UIShaderLocations.sampler			= SHADERMANAGER.GetShaderVariableLocation(g_UIProgram, GLT_UNIFORM, "_TS");

	g_UIShaderLocationsUntex.position	= SHADERMANAGER.GetShaderVariableLocation(g_UIProgram, GLT_ATTRIB, "_Position");
	g_UIShaderLocationsUntex.color		= SHADERMANAGER.GetShaderVariableLocation(g_UIProgram, GLT_ATTRIB, "_Color");
	g_UIShaderLocationsUntex.ortho		= SHADERMANAGER.GetShaderVariableLocation(g_UIProgram, GLT_UNIFORM, "_Ortho");

	g_TextShaderLocations.position		= SHADERMANAGER.GetShaderVariableLocation(g_TextProgram, GLT_ATTRIB, "_Position");
	g_TextShaderLocations.uv			= SHADERMANAGER.GetShaderVariableLocation(g_TextProgram, GLT_ATTRIB, "_UV");
	g_TextShaderLocations.color			= SHADERMANAGER.GetShaderVariableLocation(g_TextProgram, GLT_UNIFORM, "_Color");
	g_TextShaderLocations.ortho			= SHADERMANAGER.GetShaderVariableLocation(g_TextProgram, GLT_UNIFORM, "_Ortho");
	g_TextShaderLocations.sampler		= SHADERMANAGER.GetShaderVariableLocation(g_TextProgram, GLT_UNIFORM, "_TS");

	//create the text arrrays
	/*glGenVertexArrayBindVertexArray(&g_TextVAO);
	glGenBuffers(1, &g_TextPos);
	glGenBuffers(1, &g_TextTexCoords);

	glBindBuffer(GL_ARRAY_BUFFER, g_TextPos);
	glVertexAttribPointer(g_TextShaderLocations.position, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

	glBindBuffer(GL_ARRAY_BUFFER, g_TextTexCoords);
	glVertexAttribPointer(g_TextShaderLocations.uv, 2, GL_FLOAT, GL_FALSE, 0, nullptr);

	glBindVertexArray(0);*/

    g_TextVertexArray = std::make_shared<VertexArray>(GL_TRIANGLES, GL_STREAM_DRAW, true);
    g_TextVertexArray->AddVertexAttrib({ 4, 3, g_TextShaderLocations.position, GL_FLOAT, 0 });
    g_TextVertexArray->AddVertexAttrib({ 4, 2, g_TextShaderLocations.uv, GL_FLOAT, 12 });

    /*static std::vector<glm::u32vec3> indices =
    {
        { 3, 0, 2 },
        { 2, 0, 1 }
    };

    g_TextVertexArray->BufferIndices(indices);*/

	//initialize the freetype library.
	FT_Error err = FT_Init_FreeType(&g_FtLib);
	if (err)
	{
		GLUF_ERROR("Failed to Initialize the Freetype Library!");
		return false;
	}

	//load the texture for the controls
	g_pControlTexturePtr = controltex;


	int w, h;
	glfwGetWindowSize(g_pGLFWWindow, &w, &h);
	g_WndHeight = h;
	g_WndWidth = w;

	return true;
}

//--------------------------------------------------------------------------------------
CallbackFuncPtr ChangeCallbackFunc(CallbackFuncPtr newCallback)
{
    CallbackFuncPtr tmp = g_pCallback;
	g_pCallback = newCallback;
	return tmp;
}

//--------------------------------------------------------------------------------------
void Terminate()
{
	FT_Done_FreeType(g_FtLib);
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
*/
class Font
{	
public:

	FT_Face mFtFont;
	FontSize mHeight;
    glm::u32vec2 mAtlasSize;
	GLuint mTexId = 0;
    std::vector<CharacterInfo> mCharAtlas;
	glm::uint32 mCharacterOffset = 32;
    glm::uint32 mCharacterEnd = 128;
	

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

void Font::Refresh()
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


	//load the characters' dimmensions
	for (unsigned int i = mCharacterOffset; i < mCharacterEnd; i++)
	{
		if (FT_Load_Char(mFtFont, i, FT_LOAD_RENDER)) 
		{
			GLUF_ERROR("Loading character failed!\n");
			continue;
		}

		mAtlasSize.x += g->bitmap.width + GLYPH_PADDING;
		mAtlasSize.y = std::max(mAtlasSize.y, static_cast<glm::uint32>(g->bitmap.rows));

	}

	//generate textures
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, mTexId);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	// Fonts should be rendered at native resolution so no need for texture filtering
	//float fLargest = 0.0f;
	//glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &fLargest);
	//	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, fLargest);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	// Stop chararcters from bleeding over edges
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

	GLfloat transparent[] = { 0.0f, 0.0f, 0.0f, 0.0f };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, transparent);//any overflow will be transparent

	glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, mAtlasSize.x, mAtlasSize.y, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, 0);

	//load texture data
	char* dat0 = new char[GLYPH_PADDING * mAtlasSize.y];
    for (uint32_t i = 0; i < GLYPH_PADDING * mAtlasSize.y; ++i)
		dat0[i] = 0;

	int x = 0;
	for (unsigned int i = mCharacterOffset; i < mCharacterEnd; ++i)
	{
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

			continue;
		}

		glTexSubImage2D(GL_TEXTURE_2D, 0, x + (p * GLYPH_PADDING), 0, g->bitmap.width, g->bitmap.rows, GL_LUMINANCE, GL_UNSIGNED_BYTE, g->bitmap.buffer);

		//this adds padding to keep the characters looking clean
		glTexSubImage2D(GL_TEXTURE_2D, 0, x + (p * GLYPH_PADDING) + g->bitmap.width, 0, GLYPH_PADDING, mAtlasSize.y, GL_LUMINANCE, GL_UNSIGNED_BYTE, dat0);

		mCharAtlas[p].mTexXOffset = static_cast<float>(x + (p * GLYPH_PADDING)) / static_cast<float>(mAtlasSize.x);

		x += g->bitmap.width;

		mCharAtlas[p].mAdvance.x = (g->advance.x >> 6);
		mCharAtlas[p].mAdvance.y = (g->advance.y >> 6);

		mCharAtlas[p].mBitSize.x = g->bitmap.width;
		mCharAtlas[p].mBitSize.y = g->bitmap.rows;

		mCharAtlas[p].mBitLoc.x = g->bitmap_left;
		mCharAtlas[p].mBitLoc.y = g->bitmap_top;

	}

	//FT_Done_Face(mFtFont);
	g = 0;
}

void Font::Init(const std::vector<char>& data, FontSize fontHeight)
{
	mHeight = fontHeight;
	mCharAtlas.resize(mCharacterEnd - mCharacterOffset);

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
        throw std::out_of_range("Character Not In Atlas!");

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
        throw std::out_of_range("Character Not In Atlas!");

	float l = 0, t = 0, r = 0, b = 0;

	l = mCharAtlas[ch - mCharacterOffset].mTexXOffset;
	t = 0.0f;
    r = mCharAtlas[ch - mCharacterOffset].mTexXOffset + static_cast<float>(GetCharWidth(ch)) / static_cast<float>(mAtlasSize.x);
	b = static_cast<float>(mCharAtlas[ch - mCharacterOffset].mBitSize.y) / static_cast<float>(mAtlasSize.y);

	return{ l, t, r, b };
};

FontSize Font::GetCharAdvance(wchar_t ch)
{
    if (ch < mCharacterOffset || ch >= mCharacterEnd)
        throw std::out_of_range("Character Not In Atlas!");

	return mCharAtlas[ch - mCharacterOffset].mAdvance.x;
}

FontSize Font::GetCharWidth(wchar_t ch)
{
    if (ch < mCharacterOffset || ch >= mCharacterEnd)
        throw std::out_of_range("Character Not In Atlas!");

	return mCharAtlas[ch - mCharacterOffset].mBitSize.x;
}

FontSize Font::GetCharHeight(wchar_t ch)
{
    if (ch < mCharacterOffset || ch >= mCharacterEnd)
        throw std::out_of_range("Character Not In Atlas!");

	return mCharAtlas[ch - mCharacterOffset].mBitSize.y;
}

FontSize Font::GetStringWidth(const std::wstring& str)
{
    for (auto ch : str)
    {
        if (ch < mCharacterOffset || ch >= mCharacterEnd)
            throw std::out_of_range("Character Not In Atlas!");
    }

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



/*
======================================================================================================================================================================================================
BlendColor Functions


*/

//--------------------------------------------------------------------------------------
BlendColor::BlendColor()
{
    SetAll({ 255, 255, 255, 255 });
}

//--------------------------------------------------------------------------------------
void BlendColor::Init(const Color& defaultColor, const Color& disabledColor, const Color& hiddenColor)
{
    HighBitColor defColor = static_cast<HighBitColor>(defaultColor);
    for (auto it = mStates.begin(); it != mStates.end(); ++it)
    {
        it->second = defColor;
    }

	mStates[STATE_DISABLED] = static_cast<HighBitColor>(disabledColor);
	mStates[STATE_HIDDEN] = static_cast<HighBitColor>(hiddenColor);
	mCurrentColor = mStates[STATE_HIDDEN];//start hidden
}


//--------------------------------------------------------------------------------------
void BlendColor::Blend(ControlState state, float elapsedTime, float rate)
{
	//this is quite condensed, this basically interpolates from the current state to the destination state based on the time
    //the speed of this transition is a recurisve version of e^kx - 1.0f
    float delta = elapsedTime - mPrevBlendTime;
    HighBitColor col = mStates[state];
    //if (col == Color{ 0, 0, 0, 0 })
    //    return;
    float trans = powf(_E_F, rate * delta) - 1.0f;
    float clamped = glm::clamp(trans, 0.0f, 1.0f);
    HighBitColor old = mCurrentColor;
    mCurrentColor = glm::mix(mCurrentColor, col, clamped);
    mPrevBlendTime = elapsedTime;
    //mCurrentColor = glm::mix(mCurrentColor, mStates[state], glm::clamp(powf(_E_F, rate * elapsedTime) - 1.0f, 0.0f, 1.0f));
}

//--------------------------------------------------------------------------------------
void BlendColor::SetCurrent(const Color& current)
{
	mCurrentColor = current;
}

//--------------------------------------------------------------------------------------
void BlendColor::SetCurrent(ControlState state)
{
	mCurrentColor = mStates[state];
}

//--------------------------------------------------------------------------------------
void BlendColor::SetAll(const Color& color)
{
    for (unsigned int i = STATE_NORMAL; i <= STATE_HIDDEN; ++i)
    {
        mStates[STATE_NORMAL] = color;
    }

	SetCurrent(color);
}

//--------------------------------------------------------------------------------------
Color BlendColor::GetState(ControlState state) const noexcept
{
    return static_cast<Color>(mStates.at(state));
}

//--------------------------------------------------------------------------------------
void BlendColor::SetState(ControlState state, const Color& col) noexcept
{
    mStates[state] = static_cast<HighBitColor>(col);
}

//--------------------------------------------------------------------------------------
Color BlendColor::GetCurrent() const noexcept
{
    return static_cast<Color>(mCurrentColor);
}



/*
======================================================================================================================================================================================================
Element Functions


*/

//--------------------------------------------------------------------------------------
void Element::SetTexture(TextureIndex textureIndex, const Rectf& uvRect, const Color& defaultTextureColor)
{
    mTextureIndex = textureIndex;
    mUVRect = uvRect;
    mTextureColor.Init(defaultTextureColor);
}


//--------------------------------------------------------------------------------------
void Element::SetFont(FontIndex font, const Color& defaultFontColor, Bitfield textFormat)
{
    mFontIndex = font;
    mFontColor.Init(defaultFontColor);
    mTextFormatFlags = textFormat;
}


//--------------------------------------------------------------------------------------
void Element::Refresh()
{
	//mTextureColor.SetCurrent(STATE_HIDDEN);
	//mFontColor.SetCurrent(STATE_HIDDEN);
}



/*
======================================================================================================================================================================================================
Dialog Functions


*/

//--------------------------------------------------------------------------------------
Dialog::Dialog()
{
#ifdef _DEBUG
    //TODO: get a more graceful way to test this
    //This is to make sure all dialogs are being destroyed
    printf("Dialog Created");
#endif
}

double Dialog::sTimeRefresh = _60HZ;
ControlPtr Dialog::sControlFocus = nullptr;
ControlPtr Dialog::sControlPressed = nullptr;

//--------------------------------------------------------------------------------------
Dialog::~Dialog()
{
#ifdef _DEBUG
    //TODO: get a more graceful way to test this
    //This is to make sure all dialogs are being destroyed
    printf("Dialog Destroyed");
#endif

	RemoveAllControls();
}


//--------------------------------------------------------------------------------------
void Dialog::Init(DialogResourceManagerPtr& manager, bool registerDialog)
{
	if (g_ControlTextureResourceManLocation == -1)
	{
        g_ControlTextureResourceManLocation = manager->AddTexture(g_pControlTexturePtr);
	}

    Init(manager, registerDialog, g_ControlTextureResourceManLocation);
}


//--------------------------------------------------------------------------------------
void Dialog::Init(DialogResourceManagerPtr& manager, bool registerDialog, TextureIndex textureIndex)
{
    if (manager == nullptr)
        throw std::invalid_argument("Nullptr DRM");

    mDialogManager = manager;
    if (registerDialog)
        mDialogManager->RegisterDialog(shared_from_this());

    SetTexture(0, textureIndex);//this will always be the first one in our buffer of indices
	InitDefaultElements();
}


//--------------------------------------------------------------------------------------

/*void Dialog::Init(DialogResourceManager* pManager, bool bRegisterDialog, LPCWSTR szControlTextureResourceName, HMODULE hControlTextureResourceModule)
{
	m_pManager = pManager;
	if (bRegisterDialog)
		pManager->RegisterDialog(this);

	SetTexture(0, szControlTextureResourceName, hControlTextureResourceModule);
	InitDefaultElements();
}*/


//--------------------------------------------------------------------------------------
void Dialog::SetCallback(EventCallbackFuncPtr callback, EventCallbackReceivablePtr userContext) noexcept
{
    NOEXCEPT_REGION_START
	// If this assert triggers, you need to call Dialog::Init() first.  This change
	// was made so that the 's GUI could become separate and optional from 's core.  The 
	// creation and interfacing with DialogResourceManager is now the responsibility 
	// of the application if it wishes to use 's GUI.
	_ASSERT(mDialogManager && L"To fix call Dialog::Init() first.  See comments for details.");

    mCallbackEvent = callback;
	mCallbackContext = userContext;

    NOEXCEPT_REGION_END
}


//--------------------------------------------------------------------------------------
void Dialog::RemoveControl(ControlIndex ID)
{
    NOEXCEPT_REGION_START

    auto it = mControls.find(ID);

    if (it != mControls.end())
    {
        // Clean focus first
        ClearFocus();

        // Clear references to this control
        if (sControlFocus == it->second)
            sControlFocus = nullptr;
        if (sControlPressed == it->second)
            sControlPressed = nullptr;
        if (mControlMouseOver == it->second)
            mControlMouseOver = nullptr;

        mControls.erase(it);

    }

    NOEXCEPT_REGION_END
}


//--------------------------------------------------------------------------------------
void Dialog::RemoveAllControls() noexcept
{
    NOEXCEPT_REGION_START

	if (sControlFocus && &sControlFocus->mDialog == this)
		sControlFocus = nullptr;
	if (sControlPressed && &sControlPressed->mDialog == this)
		sControlPressed = nullptr;
	mControlMouseOver = nullptr;

	mControls.clear();

    NOEXCEPT_REGION_END
}


//--------------------------------------------------------------------------------------
void Dialog::Refresh() noexcept
{
    NOEXCEPT_REGION_START

	if (sControlFocus)
		sControlFocus->OnFocusOut();

	if (mControlMouseOver)
		mControlMouseOver->OnMouseLeave();

	sControlFocus = nullptr;
	sControlPressed = nullptr;
	mControlMouseOver = nullptr;

	for (auto it : mControls)
	{
		it.second->Refresh();
	}

	if (mKeyboardInput)
		FocusDefaultControl();

    NOEXCEPT_REGION_END
}


//--------------------------------------------------------------------------------------
void Dialog::OnRender(float elapsedTime) noexcept
{
	// If this assert triggers, you need to call DialogResourceManager::On*Device() from inside
	// the application's device callbacks.  See the SDK samples for an example of how to do this.
	//_ASSERT(m_pManager->GetD3D11Device() &&
	//	L"To fix hook up DialogResourceManager to device callbacks.  See comments for details");
	//no need for "devices", this is all handled by GLFW

    NOEXCEPT_REGION_START

	// See if the dialog needs to be refreshed
	if (mTimePrevRefresh < sTimeRefresh)
	{
        mTimePrevRefresh = GetTime();
		Refresh();
	}

	// For invisible dialog, out now.
	if (!mVisible ||
		(mMinimized && !mCaptionEnabled))
		return;

	// Enable depth test
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_CLAMP);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	mDialogManager->BeginSprites();

	if (!mMinimized)
	{
		// Convert the draw rectangle from screen coordinates to clip space coordinates.(where the origin is in the middle of the screen, and the edges are 1, or negative 1
		Rect windowCoords = { 0, GetHeight(), GetWidth(), 0 };
		//windowCoords = ScreenToClipspace(windowCoords);

		DrawSprite(mDlgElement, windowCoords, -0.99f, false);
	}

	// Sort depth back to front
	Text::BeginText(mDialogManager->GetOrthoMatrix());


	//m_pManager->ApplyRenderUI();
	// If the dialog is minimized, skip rendering
	// its controls.
	if (!mMinimized)
	{
		for (auto it : mControls)
		{
			// Focused control is drawn last
			if (it.second == sControlFocus)
				continue;

			it.second->Render(elapsedTime);
		}

		if (sControlFocus && &sControlFocus->mDialog == this)
			sControlFocus->Render(elapsedTime);
	}

	// Render the caption if it's enabled.
	if (mCaptionEnabled)
	{
		// DrawSprite will offset the rect down by
		// m_nCaptionHeight, so adjust the rect higher
		// here to negate the effect.

		mCapElement.mTextureColor.SetCurrent(STATE_NORMAL);
		mCapElement.mFontColor.SetCurrent(STATE_NORMAL);
		Rect rc = { 0, 0, GetWidth(), -mCaptionHeight };

		mDialogManager->ApplyRenderUIUntex();
		DrawSprite(mCapElement, rc, -0.99f, false);

		rc.left += 5; // Make a left margin

		if (mMinimized)
		{
            std::wstringstream wss;
            wss << mCaptionText;
            wss << L" (Minimized)";
			DrawText(wss.str(), mCapElement, rc);
		}
		else
			DrawText(mCaptionText, mCapElement, rc);
	}

	// End sprites
	/*if (m_bCaption)
	{
		m_pManager->EndSprites();
		EndText();
	}*/
	//m_pManager->RestoreD3D11State(pd3dDeviceContext);

	glDisable(GL_BLEND);
	glEnable(GL_DEPTH_CLAMP);//set this back because it is the default

    NOEXCEPT_REGION_END
}


//--------------------------------------------------------------------------------------
void Dialog::SendEvent(Event ctrlEvent, bool triggeredByUser, ControlPtr control) noexcept
{
    NOEXCEPT_REGION_START

	// If no callback has been registered there's nowhere to send the event to
	if (!mCallbackEvent)
		return;

	// Discard events triggered programatically if these types of events haven't been
	// enabled
	if (!triggeredByUser && !mNonUserEvents)
		return;

	mCallbackEvent(ctrlEvent, control, mCallbackContext);

    NOEXCEPT_REGION_END
}


//--------------------------------------------------------------------------------------
void Dialog::SetFont(FontIndex index, FontIndex resManFontIndex)
{
	// If this assert triggers, you need to call Dialog::Init() first.  This change
	// was made so that the 's GUI could become separate and optional from 's core.  The 
	// creation and interfacing with DialogResourceManager is now the responsibility 
	// of the application if it wishes to use 's GUI.
	_ASSERT(mDialogManager && L"To fix call Dialog::Init() first.  See comments for details.");
	//_Analysis_assume_(m_pManager);


    //call this to trigger an exception if the font index does not exist
    mDialogManager->GetFontNode(resManFontIndex);

	mFonts[index] = resManFontIndex;
}


//--------------------------------------------------------------------------------------
FontNodePtr Dialog::GetFont(FontIndex index) const
{
	if (!mDialogManager)
		return nullptr;
	return mDialogManager->GetFontNode(index);
}


//--------------------------------------------------------------------------------------
void Dialog::SetTexture(TextureIndex index, TextureIndex resManTexIndex)
{
	// If this assert triggers, you need to call Dialog::Init() first.  This change
	// was made so that the 's GUI could become separate and optional from 's core.  The 
	// creation and interfacing with DialogResourceManager is now the responsibility 
	// of the application if it wishes to use 's GUI.
	_ASSERT(mDialogManager && L"To fix this, call Dialog::Init() first.  See comments for details.");
	//_Analysis_assume_(m_pManager);
    
    //call this to trigger an exception if the texture index does not exist
    mDialogManager->GetTextureNode(resManTexIndex);

    mTextures[index] = resManTexIndex;
}

//--------------------------------------------------------------------------------------
TextureNodePtr Dialog::GetTexture(TextureIndex index) const
{
	if (!mDialogManager)
		return nullptr;
	return mDialogManager->GetTextureNode(index);
}

//--------------------------------------------------------------------------------------
bool Dialog::MsgProc(MessageType msg, int32_t param1, int32_t param2, int32_t param3, int32_t param4) noexcept
{
    NOEXCEPT_REGION_START

	if (mFirstTime)
        mFirstTime = false;
	else
		mMousePositionOld = mMousePosition;

	mDialogManager->MsgProc(_PASS_CALLBACK_PARAM);


	//first, even if we are not going to use it, snatch up the cursor position just in case it moves in the time it takes to do this
	double x, y;
	glfwGetCursorPos(g_pGLFWWindow, &x, &y);
	mMousePosition = Point(static_cast<long>(x), g_WndHeight - static_cast<long>(y));
    
    //this gets broken when window is too big
	mMousePositionDialogSpace.x = mMousePosition.x - mRegion.x;
	mMousePositionDialogSpace.y = mMousePosition.y - mRegion.y - mCaptionHeight;//TODO: fix

	//if (m_bCaption)
	//	m_MousePositionDialogSpace.y -= m_nCaptionHeight;

	bool bHandled = false;

	// For invisible dialog, do not handle anything.
	if (!mVisible)
		return false;

    if (!mKeyboardInput && (msg == KEY || msg == UNICODE_CHAR))
		return false;

	// If caption is enable, check for clicks in the caption area.
	if (mCaptionEnabled && !mLocked)
	{
		static Point totalDelta;

		if (((msg == MB) == true) &&
			((param1 == GLFW_MOUSE_BUTTON_LEFT) == true) &&
			((param2 == GLFW_PRESS) == true) )
		{

			if (mMousePositionDialogSpace.x >= 0 && mMousePositionDialogSpace.x < RectWidth(mRegion) &&
				mMousePositionDialogSpace.y >= -mCaptionHeight && mMousePositionDialogSpace.y < 0)
			{
				mDrag = true;
				mDragged = false;
				//SetCapture(GetHWND());
				return true;
			}
			else if (!mMinimized && mGrabAnywhere && !GetControlAtPoint(mMousePositionDialogSpace))
			{
				//ONLY allow this if it is not on top of a control
				mDrag = true;
				return true;
			}
		}
		else if ((msg == MB) == true &&
				(param1 == GLFW_MOUSE_BUTTON_LEFT) == true &&
				(param2 == GLFW_RELEASE) == true && 
				(mDrag))
		{
			if (mMousePositionDialogSpace.x >= 0 && mMousePositionDialogSpace.x < RectWidth(mRegion) &&
				mMousePositionDialogSpace.y >= -mCaptionHeight && mMousePositionDialogSpace.y < 0)
			{
				//ReleaseCapture();

				mDrag = false;

				//only minimize if the dialog WAS NOT moved
				if (!mDragged)
				{
					//reset this when it passes its threshhold, which is when m_bDragged is toggled
					totalDelta = { 0L, 0L };
					mMinimized = !mMinimized;
				}

				return true;
			}
			else if (!mMinimized && mGrabAnywhere)
			{
				mDrag = false;
				return true;
			}
		}
		else if ((msg == CURSOR_POS))
		{
			//is it over the caption?
			if (glfwGetMouseButton(g_pGLFWWindow, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
			{
				if (mDrag)
				{
					//if (m_MousePosition.x < 0.0f || m_MousePosition.y < 0.0f)
					//{
					//	glfwSetCursorPos(g_pGLFWWindow, (m_MousePosition.x < 0.0f) ? 0 : param1, (m_MousePosition.y < 0.0f) ? param2 : 0);
					//}


					Point delta = mMousePosition - mMousePositionOld;
					totalDelta = { totalDelta.x + delta.x, totalDelta.y + delta.y };
                    
					RepositionRect(mRegion, 
                        std::clamp(delta.x + mRegion.x, 0L, static_cast<long>(g_WndWidth) - RectWidth(mRegion)),
                        std::clamp(delta.y + mRegion.y, 0L, static_cast<long>(g_WndHeight) - mCaptionHeight));
                    

					//give a threshhold, because sometimes when a use clicks, the user will move the mouse a bit
					if (totalDelta.x > 3 || totalDelta.y > 3)
						mDragged = true;

					return true;
				}
			}
		}
	}

	//this is important, if the window is resized, then make sure to reclamp the dialog position
	if (mAutoClamp && msg == RESIZE)
	{
		ClampToScreen();
	}

	// If the dialog is minimized, don't send any messages to controls.
	if (mMinimized)
		return false;

	// If a control is in focus, it belongs to this dialog, and it's enabled, then give
	// it the first chance at handling the message.
	if (sControlFocus &&
		&sControlFocus->mDialog == this &&
		sControlFocus->GetEnabled())
	{
		// If the control MsgProc handles it, then we don't.
		if (sControlFocus->MsgProc(msg, param1, param2, param3, param4))
			return true;
	}

	switch (msg)
	{
	case RESIZE:
	case POS:
	{
		// Handle sizing and moving messages so that in case the mouse cursor is moved out
		// of an UI control because of the window adjustment, we can properly
		// unhighlight the highlighted control.
		Point pt =
		{
			-1, -1
		};
		OnMouseMove(pt);
		break;
	}

	case FOCUS:
		// Call OnFocusIn()/OnFocusOut() of the control that currently has the focus
		// as the application is activated/deactivated.  This matches the Windows
		// behavior.
		if (sControlFocus &&
			&sControlFocus->mDialog == this &&
			sControlFocus->GetEnabled())
		{
			if (param1 == GL_TRUE)
				sControlFocus->OnFocusIn();
			else
				sControlFocus->OnFocusOut();
		}
		break;

		// Keyboard messages
	case KEY:
	//case WM_SYSKEYDOWN:
	//case WM_KEYUP:
	//case WM_SYSKEYUP:
	{
		// If a control is in focus, it belongs to this dialog, and it's enabled, then give
		// it the first chance at handling the message.
		if (sControlFocus &&
			&sControlFocus->mDialog == this &&
			sControlFocus->GetEnabled())
			//for (auto it : mControls)     --> Not Quite sure what this loop was here for
			//{
			    if (sControlFocus->MsgProc(msg, param1, param2, param3, param4))
				    return true;
			//}

		// Not yet handled, see if this matches a control's hotkey
		// Activate the hotkey if the focus doesn't belong to an
		// edit box.
		if (param3 == GLFW_PRESS && (!sControlFocus ||
			(sControlFocus->GetType() != CONTROL_EDITBOX
			&& sControlFocus->GetType() != CONTROL_IMEEDITBOX)))
		{
			for (auto it : mControls)
			{
				if (it.second->GetHotkey() == param1)
				{
					it.second->OnHotkey();
					return true;
				}
			}
		}

		// Not yet handled, check for focus messages
		if (param3 == GLFW_PRESS)
		{
			// If keyboard input is not enabled, this message should be ignored
			if (!mKeyboardInput)
				return false;

			switch (param1)
			{
			case GLFW_KEY_RIGHT:
			case GLFW_KEY_DOWN:
				if (sControlFocus)
				{
					return OnCycleFocus(true);
				}
				break;

			case GLFW_KEY_LEFT:
			case GLFW_KEY_UP:
				if (sControlFocus)
				{
					return OnCycleFocus(false);
				}
				break;

			case GLFW_KEY_TAB:
			{
				bool bShiftDown =(glfwGetKey(g_pGLFWWindow, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) ||
								(glfwGetKey(g_pGLFWWindow, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS);
				return OnCycleFocus(!bShiftDown);
			}
			}
		}

		break;
	}


		// Mouse messages
	//case WM_MOUSEMOVE:
	//case WM_LBUTTONDOWN:
	//case WM_LBUTTONUP:
	//case WM_MBUTTONDOWN:
	//case WM_MBUTTONUP:
	//case WM_RBUTTONDOWN:
	//case WM_RBUTTONUP:
	//case WM_XBUTTONDOWN:
	//case WM_XBUTTONUP:
	//case WM_LBUTTONDBLCLK:
	//case WM_MBUTTONDBLCLK:
	//case WM_RBUTTONDBLCLK:
	//case WM_XBUTTONDBLCLK:
	//case WM_MOUSEWHEEL:
	case MB:
	case SCROLL:
	case CURSOR_POS:
	{
		// If not accepting mouse input, return false to indicate the message should still 
		// be handled by the application (usually to move the camera).
		if (!mMouseInput)
			return false;


		// If caption is enabled, offset the Y coordinate by its height.
		//if (m_bCaption)
		//	m_MousePosition.y += m_nCaptionHeight;

		// If a control is in focus, it belongs to this dialog, and it's enabled, then give
		// it the first chance at handling the message.
		if (sControlFocus &&
			&sControlFocus->mDialog == this &&
			sControlFocus->GetEnabled())
		{
			if (sControlFocus->MsgProc(msg, param1, param2, param3, param4))
				return true;
		}

		// Not yet handled, see if the mouse is over any controls
		ControlPtr pControl = GetControlAtPoint(mMousePositionDialogSpace);
		if (pControl && pControl->GetEnabled())
		{
			bHandled = pControl->MsgProc(msg, param1, param2, param3, param4);
			if (bHandled)
				return true;
		}
		else
		{
			// Mouse not over any controls in this dialog, if there was a control
			// which had focus it just lost it
			if (param1 == GLFW_MOUSE_BUTTON_LEFT &&
				param2 == GLFW_PRESS &&
				sControlFocus &&
				&sControlFocus->mDialog == this)
			{
				sControlFocus->OnFocusOut();
				sControlFocus = nullptr;
			}
		}

		// Still not handled, hand this off to the dialog. Return false to indicate the
		// message should still be handled by the application (usually to move the camera).
		switch (msg)
		{
		case CURSOR_POS:
			OnMouseMove(mMousePositionDialogSpace);
			return false;
		}

		break;
	}

	case CURSOR_ENTER:
	{
		// The application has lost mouse capture.
		// The dialog object may not have received
		// a WM_MOUSEUP when capture changed. Reset
		// mDrag so that the dialog does not mistakenly
		// think the mouse button is still held down.
		if (param1 == GL_FALSE)
			mDrag = false;
	}
	}

    NOEXCEPT_REGION_END

	return false;
}


//--------------------------------------------------------------------------------------
void Dialog::ClampToScreen() noexcept
{
    NOEXCEPT_REGION_START

	mRegion.x = std::clamp(mRegion.x, 0L, static_cast<long>(g_WndWidth) - RectWidth(mRegion));
	mRegion.y = std::clamp(mRegion.y, 0L, static_cast<long>(g_WndHeight) - mCaptionHeight);

    NOEXCEPT_REGION_END
}

//--------------------------------------------------------------------------------------
ControlPtr Dialog::GetControlAtPoint(const Point& pt) const
{
	// Search through all child controls for the first one which
	// contains the mouse point
	for (auto it : mControls)
	{
		if (!it.second)
		{
			continue;
		}

		// We only return the current control if it is visible
		// and enabled.  Because GetControlAtPoint() is used to do mouse
		// hittest, it makes sense to perform this filtering.
		if (it.second->ContainsPoint(pt) && it.second->GetEnabled() && it.second->GetVisible())
		{
			return it.second;
		}
	}

    return nullptr;
}


//--------------------------------------------------------------------------------------
bool Dialog::GetControlEnabled(ControlIndex ID) const
{
	ControlPtr pControl = GetControl<Control>(ID);
	if (!pControl)
		return false;

	return pControl->GetEnabled();
}



//--------------------------------------------------------------------------------------
void Dialog::SetControlEnabled(ControlIndex ID, bool bEnabled)
{
	ControlPtr pControl = GetControl<Control>(ID);
	if (!pControl)
		return;

	pControl->SetEnabled(bEnabled);
}


//--------------------------------------------------------------------------------------
void Dialog::OnMouseUp(const Point& pt) noexcept
{
    NOEXCEPT_REGION_START

    //TODO: do something here?
    GLUF_UNREFERENCED_PARAMETER(pt);
    sControlPressed = nullptr;
    mControlMouseOver = nullptr;

    NOEXCEPT_REGION_END
}


//--------------------------------------------------------------------------------------
void Dialog::OnMouseMove(const Point& pt) noexcept
{
    NOEXCEPT_REGION_START

	// Figure out which control the mouse is over now
	ControlPtr pControl = GetControlAtPoint(pt);

	// If the mouse is still over the same control, nothing needs to be done
	if (pControl == mControlMouseOver)
		return;

	// Handle mouse leaving the old control
    if (mControlMouseOver)
    {
        mControlMouseOver->OnMouseLeave();
        mControlMouseOver = nullptr;
    }

	// Handle mouse entering the new control
	mControlMouseOver = pControl;
    if (pControl)
    {
        mControlMouseOver->OnMouseEnter();
    }

    NOEXCEPT_REGION_END
}


//--------------------------------------------------------------------------------------
void Dialog::SetDefaultElement(ControlType controlType, ElementIndex elementIndex, const Element& element)
{
	// If this Element type already exist in the list, simply update the stored Element
    for (auto it : mDefaultElements)
    {
        if (it->mControlType == controlType && it->mElementIndex == elementIndex)
        {
            it->mElement = element;
            return;
        }
    }

	// Otherwise, add a new entry
    ElementHolderPtr pNewHolder = std::make_shared<ElementHolder>();

	pNewHolder->mControlType = controlType;
	pNewHolder->mElementIndex = elementIndex;
	pNewHolder->mElement = element;

	mDefaultElements.push_back(pNewHolder);
}


//--------------------------------------------------------------------------------------
Element Dialog::GetDefaultElement(ControlType controlType, ElementIndex elementIndex) const
{
    for (auto it : mDefaultElements)
    {
        if (it->mControlType == controlType && it->mElementIndex == elementIndex)
        {
            return it->mElement;
        }
    }

    GLUF_NON_CRITICAL_EXCEPTION(std::invalid_argument("GetDefaultElement: elementIndex could not be found within controlType"));

    return Element();
}

//--------------------------------------------------------------------------------------
void Dialog::AddStatic(ControlIndex ID, const std::wstring& strText, const Rect& region, Bitfield textFlags, bool isDefault, std::shared_ptr<StaticPtr> ctrlPtr)
{
    auto pStatic = CreateStatic(textFlags, *this);

    if (ctrlPtr)
        *ctrlPtr = pStatic;

	// Set the ID and list index
	pStatic->SetID(ID);
	pStatic->SetText(strText);
    pStatic->SetRegion(region);
	pStatic->mIsDefault = isDefault;

	AddControl(std::dynamic_pointer_cast<Control>(pStatic));
}


//--------------------------------------------------------------------------------------
void Dialog::AddButton(ControlIndex ID, const std::wstring& strText, const Rect& region, int hotkey, bool isDefault, std::shared_ptr<ButtonPtr> ctrlPtr)
{
    auto pButton = CreateButton(*this);

    if (ctrlPtr)
        *ctrlPtr = pButton;

	// Set the ID and list index
	pButton->SetID(ID);
	pButton->SetText(strText);
    pButton->SetRegion(region);
	pButton->SetHotkey(hotkey);
	pButton->mIsDefault = isDefault;

    AddControl(std::dynamic_pointer_cast<Control>(pButton));
}


//--------------------------------------------------------------------------------------
void Dialog::AddCheckBox(ControlIndex ID, const std::wstring& strText, const Rect& region, bool checked , int hotkey, bool isDefault, std::shared_ptr<CheckBoxPtr> ctrlPtr)
{
    auto pCheckBox = CreateCheckBox(checked, *this);

    if (ctrlPtr)
        *ctrlPtr = pCheckBox;

	// Set the ID and list index
	pCheckBox->SetID(ID);
	pCheckBox->SetText(strText);
    pCheckBox->SetRegion(region);
	pCheckBox->SetHotkey(hotkey);
	pCheckBox->mIsDefault = isDefault;
	pCheckBox->SetChecked(checked);

    AddControl(std::dynamic_pointer_cast<Control>(pCheckBox));
}


//--------------------------------------------------------------------------------------
void Dialog::AddRadioButton(ControlIndex ID, RadioButtonGroup buttonGroup, const std::wstring& strText, const Rect& region, bool checked, int hotkey, bool isDefault, std::shared_ptr<RadioButtonPtr> ctrlPtr)
{
    auto pRadioButton = CreateRadioButton(*this);

    if (ctrlPtr)
        *ctrlPtr = pRadioButton;

	// Set the ID and list index
	pRadioButton->SetID(ID);
	pRadioButton->SetText(strText);
	pRadioButton->SetButtonGroup(buttonGroup);
    pRadioButton->SetRegion(region);
	pRadioButton->SetHotkey(hotkey);
	pRadioButton->SetChecked(checked);
	pRadioButton->mIsDefault = isDefault;
	pRadioButton->SetChecked(checked);

    AddControl(std::dynamic_pointer_cast<Control>(pRadioButton));
}


//--------------------------------------------------------------------------------------
void Dialog::AddComboBox(ControlIndex ID, const Rect& region, int hotKey, bool isDefault, std::shared_ptr<ComboBoxPtr> ctrlPtr)
{
    auto pComboBox = CreateComboBox(*this);

    if (ctrlPtr)
        *ctrlPtr = pComboBox;

	// Set the ID and list index
	pComboBox->SetID(ID);
    pComboBox->SetRegion(region);
	pComboBox->SetHotkey(hotKey);
	pComboBox->mIsDefault = isDefault;

    AddControl(std::dynamic_pointer_cast<Control>(pComboBox));
}


//--------------------------------------------------------------------------------------
void Dialog::AddSlider(ControlIndex ID, const Rect& region, long min, long max, long value, bool isDefault, std::shared_ptr<SliderPtr> ctrlPtr)
{
    auto pSlider = CreateSlider(*this);

    if (ctrlPtr)
        *ctrlPtr = pSlider;

	// Set the ID and list index
	pSlider->SetID(ID);
    pSlider->SetRegion(region);
	pSlider->mIsDefault = isDefault;
	pSlider->SetRange(min, max);
	pSlider->SetValue(value);
	pSlider->UpdateRects();

    AddControl(std::dynamic_pointer_cast<Control>(pSlider));
}


//--------------------------------------------------------------------------------------
/*void Dialog::AddEditBox(ControlIndex ID, const std::wstring& strText, const Rect& region, Charset charset = Unicode, GLbitfield textFlags = GT_LEFT | GT_TOP, bool isDefault = false, std::shared_ptr<EditBoxPtr> ctrlPtr = nullptr)
{
	auto pEditBox = std::make_shared<EditBox>(charset, (textFlags & GT_MULTI_LINE) == GT_MULTI_LINE, this);

	if (ctrlPtr)
		*ctrlPtr = pEditBox;

	AddControl(std::dynamic_pointer_cast<Control>(pEditBox));

	pEditBox->GetElement(0)->dwTextFormat = textFlags;

	// Set the ID and position
	pEditBox->SetID(ID);
    pEditBox->SetRegion(region);
	pEditBox->mIsDefault = isDefault;

	pEditBox->SetText(strText);
}*/


//--------------------------------------------------------------------------------------
void Dialog::AddListBox(ControlIndex ID, const Rect& region, Bitfield style, std::shared_ptr<ListBoxPtr> ctrlPtr)
{
	auto pListBox = CreateListBox(*this);

	if (ctrlPtr)
		*ctrlPtr = pListBox;

	// Set the ID and position
	pListBox->SetID(ID);
    pListBox->SetRegion(region);
	pListBox->SetStyle(style);

    AddControl(std::dynamic_pointer_cast<Control>(pListBox));
}


//--------------------------------------------------------------------------------------
void Dialog::AddControl(ControlPtr& pControl)
{
	InitControl(pControl);

    if (!pControl)
        return;

	// Add to the list
	mControls[pControl->mID] = pControl;
}


//--------------------------------------------------------------------------------------
void Dialog::InitControl(ControlPtr& pControl)
{
	//Result hr;

	if (!pControl)
		return;

	pControl->mIndex = static_cast<unsigned int>(mControls.size());

	// Look for a default Element entry
	for (auto it : mDefaultElements)
	{
		if (it->mControlType == pControl->GetType())
            pControl->SetElement(it->mElementIndex, it->mElement);
	}

	pControl->OnInit();
}


//--------------------------------------------------------------------------------------
ControlPtr Dialog::GetControl(ControlIndex ID, ControlType controlType) const
{
	// Try to find the control with the given ID
	for (auto it : mControls)
	{
		if (it.second->GetID() == ID && it.second->GetType() == controlType)
		{
			return it.second;
		}
    }

    GLUF_NON_CRITICAL_EXCEPTION(std::invalid_argument("Control ID Not Found"));

	// Not found
	return nullptr;
}


//--------------------------------------------------------------------------------------
ControlPtr Dialog::GetNextControl(ControlPtr control)
{
    Dialog& dialog = control->mDialog;


    auto indexIt = dialog.mControls.find(control->mID);

    //get the 'next' one
    ++indexIt;

    //is this still a valid control?
    if (indexIt != dialog.mControls.end())
        return indexIt->second;//yes

    //if not, get the next dialog
    DialogPtr nextDlg = dialog.mNextDialog;

    //in the event that there is only one control, or the dialogs were not hooked up correctly
    if (!nextDlg)
        return control;

    auto* nextDialogIndexIt = &nextDlg->mControls.begin();

    //keep going through dialogs until one with a control is found, but prevent looping back through
    while (*nextDialogIndexIt == nextDlg->mControls.end())
    {
        nextDlg = nextDlg->mNextDialog;
        nextDialogIndexIt = &nextDlg->mControls.begin();

        //if the same dialog is looped back through, return the current control
        if (nextDlg.get() == &dialog)
        {
            return control;
        }
    }

    //a control was found before the dialog chain was looped through
    return (*nextDialogIndexIt)->second;
}


//--------------------------------------------------------------------------------------
ControlPtr Dialog::GetPrevControl(ControlPtr control)
{
    Dialog& dialog = control->mDialog;

    auto indexIt = dialog.mControls.find(control->mID);

    //get the 'previous' one
    --indexIt;

    //is this still a valid control?
    if (indexIt != dialog.mControls.end())
        return indexIt->second;

    //if not get the previous dialog
    DialogPtr prevDlg = dialog.mPrevDialog;

    //in the event that there is only one control, or the dialogs were not hooked up correctly
    if (!prevDlg)
        return control;

    auto* prevDialogIndexIt = &prevDlg->mControls.rbegin();

    //keep going through the dialogs until one with a control is found, but prevent looping back through
    while (*prevDialogIndexIt == prevDlg->mControls.rend())
    {
        prevDlg = prevDlg->mPrevDialog;
        prevDialogIndexIt = &prevDlg->mControls.rbegin();

        //if the same dialog is looped back through, return the current control
        if (prevDlg.get() == &dialog)
        {
            return control;
        }
    }

    //a control was found before the dialog chain was looped through
    return (*prevDialogIndexIt)->second;
}


//--------------------------------------------------------------------------------------
void Dialog::ClearRadioButtonGroup(RadioButtonGroup buttonGroup)
{
	// Find all radio buttons with the given group number
	for (auto it : mControls)
	{
		if (it.second->GetType() == CONTROL_RADIOBUTTON)
		{
			RadioButtonPtr radioButton = std::dynamic_pointer_cast<RadioButton>(it.second);

            if (radioButton->GetButtonGroup() == buttonGroup)
                radioButton->SetChecked(false, false);
		}
	}
}


//--------------------------------------------------------------------------------------
std::vector<RadioButtonPtr> Dialog::GetRadioButtonGroup(RadioButtonGroup buttonGroup)
{
    std::vector<RadioButtonPtr> ret;

    // Find all radio buttons with the given group number
    for (auto it : mControls)
    {
        if (it.second->GetType() == CONTROL_RADIOBUTTON)
        {
            RadioButtonPtr radioButton = std::dynamic_pointer_cast<RadioButton>(it.second);

            if (radioButton->GetButtonGroup() == buttonGroup)
                ret.push_back(radioButton);
        }
    }

    return ret;
}


//--------------------------------------------------------------------------------------
void Dialog::ClearComboBox(ControlIndex ID)
{
	ComboBoxPtr comboBox = GetControl<ComboBox>(ID);
	if (!comboBox)
		return;

	comboBox->RemoveAllItems();
}


//--------------------------------------------------------------------------------------
void Dialog::RequestFocus(ControlPtr& control)
{
    if (sControlFocus == control)
		return;

    if (!control->CanHaveFocus())
		return;

	if (sControlFocus)
		sControlFocus->OnFocusOut();

	control->OnFocusIn();
	sControlFocus = control;
}


//--------------------------------------------------------------------------------------
void Dialog::DrawRect(const Rect& rect, const Color& color)
{
	Rect rcScreen = rect;
	OffsetRect(rcScreen, mRegion.x - long(g_WndWidth / 2), mCaptionHeight + mRegion.y - long(g_WndHeight / 2));

	//if (m_bCaption)
	//	OffsetRect(rcScreen, 0, m_nCaptionHeight);

	//rcScreen = ScreenToClipspace(rcScreen);

    auto thisSprite = SpriteVertexStruct::MakeMany(4);
    thisSprite[0] = 
    {
        glm::vec3(rcScreen.left, rcScreen.top, _NEAR_BUTTON_DEPTH),
        ColorToFloat(color),
        glm::vec2() 
    };

    thisSprite[1] =
    {
        glm::vec3(rcScreen.right, rcScreen.top, _NEAR_BUTTON_DEPTH),
        ColorToFloat(color),
        glm::vec2()
    };

    thisSprite[2] =
    {
        glm::vec3(rcScreen.left, rcScreen.bottom, _NEAR_BUTTON_DEPTH),
        ColorToFloat(color),
        glm::vec2()
    };

    thisSprite[3] =
    {
        glm::vec3(rcScreen.right, rcScreen.bottom, _NEAR_BUTTON_DEPTH),
        ColorToFloat(color),
        glm::vec2()
    };

    mDialogManager->mSpriteBuffer.BufferData(thisSprite);

	// Why are we drawing the sprite every time?  This is very inefficient, but the sprite workaround doesn't have support for sorting now, so we have to
	// draw a sprite every time to keep the order correct between sprites and text.
    mDialogManager->EndSprites(nullptr, false);//render in untextured mode
}


//--------------------------------------------------------------------------------------

void Dialog::DrawSprite(const Element& element, const Rect& rect, float depth, bool textured)
{
	// No need to draw fully transparent layers
    if (element.mTextureColor.GetCurrent().a == 0)
        return;

    /*if (element->mTextureColor.GetCurrent() == element->mTextureColor.mStates[STATE_HIDDEN])
		return;*/


    Rectf uvRect = element.mUVRect;

	Rect rcScreen = rect;

	OffsetRect(rcScreen, mRegion.x - long(g_WndWidth / 2), mCaptionHeight + mRegion.y - long(g_WndHeight / 2));
    
	/*TextureNodePtr textureNode = GetTexture(pElement->iTexture);
    if (!textureNode)
		return;*/

    auto thisSprite = SpriteVertexStruct::MakeMany(4);

    thisSprite[0] =
    {
        glm::vec3(rcScreen.left, rcScreen.top, depth),
        ColorToFloat(element.mTextureColor.GetCurrent()),
        glm::vec2(uvRect.left, uvRect.top)
    };

    thisSprite[1] =
    {
        glm::vec3(rcScreen.right, rcScreen.top, depth),
        ColorToFloat(element.mTextureColor.GetCurrent()),
        glm::vec2(uvRect.right, uvRect.top)
    };

    thisSprite[2] =
    {
        glm::vec3(rcScreen.left, rcScreen.bottom, depth),
        ColorToFloat(element.mTextureColor.GetCurrent()),
        glm::vec2(uvRect.left, uvRect.bottom)
    };

    thisSprite[3] =
    {
        glm::vec3(rcScreen.right, rcScreen.bottom, depth),
        ColorToFloat(element.mTextureColor.GetCurrent()),
        glm::vec2(uvRect.right, uvRect.bottom)
    };

    mDialogManager->mSpriteBuffer.BufferData(thisSprite);

	// Why are we drawing the sprite every time?  This is very inefficient, but the sprite workaround doesn't have support for sorting now, so we have to
	// draw a sprite every time to keep the order correct between sprites and text.
	mDialogManager->EndSprites(&element, textured);
}


//--------------------------------------------------------------------------------------
void Dialog::DrawText(const std::wstring& text, const Element& element, const Rect& rect, bool shadow, bool hardRect)
{
	// No need to draw fully transparent layers
    if (element.mFontColor.GetCurrent().a == 0)
		return;

	Rect screen = rect;
    OffsetRect(screen, mRegion.x, mRegion.y);


	OffsetRect(screen, 0, mCaptionHeight);

	/*if (bShadow)
	{
		Rect rcShadow = rcScreen;
		OffsetRect(rcShadow, 1 / m_pManager->GetWindowSize().x, 1 / m_pManager->GetWindowSize().y);

		Color vShadowColor(0, 0, 0, 255);
		DrawText(*m_pManager->GetFontNode(pElement->mFontIndex), strText, rcShadow, vShadowColor, bCenter, bHardRect);

	}*/

    Color vFontColor = element.mFontColor.GetCurrent();
    Text::DrawText(mDialogManager->GetFontNode(element.mFontIndex), text, screen, element.mFontColor.GetCurrent(), element.mTextFormatFlags, hardRect);
}


//--------------------------------------------------------------------------------------
void Dialog::CalcTextRect(const std::wstring& text, const Element& element, Rect& rect) const
{
	FontNodePtr pFontNode = GetFont(element.mFontIndex);
	if (!pFontNode)
		return;

    GLUF_UNREFERENCED_PARAMETER(text);
    GLUF_UNREFERENCED_PARAMETER(element);
    GLUF_UNREFERENCED_PARAMETER(rect);
	// TODO -

}

//--------------------------------------------------------------------------------------
void Dialog::SetNextDialog(DialogPtr nextDialog) noexcept
{
    NOEXCEPT_REGION_START

    if (!nextDialog)
        mNextDialog = shared_from_this();
    else
    {
        mNextDialog = nextDialog;
        nextDialog->mPrevDialog = shared_from_this();
    }

    NOEXCEPT_REGION_END
}


//--------------------------------------------------------------------------------------
void Dialog::ClearFocus() noexcept
{
    NOEXCEPT_REGION_START

    if (sControlFocus)
    {
        sControlFocus->OnFocusOut();
        sControlFocus = nullptr;
    }

    NOEXCEPT_REGION_END
}


//--------------------------------------------------------------------------------------
void Dialog::FocusDefaultControl() noexcept
{
    NOEXCEPT_REGION_START

    // Check for default control in this dialog
    for (auto it : mControls)
    {
        if (it.second->mIsDefault)
        {
            // Remove focus from the current control
            ClearFocus();

            // Give focus to the default control
            sControlFocus = it.second;
            sControlFocus->OnFocusIn();
            return;
        }
    }

    NOEXCEPT_REGION_END
}


//--------------------------------------------------------------------------------------
bool Dialog::OnCycleFocus(bool forward) noexcept
{
    NOEXCEPT_REGION_START

    ControlPtr pControl = nullptr;
    DialogPtr pDialog = nullptr; // pDialog and pLastDialog are used to track wrapping of
    DialogPtr pLastDialog;    // focus from first control to last or vice versa.

    if (!sControlFocus)
    {
        // If sControlFocus is nullptr, we focus the first control of first dialog in
        // the case that bForward is true, and focus the last control of last dialog when
        // bForward is false.
        //
        if (forward)
        {
            // Search for the first control from the start of the dialog
            // array.
            for (auto it : mDialogManager->mDialogs)
            {
                pDialog = pLastDialog = it;
                if (pDialog && !pDialog->mControls.empty())
                {
                    pControl = pDialog->mControls[0];
                    break;
                }
            }

            if (!pDialog || !pControl)
            {
                // No dialog has been registered yet or no controls have been
                // added to the dialogs. Cannot proceed.
                return true;
            }
        }
        else
        {
            // Search for the first control from the end of the dialog
            // array.
            for (auto it = mDialogManager->mDialogs.crbegin(); it != mDialogManager->mDialogs.crend(); ++it)
            {
                pDialog = pLastDialog = *it;
                if (pDialog && !pDialog->mControls.empty())
                {
                    pControl = pDialog->mControls[pDialog->mControls.size() - 1];
                    break;
                }
            }

            if (!pDialog || !pControl)
            {
                // No dialog has been registered yet or no controls have been
                // added to the dialogs. Cannot proceed.
                return true;
            }
        }
    }
    else if (&sControlFocus->mDialog != this)
    {
        // If a control belonging to another dialog has focus, let that other
        // dialog handle this event by returning false.
        //
        return false;
    }
    else
    {
        // Focused control belongs to this dialog. Cycle to the
        // next/previous control.
        _ASSERT(pControl != 0);

        //this is safe to assume that the dialog is 'this' because of the line 'else if (&sControlFocus->mDialog != this)'
        pLastDialog = shared_from_this();

        pControl = (forward) ? GetNextControl(sControlFocus) : GetPrevControl(sControlFocus);

        //this is kind of inefficient
        if (!(pDialog = mDialogManager->GetDialogPtrFromRef(pControl->mDialog)))
            pDialog = shared_from_this();//not sure if this is what to do if the dialog is not found, but its the best thing I could think of
    }

    _ASSERT(pControl != 0);

    // If we just wrapped from last control to first or vice versa,
    // set the focused control to nullptr. This state, where no control
    // has focus, allows the camera to work.
    int nLastDialogIndex = -1;
    for (uint32_t i = 0; i < mDialogManager->mDialogs.size(); ++i)
    {
        if (mDialogManager->mDialogs[i] == pLastDialog)
        {
            nLastDialogIndex = i;
            break;
        }
    }

    int nDialogIndex = -1;
    for (uint32_t i = 0; i < mDialogManager->mDialogs.size(); ++i)
    {
        if (mDialogManager->mDialogs[i] == pDialog)
        {
            nDialogIndex = i;
            break;
        }
    }

    if ((!forward && nLastDialogIndex < nDialogIndex) ||
        (forward && nDialogIndex < nLastDialogIndex))
    {
        if (sControlFocus)
            sControlFocus->OnFocusOut();
        sControlFocus = nullptr;
        return true;
    }

    // If we've gone in a full circle then focus doesn't change
    if (pControl == sControlFocus)
        return true;

    // If the dialog accepts keybord input and the control can have focus then
    // move focus
    if (pControl->mDialog.mKeyboardInput && pControl->CanHaveFocus())
    {
        if (sControlFocus)
            sControlFocus->OnFocusOut();
        sControlFocus = pControl;
        if (sControlFocus)
            sControlFocus->OnFocusIn();
        return true;
    }

    pLastDialog = pDialog;
    pControl = (forward) ? GetNextControl(pControl) : GetPrevControl(pControl);
    if (!(pDialog = mDialogManager->GetDialogPtrFromRef(pControl->mDialog)))
        pDialog = shared_from_this();

    // If we reached this point, the chain of dialogs didn't form a complete loop
    GLUF_ERROR("Dialog: Multiple dialogs are improperly chained together");
    return false;

    NOEXCEPT_REGION_END

    return false;
}

FontPtr g_ArialDefault = nullptr;
//--------------------------------------------------------------------------------------
void Dialog::InitDefaultElements()
{
	//this makes it more efficient
	int fontIndex = 0;
	if (g_DefaultFont == nullptr)
	{
		if (g_ArialDefault == nullptr)
		{

            std::vector<char> rawData;
            LoadFileIntoMemory(L"Arial.ttf", rawData);
			LoadFont(g_ArialDefault, rawData, 15L);
		}

		fontIndex = mDialogManager->AddFont(g_ArialDefault, 20, FONT_WEIGHT_NORMAL);
	}
	else
	{
        fontIndex = mDialogManager->AddFont(g_DefaultFont, 20, FONT_WEIGHT_NORMAL);
	}

	SetFont(0, fontIndex);

	Element Element;
	Rectf rcTexture;

	//-------------------------------------
	// Element for the caption
	//-------------------------------------
    //mCapElement = Element();
	mCapElement.SetFont(0);
	SetRect(rcTexture, 0.0f, 0.078125f, 0.4296875f, 0.0f);//blank part of the texture
    mCapElement.SetTexture(0, rcTexture);
    mCapElement.mTextureColor.Init({ 255, 255, 255, 255 });
    mCapElement.mFontColor.Init({ 255, 255, 255, 255 });
    mCapElement.SetFont(0, { 0, 0, 0, 255 }, GT_LEFT | GT_VCENTER);
	// Pre-blend as we don't need to transition the state
    mCapElement.mTextureColor.Blend(STATE_NORMAL, 10.0f);
    mCapElement.mFontColor.Blend(STATE_NORMAL, 10.0f);

    //mDlgElement = Element()
	mDlgElement.SetFont(0);
	SetRect(rcTexture, 0.0f, 0.078125f, 0.4296875f, 0.0f);//blank part of the texture
	//SetRect(rcTexture, 0.0f, 1.0f, 1.0f, 0.0f);//blank part of the texture
	mDlgElement.SetTexture(0, rcTexture);
    mDlgElement.mTextureColor.Init({ 255, 0, 0, 128 });
    mDlgElement.mFontColor.Init({ 0, 0, 0, 255 });
    mDlgElement.SetFont(0, { 0, 0, 0, 255 }, GT_LEFT | GT_VCENTER);
	// Pre-blend as we don't need to transition the state
	mDlgElement.mTextureColor.Blend(STATE_NORMAL, 10.0f);
	mDlgElement.mFontColor.Blend(STATE_NORMAL, 10.0f);

    /*
    
    Streamline the control blending
    
    */
	Element.mFontColor.SetState(STATE_NORMAL, {0, 0, 0, 255});
	Element.mFontColor.SetState(STATE_DISABLED, {0, 0, 0, 128});
	Element.mFontColor.SetState(STATE_HIDDEN, {0, 0, 0, 0});
	Element.mFontColor.SetState(STATE_FOCUS, {0, 0, 0, 255});
	Element.mFontColor.SetState(STATE_MOUSEOVER, {0, 0, 0, 255});
	Element.mFontColor.SetState(STATE_PRESSED, {0, 0, 0, 255});
    Element.mFontColor.SetCurrent(STATE_NORMAL);

    Element.mTextureColor.SetState(STATE_NORMAL, { 180, 180, 180, 255 });
    Element.mTextureColor.SetState(STATE_DISABLED, { 128, 128, 128, 128 });
    Element.mTextureColor.SetState(STATE_HIDDEN, { 0, 0, 0, 0 });
    Element.mTextureColor.SetState(STATE_FOCUS, { 200, 200, 200, 255 });
    Element.mTextureColor.SetState(STATE_MOUSEOVER, { 255, 255, 255, 255 });
    Element.mTextureColor.SetState(STATE_PRESSED, { 200, 200, 200, 255 });
    Element.mTextureColor.SetCurrent(STATE_NORMAL);

    Element.mFontIndex = 0;
    Element.mTextureIndex = 0;

	//-------------------------------------
	// Static
	//-------------------------------------
	Element.mTextFormatFlags = GT_LEFT | GT_VCENTER;

	// Assign the Element
	SetDefaultElement(CONTROL_STATIC, 0, Element);


	//-------------------------------------
	// Button - Button
	//-------------------------------------
	SetRect(rcTexture, 0.0f, 1.0f, 0.53125f, 0.7890625f);
    Element.mUVRect = rcTexture;

	// Assign the Element
	SetDefaultElement(CONTROL_BUTTON, 0, Element);


	//-------------------------------------
	// Button - Fill layer
	//-------------------------------------
    SetRect(rcTexture, 0.53125f, 1.0f, 0.984375f, 0.7890625f);
    Element.mUVRect = rcTexture;

	// Assign the Element
	SetDefaultElement(CONTROL_BUTTON, 1, Element);


	//-------------------------------------
	// CheckBox - Box
	//-------------------------------------
	SetRect(rcTexture, 0.0f, 0.7890625f, 0.10546875f, 0.68359375f);
    Element.mUVRect = rcTexture;

	// Assign the Element
	SetDefaultElement(CONTROL_CHECKBOX, 0, Element);


	//-------------------------------------
	// CheckBox - Check
	//-------------------------------------
    SetRect(rcTexture, 0.10546875f, 0.7890625f, 0.2109375f, 0.68359375f);
    Element.mUVRect = rcTexture;

	// Assign the Element
	SetDefaultElement(CONTROL_CHECKBOX, 1, Element);


	//-------------------------------------
	// RadioButton - Box
	//-------------------------------------
    SetRect(rcTexture, 0.2109375f, 0.7890625f, 0.31640625f, 0.68359375f);
    Element.mUVRect = rcTexture;

	// Assign the Element
	SetDefaultElement(CONTROL_RADIOBUTTON, 0, Element);


	//-------------------------------------
	// RadioButton - Check
	//-------------------------------------
    SetRect(rcTexture, 0.31640625f, 0.7890625f, 0.421875f, 0.68359375f);
    Element.mUVRect = rcTexture;

	// Assign the Element
	SetDefaultElement(CONTROL_RADIOBUTTON, 1, Element);


	//-------------------------------------
	// ComboBox - Main
	//-------------------------------------
    //SetRect(rcTexture, 0.02734375f, 0.5234375f, 0.96484375f, 0.3671875f);
    SetRect(rcTexture, 0.05078125f, 0.5234375f, 0.96484375f, 0.3671875f);
    Element.mUVRect = rcTexture;


	// Assign the Element
	SetDefaultElement(CONTROL_COMBOBOX, 0, Element);


	//-------------------------------------
	// ComboBox - Button
	//-------------------------------------
    SetRect(rcTexture, 0.3828125f, 0.26171875f, 0.58984375f, 0.0703125f);
    Element.mUVRect = rcTexture;

	// Assign the Element
	SetDefaultElement(CONTROL_COMBOBOX, 1, Element);


	//-------------------------------------
	// ComboBox - Dropdown
	//-------------------------------------
    SetRect(rcTexture, 0.05078125f, 0.51953125f, 0.94140625f, 0.37109375f);
    Element.mUVRect = rcTexture;
    Element.mTextFormatFlags = GT_LEFT | GT_TOP;

	// Assign the Element
	SetDefaultElement(CONTROL_COMBOBOX, 2, Element);


	//-------------------------------------
	// ComboBox - Selection
	//-------------------------------------
    SetRect(rcTexture, 0.046875f, 0.36328125f, 0.93359375f, 0.28515625f);
    Element.mUVRect = rcTexture;

	// Assign the Element
	SetDefaultElement(CONTROL_COMBOBOX, 3, Element);


	//-------------------------------------
	// Slider - Track
	//-------------------------------------
    SetRect(rcTexture, 0.00390625f, 0.26953125f, 0.36328125f, 0.109375f);
    Element.mUVRect = rcTexture;

	// Assign the Element
	SetDefaultElement(CONTROL_SLIDER, 0, Element);

	//-------------------------------------
	// Slider - Button
	//-------------------------------------
    SetRect(rcTexture, 0.58984375f, 0.24609375f, 0.75f, 0.0859375f);
    Element.mUVRect = rcTexture;

	// Assign the Element
	SetDefaultElement(CONTROL_SLIDER, 1, Element);

	//-------------------------------------
	// ScrollBar - Track
	//-------------------------------------
	float nScrollBarStartX = 0.76470588f;
	float nScrollBarStartY = 0.046875f;
    SetRect(rcTexture, nScrollBarStartX + 0.0f, nScrollBarStartY + 0.12890625f, nScrollBarStartX + 0.09076287f, nScrollBarStartY + 0.125f);
    Element.mUVRect = rcTexture;

	// Assign the Element
	SetDefaultElement(CONTROL_SCROLLBAR, 0, Element);

	//-------------------------------------
	// ScrollBar - Down Arrow
	//-------------------------------------
    SetRect(rcTexture, nScrollBarStartX + 0.0f, nScrollBarStartY + 0.08203125f, nScrollBarStartX + 0.09076287f, nScrollBarStartY + 0.00390625f);
    Element.mUVRect = rcTexture;


	// Assign the Element
	SetDefaultElement(CONTROL_SCROLLBAR, 2, Element);

	//-------------------------------------
	// ScrollBar - Up Arrow
	//-------------------------------------
    SetRect(rcTexture, nScrollBarStartX + 0.0f, nScrollBarStartY + 0.20703125f, nScrollBarStartX + 0.09076287f, nScrollBarStartY + 0.125f);
    Element.mUVRect = rcTexture;


	// Assign the Element
	SetDefaultElement(CONTROL_SCROLLBAR, 1, Element);

	//-------------------------------------
	// ScrollBar - Button
	//-------------------------------------
    SetRect(rcTexture, 0.859375f, 0.25f, 0.9296875f, 0.0859375f);
    Element.mUVRect = rcTexture;

	// Assign the Element
	SetDefaultElement(CONTROL_SCROLLBAR, 3, Element);

	//-------------------------------------
	// EditBox
	//-------------------------------------
	// Element assignment:
	//   0 - text area
	//   1 - top left border
	//   2 - top border
	//   3 - top right border
	//   4 - left border
	//   5 - right border
	//   6 - lower left border
	//   7 - lower border
	//   8 - lower right border

	//TODO: this
	// Assign the style
    SetRect(rcTexture, 0.0546875f, 0.6484375f, 0.94140625f, 0.55859375f);
    Element.mUVRect = rcTexture;
	SetDefaultElement(CONTROL_EDITBOX, 0, Element);

    SetRect(rcTexture, 0.03125f, 0.6796875f, 0.0546875f, 0.6484375f);
    Element.mUVRect = rcTexture;
	SetDefaultElement(CONTROL_EDITBOX, 1, Element);

    SetRect(rcTexture, 0.0546875f, 0.6796875f, 0.94140625f, 0.6484375f);
    Element.mUVRect = rcTexture;
	SetDefaultElement(CONTROL_EDITBOX, 2, Element);

    SetRect(rcTexture, 0.94140625f, 0.6796875f, 0.9609375f, 0.6484375f);
    Element.mUVRect = rcTexture;
	SetDefaultElement(CONTROL_EDITBOX, 3, Element);

    SetRect(rcTexture, 0.03125f, 0.6484375f, 0.0546875f, 0.55859375f);
    Element.mUVRect = rcTexture;
	SetDefaultElement(CONTROL_EDITBOX, 4, Element);

    SetRect(rcTexture, 0.94140625f, 0.6484375f, 0.9609375f, 0.55859375f);
    Element.mUVRect = rcTexture;
	SetDefaultElement(CONTROL_EDITBOX, 5, Element);

    SetRect(rcTexture, 0.03125f, 0.55859375f, 0.0546875f, 0.52734375f);
    Element.mUVRect = rcTexture;
	SetDefaultElement(CONTROL_EDITBOX, 6, Element);

    SetRect(rcTexture, 0.0546875f, 0.55859375f, 0.94140625f, 0.52734375f);
    Element.mUVRect = rcTexture;
	SetDefaultElement(CONTROL_EDITBOX, 7, Element);

    SetRect(rcTexture, 0.94140625f, 0.55859375f, 0.9609375f, 0.52734375f);
    Element.mUVRect = rcTexture;
	SetDefaultElement(CONTROL_EDITBOX, 8, Element);

	//-------------------------------------
	// ListBox - Main
	//-------------------------------------
    SetRect(rcTexture, 0.05078125f, 0.51953125f, 0.94140625f, 0.375f);
    Element.mUVRect = rcTexture;

	// Assign the Element
	SetDefaultElement(CONTROL_LISTBOX, 0, Element);

	//-------------------------------------
	// ListBox - Selection
	//-------------------------------------

    SetRect(rcTexture, 0.0625f, 0.3515625f, 0.9375f, 0.28515625f);
    Element.mUVRect = rcTexture;

	// Assign the Element
	SetDefaultElement(CONTROL_LISTBOX, 1, Element);
}




/*
======================================================================================================================================================================================================
DialogResourceManager Functions


*/

//--------------------------------------------------------------------------------------
DialogResourceManager::DialogResourceManager() :
    mSpriteBuffer(GL_TRIANGLES, GL_STREAM_DRAW)//use stream draw because it will be changed every frame
{
	//glGenVertexArrayBindVertexArray(&m_pVBScreenQuadVAO);
	//glGenBuffers(1, &m_pVBScreenQuadIndicies);
	//glGenBuffers(1, &m_pVBScreenQuadPositions);
	//glGenBuffers(1, &m_pVBScreenQuadColor);
	//glGenBuffers(1, &m_pVBScreenQuadUVs);

	/*glGenBufferBindBuffer(GL_ARRAY_BUFFER, &m_pVBScreenQuadPositions);
	glVertexAttribPointer(g_UIShaderLocations.position, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

	glGenBufferBindBuffer(GL_ARRAY_BUFFER, &m_pVBScreenQuadColor);
	glVertexAttribPointer(g_UIShaderLocations.color, 4, GL_FLOAT, GL_FALSE, 0, nullptr);

	glGenBufferBindBuffer(GL_ARRAY_BUFFER, &m_pVBScreenQuadUVs);
	glVertexAttribPointer(g_UIShaderLocations.uv, 2, GL_FLOAT, GL_FALSE, 0, nullptr);*/

	//this is static
	//glGenBufferBindBuffer(GL_ELEMENT_ARRAY_BUFFER, &m_pVBScreenQuadIndicies);

	//GLubyte indices[6] = {	2, 1, 0, 
	//						2, 3, 1};
	//glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6 * sizeof(GLubyte), indices, GL_STATIC_DRAW);


	//glGenVertexArrayBindVertexArray(&m_SpriteBufferVao);
	//glBindVertexArray(m_SpriteBufferVao);

	//glGenBuffers(1, &m_SpriteBufferPos);
	//glGenBuffers(1, &m_SpriteBufferColors);
	//glGenBuffers(1, &m_SpriteBufferTexCoords);
	//glGenBuffers(1, &m_SpriteBufferIndices);

    mSpriteBuffer.AddVertexAttrib({ 4, 3, g_UIShaderLocations.position, GL_FLOAT, 0 }, 0);
    mSpriteBuffer.AddVertexAttrib({ 4, 4, g_UIShaderLocations.color, GL_FLOAT, 0 }, 12);
    mSpriteBuffer.AddVertexAttrib({ 4, 2, g_UIShaderLocations.sampler, GL_FLOAT, 0 }, 28);

	//this is static
	//glGenBufferBindBuffer(GL_ELEMENT_ARRAY_BUFFER, &m_SpriteBufferIndices);

    mSpriteBuffer.BufferIndices(
    { 
        2, 1, 0,
        2, 3, 1 
    });

	//GLubyte indicesS[6] = { 2, 1, 0,
	//						2, 3, 1 };
	//glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6 * sizeof(GLubyte), indicesS, GL_STATIC_DRAW);


	GetWindowSize();
}


//--------------------------------------------------------------------------------------
DialogResourceManager::~DialogResourceManager()
{
	//mFontCache.clear();
	//mTextureCache.clear();

	//TODO: make this with a class in the buffer sections
	/*glBindVertexArray(m_pVBScreenQuadVAO);
	glDeleteBuffers(1, &m_pVBScreenQuadPositions);
	glDeleteBuffers(1, &m_pVBScreenQuadColor);
	glDeleteBuffers(1, &m_pVBScreenQuadUVs);
	glDeleteVertexArrays(1, &m_pVBScreenQuadVAO);

	glBindVertexArray(m_SpriteBufferVao);
	glDeleteBuffers(1, &m_SpriteBufferPos);
	glDeleteBuffers(1, &m_SpriteBufferColors);
	glDeleteBuffers(1, &m_SpriteBufferTexCoords);
	glDeleteVertexArrays(1, &m_SpriteBufferVao);
	glBindVertexArray(0);*/
}


//--------------------------------------------------------------------------------------
bool DialogResourceManager::MsgProc(MessageType msg, int32_t param1, int32_t param2, int32_t param3, int32_t param4) noexcept
{
    NOEXCEPT_REGION_START

    GLUF_UNREFERENCED_PARAMETER(msg);
    //GLUF_UNREFERENCED_PARAMETER(param1);
    //GLUF_UNREFERENCED_PARAMETER(param2);
    GLUF_UNREFERENCED_PARAMETER(param3);
    GLUF_UNREFERENCED_PARAMETER(param4);

    switch (msg)
    {
    case RESIZE:
        mWndSize.width = 0L;
        mWndSize.height = 0L;
        GetWindowSize();

        //refresh the fonts to the new window size
        /*for (auto it : m_FontCache)
            if (it != nullptr)
            it->mFontType->Refresh();*/
    }

    return false;

    NOEXCEPT_REGION_END

    return false;
}

//--------------------------------------------------------------------------------------
void DialogResourceManager::ApplyRenderUI() noexcept
{
    NOEXCEPT_REGION_START

        // Shaders
        /*glEnableVertexAttribArray(g_UIShaderLocations.position);
        glEnableVertexAttribArray(g_UIShaderLocations.color);
        glEnableVertexAttribArray(g_UIShaderLocations.uv);*/
    SHADERMANAGER.UseProgram(g_UIProgram);

    ApplyOrtho();

    NOEXCEPT_REGION_END
}


//--------------------------------------------------------------------------------------
void DialogResourceManager::ApplyRenderUIUntex() noexcept
{
    NOEXCEPT_REGION_START

    /*glEnableVertexAttribArray(g_UIShaderLocationsUntex.position);
    glEnableVertexAttribArray(g_UIShaderLocationsUntex.color);*/
    SHADERMANAGER.UseProgram(g_UIProgramUntex);

    ApplyOrtho();

    NOEXCEPT_REGION_END
}

glm::mat4 DialogResourceManager::GetOrthoMatrix() noexcept
{
	Point pt = GetWindowSize();
	float x2 = (float)pt.x / 2.0f;
	float y2 = (float)pt.y / 2.0f;
	return glm::ortho((float)-x2, (float)x2, (float)-y2, (float)y2);
}

DialogPtr DialogResourceManager::GetDialogPtrFromRef(const Dialog& ref) noexcept
{
    for (auto it = mDialogs.cbegin(); it != mDialogs.cend(); ++it)
    {
        if (it->get() == &ref)
        {
            return *it;
        }
    }

    return nullptr;
}

void DialogResourceManager::ApplyOrtho() noexcept
{
	glm::mat4 mat = GetOrthoMatrix();
	glUniformMatrix4fv(g_UIShaderLocations.ortho, 1, GL_FALSE, &mat[0][0]);
}

//--------------------------------------------------------------------------------------
void DialogResourceManager::BeginSprites() noexcept
{
}


//--------------------------------------------------------------------------------------

void DialogResourceManager::EndSprites(const Element* element, bool textured)
{
	/*if (textured)
	{
        mSpriteBuffer.EnableVertexAttribute(2);
	}
	else
	{
        mSpriteBuffer.DisableVertexAttribute(2);
	}*/

	
	if (textured && element)
	{
		ApplyRenderUI();

		TextureNodePtr pTexture = GetTextureNode(element->mTextureIndex);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, pTexture->mTextureElement);
        glUniform1i(g_UIShaderLocations.sampler, 0);
	}
	else
	{
        ApplyRenderUIUntex();
	}

	
    mSpriteBuffer.Draw();
}


//--------------------------------------------------------------------------------------
void DialogResourceManager::RegisterDialog(const DialogPtr& dialog) noexcept
{
    NOEXCEPT_REGION_START

    if (!dialog)
        return;

    // Check that the dialog isn't already registered.
    for (auto it : mDialogs)
    {
        if (it == dialog)
            return;
    }

    // Add to the list.
    mDialogs.push_back(dialog);

    // Set up next and prev pointers.
    if (mDialogs.size() > 1)
        mDialogs[mDialogs.size() - 2]->SetNextDialog(dialog);
    mDialogs[mDialogs.size() - 1]->SetNextDialog(mDialogs[0]);

    NOEXCEPT_REGION_END
}


//--------------------------------------------------------------------------------------
void DialogResourceManager::UnregisterDialog(const DialogPtr& pDialog)
{
	// Search for the dialog in the list.
	for (size_t i = 0; i < mDialogs.size(); ++i)
	{
		if (mDialogs[i] == pDialog)
		{
			mDialogs.erase(mDialogs.begin() + i);
			if (!mDialogs.empty())
			{
				int l, r;

				if (0 == i)
					l = int(mDialogs.size() - 1);
				else
					l = int(i) - 1;

				if (mDialogs.size() == i)
					r = 0;
				else
					r = int(i);

				mDialogs[l]->SetNextDialog(mDialogs[r]);
			}
			return;
		}
	}
}


//--------------------------------------------------------------------------------------
void DialogResourceManager::EnableKeyboardInputForAllDialogs() noexcept
{
    NOEXCEPT_REGION_START

    // Enable keyboard input for all registered dialogs
    for (auto it : mDialogs)
        it->EnableKeyboardInput(true);

    NOEXCEPT_REGION_END
}

//--------------------------------------------------------------------------------------
Point DialogResourceManager::GetWindowSize()
{
	if (mWndSize.x == 0L || mWndSize.y == 0L)
	{
		int w, h;
		glfwGetWindowSize(g_pGLFWWindow, &w, &h);
		mWndSize.width = (long)w;
		mWndSize.height = (long)h;
		g_WndHeight = (unsigned short)h;
		g_WndWidth = (unsigned short)w;
	}
	return mWndSize;
}

//--------------------------------------------------------------------------------------
FontIndex DialogResourceManager::AddFont(const FontPtr& font, FontSize leading, FontWeight weight) noexcept
{
    NOEXCEPT_REGION_START

    // See if this font already exists (this is simple)
    for (size_t i = 0; i < mFontCache.size(); ++i)
    {
        FontNodePtr node = mFontCache[i];
        if (node->mFontType == font && node->mWeight == weight && node->mLeading == leading)
            return i;
    }

    // Add a new font and try to create it
    auto newFontNode = std::make_shared<FontNode>();

    //wcscpy_s(pNewFontNode->strFace, MAX_PATH, strFaceName);
    newFontNode->mFontType = font;
    newFontNode->mLeading = leading;
    //pNewFontNode->mSize = height;
    newFontNode->mWeight = weight;
    mFontCache.push_back(newFontNode);

    return mFontCache.size() - 1;

    NOEXCEPT_REGION_END
}


//--------------------------------------------------------------------------------------
TextureIndex DialogResourceManager::AddTexture(GLuint texture) noexcept
{
    NOEXCEPT_REGION_START

    // See if this texture already exists
    for (size_t i = 0; i < mTextureCache.size(); ++i)
    {
        TextureNodePtr pTextureNode = mTextureCache[i];
        if (texture == pTextureNode->mTextureElement)
            return i;
    }

    // Add a new texture and try to create it
    auto newTextureNode = std::make_shared<TextureNode>();

    newTextureNode->mTextureElement = texture;
    mTextureCache.push_back(newTextureNode);

    return mTextureCache.size() - 1;

    NOEXCEPT_REGION_END
}


/*
======================================================================================================================================================================================================
Control Functions


*/

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
    NOEXCEPT_REGION_START

    Element& element = mElements[0];

    element.mFontColor.mStates[STATE_NORMAL] = color;

    NOEXCEPT_REGION_END
}


//--------------------------------------------------------------------------------------
void Control::SetElement(ElementIndex elementId, const Element& element) noexcept
{
    NOEXCEPT_REGION_START

    mElements[elementId] = element;

    NOEXCEPT_REGION_END
}


//--------------------------------------------------------------------------------------
void Control::Refresh()
{
	mMouseOver = false;
	mHasFocus = false;

	for (auto it : mElements)
	{
		it.second.Refresh();
	}
}


/*
======================================================================================================================================================================================================
Static Functions


*/

//--------------------------------------------------------------------------------------
Static::Static(const Bitfield& textFlags, Dialog& dialog) : Control(dialog), mTextFlags(textFlags)
{
	mType = CONTROL_STATIC;
}


//--------------------------------------------------------------------------------------
void Static::Render(float elapsedTime) noexcept
{
    NOEXCEPT_REGION_START

    if (!mVisible)
        return;

    ControlState state = STATE_NORMAL;

    if (mEnabled == false)
        state = STATE_DISABLED;

    Element& element = mElements[0];
    element.mTextFormatFlags = mTextFlags;

    element.mFontColor.Blend(state, elapsedTime);

    mDialog.DrawText(mText, element, mRegion, false, false);

    NOEXCEPT_REGION_END
}


/*
======================================================================================================================================================================================================
Button Functions


*/

Button::Button(Dialog& dialog) : Static(GT_CENTER | GT_VCENTER, dialog)
{
	mType = CONTROL_BUTTON;

	mPressed = false;
}

//--------------------------------------------------------------------------------------
void Button::OnHotkey() noexcept
{
    NOEXCEPT_REGION_START

    if (mDialog.IsKeyboardInputEnabled())
    {
        mDialog.RequestFocus(shared_from_this());
        mDialog.SendEvent(_EVENT_BUTTON_CLICKED, true, shared_from_this());
    }

    NOEXCEPT_REGION_END
}

//--------------------------------------------------------------------------------------
bool Button::MsgProc(MessageType msg, int32_t param1, int32_t param2, int32_t param3, int32_t param4) noexcept
{
    NOEXCEPT_REGION_START

    if (!mEnabled || !mVisible)
        return false;

    Point mousePos = mDialog.GetMousePositionDialogSpace();

    switch (msg)
    {

    case CURSOR_POS:

        if (mPressed)
        {
            //if the button is pressed and the mouse is moved off, then unpress it
            if (!ContainsPoint(mousePos))
            {
                mPressed = false;

                ContainsPoint(mousePos);

                if (!mDialog.IsKeyboardInputEnabled())
                    mDialog.ClearFocus();
            }
        }

        break;
    case MB:
        if (param1 == GLFW_MOUSE_BUTTON_LEFT)
        {
            if (param2 == GLFW_PRESS)
            {
                if (ContainsPoint(mousePos))
                {
                    // Pressed while inside the control
                    mPressed = true;
                    //SetCapture(GetHWND());

                    if (!mHasFocus)
                        mDialog.RequestFocus(shared_from_this());

                    return true;

                }
            }
            else if (param2 == GLFW_RELEASE)
            {
                if (mPressed)
                {
                    mPressed = false;

                    if (!mDialog.IsKeyboardInputEnabled())
                        mDialog.ClearFocus();

                    // Button click
                    if (ContainsPoint(mousePos))
                        mDialog.SendEvent(_EVENT_BUTTON_CLICKED, true, shared_from_this());

                    return true;
                }

            }
        }
    break;

    case KEY:
    {

        if (param1 == mHotkey)
        {
            if (param3 == GLFW_PRESS)
            {
                mPressed = true;
            }
            if (param3 == GLFW_RELEASE)
            {
                mPressed = false;

                mDialog.SendEvent(_EVENT_BUTTON_CLICKED, true, shared_from_this());
            }

            return true;
        }

        return true;
    }
    };

    return false;

    NOEXCEPT_REGION_END

    return false;
}

//--------------------------------------------------------------------------------------
void Button::Render(float elapsedTime) noexcept
{
    NOEXCEPT_REGION_START

	int nOffsetX = 0;
	int nOffsetY = 0;

	Point wndSize = mDialog.GetManager()->GetWindowSize();

	ControlState iState = STATE_NORMAL;

	if (mVisible == false)
	{
		iState = STATE_HIDDEN;
	}
	else if (mEnabled == false)
	{
		iState = STATE_DISABLED;
	}
	else if (mPressed)
	{
		iState = STATE_PRESSED;

		nOffsetX = 1;
		nOffsetY = 2;
	}
	else if (mMouseOver)
	{
		iState = STATE_MOUSEOVER;

		nOffsetX = -1;
		nOffsetY = -2;
	}
	else if (mHasFocus)
	{
		iState = STATE_FOCUS;
	}

    float fBlendRate = 5.0f;//(iState == STATE_PRESSED) ? 0.0f : 0.8f;

	Rect rcWindow = mRegion;
	OffsetRect(rcWindow, nOffsetX, nOffsetY);


	// Background fill layer
	Element* pElement = &mElements[0];

	// Blend current color
	pElement->mTextureColor.Blend(iState, elapsedTime, fBlendRate);
	pElement->mFontColor.Blend(iState, elapsedTime, fBlendRate);

	mDialog.DrawSprite(*pElement, rcWindow, _FAR_BUTTON_DEPTH);
	//mDialog.DrawText(m_strText, pElement, rcWindow, false, true);

	// Main button
	pElement = &mElements[1];

	// Blend current color
	pElement->mTextureColor.Blend(iState, elapsedTime, fBlendRate);
	pElement->mFontColor.Blend(iState, elapsedTime, fBlendRate);

	mDialog.DrawSprite(*pElement, rcWindow, _NEAR_BUTTON_DEPTH);
	mDialog.DrawText(mText, *pElement, rcWindow, false, true);

    NOEXCEPT_REGION_END
}


/*
======================================================================================================================================================================================================
CheckBox Functions


*/

CheckBox::CheckBox(bool checked, Dialog& dialog) : Button(dialog)
{
	mType = CONTROL_CHECKBOX;

    mChecked = checked;
}

//--------------------------------------------------------------------------------------

bool CheckBox::MsgProc(MessageType msg, int32_t param1, int32_t param2, int32_t param3, int32_t param4) noexcept
{
    NOEXCEPT_REGION_START

    if (!mEnabled || !mVisible)
        return false;

    Point mousePos = mDialog.GetMousePositionDialogSpace();

    switch (msg)
    {

    case CURSOR_POS:

        if (mPressed)
        {
            //if the button is pressed and the mouse is moved off, then unpress it
            if (!ContainsPoint(mousePos))
            {
                mPressed = false;

                //ContainsPoint(mousePos);

                if (!mDialog.IsKeyboardInputEnabled())
                    mDialog.ClearFocus();
            }
        }

        break;
    case MB:
    {
        if (param1 == GLFW_MOUSE_BUTTON_LEFT)
        {
            if (param2 == GLFW_PRESS)
            {
                if (ContainsPoint(mousePos))
                {
                    // Pressed while inside the control
                    mPressed = true;
                    //SetCapture(GetHWND());

                    if (!mHasFocus)
                        mDialog.RequestFocus(shared_from_this());

                    return true;

                }
            }
            else if (param2 == GLFW_RELEASE)
            {
                if (mPressed && ContainsPoint(mousePos))
                {
                    mPressed = false;
                    //ReleaseCapture();

                    if (!mDialog.IsKeyboardInputEnabled())
                        mDialog.ClearFocus();

                    // Button click
                    if (ContainsPoint(mousePos))
                        SetCheckedInternal(!mChecked, true);

                    return true;
                }

            }
        }
        break;
    }

    /*case KEY:
    {

        if (param1 = GLFW_KEY_SPACE)
        {
            if (param3 == GLFW_PRESS)
            {
                mPressed = true;
            }
            if (param3 == GLFW_RELEASE)
            {
                mPressed = false;

                SetCheckedInternal(!mChecked, true);
            }

            return true;
        }


        return true;

        break;
    }*/
    };

    return false;

    NOEXCEPT_REGION_END

    return false;
}


//--------------------------------------------------------------------------------------
void CheckBox::SetCheckedInternal(bool checked, bool fromInput)
{
    mChecked = checked;

    mDialog.SendEvent(_EVENT_CHECKBOXCHANGED, fromInput, shared_from_this());
}


//--------------------------------------------------------------------------------------
bool CheckBox::ContainsPoint(const Point& pt) const noexcept
{
    NOEXCEPT_REGION_START

    return (PtInRect(mRegion, pt) ||
    PtInRect(mButtonRegion, pt) ||
    PtInRect(mTextRegion, pt));

    NOEXCEPT_REGION_END
}


//--------------------------------------------------------------------------------------
void CheckBox::UpdateRects() noexcept
{
    NOEXCEPT_REGION_START

    Button::UpdateRects();

    mButtonRegion = mRegion;
    mButtonRegion.right = mButtonRegion.left + RectHeight(mButtonRegion);

    mTextRegion = mRegion;
    OffsetRect(mTextRegion, RectWidth(mButtonRegion)/*1.25f*/, 0L);

    //resize the text rect based on the length of the string
    mTextRegion.right = mTextRegion.left + mDialog.GetFont(mElements[0].mFontIndex)->mFontType->GetStringWidth(mText);

    NOEXCEPT_REGION_END
}


//--------------------------------------------------------------------------------------
void CheckBox::Render(float elapsedTime) noexcept
{
    NOEXCEPT_REGION_START
	ControlState iState = STATE_NORMAL;

	if (mVisible == false)
		iState = STATE_HIDDEN;
	else if (mEnabled == false)
		iState = STATE_DISABLED;
	else if (mPressed)
		iState = STATE_PRESSED;
	else if (mMouseOver)
		iState = STATE_MOUSEOVER;
	else if (mHasFocus)
		iState = STATE_FOCUS;

	Element* pElement = &mElements[0];

	float fBlendRate = 5.0f;

	pElement->mTextureColor.Blend(iState, elapsedTime, fBlendRate);
	pElement->mFontColor.Blend(iState, elapsedTime, fBlendRate);

	mDialog.DrawSprite(*pElement, mButtonRegion, _FAR_BUTTON_DEPTH);
	mDialog.DrawText(mText, *pElement, mTextRegion, false, false);

	if (mChecked)
	{
		pElement = &mElements[1];

		pElement->mTextureColor.Blend(iState, elapsedTime, fBlendRate);
		mDialog.DrawSprite(*pElement, mButtonRegion, _NEAR_BUTTON_DEPTH);
	}

    NOEXCEPT_REGION_END
}


//--------------------------------------------------------------------------------------
void CheckBox::OnHotkey() noexcept
{
    NOEXCEPT_REGION_START

    if (mDialog.IsKeyboardInputEnabled())
        mDialog.RequestFocus(shared_from_this());
    SetCheckedInternal(!mChecked, true);

    NOEXCEPT_REGION_END
}


/*
======================================================================================================================================================================================================
RadioButton Functions


*/

RadioButton::RadioButton(Dialog& dialog) : CheckBox(false, dialog)
{
	mType = CONTROL_RADIOBUTTON;
}

//--------------------------------------------------------------------------------------
bool RadioButton::MsgProc(MessageType msg, int32_t param1, int32_t param2, int32_t param3, int32_t param4) noexcept
{
    NOEXCEPT_REGION_START

    if (!mEnabled || !mVisible)
        return false;

    Point mousePos = mDialog.GetMousePositionDialogSpace();

    switch (msg)
    {

    case CURSOR_POS:

        if (mPressed)
        {
            //if the button is pressed and the mouse is moved off, then unpress it
            if (!ContainsPoint(mousePos))
            {
                mPressed = false;

                if (!mDialog.IsKeyboardInputEnabled())
                    mDialog.ClearFocus();
            }
        }

        break;
    case MB:
    {
        if (param1 == GLFW_MOUSE_BUTTON_LEFT)
        {
            if (param2 == GLFW_PRESS)
            {
                if (ContainsPoint(mousePos))
                {
                    // Pressed while inside the control
                    mPressed = true;
                    //SetCapture(GetHWND());

                    if (!mHasFocus)
                        mDialog.RequestFocus(shared_from_this());

                    return true;

                }
            }
            else if (param2 == GLFW_RELEASE)
            {
                if (mPressed || !mChecked)
                {
                    if (ContainsPoint(mousePos))
                    {
                        mPressed = false;

                        SetCheckedInternal(true, true, true);
                        return true;
                    }
                }

            }
        }
    }
    break;

    case KEY:
    {

        if (param1 = GLFW_KEY_SPACE)
        {
            if (param3 == GLFW_PRESS)
            {
                mPressed = true;
            }
            else if (param3 == GLFW_RELEASE)
            {
                if (mChecked)
                    return false;

                mPressed = false;

                SetCheckedInternal(true, true, true);
            }
        }


        return true;

        break;
    }
    };

    return false;

    NOEXCEPT_REGION_END

    return false;
}

//--------------------------------------------------------------------------------------
void RadioButton::SetCheckedInternal(bool checked, bool clearGroup, bool fromInput)
{
    if (checked && clearGroup)
		mDialog.ClearRadioButtonGroup(mButtonGroup);

	mChecked = checked;
	mDialog.SendEvent(_EVENT_RADIOBUTTON_CHANGED, fromInput, shared_from_this());
}

//--------------------------------------------------------------------------------------
void RadioButton::OnHotkey() noexcept
{
    NOEXCEPT_REGION_START

    if (mDialog.IsKeyboardInputEnabled())
        mDialog.RequestFocus(shared_from_this());

    SetCheckedInternal(true, true, true);

    NOEXCEPT_REGION_END
}

//--------------------------------------------------------------------------------------
void RadioButton::OnMouseEnter() noexcept
{
    NOEXCEPT_REGION_START

    mMouseOver = true;

    auto thisGroup = mDialog.GetRadioButtonGroup(mButtonGroup);
    for (auto it : thisGroup)
    {
        it->OnMouseEnterNoRecurse();
    }

    NOEXCEPT_REGION_END
}

//--------------------------------------------------------------------------------------
void RadioButton::OnMouseLeave() noexcept
{
    NOEXCEPT_REGION_START

    mMouseOver = false;

    auto thisGroup = mDialog.GetRadioButtonGroup(mButtonGroup);
    for (auto it : thisGroup)
    {
        it->OnMouseLeaveNoRecurse();
    }

    NOEXCEPT_REGION_END
}

//--------------------------------------------------------------------------------------
void RadioButton::OnMouseEnterNoRecurse() noexcept
{
    NOEXCEPT_REGION_START

    mMouseOver = true;

    NOEXCEPT_REGION_END
}

//--------------------------------------------------------------------------------------
void RadioButton::OnMouseLeaveNoRecurse() noexcept
{
    NOEXCEPT_REGION_START

    mMouseOver = false;

    NOEXCEPT_REGION_END
}

//--------------------------------------------------------------------------------------
void RadioButton::OnFocusIn() noexcept
{
    NOEXCEPT_REGION_START

    mHasFocus = true;

    auto thisGroup = mDialog.GetRadioButtonGroup(mButtonGroup);
    for (auto it : thisGroup)
    {
        it->OnFocusInNoRecurse();
    }

    NOEXCEPT_REGION_END
}

//--------------------------------------------------------------------------------------
void RadioButton::OnFocusOut() noexcept
{
    NOEXCEPT_REGION_START

    mHasFocus = false;

    auto thisGroup = mDialog.GetRadioButtonGroup(mButtonGroup);
    for (auto it : thisGroup)
    {
        it->OnFocusOutNoRecurse();
    }

    NOEXCEPT_REGION_END
}

//--------------------------------------------------------------------------------------
void RadioButton::OnFocusInNoRecurse() noexcept
{
    NOEXCEPT_REGION_START

    mHasFocus = true;

    NOEXCEPT_REGION_END
}

//--------------------------------------------------------------------------------------
void RadioButton::OnFocusOutNoRecurse() noexcept
{
    NOEXCEPT_REGION_START

    mHasFocus = false;

    NOEXCEPT_REGION_END
}


/*
======================================================================================================================================================================================================
ScrollBar Functions


*/

ScrollBar::ScrollBar(Dialog& dialog) : Control(dialog)
{
    mType = CONTROL_SCROLLBAR;

    mShowThumb = true;
    mDrag = false;

    SetRect(mUpButtonRegion, 0, 0, 0, 0);
    SetRect(mDownButtonRegion, 0, 0, 0, 0);
    SetRect(mTrackRegion, 0, 0, 0, 0);
    SetRect(mThumbRegion, 0, 0, 0, 0);
    mPosition = 0;
    mPageSize = 1;
    mStart = 0;
    mEnd = 1;
    mArrow = CLEAR;
    mArrowTS = 0.0;
}


//--------------------------------------------------------------------------------------
ScrollBar::~ScrollBar()
{
}


//--------------------------------------------------------------------------------------
void ScrollBar::UpdateRects() noexcept
{
    NOEXCEPT_REGION_START

    Control::UpdateRects();

    // Make the buttons square

    SetRect(mUpButtonRegion, mRegion.left, mRegion.top,
        mRegion.right, mRegion.top - RectWidth(mRegion));
    //OffsetRect(mDownButtonRegion, 0.0f, RectHeight(mDownButtonRegion));

    SetRect(mDownButtonRegion, mRegion.left, mRegion.bottom + RectWidth(mRegion),
        mRegion.right, mRegion.bottom);
    //OffsetRect(mDownButtonRegion, 0.0f, RectHeight(mDownButtonRegion));

    SetRect(mTrackRegion, mUpButtonRegion.left, mUpButtonRegion.bottom,
        mDownButtonRegion.right, mDownButtonRegion.top);
    //OffsetRect(mDownButtonRegion, 0.0f, RectHeight(mDownButtonRegion));

    mThumbRegion.left = mUpButtonRegion.left;
    mThumbRegion.right = mUpButtonRegion.right;

    UpdateThumbRect();

    NOEXCEPT_REGION_END
}


//--------------------------------------------------------------------------------------
// Compute the dimension of the scroll thumb
void ScrollBar::UpdateThumbRect()
{
    //TODO: fix bug where the icon can go just below the max it should
    if (mEnd - mStart > mPageSize)
    {
        int nThumbHeight = std::clamp((int)RectHeight(mTrackRegion) * mPageSize / (mEnd - mStart),
            SCROLLBAR_MINTHUMBSIZE, (int)RectHeight(mTrackRegion));
        int nMaxPosition = mEnd - mStart - mPageSize + 1;
        mThumbRegion.top = mTrackRegion.top - (mPosition - mStart) * (RectHeight(mTrackRegion) - nThumbHeight)
            / nMaxPosition;
        mThumbRegion.bottom = mThumbRegion.top - nThumbHeight;
        mShowThumb = true;

    }
    else
    {
        // No content to scroll
        mThumbRegion.bottom = mThumbRegion.top;
        mShowThumb = false;
    }
}


//--------------------------------------------------------------------------------------
// Scroll() scrolls by nDelta items.  A positive value scrolls down, while a negative
// value scrolls up.
void ScrollBar::Scroll(int nDelta)
{
    // Perform scroll
    mPosition += nDelta;

    // Cap position
    Cap();

    // Update thumb position
    UpdateThumbRect();
}


//--------------------------------------------------------------------------------------
void ScrollBar::ShowItem(int nIndex)
{
    // Cap the index

    nIndex = std::clamp(nIndex, 0, mEnd - 1);

    // Adjust position

    if (mPosition > nIndex)
        mPosition = nIndex;
    else if (mPosition + mPageSize <= nIndex)
        mPosition = nIndex - mPageSize + 1;

    UpdateThumbRect();
}

//--------------------------------------------------------------------------------------

bool ScrollBar::MsgProc(MessageType msg, int32_t param1, int32_t param2, int32_t param3, int32_t param4) noexcept
{
    //UNREFERENCED_PARAMETER(wParam);

    NOEXCEPT_REGION_START

    if (FOCUS == msg && param1 == GL_FALSE)
    {
        // The application just lost mouse capture. We may not have gotten
        // the WM_MOUSEUP message, so reset mDrag here.
        //if ((HWND)lParam != GetHWND())
        mDrag = false;
    }
    static int nThumbYOffset;

    Point pt = mDialog.GetMousePositionDialogSpace();
    mPreviousMousePos = pt;


    //if the mousebutton is NOT PRESSED, the stop scrolling(since this does not get the mouse release button if it is not over the control)
    //int mbPressed = glfwGetMouseButton(g_pGLFWWindow, GLFW_MOUSE_BUTTON_LEFT);
    //if (mbPressed == GLFW_RELEASE)
    //	mDrag = false;

    switch (msg)
    {
    case MB:
        if (param1 == GLFW_MOUSE_BUTTON_LEFT)
        {
            if (param2 == GLFW_PRESS)
            {
                // Check for click on up button

                if (PtInRect(mUpButtonRegion, pt))
                {
                    //SetCapture(GetHWND());
                    if (mPosition > mStart)
                        --mPosition;

                    UpdateThumbRect();
                    mArrow = CLICKED_UP;
                    mArrowTS = GetTime();
                    return true;
                }

                // Check for click on down button

                if (PtInRect(mDownButtonRegion, pt))
                {
                    //SetCapture(GetHWND());
                    if (mPosition + mPageSize <= mEnd)
                        ++mPosition;

                    UpdateThumbRect();
                    mArrow = CLICKED_DOWN;
                    mArrowTS = GetTime();
                    return true;
                }

                // Check for click on thumb

                if (PtInRect(mThumbRegion, pt))
                {
                    //SetCapture(GetHWND());
                    mDrag = true;
                    nThumbYOffset = mThumbRegion.top - pt.y;
                    return true;
                }

                // Check for click on track

                if (mThumbRegion.left <= pt.x &&
                    mThumbRegion.right > pt.x)
                {
                    //SetCapture(GetHWND());
                    if (mThumbRegion.top > pt.y &&
                        mTrackRegion.top <= pt.y)
                    {
                        Scroll(-(mPageSize - 1));
                        return true;
                    }
                    else if (mThumbRegion.bottom <= pt.y &&
                        mTrackRegion.bottom > pt.y)
                    {
                        Scroll(mPageSize - 1);
                        return true;
                    }
                }

                break;
            }
            else if (param2 == GLFW_RELEASE)
            {
                mDrag = false;
                //ReleaseCapture();
                UpdateThumbRect();
                mArrow = CLEAR;
                break;
            }
        }

    case CURSOR_POS:
    {
        if (mDrag)
        {
            static int nThumbHeight;
            nThumbHeight = RectHeight(mThumbRegion);

            mThumbRegion.top = pt.y + nThumbYOffset;
            mThumbRegion.bottom = mThumbRegion.top - nThumbHeight;
            if (mThumbRegion.top > mTrackRegion.top)
                OffsetRect(mThumbRegion, 0, -(mThumbRegion.top - mTrackRegion.top));
            else if (mThumbRegion.bottom < mTrackRegion.bottom)
                OffsetRect(mThumbRegion, 0, -(mThumbRegion.bottom - mTrackRegion.bottom));

            // Compute first item index based on thumb position

            int nMaxFirstItem = mEnd - mStart - mPageSize + 1;  // Largest possible index for first item
            int nMaxThumb = RectHeight(mTrackRegion) - RectHeight(mThumbRegion);  // Largest possible thumb position from the top

            mPosition = mStart + int(((mTrackRegion.top - mThumbRegion.top) * (float)nMaxFirstItem) / nMaxThumb);
            /*(mTrackRegion.top - mThumbRegion.top +
            nMaxThumb / (nMaxFirstItem * 2)) * // Shift by half a row to avoid last row covered by only one pixel
            nMaxFirstItem / nMaxThumb;*/

            return true;
        }

        break;
    }
    }

    return false;

    NOEXCEPT_REGION_END

    return false;
}


//--------------------------------------------------------------------------------------
void ScrollBar::Render(float elapsedTime) noexcept
{
    NOEXCEPT_REGION_START

    if (mVisible == false)
        return;

    // Check if the arrow button has been held for a while.
    // If so, update the thumb position to simulate repeated
    // scroll.
    if (mArrow != CLEAR)
    {
        double dCurrTime = GetTime();
        if (PtInRect(mUpButtonRegion, mPreviousMousePos))
        {
            switch (mArrow)
            {
            case CLICKED_UP:
                if (SCROLLBAR_ARROWCLICK_DELAY < dCurrTime - mArrowTS)
                {
                    Scroll(-1);
                    mArrow = HELD_UP;
                    mArrowTS = dCurrTime;
                }
                break;
            case HELD_UP:
                if (SCROLLBAR_ARROWCLICK_REPEAT < dCurrTime - mArrowTS)
                {
                    Scroll(-1);
                    mArrowTS = dCurrTime;
                }
                break;
            }
        }
        else if (PtInRect(mDownButtonRegion, mPreviousMousePos))
        {
            switch (mArrow)
            {
            case CLICKED_DOWN:
                if (SCROLLBAR_ARROWCLICK_DELAY < dCurrTime - mArrowTS)
                {
                    Scroll(1);
                    mArrow = HELD_DOWN;
                    mArrowTS = dCurrTime;
                }
                break;
            case HELD_DOWN:
                if (SCROLLBAR_ARROWCLICK_REPEAT < dCurrTime - mArrowTS)
                {
                    Scroll(1);
                    mArrowTS = dCurrTime;
                }
                break;
            }
        }
    }

    ControlState iState = STATE_NORMAL;

    if (mVisible == false)
        iState = STATE_HIDDEN;
    else if (mEnabled == false || mShowThumb == false)
        iState = STATE_DISABLED;
    else if (mMouseOver)
        iState = STATE_MOUSEOVER;
    else if (mHasFocus)
        iState = STATE_FOCUS;


    float fBlendRate = 5.0f;

    // Background track layer
    Element* pElement = &mElements[0];

    // Blend current color
    pElement->mTextureColor.Blend(iState, elapsedTime, fBlendRate);
    mDialog.DrawSprite(*pElement, mTrackRegion, _FAR_BUTTON_DEPTH);


    ControlState iArrowState = iState;
    //if it is all the way at the top, then disable
    if (mPosition == 0 && iState != STATE_HIDDEN)
        iArrowState = STATE_DISABLED;

    // Up Arrow
    pElement = &mElements[1];

    // Blend current color
    pElement->mTextureColor.Blend(iArrowState, elapsedTime, fBlendRate);
    mDialog.DrawSprite(*pElement, mUpButtonRegion, _NEAR_BUTTON_DEPTH);


    //similar with the bottom
    iArrowState = iState;
    if ((mPosition + mPageSize - 1 == mEnd && iState != STATE_HIDDEN) || mEnd == 1/*when no scrolling is necesary*/)
        iArrowState = STATE_DISABLED;

    // Down Arrow
    pElement = &mElements[2];

    // Blend current color
    pElement->mTextureColor.Blend(iArrowState, elapsedTime, fBlendRate);
    mDialog.DrawSprite(*pElement, mDownButtonRegion, _NEAR_BUTTON_DEPTH);

    // Thumb button
    pElement = &mElements[3];

    // Blend current color
    pElement->mTextureColor.Blend(iState, elapsedTime, fBlendRate);
    mDialog.DrawSprite(*pElement, mThumbRegion, _NEAR_BUTTON_DEPTH);

    NOEXCEPT_REGION_END
}


//--------------------------------------------------------------------------------------
void ScrollBar::SetTrackRange(int nStart, int nEnd) noexcept
{
    NOEXCEPT_REGION_START

    mStart = nStart; mEnd = nEnd;
    Cap();
    UpdateThumbRect();

    NOEXCEPT_REGION_END
}


//--------------------------------------------------------------------------------------
void ScrollBar::Cap()  // Clips position at boundaries. Ensures it stays within legal range.
{
    if (mPosition < mStart ||
        mEnd - mStart <= mPageSize)
    {
        mPosition = mStart;
    }
    else if (mPosition + mPageSize > mEnd)
        mPosition = mEnd - mPageSize + 1;
}


/*
======================================================================================================================================================================================================
ListBox Functions


*/

ListBox::ListBox(Dialog& dialog) : mScrollBar(CreateScrollBar(dialog)), Control(dialog)
{
    mType = CONTROL_LISTBOX;

    Point pt = mDialog.GetManager()->GetWindowSize();

    mStyle = 0;
    mSBWidth = 16;
    mSelected.push_back(-1);
    mDrag = false;
    mVerticalMargin = 6;
    mHorizontalMargin = 5;
    mTextHeight = 20;
}


//--------------------------------------------------------------------------------------
ListBox::~ListBox()
{
    RemoveAllItems();
}

//--------------------------------------------------------------------------------------
GenericData& ListBox::GetItemData(const std::wstring& text, Index start) const
{
    return FindItem(text, start)->mData;
}

//--------------------------------------------------------------------------------------
GenericData& ListBox::GetItemData(Index index) const
{
    return mItems[index]->mData;
}


//--------------------------------------------------------------------------------------
void ListBox::UpdateRects() noexcept
{
    NOEXCEPT_REGION_START

    Control::UpdateRects();

    FontNodePtr pFont = mDialog.GetFont(GetElement(0).mFontIndex);
    mTextHeight = pFont->mLeading;

    mSelectionRegion = mRegion;
    mSelectionRegion.right -= mSBWidth;
    InflateRect(mSelectionRegion, -(int32_t)mHorizontalMargin, -(int32_t)mVerticalMargin);
    mTextRegion = mSelectionRegion;
    InflateRect(mTextRegion, -(int32_t)mHorizontalMargin, mVerticalMargin);

    // Update the scrollbar's rects
    //mScrollBar->SetLocation(mRegion.right - mSBWidth, mRegion.top);
    //mScrollBar->SetSize(mSBWidth, m_height);

    mScrollBar->SetLocation(mRegion.right, mRegion.bottom);
    mScrollBar->SetSize(mSBWidth, RectHeight(mRegion));
    Rect tmpRegion = mScrollBar->GetRegion();
    tmpRegion.y = mTextRegion.top;
    mScrollBar->SetRegion(tmpRegion);
    FontNodePtr pFontNode = mDialog.GetFont(mElements[0].mFontIndex);
    if (pFontNode && pFontNode->mFontType->mHeight)
    {
        mScrollBar->SetPageSize(int(RectHeight(mTextRegion) / pFontNode->mLeading));

        // The selected item may have been scrolled off the page.
        // Ensure that it is in page again.
        mScrollBar->ShowItem(mSelected[mSelected.size() - 1]);
    }

    UpdateItemRects();

    NOEXCEPT_REGION_END
}


//--------------------------------------------------------------------------------------

void ListBox::AddItem(const std::wstring& text, GenericData& data) noexcept
{
    InsertItem(mItems.size(), text, data);
}


//--------------------------------------------------------------------------------------

void ListBox::InsertItem(Index index, const std::wstring& text, GenericData& data) noexcept
{
    NOEXCEPT_REGION_START

    auto newItem = std::make_shared<ListBoxItem>(data);

    //clear the selection vector
    mSelected.clear();//this makes it so we do not have to offset the selection
    mSelected.push_back(-1);

    //wcscpy_s(pNewItem->mText, 256, wszText);
    newItem->mText = text;
    SetRect(newItem->mActiveRegion, 0, 0, 0, 0);
    //pNewItem->bSelected = false;

    if (index >= mItems.size())
    {
        mItems.push_back(nullptr);
        index = mItems.size() - 1;
    }

    mItems[index] = newItem;
    mScrollBar->SetTrackRange(0, (int)mItems.size());

    NOEXCEPT_REGION_END
}


//--------------------------------------------------------------------------------------
void ListBox::RemoveItem(Index index)
{
    if (index >= (int)mItems.size())
    {
        GLUF_NON_CRITICAL_EXCEPTION(std::out_of_range("Attempt To Remove List Box Item Out Of Range"));
        return;
    }

    auto it = mItems.begin() + index;
    mItems.erase(it);
    mScrollBar->SetTrackRange(0, (int)mItems.size());
    if (mSelected[0] >= (int)mItems.size())
        mSelected[0] = int(mItems.size()) - 1;

    mDialog.SendEvent(_EVENT_LISTBOX_SELECTION, true, shared_from_this());
}


//--------------------------------------------------------------------------------------
void ListBox::RemoveAllItems() noexcept
{
    NOEXCEPT_REGION_START

    mItems.clear();
    mScrollBar->SetTrackRange(0, 1);
    mSelected.clear();
    //mSelected.push_back(-1);  

    NOEXCEPT_REGION_END
}


//--------------------------------------------------------------------------------------
ListBoxItemPtr ListBox::GetItem(const std::wstring& text, Index start) const
{
    for (auto it : mItems)
    {
        if (it->mText == text)
            return it;
    }

    return nullptr;
}

//--------------------------------------------------------------------------------------
Index ListBox::GetSelectedIndex(Index previousSelected) const
{
    if (mStyle & MULTISELECTION)
    {
        // Multiple selection enabled. Search for the next item with the selected flag.
        /*for (int i = nPreviousSelected + 1; i < (int)mItems.size(); ++i)
        {
        ListBoxItemPtr pItem = mItems[i];

        if (pItem->bSelected)
        return i;
        }*/

        auto in = std::find(mSelected.begin(), mSelected.end(), previousSelected) + 1;
        if (in == mSelected.end())
            throw NoItemSelectedException();

        return *in;
    }
    else
    {
        // Single selection
        return GetSelectedIndex();
    }
}

//--------------------------------------------------------------------------------------
Index ListBox::GetSelectedIndex() const
{
    if (mSelected.size() == 0)
        throw NoItemSelectedException();

    return mSelected[0];
}

//--------------------------------------------------------------------------------------
void ListBox::SelectItem(Index index)
{
    // If no item exists, do nothing.
    if (index >= mItems.size())
    {
        GLUF_NON_CRITICAL_EXCEPTION(std::out_of_range("Item To Select Does Not Exist"));
        return;
    }

    int nOldSelected = mSelected[0];

    // Adjust m_nSelected
    mSelected[0] = index;

    // Perform capping
    if (mSelected[0] < 0)
        mSelected[0] = 0;
    if (mSelected[0] >= (int)mItems.size())
        mSelected[0] = int(mItems.size()) - 1;

    if (nOldSelected != mSelected[0])
    {
        index = mSelected[0];
        mSelected.clear();
        mSelected.push_back(index);

        // Adjust scroll bar
        mScrollBar->ShowItem(mSelected[mSelected.size() - 1]);
    }

    mDialog.SendEvent(_EVENT_LISTBOX_SELECTION, true, shared_from_this());
}

//--------------------------------------------------------------------------------------
void ListBox::SelectItem(const std::wstring& text, Index start)
{
    for (unsigned int i = start; i < mItems.size(); ++i)
    {
        if (mItems[i]->mText == text)
        {
            SelectItem(i);
            return;
        }
    }

    //get to this point, then text is not found
    GLUF_NON_CRITICAL_EXCEPTION(std::invalid_argument("\"text\" not found in mSelected in ListBox"));
}

//--------------------------------------------------------------------------------------
void ListBox::ClearSelected() noexcept
{
    NOEXCEPT_REGION_START

    mSelected.clear();

    NOEXCEPT_REGION_END
}


//--------------------------------------------------------------------------------------
bool ListBox::ContainsItem(const std::wstring& text, Index start) const noexcept
{
    NOEXCEPT_REGION_START

    for (auto it : mItems)
    {
        if (it->mText == text)
            return true;
    }

    return false;

    NOEXCEPT_REGION_END
}


//--------------------------------------------------------------------------------------
Index ListBox::FindItemIndex(const std::wstring& text, Index start) const
{
	for (Index i = start; i < mItems.size(); ++i)
	{
		ListBoxItemPtr pItem = mItems[i];

		if (pItem->mText == text)//REMEMBER if this returns 0, they are the same
		{
			return static_cast<int>(i);
		}
	}

    throw std::invalid_argument("\"text\" was not found in combo box");
}


//--------------------------------------------------------------------------------------
 ListBoxItemPtr ListBox::FindItem(const std::wstring& text, Index start) const
{
    for (auto it : mItems)
    {
        if (it->mText == text)//REMEMBER if this returns 0, they are the same
        {
            return it;
        }
    }

    throw std::invalid_argument("\"text\" was not found in combo box");
}

//--------------------------------------------------------------------------------------
bool ListBox::MsgProc(MessageType msg, int32_t param1, int32_t param2, int32_t param3, int32_t param4) noexcept
{
    NOEXCEPT_REGION_START


    if (FOCUS == msg && param1 == GL_FALSE)
    {
        // The application just lost mouse capture. We may not have gotten
        // the WM_MOUSEUP message, so reset mDrag here.
        mDrag = false;
    }

    Point pt = mDialog.GetMousePositionDialogSpace();

    if (!mEnabled || !mVisible)
        return false;

    // First acquire focus
    if (MB == msg && param1 == GLFW_MOUSE_BUTTON_LEFT && param2 == GLFW_PRESS)
        if (!mHasFocus)
            mDialog.RequestFocus(shared_from_this());

    // Let the scroll bar have a chance to handle it first
    if (mScrollBar->MsgProc(_PASS_CALLBACK_PARAM))
        return true;

    switch (msg)
    {
    case KEY:
        if (param3 == GLFW_RELEASE)
        {
            switch (param1)
            {
            case GLFW_KEY_UP:
            case GLFW_KEY_DOWN:
            case GLFW_KEY_PAGE_DOWN:
            case GLFW_KEY_PAGE_UP:
            case GLFW_KEY_HOME:
            case GLFW_KEY_END:
            {
                // If no item exists, do nothing.
                if (mItems.size() == 0)
                    return true;

                int nOldSelected = mSelected[0];
                mSelected.clear();
                mSelected.push_back(nOldSelected);

                // Adjust m_nSelected
                switch (param1)
                {
                case GLFW_KEY_UP:
                    --mSelected[0]; break;
                case GLFW_KEY_DOWN:
                    ++mSelected[0]; break;
                case GLFW_KEY_PAGE_DOWN:
                    mSelected[0] += mScrollBar->GetPageSize() - 1; break;
                case GLFW_KEY_PAGE_UP:
                    mSelected[0] -= mScrollBar->GetPageSize() - 1; break;
                case GLFW_KEY_HOME:
                    mSelected[0] = 0; break;
                case GLFW_KEY_END:
                    mSelected[0] = int(mItems.size()) - 1; break;
                }

                // Perform capping
                if (mSelected[0] < 0)
                    mSelected[0] = 0;
                if (mSelected[0] >= (int)mItems.size())
                    mSelected[0] = int(mItems.size()) - 1;

                if (nOldSelected != mSelected[0])
                {
                    if (mStyle & MULTISELECTION)
                    {
                        // Multiple selection

                        // Clear all selection
                        /*for (int i = 0; i < (int)mItems.size(); ++i)
                        {
                        ListBoxItemPtr pItem = mItems[i];
                        pItem->bSelected = false;
                        }

                        mSelected.clear();

                        if (param4 & GLFW_MOD_SHIFT)
                        {
                        // Select all items from m_nSelStart to
                        // m_nSelected
                        int nEnd = mSelected[mSelected.size() - 1];//std::max(m_nSelStart, m_nSelected);

                        for (int n = mSelected[0]; n <= nEnd; ++n)
                        mItems[n]->bSelected = true;
                        }
                        else
                        {
                        mItems[mSelected[0]]->bSelected = true;

                        // Update selection start
                        //m_nSelStart = m_nSelected;
                        }*/

                        //TODO: key callback
                    }
                    else;
                    //m_nSelStart = m_nSelected;

                    // Adjust scroll bar

                    mScrollBar->ShowItem(mSelected[0]);

                    // Send notification

                    mDialog.SendEvent(_EVENT_LISTBOX_SELECTION, true, shared_from_this());
                }
                return true;
            }

            // Space is the hotkey for double-clicking an item.
            //
            case GLFW_KEY_SPACE:
                mDialog.SendEvent(_EVENT_LISTBOX_ITEM_DBLCLK, true, shared_from_this());
                return true;
            }
        }
        break;
        //case WM_LBUTTONDOWN:
        //case WM_LBUTTONDBLCLK:
    case MB:
        if (param2 == GLFW_PRESS)
        {
            // Check for clicks in the text area
            if (!mItems.empty() && PtInRect(mSelectionRegion, pt))
            {
                // Compute the index of the clicked item

                //int nClicked;
                //mDialog.GetManager()->GetFontNode(p
                //nClicked = int(mScrollBar->GetTrackPos() + (pt.y - mTextRegion.top) / m_fTextHeight);

                // Only proceed if the click falls on top of an item.

                /*if (nClicked >= mScrollBar->GetTrackPos() &&
                nClicked < (int)mItems.size() &&
                nClicked < mScrollBar->GetTrackPos() + mScrollBar->GetPageSize())
                {
                //SetCapture(GetHWND());
                mDrag = true;

                // If this is a double click, fire off an event and exit
                // since the first click would have taken care of the selection
                // updating.
                //TODO: handle doubleclicking
                if (uMsg == WM_LBUTTONDBLCLK)
                {
                mDialog.SendEvent(_EVENT_LISTBOX_ITEM_DBLCLK, true, this);
                return true;
                }

                m_nSelected = nClicked;
                if (!(param3 & GLFW_MOD_SHIFT))
                m_nSelStart = m_nSelected;*/

                // If this is a multi-selection listbox, update per-item
                // selection data.

                Index currSelectedIndex = 0;

                //the easy way
                for (unsigned int it = 0; it < mItems.size(); ++it)
                {
                    if (PtInRect(mItems[it]->mActiveRegion, pt))
                    {
                        currSelectedIndex = it;
                        break;
                    }
                }

                if (mStyle & MULTISELECTION)
                {
                    // Determine behavior based on the state of Shift and Ctrl

                    //ListBoxItemPtr pSelItem = mItems[currSelectedIndex];
                    if (param3 & GLFW_MOD_CONTROL)
                    {
                        // Control click. Reverse the selection of this item.

                        //pSelItem->bSelected = !pSelItem->bSelected;
                        auto it = std::find(mSelected.begin(), mSelected.end(), currSelectedIndex);
                        if (it == mSelected.end())
                        {
                            mSelected.push_back(currSelectedIndex);
                        }
                        else
                        {
                            mSelected.erase(it);//this should never fail

                            //make sure that if it is the last one, then add the -1
                            /*if (mSelected.size() == 0)
                            {
                            mSelected.push_back(-1);
                            }*/
                        }
                    }
                    else if (param3 & GLFW_MOD_SHIFT)
                    {
                        // Shift click. Set the selection for all items
                        // from last selected item to the current item.
                        // Clear everything else.

                        //if (mSelected[0] == -1)
                        //    mSelected[0] = 0;//this just fixes any issues with accidently keeping -1 in here

                        Index nBegin = mSelected[0];
                        Index nEnd = currSelectedIndex;

                        mSelected.clear();

                        /*for (int i = 0; i < nBegin; ++i)
                        {
                        ListBoxItemPtr pItem = mItems[i];
                        pItem->bSelected = false;
                        }

                        for (int i = nEnd + 1; i < (int)mItems.size(); ++i)
                        {
                        ListBoxItemPtr pItem = mItems[i];
                        pItem->bSelected = false;
                        }*/

                        if (nBegin < nEnd)
                        {
                            for (uint32_t i = nBegin; i <= nEnd; ++i)
                            {
                                /*ListBoxItemPtr pItem = mItems[i];
                                pItem->bSelected = true;
                                */
                                mSelected.push_back(i);
                            }
                        }
                        else if (nBegin > nEnd)
                        {
                            for (uint32_t i = nBegin; i >= nEnd; --i)
                            {
                                /*ListBoxItemPtr pItem = mItems[i];
                                pItem->bSelected = true;
                                */
                                mSelected.push_back(i);
                            }
                        }
                        /*else
                        {
                        mSelected.push_back(-1);
                        */
                    }
                    else if (param3 & (GLFW_MOD_SHIFT | GLFW_MOD_CONTROL))
                    {
                        //No one uses shift control anyway (i see no use in it

                        // Control-Shift-click.

                        // The behavior is:
                        //   Set all items from m_nSelStart to m_nSelected to
                        //     the same state as m_nSelStart, not including m_nSelected.
                        //   Set m_nSelected to selected.

                        /*int nBegin = mSelected[0];
                        int nEnd = currSelectedIndex;
                        mSelected.clear();

                        // The two ends do not need to be set here.

                        bool bLastSelected = mItems[nBegin]->bSelected;
                        for (int i = nBegin + 1; i < nEnd; ++i)
                        {
                        ListBoxItemPtr pItem = mItems[i];
                        pItem->bSelected = bLastSelected;

                        mSelected.push_back(i);
                        }

                        pSelItem->bSelected = true;*/

                        // Restore m_nSelected to the previous value
                        // This matches the Windows behavior

                        //m_nSelected = m_nSelStart;
                    }
                    else
                    {
                        // Simple click.  Clear all items and select the clicked
                        // item.


                        /*for (int i = 0; i < (int)mItems.size(); ++i)
                        {
                        ListBoxItemPtr pItem = mItems[i];
                        pItem->bSelected = false;
                        }

                        pSelItem->bSelected = true;
                        */

                        mSelected.clear();
                        mSelected.push_back(currSelectedIndex);

                        //NOTE: clicking not on an item WILL lead to a clearing of the selection
                    }
                }  // End of multi-selection case
                else
                {
                    mSelected[0] = currSelectedIndex;
                }

                //always make sure we have one
                //if (mSelected.size() == 0)
                //    mSelected.push_back(-1);

                //sort it for proper functionality when using shift-clicking (NOT HELPFUL)
                //std::sort(mSelected.begin(), mSelected.end());

                mDialog.SendEvent(_EVENT_LISTBOX_SELECTION, true, shared_from_this());


                return true;
            }
            break;
        }
        else
        {
            //TODO: drag click
            //ReleaseCapture();
            /*mDrag = false;

            if (mSelected[0] != -1)
            {
            // Set all items between m_nSelStart and m_nSelected to
            // the same state as m_nSelStart
            int nEnd = mSelected[mSelected.size() - 1];

            for (int n = mSelected[0] + 1; n < nEnd; ++n)
            mItems[n]->bSelected = mItems[mSelected[0]]->bSelected;
            mItems[mSelected[0]]->bSelected = mItems[mSelected[0]]->bSelected;

            // If m_nSelStart and m_nSelected are not the same,
            // the user has dragged the mouse to make a selection.
            // Notify the application of this.
            if (mSelected[0] != mSelected[mSelected.size() - 1])
            mDialog.SendEvent(_EVENT_LISTBOX_SELECTION, true, this);

            mDialog.SendEvent(_EVENT_LISTBOX_SELECTION_END, true, this);
            }*/
            return false;
        }

    case CURSOR_POS:
        /*if (mDrag)
        {
        // Compute the index of the item below cursor

        int nItem = -1;
        for (unsigned int it = 0; it < mItems.size(); ++it)
        {
        if (PtInRect(mItems[it]->mActiveRegion, pt))
        {
        nItem = it;
        break;
        }
        }

        // Only proceed if the cursor is on top of an item.

        if (nItem >= (int)mScrollBar->GetTrackPos() &&
        nItem < (int)mItems.size() &&
        nItem < mScrollBar->GetTrackPos() + mScrollBar->GetPageSize())
        {
        mSelected[0] = nItem;
        mDialog.SendEvent(_EVENT_LISTBOX_SELECTION, true, this);
        }
        else if (nItem < (int)mScrollBar->GetTrackPos())
        {
        // User drags the mouse above window top
        mScrollBar->Scroll(-1);
        mSelected[0] = mScrollBar->GetTrackPos();
        mDialog.SendEvent(_EVENT_LISTBOX_SELECTION, true, this);
        }
        else if (nItem >= mScrollBar->GetTrackPos() + mScrollBar->GetPageSize())
        {
        // User drags the mouse below window bottom
        mScrollBar->Scroll(1);
        mSelected[0] = std::min((int)mItems.size(), mScrollBar->GetTrackPos() +
        mScrollBar->GetPageSize()) - 1;
        mDialog.SendEvent(_EVENT_LISTBOX_SELECTION, true, this);
        }
        }*/
        break;

    case SCROLL:
        //UINT uLines = 0;
        //if (!SystemParametersInfo(SPI_GETWHEELSCROLLLINES, 0, &uLines, 0))
        //	uLines = 0;
        //int nScrollAmount = int((short)HIWORD(wParam)) / WHEEL_DELTA * uLines;
        mScrollBar->Scroll(-(param2 / WHEEL_DELTA));
        return true;
    }

    return false;

    NOEXCEPT_REGION_END

    return false;
}

//--------------------------------------------------------------------------------------
void ListBox::UpdateItemRects() noexcept
{
    NOEXCEPT_REGION_START

    FontNodePtr pFont = mDialog.GetFont(GetElement(0).mFontIndex);
    if (pFont)
    {
        int curY = mTextRegion.top - pFont->mLeading / 2;// +((mScrollBar->GetTrackPos() - 1) * pFont->mSize);
        int nRemainingHeight = RectHeight(mRegion) - pFont->mLeading;


        //int nRemainingHeight = RectHeight(mRegion) - pFont->mLeading;

        //for all of the ones before the displayed, just set them to something impossible
        for (size_t i = 0; i < (size_t)mScrollBar->GetTrackPos(); ++i)
        {
            SetRect(mItems[i]->mActiveRegion, 0, 0, 0, 0);
        }
        for (size_t i = mScrollBar->GetTrackPos(); i < mItems.size(); i++)
        {
            ListBoxItemPtr pItem = mItems[i];

            // Make sure there's room left in the box
            nRemainingHeight -= pFont->mLeading;
            if (nRemainingHeight - mVerticalMargin <= 0)
            {
                pItem->mVisible = false;
                continue;
            }

            pItem->mVisible = true;

            SetRect(pItem->mActiveRegion, mRegion.left + mHorizontalMargin, curY + mVerticalMargin, mRegion.right - mHorizontalMargin, curY - pFont->mFontType->mHeight - mVerticalMargin);
            curY -= pFont->mLeading;
        }
    }

    NOEXCEPT_REGION_END
}

//--------------------------------------------------------------------------------------
void ListBox::Render(float elapsedTime) noexcept
{
    NOEXCEPT_REGION_START

    if (mVisible == false)
        return;

    Element* pElement = &mElements[0];
    pElement->mTextureColor.Blend(STATE_NORMAL, elapsedTime);
    pElement->mFontColor.Blend(STATE_NORMAL, elapsedTime);

    Element& pSelElement = mElements[1];
    pSelElement.mTextureColor.Blend(STATE_NORMAL, elapsedTime);
    pSelElement.mFontColor.Blend(STATE_NORMAL, elapsedTime);

    mDialog.DrawSprite(*pElement, mRegion, _FAR_BUTTON_DEPTH);

    FontNodePtr pFont = mDialog.GetFont(pElement->mFontIndex);
    // Render the text
    if (!mItems.empty() && pFont)
    {

        UpdateItemRects();

        static bool bSBInit;
        if (!bSBInit)
        {
            // Update the page size of the scroll bar
            if (mTextHeight > 0)
                mScrollBar->SetPageSize(int((RectHeight(mRegion) - (2 * mVerticalMargin)) / mTextHeight) + 1);
            else
                mScrollBar->SetPageSize(0);
            bSBInit = true;
        }

        for (int i = mScrollBar->GetTrackPos(); i < (int)mItems.size(); ++i)
        {

            ListBoxItemPtr pItem = mItems[i];

            if (!pItem->mVisible)
                continue;

            // Determine if we need to render this item with the
            // selected element.
            bool bSelectedStyle = false;

            //mItems[i]->bSelected = false;

            if (!(mStyle & MULTISELECTION) && i == mSelected[0])
                bSelectedStyle = true;
            else if (mStyle & MULTISELECTION)
            {
                for (auto it : mSelected)
                    if (i == it)
                        bSelectedStyle = true;


                /*if (mDrag &&
                ((i >= mSelected[0] && i < mSelected[0]) ||
                (i <= mSelected[0] && i > mSelected[0])))
                bSelectedStyle = mItems[mSelected[0]]->bSelected;
                else if (pItem->bSelected)
                bSelectedStyle = true;*/
            }

            //bSelectedStyle = mItems[i]->bSelected;
            if (bSelectedStyle)
            {
                //rcSel.top = rc.top; rcSel.bottom = rc.bottom;
                mDialog.DrawSprite(pSelElement, pItem->mActiveRegion, _NEAR_BUTTON_DEPTH);
                mDialog.DrawText(pItem->mText, pSelElement, pItem->mActiveRegion);
            }
            else
                mDialog.DrawText(pItem->mText, *pElement, pItem->mActiveRegion);

            //OffsetRect(rc, 0, m_fTextHeight);
        }

    }

    // Render the scroll bar

    mScrollBar->Render(elapsedTime);

    NOEXCEPT_REGION_END
}

/*
======================================================================================================================================================================================================
ComboBox Functions


*/

ComboBox::ComboBox(Dialog& dialog) : mScrollBar(CreateScrollBar(dialog)), Button(dialog)
{
	mType = CONTROL_COMBOBOX;

	mDropHeight = 100L;

	mSBWidth = 16L;
	mOpened = false;
	mSelected = -1;
	mFocused = -1;
}


//--------------------------------------------------------------------------------------
ComboBox::~ComboBox()
{
	RemoveAllItems();
}


//--------------------------------------------------------------------------------------
void ComboBox::SetTextColor(const Color& Color) noexcept
{
    NOEXCEPT_REGION_START

    Element* pElement = &mElements[0];

    pElement->mFontColor.mStates[STATE_NORMAL] = Color;

    pElement = &mElements[2];

    if (pElement)
        pElement->mFontColor.mStates[STATE_NORMAL] = Color;

    NOEXCEPT_REGION_END
}


//--------------------------------------------------------------------------------------
void ComboBox::UpdateRects() noexcept
{
    NOEXCEPT_REGION_START


    Button::UpdateRects();

    mButtonRegion = mRegion;
    mButtonRegion.left = mButtonRegion.right - RectHeight(mButtonRegion);

    mTextRegion = mRegion;
    mTextRegion.right = mButtonRegion.left;
    mTextRegion.left += 5;//so it isn't falling off the edges

    mDropdownRegion.left = long(mRegion.left/* * 1.019f*/);
    mDropdownRegion.top = long(/*1.02f * */mTextRegion.bottom);
    mDropdownRegion.right = mTextRegion.right;
    mDropdownRegion.bottom = mDropdownRegion.top - mDropHeight;
    //OffsetRect(mDropdownRegion, 0, -RectHeight(mTextRegion));

    mDropdownTextRegion = mDropdownRegion;
    mDropdownTextRegion.left += long(0.05f * RectWidth(mDropdownRegion));
    mDropdownTextRegion.right -= long(0.05f * RectWidth(mDropdownRegion));
    mDropdownTextRegion.top += long(0.05f * RectHeight(mDropdownRegion));
    mDropdownTextRegion.bottom -= long(0.05f * RectHeight(mDropdownRegion));

    // Update the scrollbar's rects
    mScrollBar->SetLocation(mDropdownRegion.right, mDropdownRegion.bottom);
    mScrollBar->SetSize(mSBWidth, abs(mButtonRegion.bottom - mDropdownRegion.bottom));
    Rect tmpRect = mScrollBar->GetRegion();
    tmpRect.y = mTextRegion.top;
    mScrollBar->SetRegion(tmpRect);
    FontNodePtr pFontNode = mDialog.GetFont(mElements[2].mFontIndex);
    if (pFontNode/* && pFontNode->mSize*/)
    {
        mScrollBar->SetPageSize(int(RectHeight(mDropdownTextRegion) / pFontNode->mFontType->mHeight));

        // The selected item may have been scrolled off the page.
        // Ensure that it is in page again.
        mScrollBar->ShowItem(mSelected);
    }

    mRegion.right = mButtonRegion.left;

    NOEXCEPT_REGION_END

}


//--------------------------------------------------------------------------------------
void ComboBox::OnInit()
{ 
    UpdateRects();

    mDialog.InitControl(std::dynamic_pointer_cast<Control>(mScrollBar)); 
}

//--------------------------------------------------------------------------------------
void ComboBox::UpdateItemRects() noexcept
{
    NOEXCEPT_REGION_START

    FontNodePtr pFont = mDialog.GetFont(GetElement(2).mFontIndex);
    if (pFont)
    {
        int curY = mTextRegion.bottom - 4;// +((mScrollBar->GetTrackPos() - 1) * pFont->mSize);
        int fRemainingHeight = RectHeight(mDropdownTextRegion) - pFont->mLeading;//subtract the font size initially too, because we do not want it hanging off the edge


        for (size_t i = mScrollBar->GetTrackPos(); i < mItems.size(); i++)
        {
            ComboBoxItemPtr pItem = mItems[i];

            // Make sure there's room left in the dropdown
            fRemainingHeight -= pFont->mLeading;
            if (fRemainingHeight <= 0.0f)
            {
                pItem->mVisible = false;
                continue;
            }

            pItem->mVisible = true;

            SetRect(pItem->mActiveRegion, mDropdownTextRegion.left, curY, mDropdownTextRegion.right, curY - pFont->mFontType->mHeight);
            curY -= pFont->mLeading;
        }
    }

    NOEXCEPT_REGION_END
}

//--------------------------------------------------------------------------------------
void ComboBox::OnFocusOut() noexcept
{
    NOEXCEPT_REGION_START

    Button::OnFocusOut();

    mOpened = false;

    NOEXCEPT_REGION_END
}



//--------------------------------------------------------------------------------------
bool ComboBox::MsgProc(MessageType msg, int32_t param1, int32_t param2, int32_t param3, int32_t param4) noexcept
{
    NOEXCEPT_REGION_START

    if (!mEnabled || !mVisible)
        return false;

    // Let the scroll bar handle it first.
    if (mScrollBar->MsgProc(msg, param1, param2, param3, param4))
        return true;

    Point pt = mDialog.GetMousePositionDialogSpace();

    switch (msg)
    {
    case CURSOR_POS:
    {
        /*if (mPressed)
        {
        //if the button is pressed and the mouse is moved off, then unpress it
        if (!ContainsPoint(pt))
        {
        mPressed = false;

        ContainsPoint(pt);

        if (!mDialog.IsKeyboardInputEnabled())
        mDialog.ClearFocus();
        }
        }*/

        if (mOpened && PtInRect(mDropdownRegion, pt))
        {
            // Determine which item has been selected
            for (size_t i = 0; i < mItems.size(); i++)
            {
                ComboBoxItemPtr pItem = mItems[i];
                if (pItem->mVisible &&
                    PtInRect(pItem->mActiveRegion, pt))
                {
                    mFocused = static_cast<int>(i);
                }
            }
            return true;
        }
        break;
    }

    case MB:
        if (param1 == GLFW_MOUSE_BUTTON_LEFT)
        {
            if (param2 == GLFW_PRESS)
            {
                if (ContainsPoint(pt))
                {
                    // Pressed while inside the control
                    mPressed = true;
                    //SetCapture(GetHWND());

                    if (!mHasFocus)
                        mDialog.RequestFocus(shared_from_this());

                    return true;
                }

                // Perhaps this click is within the dropdown
                if (mOpened && PtInRect(mDropdownRegion, pt))
                {
                    // Determine which item has been selected
                    for (size_t i = mScrollBar->GetTrackPos(); i < mItems.size(); i++)
                    {
                        ComboBoxItemPtr pItem = mItems[i];
                        if (pItem->mVisible &&
                            PtInRect(pItem->mActiveRegion, pt))
                        {
                            mFocused = mSelected = static_cast<int>(i);
                            mDialog.SendEvent(_EVENT_COMBOBOX_SELECTION_CHANGED, true, shared_from_this());
                            mOpened = false;

                            if (!mDialog.IsKeyboardInputEnabled())
                                mDialog.ClearFocus();

                            break;
                        }
                    }

                    return true;
                }

                // Mouse click not on main control or in dropdown, fire an event if needed
                if (mOpened)
                {
                    mFocused = mSelected;

                    mDialog.SendEvent(_EVENT_COMBOBOX_SELECTION_CHANGED, true, shared_from_this());
                    mOpened = false;
                }


                break;
            }
            else if (param2 == GLFW_RELEASE)
            {
                if (mPressed && ContainsPoint(pt))
                {
                    // Button click
                    mPressed = false;

                    // Toggle dropdown
                    if (mHasFocus)
                    {
                        mOpened = !mOpened;

                        if (!mOpened)
                        {
                            if (!mDialog.IsKeyboardInputEnabled())
                                mDialog.ClearFocus();

                            mFocused = mSelected;
                        }

                        //setup the scroll bar to the correct position (if it is still within the range, it looks better to keep its old position)
                        int pageMin, pageMax;
                        pageMin = mScrollBar->GetTrackPos();
                        pageMax = mScrollBar->GetTrackPos() + mScrollBar->GetPageSize() - 2;
                        if (mFocused > pageMax || mFocused < pageMin)
                            mScrollBar->SetTrackPos(mFocused);
                    }

                    //ReleaseCapture();
                    return true;
                }

                break;
            }
        }

    case SCROLL:
    {
        int zDelta = (param2) / WHEEL_DELTA;
        if (mOpened)
        {
            //UINT uLines = 0;
            //if (!SystemParametersInfo(SPI_GETWHEELSCROLLLINES, 0, &uLines, 0))
            //	uLines = 0;
            mScrollBar->Scroll(-zDelta/* * uLines*/);

            //if it is scroll, then make sure to also send a mouse move event to select the newly hovered item
            UpdateItemRects();
            this->MsgProc(CURSOR_POS, 0, 0, 0, 0);//all blank params may be sent because it retrieves the mouse position from the old message
            //TODO: make this work, but for now:

            /*if (PtInRect(mDropdownRegion, pt))
            {
            // Determine which item has been selected
            for (size_t i = 0; i < mItems.size(); i++)
            {
            ComboBoxItemPtr pItem = mItems[i];
            Rect oldRect = pItem->mActiveRegion;

            OffsetRect(oldRect, 0, float(mScrollBar->GetTrackPos() - oldValue) * RectHeight(pItem->mActiveRegion));
            if (pItem->mVisible &&
            PtInRect(oldRect, pt))
            {
            mFocused = static_cast<int>(i);
            }
            }
            }*/

        }
        else
        {
            if (zDelta > 0)
            {
                if (mFocused > 0)
                {
                    mFocused--;
                    mSelected = mFocused;

                    if (!mOpened)
                        mDialog.SendEvent(_EVENT_COMBOBOX_SELECTION_CHANGED, true, shared_from_this());
                }
            }
            else
            {
                if (mFocused + 1 < (int)GetNumItems())
                {
                    mFocused++;
                    mSelected = mFocused;

                    if (!mOpened)
                        mDialog.SendEvent(_EVENT_COMBOBOX_SELECTION_CHANGED, true, shared_from_this());
                }
            }

        }
        return true;
    }
    case KEY:
    {
        if (param3 != GLFW_RELEASE)
            return true;

        switch (param1)
        {
        case GLFW_KEY_ENTER:
            if (mOpened)
            {
                if (mSelected != mFocused)
                {
                    mSelected = mFocused;
                    mDialog.SendEvent(_EVENT_COMBOBOX_SELECTION_CHANGED, true, shared_from_this());
                }
                mOpened = false;

                if (!mDialog.IsKeyboardInputEnabled())
                    mDialog.ClearFocus();

                return true;
            }
            break;

        case GLFW_KEY_F4:
            // Filter out auto-repeats
            /*if (param3 == GLFW_REPEAT)
                return true;*/

            mOpened = !mOpened;

            if (!mOpened)
            {
                mDialog.SendEvent(_EVENT_COMBOBOX_SELECTION_CHANGED, true, shared_from_this());

                if (!mDialog.IsKeyboardInputEnabled())
                    mDialog.ClearFocus();
            }

            return true;

        case GLFW_KEY_UP:
        case GLFW_KEY_LEFT:
            if (mFocused > 0)
            {
                mFocused--;
                mSelected = mFocused;

                if (!mOpened)
                    mDialog.SendEvent(_EVENT_COMBOBOX_SELECTION_CHANGED, true, shared_from_this());
            }

            return true;

        case GLFW_KEY_RIGHT:
        case GLFW_KEY_DOWN:
            if (mFocused + 1 < (int)GetNumItems())
            {
                mFocused++;
                mSelected = mFocused;

                if (!mOpened)
                    mDialog.SendEvent(_EVENT_COMBOBOX_SELECTION_CHANGED, true, shared_from_this());
            }

            return true;
        }
        break;
    }
    };

    return false;

    NOEXCEPT_REGION_END

    return false;
}

//--------------------------------------------------------------------------------------
void ComboBox::OnHotkey()
{
	if (mOpened)
		return;

	if (mSelected == -1)
		return;

	if (mDialog.IsKeyboardInputEnabled())
		mDialog.RequestFocus(shared_from_this());

	mSelected++;

	if (mSelected >= (int)mItems.size())
		mSelected = 0;

	mFocused = mSelected;
    mDialog.SendEvent(_EVENT_COMBOBOX_SELECTION_CHANGED, true, shared_from_this());
}

bool ComboBox::ContainsPoint(const Point& pt) const noexcept
{
    NOEXCEPT_REGION_START

    return (PtInRect(mRegion, pt) || PtInRect(mButtonRegion, pt));// || (PtInRect(mDropdownRegion, pt) && mOpened));

    NOEXCEPT_REGION_END
}


//--------------------------------------------------------------------------------------
void ComboBox::Render( float elapsedTime) noexcept
{
    NOEXCEPT_REGION_START

    if (mVisible == false)
        return;
    ControlState iState = STATE_NORMAL;

    //if (!mOpened)
    //    iState = STATE_HIDDEN;

    // Dropdown box
    Element* pElement = &mElements[2];

    // If we have not initialized the scroll bar page size,
    // do that now.
    static bool bSBInit;
    if (!bSBInit)
    {
        // Update the page size of the scroll bar
        auto fontNode = mDialog.GetFont(pElement->mFontIndex);
        if (fontNode->mFontType->mHeight)
        {
            mScrollBar->SetPageSize(
                static_cast<int>(glm::round(
                static_cast<float>(RectHeight(mDropdownTextRegion)) / 
                static_cast<float>(fontNode->mLeading))));
        }
        else
            mScrollBar->SetPageSize(0);
        bSBInit = true;
    }

    // Scroll bar --EDITED, only render any of this stuff if OPENED
    if (mOpened)
    {
        mScrollBar->Render(elapsedTime);

        // Blend current color
        pElement->mTextureColor.Blend(iState, elapsedTime);
        pElement->mFontColor.Blend(iState, elapsedTime);

        mDialog.DrawSprite(*pElement, mDropdownRegion, _NEAR_BUTTON_DEPTH);


        // Selection outline
        Element* pSelectionElement = &mElements[3];
        pSelectionElement->mTextureColor.GetCurrent() = pElement->mTextureColor.GetCurrent();
        pSelectionElement->mFontColor.SetCurrent(/*pSelectionElement->mFontColor.mStates[STATE_NORMAL]*/{ 0, 0, 0, 255 });

        FontNodePtr pFont = mDialog.GetFont(pElement->mFontIndex);
        if (pFont)
        {
            //float curY = mDropdownTextRegion.top - 0.02f;
            //float fRemainingHeight = RectHeight(mDropdownTextRegion) - pFont->mSize;//subtract the font size initially too, because we do not want it hanging off the edge
            //WCHAR strDropdown[4096] = {0};

            UpdateItemRects();

            for (size_t i = mScrollBar->GetTrackPos(); i < mItems.size(); i++)
            {
                ComboBoxItemPtr pItem = mItems[i];
                Rect active = pItem->mActiveRegion;

                active.top = active.bottom + pFont->mLeading;

                // Make sure there's room left in the dropdown

                if (!pItem->mVisible)
                    continue;
                //SetRect(pItem->mActiveRegion, mDropdownTextRegion.left, curY, mDropdownTextRegion.right, curY - pFont->mSize);
                //curY -= pFont->mSize;

                //debug
                //int blue = 50 * i;
                //mDialog.DrawRect( &pItem->mActiveRegion, 0xFFFF0000 | blue );

                //pItem->mVisible = true;

                //SetRect(rc, mDropdownRegion.left, pItem->mActiveRegion.top - (2 / mDialog.GetManager()->GetWindowSize().y), mDropdownRegion.right,
                //	pItem->mActiveRegion.bottom + (2 / mDialog.GetManager()->GetWindowSize().y));
                //SetRect(rc, mDropdownRegion.left + RectWidth(mDropdownRegion) / 12.0f, mDropdownRegion.top - (RectHeight(pItem->mActiveRegion) * i), mDropdownRegion.right,
                //	mDropdownRegion.top - (RectHeight(pItem->mActiveRegion) * (i + 1)));

                if ((int)i == mFocused)
                {
                    //SetRect(rc, mDropdownRegion.left, pItem->mActiveRegion.top - (2 / mDialog.GetManager()->GetWindowSize().y), mDropdownRegion.right,
                    //	pItem->mActiveRegion.bottom + (2 / mDialog.GetManager()->GetWindowSize().y));
                    /*SetRect(rc, mDropdownRegion.left, mDropdownRegion.top - (RectHeight(pItem->mActiveRegion) * i), mDropdownRegion.right,
                        mDropdownRegion.top - (RectHeight(pItem->mActiveRegion) * (i + 1)));*/
                    //mDialog.DrawText(pItem->mText, pSelectionElement, rc);
                    mDialog.DrawSprite(*pSelectionElement, active, _NEAR_BUTTON_DEPTH);
                    mDialog.DrawText(pItem->mText, *pSelectionElement, pItem->mActiveRegion);
                }
                else
                {
                    mDialog.DrawText(pItem->mText, *pElement, pItem->mActiveRegion);
                }
            }
        }
    }

    int OffsetX = 0;
    int OffsetY = 0;

    iState = STATE_NORMAL;

    if (mVisible == false)
        iState = STATE_HIDDEN;
    else if (mEnabled == false)
        iState = STATE_DISABLED;
    else if (mPressed)
    {
        iState = STATE_PRESSED;

        OffsetX = 1;
        OffsetY = 2;
    }
    else if (mMouseOver)
    {
        iState = STATE_MOUSEOVER;

        OffsetX = -1;
        OffsetY = -2;
    }
    else if (mHasFocus)
        iState = STATE_FOCUS;

    float fBlendRate = 5.0f;

    // Button
    pElement = &mElements[1];

    // Blend current color
    pElement->mTextureColor.Blend(iState, elapsedTime, fBlendRate);

    Rect rcWindow = mButtonRegion;
    OffsetRect(rcWindow, OffsetX, OffsetY);
    mDialog.DrawSprite(*pElement, rcWindow, _FAR_BUTTON_DEPTH);

    if (mOpened)
        iState = STATE_PRESSED;


    // Main text box
    pElement = &mElements[0];

    // Blend current color
    pElement->mTextureColor.Blend(iState, elapsedTime, fBlendRate);
    pElement->mFontColor.Blend(iState, elapsedTime, fBlendRate);


    mDialog.DrawSprite(*pElement, mRegion, _NEAR_BUTTON_DEPTH);

    if (mSelected >= 0 && mSelected < (int)mItems.size())
    {
        ComboBoxItemPtr pItem = mItems[mSelected];
        if (pItem)
        {
            mDialog.DrawText(pItem->mText, *pElement, mTextRegion, false, true);

        }
    }


    NOEXCEPT_REGION_END
}


//--------------------------------------------------------------------------------------

void ComboBox::AddItem(const std::wstring& text, GenericData& data) noexcept
{
    NOEXCEPT_REGION_START

    // Create a new item and set the data
    auto pItem = std::make_shared<ComboBoxItem>(data);

    pItem->mText = text;
    //pItem->mData = data;

    mItems.push_back(pItem);

    // Update the scroll bar with new range
    mScrollBar->SetTrackRange(0, (int)mItems.size());

    // If this is the only item in the list, it's selected
    if (GetNumItems() == 1)
    {
        mSelected = 0;
        mFocused = 0;
        mDialog.SendEvent(_EVENT_COMBOBOX_SELECTION_CHANGED, false, shared_from_this());
    }

    NOEXCEPT_REGION_END
}


//--------------------------------------------------------------------------------------
void ComboBox::RemoveItem(Index index)
{
    if (index >= mItems.size())
    {
        GLUF_NON_CRITICAL_EXCEPTION(std::out_of_range("Error Removing Item From Combo Box"));
        return;
    }

    //erase the item (kinda sloppy)
    std::vector<ComboBoxItemPtr> newItemList;
    newItemList.resize(mItems.size() - 1);
    for (Index i = 0; i < mItems.size(); ++i)
    {
        if (i == index)
            continue;

        newItemList[i] = mItems[i];
    }
    mItems = newItemList;

	mScrollBar->SetTrackRange(0, (int)mItems.size());
	if (mSelected >= (int)mItems.size())
		mSelected = (int)mItems.size() - 1;
}


//--------------------------------------------------------------------------------------
void ComboBox::RemoveAllItems() noexcept
{
    NOEXCEPT_REGION_START
    
    mItems.clear();
    mScrollBar->SetTrackRange(0, 1);
    mFocused = mSelected = -1;

    NOEXCEPT_REGION_END
}


//--------------------------------------------------------------------------------------
bool ComboBox::ContainsItem(const std::wstring& text, Index start) const noexcept
{ 
    NOEXCEPT_REGION_START

    return (-1 != FindItemIndex(text, start));

    NOEXCEPT_REGION_END
}


//--------------------------------------------------------------------------------------
Index ComboBox::FindItemIndex(const std::wstring& text, Index start) const
{
	for (Index i = start; i < mItems.size(); ++i)
	{
		ComboBoxItemPtr pItem = mItems[i];

		if (pItem->mText == text)//REMEMBER if this returns 0, they are the same
		{
			return static_cast<int>(i);
		}
	}

    throw std::invalid_argument("\"text\" was not found in combo box");
}


//--------------------------------------------------------------------------------------
ComboBoxItemPtr ComboBox::FindItem(const std::wstring& text, Index start) const
{
    for (auto it : mItems)
    {
        if (it->mText == text)//REMEMBER if this returns 0, they are the same
        {
            return it;
        }
    }

    throw std::invalid_argument("\"text\" was not found in combo box");
}


//--------------------------------------------------------------------------------------
GenericData& ComboBox::GetSelectedData() const
{
    if (mSelected < 0)
        throw NoItemSelectedException();

	ComboBoxItemPtr pItem = mItems[mSelected];
	return pItem->mData;
}


//--------------------------------------------------------------------------------------
ComboBoxItemPtr ComboBox::GetSelectedItem() const
{
    if (mSelected < 0)
        throw NoItemSelectedException();

    return mItems[mSelected];
}


//--------------------------------------------------------------------------------------
GenericData& ComboBox::GetItemData(const std::wstring& text, Index start) const
{
    return FindItem(text, start)->mData;
}


//--------------------------------------------------------------------------------------
GenericData& ComboBox::GetItemData(Index index) const
{
    return mItems[index]->mData;
}


//--------------------------------------------------------------------------------------
void ComboBox::SelectItem(Index index)
{
    if (index >= mItems.size())
    {
        GLUF_NON_CRITICAL_EXCEPTION(std::out_of_range("Index Too Large"));
        return;
    }

	mFocused = mSelected = index;
	mDialog.SendEvent(_EVENT_COMBOBOX_SELECTION_CHANGED, false, shared_from_this());
}



//--------------------------------------------------------------------------------------
void ComboBox::SelectItem(const std::wstring& text, Index start)
{
    Index itemIndex = 0;
    try
    {
        itemIndex = FindItemIndex(text, start);
    }
    catch (...)
    {
        GLUF_NON_CRITICAL_EXCEPTION(std::invalid_argument("\"text\" not found in combo box"));
        return;
    }

    mFocused = mSelected = itemIndex;
	mDialog.SendEvent(_EVENT_COMBOBOX_SELECTION_CHANGED, false, shared_from_this());
}



//--------------------------------------------------------------------------------------
void ComboBox::SelectItem(const GenericData& data)
{
    sIndex itemIndex = -1;
    for (unsigned int i = 0; i < mItems.size(); ++i)
    {
        if (&mItems[i]->mData == &data)
        {
            itemIndex = i;
            break;
        }
    }

    if (itemIndex == -1)
    {
        GLUF_NON_CRITICAL_EXCEPTION(std::invalid_argument("\"data\" not found"));
        return;
    }

    NOEXCEPT_REGION_START

    SelectItem(itemIndex);

    NOEXCEPT_REGION_END
}


/*
======================================================================================================================================================================================================
Slider Functions


*/

Slider::Slider(Dialog& dialog) : Control(dialog)
{
	mType = CONTROL_SLIDER;

	mMin = 0;
	mMax = 100;
	mValue = 0;
	mPressed = false;
}


//--------------------------------------------------------------------------------------
bool Slider::ContainsPoint(const Point& pt) const noexcept
{
    NOEXCEPT_REGION_START

    return (PtInRect(mRegion, pt) ||
    PtInRect(mButtonRegion, pt));

    NOEXCEPT_REGION_END
}


//--------------------------------------------------------------------------------------
void Slider::UpdateRects() noexcept
{
    NOEXCEPT_REGION_START

    Control::UpdateRects();

    mButtonRegion = mRegion;
    mButtonRegion.right = mButtonRegion.left + RectHeight(mButtonRegion);
    OffsetRect(mButtonRegion, -RectWidth(mButtonRegion) / 2, 0);

    mButtonX = (int)((float(mValue - mMin) / float(mMax - mMin)) * RectWidth(mRegion));
    OffsetRect(mButtonRegion, mButtonX, 0);

    NOEXCEPT_REGION_END
}


//--------------------------------------------------------------------------------------
Value Slider::ValueFromXPos(Value x) const noexcept
{
    NOEXCEPT_REGION_START

    float fValuePerPixel = (float)(mMax - mMin) / RectWidth(mRegion);
    float fPixelPerValue2 = 1.0f / (2.0f * fValuePerPixel);//use this to get it to change locations at the half way mark instead of using truncate int methods
    return int(((x - mRegion.x + fPixelPerValue2) * fValuePerPixel) + mMin);

    NOEXCEPT_REGION_END
}


//--------------------------------------------------------------------------------------
bool Slider::MsgProc(MessageType msg, int32_t param1, int32_t param2, int32_t param3, int32_t param4) noexcept
{
    NOEXCEPT_REGION_START

    if (!mEnabled || !mVisible)
        return false;

    Point pt = mDialog.GetMousePositionDialogSpace();

    switch (msg)
    {
    case MB:
        if (param1 == GLFW_MOUSE_BUTTON_LEFT)
        {
            if (param2 == GLFW_PRESS)
            {
                if (PtInRect(mButtonRegion, pt))
                {
                    // Pressed while inside the control
                    mPressed = true;
                    //SetCapture(GetHWND());

                    mDragX = pt.x;
                    //m_nDragY = pt.y;
                    mDragOffset = mButtonX - mDragX;

                    //m_nDragValue = mValue;

                    if (!mHasFocus)
                        mDialog.RequestFocus(shared_from_this());

                    return true;
                }

                if (PtInRect(mRegion, pt))
                {

                    if (!mHasFocus)
                        mDialog.RequestFocus(shared_from_this());

                    SetValueInternal(ValueFromXPos(pt.x), true);

                    return true;
                }
            }
            else if (param2 == GLFW_RELEASE)
            {
                if (mPressed)
                {
                    mPressed = false;
                    //ReleaseCapture();
                    mDialog.SendEvent(_EVENT_SLIDER_VALUE_CHANGED_UP, true, shared_from_this());

                    return true;
                }

                break;

            }
            break;
        }

    case CURSOR_POS:
    {

        if (mPressed)
        {
            SetValueInternal(ValueFromXPos(mRegion.x + pt.x + mDragOffset), true);
            return true;
        }

        break;
    }

    case SCROLL:
    {
        int nScrollAmount = param2 / WHEEL_DELTA;
        SetValueInternal(mValue - nScrollAmount, true);
        return true;
    }
    case KEY:
    {
        if (param3 == GLFW_RELEASE)
            break;

        switch (param1)
        {
        case GLFW_KEY_HOME:
            SetValueInternal(mMin, true);
            return true;

        case GLFW_KEY_END:
            SetValueInternal(mMax, true);
            return true;

        case GLFW_KEY_LEFT:
        case GLFW_KEY_DOWN:
            SetValueInternal(mValue - 3, true);
            return true;

        case GLFW_KEY_RIGHT:
        case GLFW_KEY_UP:
            SetValueInternal(mValue + 3, true);
            return true;

        case GLFW_KEY_PAGE_DOWN:
            SetValueInternal(mValue - (10 > (mMax - mMin) / 10 ? 10 : (mMax - mMin) / 10),
                true);
            return true;

        case GLFW_KEY_PAGE_UP:
            SetValueInternal(mValue + (10 > (mMax - mMin) / 10 ? 10 : (mMax - mMin) / 10),
                true);
            return true;
        }
        break;
    }
    };

    return false;

    NOEXCEPT_REGION_END

    return false;
}

//--------------------------------------------------------------------------------------
void Slider::SetRange(int nMin, int nMax) noexcept
{
    NOEXCEPT_REGION_START

    mMin = nMin;
    mMax = nMax;

    SetValueInternal(mValue, false);

    NOEXCEPT_REGION_END
}


//--------------------------------------------------------------------------------------
void Slider::SetValueInternal(int nValue, bool bFromInput) noexcept
{
    NOEXCEPT_REGION_START

    // Clamp to range
    nValue = std::clamp(nValue, mMin, mMax);


    if (nValue == mValue)
        return;

    mValue = nValue;

    UpdateRects();

    mDialog.SendEvent(_EVENT_SLIDER_VALUE_CHANGED, bFromInput, shared_from_this());

    NOEXCEPT_REGION_END
}


//--------------------------------------------------------------------------------------
void Slider::Render( float elapsedTime) noexcept
{
    NOEXCEPT_REGION_START

    if (mVisible == false)
        return;

    int OffsetX = 0;
    int OffsetY = 0;

    ControlState iState = STATE_NORMAL;

    if (mVisible == false)
    {
        iState = STATE_HIDDEN;
    }
    else if (mEnabled == false)
    {
        iState = STATE_DISABLED;
    }
    else if (mPressed)
    {
        iState = STATE_PRESSED;

        OffsetX = 1;
        OffsetY = 2;
    }
    else if (mMouseOver)
    {
        iState = STATE_MOUSEOVER;

        OffsetX = -1;
        OffsetY = -2;
    }
    else if (mHasFocus)
    {
        iState = STATE_FOCUS;
    }

    float fBlendRate = 5.0f;

    Element* pElement = &mElements[0];

    // Blend current color
    pElement->mTextureColor.Blend(iState, elapsedTime, fBlendRate);
    mDialog.DrawSprite(*pElement, mRegion, _FAR_BUTTON_DEPTH);

    pElement = &mElements[1];

    // Blend current color
    pElement->mTextureColor.Blend(iState, elapsedTime, fBlendRate);
    mDialog.DrawSprite(*pElement, mButtonRegion, _NEAR_BUTTON_DEPTH);

    NOEXCEPT_REGION_END
}

const std::wstring g_Charsets[] = 
{ 
	L" !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~",
	L" !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~ÄÅÇÉÑÖÜáàâäãåçéèêëíìîïñóòôöõúùûü†°¢£§•¶ß®©™´¨≠ÆØ∞±≤≥¥µ∂∑∏π∫ªºΩæø¿¡¬√ƒ≈∆«»… ÀÃÕŒœ–—“”‘’÷◊ÿŸ⁄€‹›ﬁﬂ‡·‚„‰ÂÊÁËÈÍÎÏÌÓÔÒÚÛÙıˆ˜¯˘˙˚¸˝˛ˇ",
	L"0123456789",
	L"abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ",
	L"abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789"
};

bool CharsetContains(wchar_t codepoint, Charset charset)
{
	switch (charset)
	{
	case Unicode:
		return true;
	default:
		for (unsigned int i = 0; i < g_Charsets[charset].size(); ++i)
		{
			if (g_Charsets[charset][i] == codepoint)
				return true;
		}
		return false;
	}
}

/*
//======================================================================================
// EditBox class
//======================================================================================

// Static member initialization
bool EditBox::s_bHideCaret;   // If true, we don't render the caret.

// When scrolling, EDITBOX_SCROLLEXTENT is reciprocal of the amount to scroll.
// If EDITBOX_SCROLLEXTENT = 4, then we scroll 1/4 of the control each time.
#define EDITBOX_SCROLLEXTENT 4

//--------------------------------------------------------------------------------------
EditBox::EditBox(Charset charset, bool isMultiline, Dialog* pDialog) : Control(pDialog), mScrollBar(pDialog), m_bMultiline(isMultiline), m_Charset(charset)
{
	mType = CONTROL_EDITBOX;
	mDialog = pDialog;

	m_fBorder  = 5;  // Default border width
	m_fSpacing = 4;  // Default spacing

	m_bCaretOn = true;
	m_dfBlink = GetCaretBlinkTime() * 0.001f;
	m_dfLastBlink = glfwGetTime();
	s_bHideCaret = false;
	m_nFirstVisible = 0;
	m_TextColor = {16, 16, 16, 255);
	m_SelTextColor = {255, 255, 255, 255);
	m_SelBkColor = {40, 50, 92, 255);
	m_CaretColor = {0, 0, 0, 255);
	m_nCaret = m_nSelStart = 0;
	m_bInsertMode = true;

	mSBWidth = 16;
	m_bMouseDrag = false;

	m_bAnalyseRequired = true;
}


//--------------------------------------------------------------------------------------
EditBox::~EditBox()
{
}


//--------------------------------------------------------------------------------------
// PlaceCaret: Set the caret to a character position, and adjust the scrolling if
//             necessary.
//--------------------------------------------------------------------------------------
void EditBox::PlaceCaret( int nCP)
{
	if (m_bMultiline)
	{
		if (nCP == -1)
		{
			mScrollBar->SetTrackPos(0);//top
			m_nCaret = nCP;
		}
		else if (nCP >= GetTextLength())
		{
			//anything past this, set to the max
			m_nCaret = GetTextLength() - 1;
		}
		else
		{
			//_ASSERT(nCP >= 0 && nCP <= GetTextLength());
			m_nCaret = nCP;

			//if it is a newline, jump to the next character
			if (m_nCaret != GetTextLength() - 1 && m_strBuffer[m_nCaret + 1] == '\n')
			{
				m_nCaret++;
			}

			int rendCaret = GetStrRenderIndexFromStrIndex(nCP);
			if (rendCaret == -2)
			{
				if (m_strRenderBuffer.size() == 0)
					rendCaret = 0;
				else
					rendCaret = m_strRenderBuffer.back();
			}

			while (rendCaret == -1)
			{
				mScrollBar->Scroll(mScrollBar->GetPageSize() - 1);
				Analyse();

				rendCaret = GetStrRenderIndexFromStrIndex(nCP);
			}

			int line = GetLineNumberFromCharPos(rendCaret);

			if (line < 0 || line >= mScrollBar->GetPageSize())
			{
				mScrollBar->SetTrackPos(line);
			}
			//if (line < mScrollBar->GetTrackPos() || line > mScrollBar->GetTrackPos() + mScrollBar->GetPageSize() - 1)
			//	mScrollBar->SetTrackPos(line);
		}
	}
	else if (!m_bMultiline)
	{
		if (nCP >= GetTextLength())
		{
			//anything past this, set to the max
			m_nCaret = GetTextLength() - 1;
		}
		else
		{
			//mScrollBar->SetTrackRange(0, GetTextLength());
			//if (nCP >= mScrollBar->GetTrackPos() + mScrollBar->GetPageSize())
			//  mScrollBar->Scroll(delta);
			//else if (nCP < mScrollBar->GetTrackPos())
			//  mScrollBar->SetTrackPos(nCP);

			m_nCaret = nCP;

			//Analyse();
			
			//if (m_nCaret >= mScrollBar->GetTrackPos() + mScrollBar->GetPageSize())
			//  mScrollBar->Scroll(-((mScrollBar->GetTrackPos() + mScrollBar->GetPageSize()) - m_nCaret));
			//else if (m_nCaret < mScrollBar->GetTrackPos())
			//  mScrollBar->SetTrackPos(m_nCaret);
		}

		//int rendCaret = GetStrRenderIndexFromStrIndex(m_nCaret);
		Analyse();
	}
}

void EditBox::PlaceCaretRndBuffer(int nRndCp)
{
	m_nCaret = GetStrIndexFromStrRenderIndex(nRndCp);
	PlaceCaret(m_nCaret);
}

int EditBox::GetLineNumberFromCharPos(unsigned int nCP)
{
	int nCPModified = nCP;
	int lin = 0;
	for (auto it : m_strInsertedNewlineLocations)
		if (nCP >= it)
			--nCPModified;
	
	for (unsigned int i = 0; i < nCPModified + m_strRenderBufferOffset; ++i)
		if (m_strBuffer[i] == '\n')
			++lin;

	//for (auto it : m_strInsertedNewlineLocations)
	//	if (nCP >= it)
	//		++lin;
	//add the difference
	lin += nCP - nCPModified;

	if (m_strRenderBuffer[nCP] == '\n')
		++lin;

	return lin;
}



//--------------------------------------------------------------------------------------
void EditBox::ClearText()
{
	m_strBuffer.clear();
	m_nFirstVisible = 0;
	PlaceCaret(0);
	m_nSelStart = 0;

	m_bAnalyseRequired = true;
}


//--------------------------------------------------------------------------------------
void EditBox::SetText( std::wstring wszText,  bool bSelected)
{
	//assert(wszText);

	m_strBuffer = wszText;
	m_nFirstVisible = 0;
	Analyse();

	// Move the caret to the end of the text
	PlaceCaret(GetTextLength() - 1);
	m_nSelStart = bSelected ? 0 : m_nCaret;


	m_bAnalyseRequired = true;
}


//--------------------------------------------------------------------------------------
void EditBox::DeleteSelectionText()
{
	int nFirst = std::min(m_nCaret, m_nSelStart);
	int nLast = std::max(m_nCaret, m_nSelStart);
	// Update caret and selection
	PlaceCaret(nFirst);
	m_nSelStart = m_nCaret;
	// Remove the characters
	for (int i = nFirst; i < nLast; ++i)
		RemoveChar(nFirst);


}


//--------------------------------------------------------------------------------------
void EditBox::UpdateRects() noexcept
{
    NOEXCEPT_REGION_START

    Control::UpdateRects();

    // Update the text rectangle
    mTextRegion = mRegion;
    // First inflate by m_nBorder to compute render rects
    InflateRect(mTextRegion, -m_fBorder, -m_fBorder);

    mTextRegion.right -= mSBWidth;

    // Update the render rectangles
    m_rcRender[0] = mTextRegion;
    SetRect(m_rcRender[1], mRegion.left, mRegion.top, mTextRegion.left, mTextRegion.top);
    SetRect(m_rcRender[2], mTextRegion.left, mRegion.top, mTextRegion.right, mTextRegion.top);
    SetRect(m_rcRender[3], mTextRegion.right, mRegion.top, mRegion.right, mTextRegion.top);
    SetRect(m_rcRender[4], mRegion.left, mTextRegion.top, mTextRegion.left, mTextRegion.bottom);
    SetRect(m_rcRender[5], mTextRegion.right, mTextRegion.top, mRegion.right, mTextRegion.bottom);
    SetRect(m_rcRender[6], mRegion.left, mTextRegion.bottom, mTextRegion.left, mRegion.bottom);
    SetRect(m_rcRender[7], mTextRegion.left, mTextRegion.bottom, mTextRegion.right, mRegion.bottom);
    SetRect(m_rcRender[8], mTextRegion.right, mTextRegion.bottom, mRegion.right, mRegion.bottom);

    FontNodePtr pFontNode = mDialog.GetManager()->GetFontNode(mElements[0]->mFontIndex);

    // Inflate further by m_nSpacing
    InflateRect(mTextRegion, -m_fSpacing, -m_fSpacing);

    mScrollBar->SetLocation(mTextRegion.right, mRegion.bottom);
    mScrollBar->SetSize(mSBWidth, RectHeight(mRegion));
    mScrollBar->m_y = mTextRegion.top;


    //Analyse();
    m_bAnalyseRequired = true;

    NOEXCEPT_REGION_END
}

#pragma warning(disable : 4018)
#pragma warning(push)
#pragma warning( disable : 4616 6386 )
void EditBox::CopyToClipboard()
{
	// Copy the selection text to the clipboard
	if (m_nCaret != m_nSelStart/* && OpenClipboard(nullptr))
	{
		//EmptyClipboard();

		/*HGLOBAL hBlock = GlobalAlloc(GMEM_MOVEABLE, sizeof(WCHAR) * (m_Buffer.GetTextSize() + 1));
		if (hBlock)
		{
			WCHAR* pwszText = (WCHAR*)GlobalLock(hBlock);
			if (pwszText)
			{
				int nFirst = std::min(m_nCaret, m_nSelStart);
				int nLast = std::max(m_nCaret, m_nSelStart);
				if (nLast - nFirst > 0)
				{
					memcpy(pwszText, m_Buffer.GetBuffer() + nFirst, (nLast - nFirst) * sizeof(WCHAR));
				}
				pwszText[nLast - nFirst] = L'\0';  // Terminate it
				GlobalUnlock(hBlock);
			}
			SetClipboardData(CF_UNICODETEXT, hBlock);
		}
		CloseClipboard();
		// We must not free the object until CloseClipboard is called.
		if (hBlock)
			GlobalFree(hBlock);

		//glfw makes this easy
		std::wstring str = L"";
		std::wstring strBuffer = m_strBuffer;
		for (int i = m_nSelStart; i < m_nCaret; ++i)
		{
			str += strBuffer[i];
		}

		char *tmp = new char[256];
		wcstombs(tmp, str.c_str(), 256);

		glfwSetClipboardString(g_pGLFWWindow, tmp);
	}
}

void EditBox::PasteFromClipboard()
{
	DeleteSelectionText();

	/*if (OpenClipboard(nullptr))
	{
		HANDLE handle = GetClipboardData(CF_UNICODETEXT);
		if (handle)
		{
			// Convert the ANSI string to Unicode, then
			// insert to our buffer.
			WCHAR* pwszText = (WCHAR*)GlobalLock(handle);
			if (pwszText)
			{
				// Copy all characters up to null.
				if (m_Buffer.InsertString(m_nCaret, pwszText))
					PlaceCaret(m_nCaret + (int)wcslen(pwszText));
				m_nSelStart = m_nCaret;
				GlobalUnlock(handle);
				}
				}
				CloseClipboard();
				}

	//glfw makes this easy
	const char* str;
	str = glfwGetClipboardString(g_pGLFWWindow);
	if (str == nullptr)//if glfw cannot support the format
		return;
	wchar_t *wStr = new wchar_t[256];

	mbstowcs(wStr, str, 256);

	if (m_nSelStart > m_strRenderBuffer.length())
		InsertString(m_nSelStart + 1, wStr);
	else
		InsertString(m_nCaret + 1, wStr);


	//when pasting, set the cursor to the end
	m_nCaret = (int)m_strBuffer.length() - 1;
	m_nSelStart = m_nCaret;

	m_bAnalyseRequired = true;
	//delete str;
}
#pragma warning(pop)


void EditBox::InsertString(int pos, std::wstring str)
{
	for (auto it : str)
	{
		if (!CharsetContains(it, m_Charset))
			return;
	}
	//_ASSERT(pos < GetTextLength() - 1);
	if (pos < 0)
	{
		m_strBuffer.insert(0, str.c_str());
	}
	if (pos == GetTextLength())
	{
		m_strBuffer.append(str.c_str());
	}
	else
	{
		m_strBuffer.insert(pos, str);
	}

	m_bAnalyseRequired = true;
}

void EditBox::RemoveString(int pos, int len)
{
	//_ASSERT(pos + len < GetTextLength() - 1);

	if (m_strBuffer.length() <= pos + len)
	{
		while (pos < m_strBuffer.length())
		{
			m_strBuffer.pop_back();//if len goes past the end, then just remove what we can
		}
	}
	else if (pos < 0)
	{

	}
	else
	{
		m_strBuffer.erase(pos, len);
	}
	m_bAnalyseRequired = true;
}

void EditBox::InsertChar(int pos, wchar_t ch)
{
	if (!CharsetContains(ch, m_Charset))
		return;
	//_ASSERT(pos < GetTextLength() - 1);
	if (pos < 0)
	{
		m_strBuffer.insert(0, 1, ch);
	}
	else if (pos >= GetTextLength())
	{
		//append char
		m_strBuffer += ch;
	}
	else
	{
		m_strBuffer.insert(pos, 1, ch);
	}

	m_bAnalyseRequired = true;
}

void EditBox::RemoveChar(int pos)
{
	//_ASSERT(pos < GetTextLength() - 1);
	if (pos < 0)
	{
		m_strBuffer.erase(0, 1);
	}
	else if (pos >= GetTextLength())
	{
		m_strBuffer.pop_back();
	}
	else
	{
		m_strBuffer.erase(pos, 1);
	}

	m_bAnalyseRequired = true;
}

void EditBox::GetNextItemPos(int pos, int& next)
{
	if (pos == m_strBuffer.size())
		next = pos;
	else
		next = pos + 1;
}

void EditBox::GetPriorItemPos(int pos, int& prior)
{
	if (pos == 0)
		prior = pos;
	else
		prior = pos - 1;
}

int EditBox::GetNumNewlines()
{
	int ret = 0;
	for (auto it : m_strBuffer)
	{
		if (it == '\n')
			ret++;
	}
	ret += (int)m_strInsertedNewlineLocations.size();
	return ret;
}

int EditBox::GetStrIndexFromStrRenderIndex(int strRenderIndex)
{
	_ASSERT(strRenderIndex < m_strRenderBuffer.length());

	if (m_bAnalyseRequired)
		Analyse();

	int ret = strRenderIndex;

	//first, offset by the offset
	ret += m_strRenderBufferOffset;

	//secondly, find all of the added newlines that are less than the index of the render index
	for (auto it : m_strInsertedNewlineLocations)
		if (it <= strRenderIndex)
			--ret;

	//next, find all of the other added characters that are less than the index of the render index
	for (auto it : m_nAdditionalInsertedCharLocations)
		if (it <= strRenderIndex)
			--ret;

	//is there anything else?
	return ret;
}

int EditBox::GetStrRenderIndexFromStrIndex(int strIndex)
{
	if (m_bAnalyseRequired)
		Analyse();

	if (strIndex < m_strRenderBufferOffset)
		return -2;

	int ret = strIndex;

	//offset by the offset
	ret -= m_strRenderBufferOffset;
	if (ret < 0 || ret > m_strRenderBuffer.length() - 1)//if it is less than 0, then just return -1
		return -1;

	if (!m_bMultiline)
		return ret;//if it is multiline, then get off here

	//this is a little more complex
	int offset = 0;
	for (unsigned int i = 0; i+offset < m_strRenderBuffer.length()/*this is usually hit first; ++i)
	{
		if (i >= ret)
			break;
		//make sure to add an offset for each unmatching character
		while (m_strRenderBuffer[i + offset] != m_strBuffer[i + m_strRenderBufferOffset])
			++offset;
	}

	//offset by the number of different characters up to that point
	ret += offset;

	//is there anything else?
	return ret;
}
#pragma warning(default : 4018)

//--------------------------------------------------------------------------------------
void EditBox::OnFocusIn() noexcept
{
    NOEXCEPT_REGION_START

    Control::OnFocusIn();

    ResetCaretBlink();

    m_bAnalyseRequired = true;

    NOEXCEPT_REGION_END
}


//--------------------------------------------------------------------------------------

bool EditBox::MsgProc(MessageType msg, int32_t param1, int32_t param2, int32_t param3, int32_t param4) noexcept
{
    NOEXCEPT_REGION_START


    // Let the scroll bar have a chance to handle it first
    if (mScrollBar->MsgProc(_PASS_CALLBACK_PARAM) && m_bMultiline)//if we are single line, the scroll bar is only here to give us auto scroll
    {
        m_bAnalyseRequired = true;
        return true;
    }

    //UNREFERENCED_PARAMETER(lParam);

    if (!mEnabled || !mVisible)
        return false;

    Point pt = mDialog.GetMousePositionDialogSpace();

    bool bHandled = false;

    switch (msg)
    {
    case MB:
        //case WM_LBUTTONDBLCLK:

        if (!mHasFocus)
            mDialog.RequestFocus(shared_from_this());

        if (param2 == GLFW_PRESS && m_strRenderBuffer.length())
        {

            if (!ContainsPoint(pt))
                return false;
            m_bAnalyseRequired = true;

            m_bMouseDrag = true;
            //SetCapture(GetHWND());
            // Determine the character corresponding to the coordinates.
            int nCP;
            bool bTrail;
            //m_Buffer.CPtoX(m_nFirstVisible, false, &nX1st);  // X offset of the 1st visible char
            if (PttoCP(Point(pt.x - mTextRegion.left, pt.y - mTextRegion.bottom), &nCP, &bTrail))
            {
                nCP = GetStrIndexFromStrRenderIndex(nCP);
                // Cap at the nul character.
                if (bTrail && nCP < GetTextLength())
                    PlaceCaret(nCP);
                else
                    PlaceCaret(nCP - 1);
                //m_nSelStart = m_nCaret;
                ResetCaretBlink();
            }
            else if (pt.y - mTextRegion.bottom < m_CharBoundingBoxes[m_strRenderBuffer.length() - 1].bottom || (
                pt.x - mTextRegion.left   > m_CharBoundingBoxes[m_strRenderBuffer.length() - 1].right &&
                pt.y - mTextRegion.bottom < m_CharBoundingBoxes[m_strRenderBuffer.length() - 1].top &&
                pt.y - mTextRegion.bottom > m_CharBoundingBoxes[m_strRenderBuffer.length() - 1].bottom))
            {
                //this is hit if the mouse pos is below the bottom char, OR on the same line of the bottom row, but past the last char
                PlaceCaret((int)m_strBuffer.length());
                ResetCaretBlink();
            }
            bHandled = true;
            break;
        }
        else
        {
            //ReleaseCapture();
            m_bMouseDrag = false;
            bHandled = true;
            break;
        }
    case CURSOR_POS:
        if (m_bMouseDrag)
        {
            // Determine the character corresponding to the coordinates.
            //int nCP, nX1st;
            //bool bTrail;
            /*m_Buffer.CPtoX(m_nFirstVisible, false, &nX1st);  // X offset of the 1st visible char
            if (m_Buffer.XtoCP(pt.x - mTextRegion.left + nX1st, &nCP, &bTrail))
            {
            // Cap at the nul character.
            if (bTrail && nCP < m_Buffer.GetTextSize())
            PlaceCaret(nCP + 1);
            else
            PlaceCaret(nCP);
            }

            //m_bAnalyseRequired = true;
        }
        break;
    case SCROLL:
        if (!m_bMultiline)
            break;

        if (!mHasFocus)
            mDialog.RequestFocus(shared_from_this());

        mScrollBar->Scroll(-(param2 / WHEEL_DELTA) / 2);
        m_bAnalyseRequired = true;

        bHandled = true;
        break;
    case UNICODE_CHAR:

        //is it within the charset?
        if (!CharsetContains(param1, m_Charset))
        {
            bHandled = true;
            break;
        }

        m_bAnalyseRequired = true;
        //printible chars

        // If there's a selection and the user
        // starts to type, the selection should
        // be deleted.
        if (m_nCaret != m_nSelStart)
            DeleteSelectionText();

        // If we are in overwrite mode and there is already
        // a char at the caret's position, simply replace it.
        // Otherwise, we insert the char as normal.
        if (!m_bInsertMode && m_nCaret < GetTextLength())
        {
            RemoveChar(m_nCaret + 1);
            InsertChar(m_nCaret + 1, param1);
            PlaceCaret(m_nCaret + 1);
            m_nSelStart = m_nCaret;
        }
        else
        {
            // Insert the char
            InsertChar(m_nCaret + 1, param1);
            PlaceCaret(m_nCaret + 1);
            m_nSelStart = m_nCaret;

        }
        ResetCaretBlink();
        mDialog.SendEvent(_EVENT_EDITBOX_CHANGE, true, this);

        bHandled = true;
        break;
    case KEY:
    {
        m_bAnalyseRequired = true;

        if (param3 == GLFW_PRESS || param3 == GLFW_REPEAT)
        {
            switch (param1)
            {
            case GLFW_KEY_TAB:
                // We don't process Tab in case keyboard input is enabled and the user
                // wishes to Tab to other controls.
                break;

            case GLFW_KEY_HOME:
                PlaceCaret(-1);
                if (!param4 & GLFW_MOD_SHIFT)
                    // Shift is not down. Update selection
                    // start along with the caret.
                    m_nSelStart = m_nCaret;
                ResetCaretBlink();
                bHandled = true;
                break;

            case GLFW_KEY_END:
                PlaceCaret(GetTextLength() - 1);
                if (!param4 & GLFW_MOD_SHIFT)
                    // Shift is not down. Update selection
                    // start along with the caret.
                    m_nSelStart = m_nCaret;
                ResetCaretBlink();
                bHandled = true;
                break;

            case GLFW_KEY_INSERT:
                if (param4 & GLFW_MOD_CONTROL)
                {
                    // Control Insert. Copy to clipboard
                    CopyToClipboard();
                }
                else if (param4 & GLFW_MOD_SHIFT)
                {
                    // Shift Insert. Paste from clipboard
                    PasteFromClipboard();
                }
                else
                {
                    // Toggle caret insert mode
                    m_bInsertMode = !m_bInsertMode;
                }
                break;

            case GLFW_KEY_DELETE:
                // Check if there is a text selection.
                if (m_nCaret != m_nSelStart)
                {
                    DeleteSelectionText();
                    mDialog.SendEvent(_EVENT_EDITBOX_CHANGE, true, this);
                }
                else
                {
                    // Deleting one character
                    if (m_nCaret != m_strBuffer.length() - 1)
                    {
                        RemoveChar(m_nCaret + 1);
                        mDialog.SendEvent(_EVENT_EDITBOX_CHANGE, true, this);
                    }
                }
                ResetCaretBlink();
                bHandled = true;
                break;

            case GLFW_KEY_LEFT:
                if (param4 & GLFW_MOD_CONTROL)
                {
                    // Control is down. Move the caret to a new item
                    // instead of a character.
                    GetPriorItemPos(m_nCaret, m_nCaret);
                    PlaceCaret(m_nCaret);
                }
                else if (m_nCaret >= 0)
                    PlaceCaret(m_nCaret - 1);
                if (!param4 & GLFW_MOD_SHIFT)
                    // Shift is not down. Update selection
                    // start along with the caret.
                    m_nSelStart = m_nCaret;
                ResetCaretBlink();
                bHandled = true;
                break;

            case GLFW_KEY_RIGHT:
                if (param4 & GLFW_MOD_CONTROL)
                {
                    // Control is down. Move the caret to a new item
                    // instead of a character.
                    GetNextItemPos(m_nCaret, m_nCaret);
                    PlaceCaret(m_nCaret);
                }
                else if (m_nCaret < GetTextLength())
                    PlaceCaret(m_nCaret + 1);
                if (!param4 & GLFW_MOD_SHIFT)
                    // Shift is not down. Update selection
                    // start along with the caret.
                    m_nSelStart = m_nCaret;
                ResetCaretBlink();
                bHandled = true;
                break;

            case GLFW_KEY_DOWN:
            case GLFW_KEY_UP:
            {
                bool bIsKeyUp;
                if (param1 == GLFW_KEY_DOWN)
                    bIsKeyUp = false;
                else
                    bIsKeyUp = true;

                //case GLFW_KEY_DOWN:
                // Trap up and down arrows so that the dialog
                // does not switch focus to another control.

                //determine the character that is closest above this

                //if it is the top line, the scroll up one
                Rect rc = { 0L, 0L, 0L, 0L };


                FontNodePtr pFont = mDialog.GetFont(GetElement(0)->mFontIndex);

                do
                {
                    int rndCP = GetStrRenderIndexFromStrIndex(m_nCaret);

                    if (m_strRenderBuffer.size() > 1 && rndCP == -1)//first character
                        rndCP = 0;

                    if (rndCP < (int)m_strRenderBuffer.size() - 1)
                    {
                        if (m_strRenderBuffer[rndCP] == '\n' && std::find(m_strInsertedNewlineLocations.begin(), m_strInsertedNewlineLocations.end(), rndCP) != m_strInsertedNewlineLocations.end())
                            rndCP++;

                        //if we have a situation like this:
                        /*

                        textextextextext
                        textextex
                        textextextextextext

                        

                        //THERE IS NOT CURRENTLY SUPPORT FOR THIS FEATURE

                        /*std::vector<std::wstring> lines = SplitStr(m_strRenderBuffer, '\n', true);
                        if (m_strRenderBuffer[rndCP + 1] == '\n')//if the newline is coming up, check to see if there is a 'clif' above or below
                        {
                        long width = 0;
                        unsigned int it = 0;
                        if (bIsKeyUp)
                        {
                        //if it is the up key, we need get to the beginning of the previous row
                        bool foundfirst = false;
                        bool foundsecond = false;
                        for (unsigned int i = rndCP; i > 0; --i)
                        {
                        if (m_strRenderBuffer[i] == '\n')
                        {
                        if (!foundfirst)
                        foundfirst = true;
                        else
                        {
                        it = i + 1;
                        break;
                        }
                        }
                        }
                        }
                        else
                        {
                        it = rndCP + 2;
                        }
                        for (; (bIsKeyUp ? it >=  0 : it < m_strRenderBuffer.length()); (bIsKeyUp ? (--it) : (++it)))
                        {
                        width += pFont->mFontType->GetCharAdvance(m_strRenderBuffer[it]);
                        if (m_strRenderBuffer[it] == '\n')
                        {
                        --it;
                        break;
                        }
                        }

                        if (width < m_CharBoundingBoxes[rndCP].right)//there is a cliff, so just put it at the end
                        {
                        PlaceCaretRndBuffer(it);
                        }

                        }

                    }

                    if (CPtoRC(rndCP, &rc))
                    {
                        //is this the top line?
                        if ((bIsKeyUp ? rc.top == RectHeight(mTextRegion) : rc.bottom < (long)pFont->mLeading))
                        {
                            mScrollBar->Scroll(bIsKeyUp ? -1 : 1);
                            Analyse();
                            rndCP = GetStrRenderIndexFromStrIndex(m_nCaret);
                            if (!CPtoRC(rndCP, &rc))
                                break;
                        }

                        //we have scrolled up if necisary, now to determine what is the character above this one
                        int newCP = 0;
                        bool trail = false;
                        if (!PttoCP(Point(rc.right, rc.bottom + long((bIsKeyUp ? 1.5 : -0.5) * (long)pFont->mLeading)), &newCP, &trail))
                            break;


                        if (trail == false)
                            --newCP;
                        //we have found something
                        PlaceCaretRndBuffer(newCP);
                    }
                } while (false);//this is nice, because it allows me to use break instead of using 'goto'
                //if it fails, then either it is less than 0.


                bHandled = true;
                break;

            }
            case GLFW_KEY_BACKSPACE:
            {
                // If there's a selection, treat this
                // like a delete key.
                if (m_nCaret != m_nSelStart)
                {
                    DeleteSelectionText();
                    mDialog.SendEvent(_EVENT_EDITBOX_CHANGE, true, this);
                }
                else if (m_nCaret >= 0)
                {
                    // Move the caret, then delete the char.
                    RemoveChar(m_nCaret);
                    PlaceCaret(m_nCaret - 1);
                    m_nSelStart = m_nCaret;
                    mDialog.SendEvent(_EVENT_EDITBOX_CHANGE, true, this);
                }
                ResetCaretBlink();
                bHandled = true;

                break;
            }

            case GLFW_KEY_X:        // Ctrl-X Cut
            case GLFW_KEY_C:		// Ctrl-C Copy
            {
                if (param4 & GLFW_MOD_CONTROL)
                {
                    CopyToClipboard();

                    // If the key is Ctrl-X, delete the selection too.
                    if (param1 == GLFW_KEY_X)
                    {
                        DeleteSelectionText();
                        mDialog.SendEvent(_EVENT_EDITBOX_CHANGE, true, this);

                    }

                    bHandled = true;
                }
                break;
            }

            // Ctrl-V Paste
            case GLFW_KEY_V:
            {
                if (param4 & GLFW_MOD_CONTROL)
                {
                    PasteFromClipboard();
                    mDialog.SendEvent(_EVENT_EDITBOX_CHANGE, true, this);

                    bHandled = true;
                }
                break;
            }

            // Ctrl-A Select All
            case GLFW_KEY_A:
                if (param4 & GLFW_MOD_CONTROL)
                {
                    if (m_nSelStart == m_nCaret)
                    {
                        m_nSelStart = 0;
                        PlaceCaret(GetTextLength());

                        bHandled = true;
                    }
                }
                break;

            case GLFW_KEY_ENTER:

                if (!m_bMultiline)
                    break;
                // Invoke the callback when the user presses Enter.
                //mDialog.SendEvent(_EVENT_EDITBOX_STRING, true, this);
                InsertChar(m_nCaret + 1, '\n');//TODO: support "natural" newlines
                PlaceCaret(m_nCaret + 1);
                bHandled = true;

                break;

            default:
                bHandled = param1 != GLFW_KEY_ESCAPE;  // Let the application handle Esc.
            }
        }
    }
    }

    if (bHandled)
        return true;

    return false;

    NOEXCEPT_REGION_END

    return false;
}


//--------------------------------------------------------------------------------------
void EditBox::Render( float elapsedTime) noexcept
{
    NOEXCEPT_REGION_START

    //UpdateRects();
    if (m_bAnalyseRequired)
        Analyse();

    //TODO: don't render scrollbar UNLESS there is necessity to scroll, change this on ALL controls



    if (mVisible == false)
        return;


    //
    // Blink the caret
    //
    if (glfwGetTime() - m_dfLastBlink >= m_dfBlink)
    {
        m_bCaretOn = !m_bCaretOn;
        m_dfLastBlink = glfwGetTime();
    }

    ElementPtr pElement = GetElement(0);

    // Render the control graphics
    for (int e = 0; e < 9; ++e)
    {
        pElement = mElements[e];
        pElement->mTextureColor.Blend(STATE_NORMAL, elapsedTime);

        mDialog.DrawSprite(pElement, m_rcRender[e], _FAR_BUTTON_DEPTH);
    }

    //render the scrollbar
    if (m_bMultiline)
        mScrollBar->Render(elapsedTime);

    FontNodePtr pFontNode = mDialog.GetManager()->GetFontNode(mElements[0]->mFontIndex);
    if (pElement)
    {
        if (mHasFocus && m_bCaretOn && !s_bHideCaret)
        {
            // Start the rectangle with insert mode caret
            Rect rcCaret;
            if (m_nCaret == -1)//if it is -1, then the leading edge of the first character
            {

                // If we are in overwrite mode, adjust the caret rectangle
                // to fill the entire character.
                if (!m_bInsertMode)
                {
                    if (m_CharBoundingBoxes.size())
                        rcCaret = m_CharBoundingBoxes[0];
                }
                else
                    SetRect(rcCaret, 0, RectHeight(mTextRegion), 2, RectHeight(mTextRegion) - pFontNode->mLeading);

                OffsetRect(rcCaret, mTextRegion.left, mTextRegion.bottom);
                mDialog.DrawRect(rcCaret, m_CaretColor);
            }
            else
            {
#pragma warning(disable : 4018)
                int rndCaret = GetStrRenderIndexFromStrIndex(m_nCaret);
                if (rndCaret != -1 && rndCaret != -2 && !(rndCaret > m_strRenderBuffer.size()))//don't render off-screen
                {
                    if (m_strRenderBuffer[rndCaret] == '\n' && (std::find(m_strInsertedNewlineLocations.begin(), m_strInsertedNewlineLocations.end(), rndCaret) == m_strInsertedNewlineLocations.end() || rndCaret == m_strRenderBuffer.back()))
                    {
                        //if it is a newline character, then give the leading edge of the first char of the line

                        SetRect(rcCaret, m_CharBoundingBoxes[0].left - 2, m_CharBoundingBoxes[rndCaret].top, 2, m_CharBoundingBoxes[rndCaret].bottom);
                    }
                    else if (m_strRenderBuffer[rndCaret] == '\n')
                        rndCaret++;

                    if (rndCaret == m_CharBoundingBoxes.size())
                        rndCaret--;
                    SetRect(rcCaret,
                        m_CharBoundingBoxes[rndCaret].right, m_CharBoundingBoxes[rndCaret].top,
                        m_CharBoundingBoxes[rndCaret].right + 2, m_CharBoundingBoxes[rndCaret].bottom);

                    // If we are in overwrite mode, adjust the caret rectangle
                    // to fill the entire character.
                    if (!m_bInsertMode)
                    {
                        if (rndCaret >= m_CharBoundingBoxes.size() - 1);
                        else
                        {
                            rcCaret.right = m_CharBoundingBoxes[rndCaret + 1].right;
                        }
                    }

                    OffsetRect(rcCaret, mTextRegion.left, mTextRegion.bottom);

                    mDialog.DrawRect(rcCaret, m_CaretColor/*::{0, 0, 0, 255));
                }
#pragma warning(default : 4018)
            }
        }
    }

    //debugging
    /*Color color = {255, 0, 255, 128);
    for (auto it : m_CharBoundingBoxes)
    {
    Rect copy = it;
    OffsetRect(copy, mTextRegion.left, mTextRegion.bottom);
    mDialog.DrawRect(copy, color);

    if (color.r == 5)
    color.g += 10;
    else
    color.r -= 10;
    }

    mElements[0]->mFontColor.SetCurrent(m_TextColor);
    mDialog.DrawText(m_strRenderBuffer.c_str(), mElements[0], mTextRegion);



    NOEXCEPT_REGION_END
}


//--------------------------------------------------------------------------------------
void EditBox::ResetCaretBlink()
{
	m_bCaretOn = true;
	m_dfLastBlink = glfwGetTime();
}

//--------------------------------------------------------------------------------------

bool EditBox::CPtoRC(int nCP, Rect *pRc)
{
	_ASSERT(pRc);
	*pRc = { 0L, 0L, 0L, 0L };

	if (nCP > (int)m_strRenderBuffer.length() - 1 || nCP < 0)
	{
		return false;
	}

	if (m_bAnalyseRequired)
		Analyse();


	*pRc = m_CharBoundingBoxes[nCP];

	return true;
}


//--------------------------------------------------------------------------------------

bool EditBox::PttoCP(Point pt, int* pCP, bool* bTrail)
{
	_ASSERT(pCP && bTrail);
	*pCP = 0; *bTrail = false;  // Default

	if (m_bAnalyseRequired)
		Analyse();

	Rect charRect = { 0L, 0L, 0L, 0L };

	for (auto it : m_CharBoundingBoxes)
	{
		charRect = it;

		//this deals with the bounding boxes leaving space between characters
		if (it != m_CharBoundingBoxes.back())
			charRect.right = m_CharBoundingBoxes[*pCP + 1].left;

		if (PtInRect(charRect, pt))
		{
			if (PtInRect({ charRect.left, charRect.top, charRect.right - RectWidth(it) / 2, charRect.bottom }, pt))
			{
				//leading edge
				*bTrail = false;
			}
			else
			{
				//trailing edge
				*bTrail = true;
			}

			//*pCP = GetStrRenderIndexFromStrIndex(*pCP);
			return true;
		}

		++*pCP;
	}
	//if it failed, then make this -1
	*pCP = -1;

	return false;
}

void EditBox::Analyse()
{
	FontNodePtr pFontNode = mDialog.GetManager()->GetFontNode(mElements[0]->mFontIndex);
	//split the string into words to make line breaks in between words

	//first set the render buffer to the string
	m_strRenderBuffer = m_strBuffer;

	//take preexisting newlines, and make sure they have a space on either side, so they are their own words
	//unsigned int addedCharactersCount = 0;
	m_nAdditionalInsertedCharLocations.clear();

	if (m_bMultiline)
	{
		for (unsigned int it = 0; it < m_strBuffer.length(); ++it)
		{
			if (m_strBuffer[it] == '\n')
			{

				if (it == 0) //if it is the first char
				{
					if (m_strBuffer[it + 1] != ' ')
					{
						m_strRenderBuffer.insert(m_nAdditionalInsertedCharLocations.size() + it + 1, 1, ' ');//insert a space after
						m_nAdditionalInsertedCharLocations.push_back(it + m_nAdditionalInsertedCharLocations.size());
					}
				}
				else if (m_strBuffer.length() - 1)//if it is the last char
				{
					if (m_strBuffer[it - 1] != ' ')
					{
						m_strRenderBuffer.insert(m_nAdditionalInsertedCharLocations.size() + it - 1, 1, ' ');//insert a space before
						m_nAdditionalInsertedCharLocations.push_back(it + m_nAdditionalInsertedCharLocations.size());
					}
				}
				else//anything else
				{
					if (m_strBuffer[it - 1] != ' ')
					{
						m_strRenderBuffer.insert(m_nAdditionalInsertedCharLocations.size() + it - 1, 1, ' ');//insert a space before
						m_nAdditionalInsertedCharLocations.push_back(it + m_nAdditionalInsertedCharLocations.size());
					}

					if (m_strBuffer[it + 1] != ' ')										//AND
					{
						m_strRenderBuffer.insert(m_nAdditionalInsertedCharLocations.size() + it + 1, 1, ' ');//insert a space after
						m_nAdditionalInsertedCharLocations.push_back(it + m_nAdditionalInsertedCharLocations.size());
					}
				}
			}
		}
	}
	else
	{
		//single line, DESTROY ALL NEWLINES
		for (auto it = m_strBuffer.begin(); it != m_strBuffer.end(); ++it)
		{
			if (*it == '\n')
			{
				m_strBuffer.erase(it);
			}
		}
		m_strRenderBuffer = m_strBuffer;
	}

	if (m_bMultiline)
	{//new block just to remove these variables
		long TextWidth = RectWidth(mTextRegion);
		long currXValue = 0;
		int charIndex = 0;
		int addedCharactersCount = 0;

		std::vector<std::wstring> strings = SplitStr(m_strRenderBuffer, ' ', true);
		m_strInsertedNewlineLocations.clear();
		for (auto it : strings)
		{


			long WordWidth = pFontNode->mFontType->GetStringWidth(it);

			//these are natural newlines
			if (it == L"\n ")
			{
				currXValue = WordWidth;
				charIndex += (int)it.length();
				continue;
			}

			//if the current word width is bigger than the whole line, then newline at the box width
			if (WordWidth > TextWidth)
			{
				//float charXPos = 0.0f;
				int chIndex = 0;

				for (auto itch : it)
				{
#pragma warning(disable : 4244)
					FontSize nCharWidth;
					nCharWidth = pFontNode->mFontType->GetCharAdvance(itch);
#pragma warning(default : 4244)
					currXValue += (int)nCharWidth;

					if (currXValue > TextWidth)
					{
						m_strRenderBuffer.insert(charIndex + chIndex + addedCharactersCount, 1, '\n');
						m_strInsertedNewlineLocations.push_back(charIndex + chIndex);
						//charXPos = 0.0f;
						currXValue = (int)nCharWidth;
						++addedCharactersCount;
					}

					chIndex++;
				}

			}
			else
			{
				currXValue += WordWidth;
			}

			if (currXValue/* + 0.0125f/*a little buffer > TextWidth)
			{

				//add a "newline" (since there is always a trailing space, hack this a little bit so the space will always be at the end

				//blank strings i.e. double spaces ( or more ) then we just keep those at the end of the line
				/*if (it == L" ")
				{

				}
				
				{
					m_strRenderBuffer.insert(charIndex + addedCharactersCount, 1, '\n');
					m_strInsertedNewlineLocations.push_back(charIndex + addedCharactersCount);
					currXValue = WordWidth;
					++addedCharactersCount;
				}
			}

			//if (charIndex == 0)
			//	--charIndex;

			charIndex += (int)it.size();
		}
	}

	//recalculate scroll bar (this has to be done in between analyzing steps)
	if (pFontNode && pFontNode->mFontType->mHeight)
	{
		if (m_bMultiline)
		{
			mScrollBar->SetPageSize(int(RectHeight(mTextRegion) / pFontNode->mFontType->mHeight));
			mScrollBar->SetTrackRange(0, GetNumNewlines() + 1);
		}
		else
		{
			mScrollBar->SetTrackRange(0, (int)m_strBuffer.length()-1);

			FontSize strWidth = 0;
			unsigned int count = 0;
			unsigned long textWidth = RectWidth(mTextRegion);
			for (unsigned int i = mScrollBar->GetTrackPos(); i < m_strBuffer.length(); ++i)
				if (strWidth < textWidth)
				{
					strWidth += pFontNode->mFontType->GetCharAdvance(m_strBuffer[i]);
					++count;
				}
				else
				{
					//strWidth -= pFontNode->mFontType->GetCharAdvance(m_strBuffer[i]);
					--count;
					break;
				}

			mScrollBar->SetPageSize(count);
			mScrollBar->SetTrackPos(m_nCaret);
		}
		// The selected item may have been scrolled off the page.
		// Ensure that it is in page again.
		//mScrollBar->ShowItem(GetLineNumberFromCharPos(m_nCaret));
	}


	//now get the char bounding boxes (this ALSO culls the characters that would go off screen, as well as offset for the scrollbar)

	m_CharBoundingBoxes.clear();
	m_strRenderBufferOffset = 0;

	//for each character, get the width, and add that to the width of the previous, also add initial 0
	//m_CalcXValues.push_back(0);
	if (m_bMultiline)
	{
		int currXValue = 0;
		int distanceFromBottom = RectHeight(mTextRegion);
		int   lineNum = 0;

		int thisCharWidth = 0;
		int fontHeight = pFontNode->mLeading;
		int scrollbarOffset = mScrollBar->GetTrackPos() * pFontNode->mLeading;
		Rect rc;

		bool topCulled = false;

		std::wstring strRenderBufferTmp = L"";
		unsigned int i = 0;
		for (auto it : m_strRenderBuffer)
		{



			//cull the characters that will not fit on the page
			if (lineNum > mScrollBar->GetTrackPos() + mScrollBar->GetPageSize() || lineNum < mScrollBar->GetTrackPos())
			{
				if (topCulled)//this is called once the top has been culled, then the body has been added
					break;
				else
				{
					//if this IS NOT a inserted newline, then we can add it
					if (it == '\n')
						if (std::find(m_strInsertedNewlineLocations.begin(), m_strInsertedNewlineLocations.end(), i) == m_strInsertedNewlineLocations.end())
						{
							++m_strRenderBufferOffset;
						}
						else;
					else
						++m_strRenderBufferOffset;
				}
			}
			else
			{
				topCulled = true;

#pragma warning(disable : 4244)
				thisCharWidth = pFontNode->mFontType->GetCharAdvance(it);
#pragma warning(default : 4244)

				rc = pFontNode->mFontType->GetCharRect(it);
				if (it == '\n')
				{
					rc.top = distanceFromBottom - fontHeight;
					rc.bottom = rc.top - fontHeight;
					rc.left = 0;
					rc.right = 0;
				}
				else
				{
					rc.top = distanceFromBottom;
					rc.bottom = distanceFromBottom - fontHeight;
					rc.right = rc.left + pFontNode->mFontType->GetCharWidth(it);//use the char width for nice carrot position
					OffsetRect(rc, currXValue, 0);
					//SetRect(rc, currXValue, distanceFromBottom - pFontNode->mFontType->Get, currXValue + thisCharWidth, distanceFromBottom - fontHeight);
				}


				//offset the whole block by the scroll level
				OffsetRect(rc, 0, scrollbarOffset);

				//does fit on page
				strRenderBufferTmp += it;
				m_CharBoundingBoxes.push_back(rc);
			}

			currXValue += thisCharWidth;
			//this MUST go after the culling process, because the new line starts AFTER this character
			if (it == '\n')
			{
				lineNum++;
				currXValue = 0;
				distanceFromBottom -= fontHeight;
				/*m_CharBoundingBoxes.pop_back();
				SetRect(rc, 0.0f, distanceFromBottom, 0.0f, distanceFromBottom - fontHeight);
				m_CharBoundingBoxes.push_back();
			}

			++i;
		}
		m_strRenderBuffer = strRenderBufferTmp;
	}
	else
	{
		//for single line, we do HORIZONTAL culling
		int currXValue = 0;
		long top = RectHeight(mTextRegion), bottom = top - pFontNode->mLeading;

		bool leftCulled = false;
		int thisCharWidth = 0;
		int scrollbarOffset = mScrollBar->GetTrackPos();
		Rect rc;

		std::wstring strRenderBufferTmp = L"";
		int i = 0;
		for (auto it : m_strRenderBuffer)
		{

			//cull the characters that will not fit on the page
			if (i < mScrollBar->GetTrackPos() || i > mScrollBar->GetTrackPos() + mScrollBar->GetPageSize() - 1)
			{
				if (!leftCulled)
					++m_strRenderBufferOffset;
				else
					break;
			}
			else
			{
				leftCulled = true;

#pragma warning(disable : 4244)
				thisCharWidth = pFontNode->mFontType->GetCharAdvance(it);
#pragma warning(default : 4244)

				rc = pFontNode->mFontType->GetCharRect(it);
				rc.top = top;
				rc.bottom = bottom;
				rc.right = rc.left + pFontNode->mFontType->GetCharWidth(it);//use the char width for nice carrot position
				OffsetRect(rc, currXValue, 0);

				//does fit on page
				strRenderBufferTmp += it;
				m_CharBoundingBoxes.push_back(rc);


				currXValue += thisCharWidth;
			}


			++i;
		}
		m_strRenderBuffer = strRenderBufferTmp;

		//for single lines, neither of these will happen
		m_strInsertedNewlineLocations.clear();
		m_nAdditionalInsertedCharLocations.clear();
	}

	m_bAnalyseRequired = false;  // Analysis is up-to-date
	mScrollBar->ShowItem(m_nCaret);

}*/






/*
======================================================================================================================================================================================================
 Text Functions


*/
namespace Text
{

glm::mat4 g_TextOrtho;
glm::mat4 g_TextModelMatrix;
//GLVector<TextVertexStruct> g_TextVertices;
Color4f g_TextColor;

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

    g_TextColor = ColorToFloat(color);

    //g_TextModelMatrix = glm::translate(glm::mat4(), glm::vec3(0.5f, 1.5f, 0.0f));//glm::mat4(1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.3f, 0.3f, 0.0f, 1.0f);

    //buffer the data
    g_TextVertexArray->BufferData(textVertices);

    //buffer the indices
    g_TextVertexArray->BufferIndices(indices);

    EndText(font->mFontType);

}


void EndText(const FontPtr& font)
{
    SHADERMANAGER.UseProgram(g_TextProgram);

    //first uniform: model-view matrix
    SHADERMANAGER.GLUniformMatrix4f(g_TextShaderLocations.ortho, g_TextOrtho);

    //second, the sampler
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, font->mTexId);
    SHADERMANAGER.GLUniform1i(g_TextShaderLocations.sampler, 0);


    //third, the color
    SHADERMANAGER.GLUniform4f(g_TextShaderLocations.color, g_TextColor);

    //make sure to enable this with text
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    g_TextVertexArray->Draw();

    //g_TextVertices.clear();
}
}


//TextHelper
TextHelper::TextHelper(DialogResourceManagerPtr& manager) :
    mManager(manager), mColor(0, 0, 0, 255), mPoint(0L, 0L), 
    mFontIndex(0), mFontSize(15L)
{}

void TextHelper::Begin(FontIndex drmFont, FontSize leading, FontSize size)
{
    mManager->GetFontNode(drmFont);

    mFontIndex = drmFont;
    mFontSize = size;
    mLeading = leading;

    Text::BeginText(mManager->GetOrthoMatrix());
}

void TextHelper::DrawTextLine(const std::wstring& text)
{
    DrawTextLine({ { mPoint.x }, mPoint.y, mPoint.x + 50L, { mPoint.y - 50L } }, GT_LEFT | GT_TOP, text);

	//set the point down however many lines were drawn
	for (auto it : text)
	{
		if (it == '\n')
            mPoint.y += mLeading;
	}
    mPoint.y -= mLeading;//once no matter what because we are drawing a LINE of text
}

void TextHelper::DrawTextLine(const Rect& rc, Bitfield flags, const std::wstring& text)
{
    mManager->GetFontNode(mFontIndex)->mLeading = mLeading;
    Text::DrawText(mManager->GetFontNode(mFontIndex), text, rc, mColor, flags, true);
}

void TextHelper::End()
{
    Text::EndText(mManager->GetFontNode(mFontIndex)->mFontType);
}

}
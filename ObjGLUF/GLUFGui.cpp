#include "stdafx.h"
#include "GLUFGui.h"
//#include "CBFG/BitmapFontClass.h"
#include <ft2build.h> 
#include FT_FREETYPE_H

#include <algorithm>

/*

Random Text Helping functions

================================================= TODO: ==================================================
    Relocate these methods into a more sensible spot/container/namespace/whatever
        Probably in a source file instead of a header file

void BeginText(const glm::mat4& orthoMatrix);


void DrawTextGLUF(const GLUFFontNode& font, const std::wstring& text, const GLUFRect& rect, const Color& color, GLUFBitfield textFlags, bool hardRect = false);
void EndText(const GLUFFontPtr& font); 
*/

namespace GLUF
{

//this defines the space in pixels between glyphs in the font
#define GLYPH_PADDING 5

// Minimum scroll bar thumb size
#define SCROLLBAR_MINTHUMBSIZE 8

// Delay and repeat period when clicking on the scroll bar arrows
#define SCROLLBAR_ARROWCLICK_DELAY  0.33f
#define SCROLLBAR_ARROWCLICK_REPEAT 0.05f

#define GLUF_NEAR_BUTTON_DEPTH -0.6f
#define GLUF_FAR_BUTTON_DEPTH -0.8f

#define GLUF_MAX_GUI_SPRITES 500
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

void MessageProcedure(GLUFMessageType, int, int, int, int);

/*
===================================================================================================
GLFW Window Callbacks

*/

//--------------------------------------------------------------------------------------
void GLFWWindowPosCallback(GLFWwindow*, int x, int y)
{
	MessageProcedure(GM_POS, x, y, 0, 0);
}

//--------------------------------------------------------------------------------------
void GLFWWindowSizeCallback(GLFWwindow*, int width, int height)
{
	MessageProcedure(GM_RESIZE, width, height, 0, 0);
}

//--------------------------------------------------------------------------------------
void GLFWWindowCloseCallback(GLFWwindow*)
{
	MessageProcedure(GM_CLOSE, 0, 0, 0, 0);
}
/*
//--------------------------------------------------------------------------------------
void GLFWWindowRefreshCallback(GLFWwindow*)
{
	MessageProcedure(GM_REFRESH, 0, 0, 0, 0);
}*/

//--------------------------------------------------------------------------------------
void GLFWWindowFocusCallback(GLFWwindow*, int focused)
{
	MessageProcedure(GM_FOCUS, focused, 0, 0, 0);
}

//--------------------------------------------------------------------------------------
void GLFWWindowIconifyCallback(GLFWwindow*, int iconified)
{
	MessageProcedure(GM_ICONIFY, iconified, 0, 0, 0);
}

//--------------------------------------------------------------------------------------
void GLFWFrameBufferSizeCallback(GLFWwindow*, int width, int height)
{
	MessageProcedure(GM_FRAMEBUFFER_SIZE, width, height, 0, 0);
}

/*
===================================================================================================
GLFW Input Callback

*/

//--------------------------------------------------------------------------------------
void GLFWMouseButtonCallback(GLFWwindow*, int button, int action, int mods)
{
	MessageProcedure(GM_MB, button, action, mods, 0);
}

//--------------------------------------------------------------------------------------
void GLFWCursorPosCallback(GLFWwindow*, double xPos, double yPos)
{
	MessageProcedure(GM_CURSOR_POS, (int)xPos, (int)yPos, 0, 0);
}

//--------------------------------------------------------------------------------------
void GLFWCursorEnterCallback(GLFWwindow*, int entered)
{
	MessageProcedure(GM_CURSOR_ENTER, entered, 0, 0, 0);
}

//--------------------------------------------------------------------------------------
void GLFWScrollCallback(GLFWwindow*, double xoffset, double yoffset)
{
	MessageProcedure(GM_SCROLL, (int)(xoffset * 1000.0), (int)(yoffset * 1000.0), 0, 0);
}

//--------------------------------------------------------------------------------------
void GLFWKeyCallback(GLFWwindow*, int key, int scancode, int action, int mods)
{
	MessageProcedure(GM_KEY, key, scancode, action, mods);
}

//--------------------------------------------------------------------------------------
void GLFWCharCallback(GLFWwindow*, unsigned int codepoint)
{
	MessageProcedure(GM_UNICODE_CHAR, (int)codepoint, 0, 0, 0);
}


GLUFCallbackFuncPtr g_pCallback;

//--------------------------------------------------------------------------------------
void MessageProcedure(GLUFMessageType msg, int param1, int param2, int param3, int param4)
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

/*

Ended here July 10 2015


*/
struct GLUFScreenVertex
{
	glm::vec3 pos;
	Color     color;
	glm::vec2 uv;
};

struct GLUFScreenVertexUntex
{
	glm::vec3 pos;
	Color     color;
};

struct GLUFTextVertexArray
{
private:
	std::vector<glm::vec3> vPos;
	Color4f                vColor;
	std::vector<glm::vec2> vTex;
public:

	glm::vec3* data_pos()  { if (size() > 0) return &vPos[0]; else return nullptr; }
	glm::vec2* data_tex()  { if (size() > 0) return &vTex[0]; else return nullptr; }
	Color4f    get_color() { return vColor;						}

	void push_back(glm::vec3 pos, glm::vec2 tex)
	{
		vPos.push_back(/*GLUFScreenToClipspace(pos)*/pos);	vTex.push_back(tex);
	}

	void set_color(Color col)
	{
		vColor = GLUFColorToFloat(col);
	}

	void clear(){ vPos.clear(); vColor = Color4f(); vTex.clear(); }
	unsigned long size(){ return (unsigned long)vPos.size(); }
};


/*
======================================================================================================================================================================================================
Initialization and Globals


*/


FT_Library g_FtLib;

unsigned short g_WndWidth = 0;
unsigned short g_WndHeight = 0;

GLUFFontPtr g_DefaultFont = nullptr;
GLUFProgramPtr g_UIProgram = nullptr;
GLUFProgramPtr g_UIProgramUntex = nullptr;
GLUFProgramPtr g_TextProgram = nullptr;
GLUFTextVertexArray g_TextVerticies;
GLUFVertexArray g_TextVertexArray(GL_TRIANGLES, GL_STREAM_DRAW, true);

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
"	Color = _Color;													\n"\
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
"	oColor = vec4(mix(oColor.rgb, Color.rgb, Color.a), oColor.a);	\n"\
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
"	//Color.a = 1.0f;												\n"\
"	Color.a *= _Color.a;											\n"\
"	//Color = vec4(1.0f, 0.0f, 0.0f, 1.0f);							\n"\
"	gl_FragColor = Color;											\n"\
"}																	\n";


/*
======================================================================================================================================================================================================
Initialization Functions


*/

//--------------------------------------------------------------------------------------
bool GLUFInitGui(GLFWwindow* pInitializedGLFWWindow, GLUFCallbackFuncPtr callback, GLuint controltex)
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
	GLUFShaderSourceList sources;
    sources.insert({ SH_VERTEX_SHADER, g_UIShaderVert });
    sources.insert({ SH_FRAGMENT_SHADER, g_UIShaderFrag });
	GLUFSHADERMANAGER.CreateProgram(g_UIProgram, sources);
	sources.clear();

    sources.insert({ SH_VERTEX_SHADER, g_UIShaderVert });
    sources.insert({ SH_FRAGMENT_SHADER, g_UIShaderFragUntex });
	GLUFSHADERMANAGER.CreateProgram(g_UIProgramUntex, sources);
	sources.clear();

    sources.insert({ SH_VERTEX_SHADER, g_TextShaderVert });
    sources.insert({ SH_FRAGMENT_SHADER, g_TextShaderFrag });
	GLUFSHADERMANAGER.CreateProgram(g_TextProgram, sources);


	//load the locations
	g_UIShaderLocations.position		= GLUFSHADERMANAGER.GetShaderVariableLocation(g_UIProgram, GLT_ATTRIB, "_Position");
	g_UIShaderLocations.uv				= GLUFSHADERMANAGER.GetShaderVariableLocation(g_UIProgram, GLT_ATTRIB, "_UV");
	g_UIShaderLocations.color			= GLUFSHADERMANAGER.GetShaderVariableLocation(g_UIProgram, GLT_ATTRIB, "_Color");
	g_UIShaderLocations.ortho			= GLUFSHADERMANAGER.GetShaderVariableLocation(g_UIProgram, GLT_UNIFORM, "_Ortho");
	g_UIShaderLocations.sampler			= GLUFSHADERMANAGER.GetShaderVariableLocation(g_UIProgram, GLT_UNIFORM, "_TS");

	g_UIShaderLocationsUntex.position	= GLUFSHADERMANAGER.GetShaderVariableLocation(g_UIProgram, GLT_ATTRIB, "_Position");
	g_UIShaderLocationsUntex.color		= GLUFSHADERMANAGER.GetShaderVariableLocation(g_UIProgram, GLT_ATTRIB, "_Color");
	g_UIShaderLocationsUntex.ortho		= GLUFSHADERMANAGER.GetShaderVariableLocation(g_UIProgram, GLT_UNIFORM, "_Ortho");

	g_TextShaderLocations.position		= GLUFSHADERMANAGER.GetShaderVariableLocation(g_TextProgram, GLT_ATTRIB, "_Position");
	g_TextShaderLocations.uv			= GLUFSHADERMANAGER.GetShaderVariableLocation(g_TextProgram, GLT_ATTRIB, "_UV");
	g_TextShaderLocations.color			= GLUFSHADERMANAGER.GetShaderVariableLocation(g_TextProgram, GLT_UNIFORM, "_Color");
	g_TextShaderLocations.ortho			= GLUFSHADERMANAGER.GetShaderVariableLocation(g_TextProgram, GLT_UNIFORM, "_Ortho");
	g_TextShaderLocations.sampler		= GLUFSHADERMANAGER.GetShaderVariableLocation(g_TextProgram, GLT_UNIFORM, "_TS");

	//create the text arrrays
	/*glGenVertexArrayBindVertexArray(&g_TextVAO);
	glGenBuffers(1, &g_TextPos);
	glGenBuffers(1, &g_TextTexCoords);

	glBindBuffer(GL_ARRAY_BUFFER, g_TextPos);
	glVertexAttribPointer(g_TextShaderLocations.position, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

	glBindBuffer(GL_ARRAY_BUFFER, g_TextTexCoords);
	glVertexAttribPointer(g_TextShaderLocations.uv, 2, GL_FLOAT, GL_FALSE, 0, nullptr);

	glBindVertexArray(0);*/

    g_TextVertexArray.AddVertexAttrib({ 4, 3, GL_FLOAT, 0 }, 0);
    g_TextVertexArray.AddVertexAttrib({ 4, 2, GL_FLOAT, 0 }, 12);

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
GLUFCallbackFuncPtr GLUFChangeCallbackFunc(GLUFCallbackFuncPtr newCallback)
{
    GLUFCallbackFuncPtr tmp = g_pCallback;
	g_pCallback = newCallback;
	return tmp;
}

//--------------------------------------------------------------------------------------
void GLUFTerminate()
{
	FT_Done_FreeType(g_FtLib);
}


/*
======================================================================================================================================================================================================
Font Stuff


*/


//--------------------------------------------------------------------------------------
void GLUFSetDefaultFont(GLUFFontPtr& pDefFont)
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
GLUFFont
    
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
class GLUFFont
{	
public:

	FT_Face mFtFont;
	GLUFFontSize mHeight;
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
	GLUFFontSize GetCharWidth(wchar_t ch);

    /*
    GetCharHeight

        Parameters:
            'ch': the character to get from  
    
        Throws:
            'std::out_of_range': if ch is not within mCharacterOffset and mCaracterEnd  
    
    */
	GLUFFontSize GetCharHeight(wchar_t ch);

    /*
    GetCharAdvance

        Note:
            Gets the distance between this character and the next character when drawing, i.e. the 't' character will have a smaller advance than width

        Parameters:
            'ch': the character to get from
    
        Throws:
            'std::out_of_range': if ch is not within mCharacterOffset and mCaracterEnd
    
    */
	GLUFFontSize GetCharAdvance(wchar_t ch);

    /*
    GetStringWidth

        Note:
            This is a gets how wide the string is.  This adds the advance of each character together for the width
    
    
        Throws:
            'std::out_of_range': if any characters within the string are not within mCharacterOffset and mCharacterEnd
    
    */
	GLUFFontSize GetStringWidth(const std::wstring& str);

    /*
    Init
    
        Note:
            Creates a font from a set of data

        Parameters:
            'data': the data to be created from
            'fontHeight': the height of the font to load
            
        Throws:
            'GLUFLoadFontException': if loading failed
    */
	void Init(const std::vector<char>& data, GLUFFontSize fontHeight);

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
	GLUFRect GetCharRect(wchar_t ch);
	GLUFRectf GetCharTexRect(wchar_t ch);
    
};

GLUFFontSize GLUFGetFontHeight(GLUFFontPtr font)
{
	return font->mHeight;
}

void GLUFFont::Refresh()
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
    for (int i = 0; i < GLYPH_PADDING * mAtlasSize.y; ++i)
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

void GLUFFont::Init(const std::vector<char>& data, GLUFFontSize fontHeight)
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

GLUFRect GLUFFont::GetCharRect(wchar_t ch)
{
    if (ch < mCharacterOffset || ch >= mCharacterEnd)
        throw std::out_of_range("Character Not In Atlas!");

	GLUFRect rc = { 0, GetCharHeight(ch), GetCharWidth(ch), 0 };

	if (ch < mCharacterOffset)
		return rc;
	//GLUFOffsetRect(rc, 0.0f, mCharAtlas[ch - mCharacterOffset].ay);

	//if there is a dropdown, make sure it is accounted for
	//float dy = ((mCharAtlas[ch - mCharacterOffset].bh - mCharAtlas[ch - mCharacterOffset].bt) / mAtlasHeight) * mHeight;
	//GLUFOffsetRect(rc, (mCharAtlas[ch - mCharacterOffset].bl * GetCharHeight(ch)) / mAtlasHeight, dy);
	GLUFOffsetRect(rc, mCharAtlas[ch - mCharacterOffset].mBitLoc.x, -(long)(mCharAtlas[ch - mCharacterOffset].mBitSize.y - mCharAtlas[ch - mCharacterOffset].mBitLoc.y));
	return rc;
}

/*GLUFRect GLUFFont::GetCharRectNDC(wchar_t ch)
{
	GLUFRect rc = { 0.0f, 2.0f * GetCharHeight(ch), 2.0f * GetCharWidth(ch), 0.0f };
	//GLUFOffsetRect(rc, 0.0f, mCharAtlas[ch - mCharacterOffset].ay);

	//if there is a dropdown, make sure it is accounted for
	float dy = ((mCharAtlas[ch - mCharacterOffset].bh - mCharAtlas[ch - mCharacterOffset].bt) / mAtlasHeight) * mHeight;
	GLUFOffsetRect(rc, (mCharAtlas[ch - mCharacterOffset].bl * GetCharHeight(ch)) / mAtlasHeight, -2 * dy);
	return rc;
}*/

GLUFRectf GLUFFont::GetCharTexRect(wchar_t ch)
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

GLUFFontSize GLUFFont::GetCharAdvance(wchar_t ch)
{
    if (ch < mCharacterOffset || ch >= mCharacterEnd)
        throw std::out_of_range("Character Not In Atlas!");

	return mCharAtlas[ch - mCharacterOffset].mAdvance.x;
}

GLUFFontSize GLUFFont::GetCharWidth(wchar_t ch)
{
    if (ch < mCharacterOffset || ch >= mCharacterEnd)
        throw std::out_of_range("Character Not In Atlas!");

	return mCharAtlas[ch - mCharacterOffset].mBitSize.x;
}

GLUFFontSize GLUFFont::GetCharHeight(wchar_t ch)
{
    if (ch < mCharacterOffset || ch >= mCharacterEnd)
        throw std::out_of_range("Character Not In Atlas!");

	return mCharAtlas[ch - mCharacterOffset].mBitSize.y;
}

GLUFFontSize GLUFFont::GetStringWidth(const std::wstring& str)
{
    for (auto ch : str)
    {
        if (ch < mCharacterOffset || ch >= mCharacterEnd)
            throw std::out_of_range("Character Not In Atlas!");
    }

	GLUFFontSize tmp = 0;
	for (auto it : str)
	{
		tmp += GetCharAdvance(it);
	}
	return tmp;
}


void GLUFLoadFont(GLUFFontPtr& font, const std::vector<char>& rawData, GLUFFontSize fontHeight)
{
    font = std::make_shared<GLUFFont>();

    font->Init(rawData, fontHeight);
}



/*
======================================================================================================================================================================================================
GLUFBlendColor Functions


*/

//--------------------------------------------------------------------------------------
void GLUFBlendColor::Init(const GLUF::Color& defaultColor, const GLUF::Color& disabledColor, const GLUF::Color& hiddenColor)
{
    for (auto it = mStates.begin(); it != mStates.end(); ++it)
    {
        it->second = defaultColor;
    }

	mStates[GLUF_STATE_DISABLED] = disabledColor;
	mStates[GLUF_STATE_HIDDEN] = hiddenColor;
	mCurrentColor = mStates[GLUF_STATE_HIDDEN];//start hidden
}


//--------------------------------------------------------------------------------------
void GLUFBlendColor::Blend(GLUFControlState state, float elapsedTime, float rate)
{
	//this is quite condensed, this basically interpolates from the current state to the destination state based on the time
    //the speed of this transition is a recurisve version of e^kx - 1.0f
    mCurrentColor = glm::mix(mCurrentColor, mStates[state], glm::clamp(powf(GLUF_E_F, rate * elapsedTime) - 1.0f, 0.0f, 1.0f));
}

//--------------------------------------------------------------------------------------
void GLUFBlendColor::SetCurrent(const Color& current)
{
	mCurrentColor = current;
}

//--------------------------------------------------------------------------------------
void GLUFBlendColor::SetCurrent(GLUFControlState state)
{
	mCurrentColor = mStates[state];
}

//--------------------------------------------------------------------------------------
void GLUFBlendColor::SetAll(const Color& color)
{
    for (auto it = mStates.begin(); it != mStates.end(); ++it)
    {
        it->second = color;
    }

	SetCurrent(color);
}



/*
======================================================================================================================================================================================================
GLUFElement Functions


*/

//--------------------------------------------------------------------------------------
void GLUFElement::SetTexture(GLUFTextureIndex textureIndex, const GLUFRectf& uvRect, const GLUF::Color& defaultTextureColor)
{
    mTextureIndex = textureIndex;
    mUVRect = uvRect;
    mTextureColor.Init(defaultTextureColor);
}


//--------------------------------------------------------------------------------------
void GLUFElement::SetFont(GLUFFontIndex font, const GLUF::Color& defaultFontColor, GLUFBitfield textFormat)
{
    mFontIndex = font;
    mFontColor.Init(defaultFontColor);
    mTextFormatFlags = textFormat;
}


//--------------------------------------------------------------------------------------
void GLUFElement::Refresh()
{
	mTextureColor.SetCurrent(GLUF_STATE_HIDDEN);
	mFontColor.SetCurrent(GLUF_STATE_HIDDEN);
}



/*
======================================================================================================================================================================================================
GLUFDialog Functions


*/

//--------------------------------------------------------------------------------------
GLUFDialog::GLUFDialog()
{
#ifdef GLUF_DEBUG
    //TODO: get a more graceful way to test this
    //This is to make sure all dialogs are being destroyed
    printf("GLUFDialog Created");
#endif
}

double GLUFDialog::sTimeRefresh = GLUF_60HZ;
GLUFControlPtr GLUFDialog::sControlFocus = nullptr;
GLUFControlPtr GLUFDialog::sControlPressed = nullptr;

//--------------------------------------------------------------------------------------
GLUFDialog::~GLUFDialog()
{
#ifdef GLUF_DEBUG
    //TODO: get a more graceful way to test this
    //This is to make sure all dialogs are being destroyed
    printf("GLUFDialog Destroyed");
#endif

	RemoveAllControls();
}


//--------------------------------------------------------------------------------------
void GLUFDialog::Init(GLUFDialogResourceManagerPtr& manager, bool registerDialog)
{
	if (g_ControlTextureResourceManLocation == -1)
	{
        g_ControlTextureResourceManLocation = mDialogManager->AddTexture(g_pControlTexturePtr);
	}

    Init(manager, registerDialog, g_ControlTextureResourceManLocation);
}


//--------------------------------------------------------------------------------------
void GLUFDialog::Init(GLUFDialogResourceManagerPtr& manager, bool registerDialog, unsigned int textureIndex)
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

/*void GLUFDialog::Init(GLUFDialogResourceManager* pManager, bool bRegisterDialog, LPCWSTR szControlTextureResourceName, HMODULE hControlTextureResourceModule)
{
	m_pManager = pManager;
	if (bRegisterDialog)
		pManager->RegisterDialog(this);

	SetTexture(0, szControlTextureResourceName, hControlTextureResourceModule);
	InitDefaultElements();
}*/


//--------------------------------------------------------------------------------------
void GLUFDialog::SetCallback(GLUFEventCallbackFuncPtr callback, GLUFEventCallbackReceivablePtr userContext)
{
    NOEXCEPT_REGION_START
	// If this assert triggers, you need to call GLUFDialog::Init() first.  This change
	// was made so that the GLUF's GUI could become separate and optional from GLUF's core.  The 
	// creation and interfacing with GLUFDialogResourceManager is now the responsibility 
	// of the application if it wishes to use GLUF's GUI.
	GLUF_ASSERT(mDialogManager && L"To fix call GLUFDialog::Init() first.  See comments for details.");

    mCallbackEvent = callback;
	mCallbackContext = userContext;

    NOEXCEPT_REGION_END
}


//--------------------------------------------------------------------------------------
void GLUFDialog::RemoveControl(GLUFControlIndex ID)
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
void GLUFDialog::RemoveAllControls()
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
void GLUFDialog::Refresh()
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
void GLUFDialog::OnRender(float elapsedTime)
{
	// If this assert triggers, you need to call GLUFDialogResourceManager::On*Device() from inside
	// the application's device callbacks.  See the SDK samples for an example of how to do this.
	//GLUF_ASSERT(m_pManager->GetD3D11Device() &&
	//	L"To fix hook up GLUFDialogResourceManager to device callbacks.  See comments for details");
	//no need for "devices", this is all handled by GLFW

    NOEXCEPT_REGION_START

	// See if the dialog needs to be refreshed
	if (mTimePrevRefresh < sTimeRefresh)
	{
        mTimePrevRefresh = GLUFGetTime();
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
		GLUFRect windowCoords = { 0, GetHeight(), GetWidth(), 0 };
		//windowCoords = GLUFScreenToClipspace(windowCoords);

		DrawSprite(mDlgElement, windowCoords, -0.99f, false);
	}

	// Sort depth back to front
	BeginText(mDialogManager->GetOrthoMatrix());


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

		mCapElement->mTextureColor.SetCurrent(GLUF_STATE_NORMAL);
		mCapElement->mFontColor.SetCurrent(GLUF_STATE_NORMAL);
		GLUFRect rc = { 0, 0, GetWidth(), -mCaptionHeight };

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
void GLUFDialog::SendEvent(GLUFEvent ctrlEvent, bool triggeredByUser, GLUFControlPtr control)
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
void GLUFDialog::SetFont(GLUFFontIndex index, GLUFFontIndex resManFontIndex)
{
	// If this assert triggers, you need to call GLUFDialog::Init() first.  This change
	// was made so that the GLUF's GUI could become separate and optional from GLUF's core.  The 
	// creation and interfacing with GLUFDialogResourceManager is now the responsibility 
	// of the application if it wishes to use GLUF's GUI.
	GLUF_ASSERT(mDialogManager && L"To fix call GLUFDialog::Init() first.  See comments for details.");
	//_Analysis_assume_(m_pManager);


    //call this to trigger an exception if the font index does not exist
    mDialogManager->GetFontNode(resManFontIndex);

	mFonts[index] = resManFontIndex;
}


//--------------------------------------------------------------------------------------
GLUFFontNodePtr GLUFDialog::GetFont(GLUFFontIndex index) const
{
	if (!mDialogManager)
		return nullptr;
	return mDialogManager->GetFontNode(mFonts.at(index));
}


//--------------------------------------------------------------------------------------
void GLUFDialog::SetTexture(GLUFTextureIndex index, GLUFTextureIndex resManTexIndex)
{
	// If this assert triggers, you need to call GLUFDialog::Init() first.  This change
	// was made so that the GLUF's GUI could become separate and optional from GLUF's core.  The 
	// creation and interfacing with GLUFDialogResourceManager is now the responsibility 
	// of the application if it wishes to use GLUF's GUI.
	GLUF_ASSERT(mDialogManager && L"To fix this, call GLUFDialog::Init() first.  See comments for details.");
	//_Analysis_assume_(m_pManager);
    
    //call this to trigger an exception if the texture index does not exist
    mDialogManager->GetTextureNode(resManTexIndex);

    mTextures[index] = resManTexIndex;
}

//--------------------------------------------------------------------------------------
GLUFTextureNodePtr GLUFDialog::GetTexture(GLUFTextureIndex index) const
{
	if (!mDialogManager)
		return nullptr;
	return mDialogManager->GetTextureNode(mTextures.at(index));
}

//--------------------------------------------------------------------------------------
bool GLUFDialog::MsgProc(GLUFMessageType msg, int32_t param1, int32_t param2, int32_t param3, int32_t param4)
{
    NOEXCEPT_REGION_START

	if (mFirstTime)
        mFirstTime = false;
	else
		mMousePositionOld = mMousePosition;

	mDialogManager->MsgProc(GLUF_PASS_CALLBACK_PARAM);


	//first, even if we are not going to use it, snatch up the cursor position just in case it moves in the time it takes to do this
	double x, y;
	glfwGetCursorPos(g_pGLFWWindow, &x, &y);
	mMousePosition = GLUFPoint(static_cast<long>(x), g_WndHeight - static_cast<long>(y));


    //this gets broken when window is too big
	mMousePositionDialogSpace.x = mMousePosition.x - mRegion.x;
	mMousePositionDialogSpace.y = mMousePosition.y - mRegion.y - mCaptionHeight;//TODO: fix

	//if (m_bCaption)
	//	m_MousePositionDialogSpace.y -= m_nCaptionHeight;

	bool bHandled = false;

	// For invisible dialog, do not handle anything.
	if (!mVisible)
		return false;

    if (!mKeyboardInput && (msg == GM_KEY || msg == GM_UNICODE_CHAR))
		return false;

	// If caption is enable, check for clicks in the caption area.
	if (mCaptionEnabled && !mLocked)
	{
		static GLUFPoint totalDelta;

		if (((msg == GM_MB) == true) &&
			((param1 == GLFW_MOUSE_BUTTON_LEFT) == true) &&
			((param2 == GLFW_PRESS) == true) )
		{

			if (mMousePositionDialogSpace.x >= 0 && mMousePositionDialogSpace.x < GLUFRectWidth(mRegion) &&
				mMousePositionDialogSpace.y >= -mCaptionHeight && mMousePositionDialogSpace.y < 0)
			{
				mDrag = true;
				mDragged = false;
				//SetCapture(GLUFGetHWND());
				return true;
			}
			else if (!mMinimized && mGrabAnywhere && !GetControlAtPoint(mMousePositionDialogSpace))
			{
				//ONLY allow this if it is not on top of a control
				mDrag = true;
				return true;
			}
		}
		else if ((msg == GM_MB) == true &&
				(param1 == GLFW_MOUSE_BUTTON_LEFT) == true &&
				(param2 == GLFW_RELEASE) == true && 
				(mDrag))
		{
			if (mMousePositionDialogSpace.x >= 0 && mMousePositionDialogSpace.x < GLUFRectWidth(mRegion) &&
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
		else if ((msg == GM_CURSOR_POS))
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


					GLUFPoint delta = mMousePosition - mMousePositionOld;
					totalDelta = { totalDelta.x + delta.x, totalDelta.y + delta.y };
                    
					GLUFRepositionRect(mRegion, 
                        std::clamp(delta.x + mRegion.x, 0L, static_cast<long>(g_WndWidth) - GLUFRectWidth(mRegion)),
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
	if (mAutoClamp && msg == GM_RESIZE)
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
	case GM_RESIZE:
	case GM_POS:
	{
		// Handle sizing and moving messages so that in case the mouse cursor is moved out
		// of an UI control because of the window adjustment, we can properly
		// unhighlight the highlighted control.
		GLUFPoint pt =
		{
			-1, -1
		};
		OnMouseMove(pt);
		break;
	}

	case GM_FOCUS:
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
	case GM_KEY:
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
			(sControlFocus->GetType() != GLUF_CONTROL_EDITBOX
			&& sControlFocus->GetType() != GLUF_CONTROL_IMEEDITBOX)))
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
	case GM_MB:
	case GM_SCROLL:
	case GM_CURSOR_POS:
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
		GLUFControlPtr pControl = GetControlAtPoint(mMousePositionDialogSpace);
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
		case GM_CURSOR_POS:
			OnMouseMove(mMousePositionDialogSpace);
			return false;
		}

		break;
	}

	case GM_CURSOR_ENTER:
	{
		// The application has lost mouse capture.
		// The dialog object may not have received
		// a WM_MOUSEUP when capture changed. Reset
		// m_bDrag so that the dialog does not mistakenly
		// think the mouse button is still held down.
		if (param1 == GL_FALSE)
			mDrag = false;
	}
	}

    NOEXCEPT_REGION_END

	return false;
}


//--------------------------------------------------------------------------------------
void GLUFDialog::ClampToScreen()
{
    NOEXCEPT_REGION_START

	mRegion.x = std::clamp(mRegion.x, 0L, static_cast<long>(g_WndWidth) - GLUFRectWidth(mRegion));
	mRegion.y = std::clamp(mRegion.y, 0L, static_cast<long>(g_WndHeight) - mCaptionHeight);

    NOEXCEPT_REGION_END
}

//--------------------------------------------------------------------------------------
GLUFControlPtr GLUFDialog::GetControlAtPoint(const GLUF::GLUFPoint& pt) const
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

    GLUF_NON_CRITICAL_EXCEPTION(std::invalid_argument("No Control Found At Point"));

    return nullptr;
}


//--------------------------------------------------------------------------------------
bool GLUFDialog::GetControlEnabled(GLUFControlIndex ID) const
{
	GLUFControlPtr pControl = GetControl<GLUFControl>(ID);
	if (!pControl)
		return false;

	return pControl->GetEnabled();
}



//--------------------------------------------------------------------------------------
void GLUFDialog::SetControlEnabled(GLUFControlIndex ID, bool bEnabled)
{
	GLUFControlPtr pControl = GetControl<GLUFControl>(ID);
	if (!pControl)
		return;

	pControl->SetEnabled(bEnabled);
}


//--------------------------------------------------------------------------------------
void GLUFDialog::OnMouseUp(const GLUF::GLUFPoint& pt)
{
    //TODO: do something here?
	GLUF_UNREFERENCED_PARAMETER(pt);
	sControlPressed = nullptr;
	mControlMouseOver = nullptr;
}


//--------------------------------------------------------------------------------------
void GLUFDialog::OnMouseMove(const GLUF::GLUFPoint& pt)
{
    NOEXCEPT_REGION_START

	// Figure out which control the mouse is over now
	GLUFControlPtr pControl = GetControlAtPoint(pt);

	// If the mouse is still over the same control, nothing needs to be done
	if (pControl == mControlMouseOver)
		return;

	// Handle mouse leaving the old control
	if (mControlMouseOver)
		mControlMouseOver->OnMouseLeave();

	// Handle mouse entering the new control
	mControlMouseOver = pControl;
	if (pControl)
		mControlMouseOver->OnMouseEnter();

    NOEXCEPT_REGION_END
}


//--------------------------------------------------------------------------------------
void GLUFDialog::SetDefaultElement(GLUFControlType controlType, GLUFElementIndex elementIndex, const GLUFElementPtr& element)
{
    if (!element)
        throw std::invalid_argument("Nullptr GLUFElement");

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
    GLUFElementHolderPtr pNewHolder = std::make_shared<GLUFElementHolder>();

	pNewHolder->mControlType = controlType;
	pNewHolder->mElementIndex = elementIndex;
	pNewHolder->mElement = element;

	mDefaultElements.push_back(pNewHolder);
}


//--------------------------------------------------------------------------------------
GLUFElementPtr GLUFDialog::GetDefaultElement(GLUFControlType controlType, GLUFElementIndex elementIndex) const
{
    for (auto it : mDefaultElements)
    {
        if (it->mControlType == controlType && it->mElementIndex == elementIndex)
        {
            return it->mElement;
        }
    }

    GLUF_NON_CRITICAL_EXCEPTION(std::invalid_argument("GetDefaultElement: elementIndex could not be found within controlType"));

    return nullptr;
}

//--------------------------------------------------------------------------------------
void GLUFDialog::AddStatic(GLUFControlIndex ID, const std::wstring& strText, const GLUFRect& region, GLUFBitfield textFlags, bool isDefault, std::shared_ptr<GLUFStaticPtr> ctrlPtr)
{
    auto pStatic = std::make_shared<GLUFStatic>(textFlags, *this);

    if (ctrlPtr)
        *ctrlPtr = pStatic;

	AddControl(std::dynamic_pointer_cast<GLUFControl>(pStatic));

	// Set the ID and list index
	pStatic->SetID(ID);
	pStatic->SetText(strText);
    pStatic->SetRegion(region);
	pStatic->mIsDefault = isDefault;
}


//--------------------------------------------------------------------------------------
void GLUFDialog::AddButton(GLUFControlIndex ID, const std::wstring& strText, const GLUFRect& region, int hotkey, bool isDefault, std::shared_ptr<GLUFButtonPtr> ctrlPtr)
{
    auto pButton = std::make_shared<GLUFButton>(*this);

    if (ctrlPtr)
        *ctrlPtr = pButton;

    AddControl(std::dynamic_pointer_cast<GLUFControl>(pButton));

	// Set the ID and list index
	pButton->SetID(ID);
	pButton->SetText(strText);
    pButton->SetRegion(region);
	pButton->SetHotkey(hotkey);
	pButton->mIsDefault = isDefault;
}


//--------------------------------------------------------------------------------------
void GLUFDialog::AddCheckBox(GLUFControlIndex ID, const std::wstring& strText, const GLUFRect& region, bool checked , int hotkey, bool isDefault, std::shared_ptr<GLUFCheckBoxPtr> ctrlPtr)
{
    auto pCheckBox = std::make_shared<GLUFCheckBox>(*this);

    if (ctrlPtr)
        *ctrlPtr = pCheckBox;

    AddControl(std::dynamic_pointer_cast<GLUFControl>(pCheckBox));

	// Set the ID and list index
	pCheckBox->SetID(ID);
	pCheckBox->SetText(strText);
    pCheckBox->SetRegion(region);
	pCheckBox->SetHotkey(hotkey);
	pCheckBox->mIsDefault = isDefault;
	pCheckBox->SetChecked(checked);
}


//--------------------------------------------------------------------------------------
void GLUFDialog::AddRadioButton(GLUFControlIndex ID, GLUFRadioButtonGroup buttonGroup, const std::wstring& strText, const GLUFRect& region, bool checked, int hotkey, bool isDefault, std::shared_ptr<GLUFRadioButtonPtr> ctrlPtr)
{
    auto pRadioButton = std::make_shared<GLUFRadioButton>(*this);

    if (ctrlPtr)
        *ctrlPtr = pRadioButton;

    AddControl(std::dynamic_pointer_cast<GLUFControl>(pRadioButton));

	// Set the ID and list index
	pRadioButton->SetID(ID);
	pRadioButton->SetText(strText);
	pRadioButton->SetButtonGroup(buttonGroup);
    pRadioButton->SetRegion(region);
	pRadioButton->SetHotkey(hotkey);
	pRadioButton->SetChecked(checked);
	pRadioButton->mIsDefault = isDefault;
	pRadioButton->SetChecked(checked);
}


//--------------------------------------------------------------------------------------
void GLUFDialog::AddComboBox(GLUFControlIndex ID, const GLUFRect& region, int hotKey, bool isDefault, std::shared_ptr<GLUFComboBoxPtr> ctrlPtr)
{
    auto pComboBox = std::make_shared<GLUFComboBox>(*this);

    if (ctrlPtr)
        *ctrlPtr = pComboBox;

    AddControl(std::dynamic_pointer_cast<GLUFControl>(pComboBox));

	// Set the ID and list index
	pComboBox->SetID(ID);
    pComboBox->SetRegion(region);
	pComboBox->SetHotkey(hotKey);
	pComboBox->mIsDefault = isDefault;
}


//--------------------------------------------------------------------------------------
void GLUFDialog::AddSlider(GLUFControlIndex ID, const GLUFRect& region, long min, long max, long value, bool isDefault, std::shared_ptr<GLUFSliderPtr> ctrlPtr)
{
    auto pSlider = std::make_shared<GLUFSlider>(*this);

    if (ctrlPtr)
        *ctrlPtr = pSlider;

    AddControl(std::dynamic_pointer_cast<GLUFControl>(pSlider));

	// Set the ID and list index
	pSlider->SetID(ID);
    pSlider->SetRegion(region);
	pSlider->mIsDefault = isDefault;
	pSlider->SetRange(min, max);
	pSlider->SetValue(value);
	pSlider->UpdateRects();
}


//--------------------------------------------------------------------------------------
/*void GLUFDialog::AddEditBox(GLUFControlIndex ID, const std::wstring& strText, const GLUFRect& region, GLUFCharset charset = Unicode, GLbitfield textFlags = GT_LEFT | GT_TOP, bool isDefault = false, std::shared_ptr<GLUFEditBoxPtr> ctrlPtr = nullptr)
{
	auto pEditBox = std::make_shared<GLUFEditBox>(charset, (textFlags & GT_MULTI_LINE) == GT_MULTI_LINE, this);

	if (ctrlPtr)
		*ctrlPtr = pEditBox;

	AddControl(std::dynamic_pointer_cast<GLUFControl>(pEditBox));

	pEditBox->GetElement(0)->dwTextFormat = textFlags;

	// Set the ID and position
	pEditBox->SetID(ID);
    pEditBox->SetRegion(region);
	pEditBox->mIsDefault = isDefault;

	pEditBox->SetText(strText);
}*/


//--------------------------------------------------------------------------------------
void GLUFDialog::AddListBox(GLUFControlIndex ID, const GLUFRect& region, GLUFBitfield style, std::shared_ptr<GLUFListBoxPtr> ctrlPtr)
{
	auto pListBox = std::make_shared<GLUFListBox>(*this);

	if (ctrlPtr)
		*ctrlPtr = pListBox;

    AddControl(std::dynamic_pointer_cast<GLUFControl>(pListBox));

	// Set the ID and position
	pListBox->SetID(ID);
    pListBox->SetRegion(region);
	pListBox->SetStyle(style);
}


//--------------------------------------------------------------------------------------
void GLUFDialog::AddControl(GLUFControlPtr& pControl)
{
	InitControl(pControl);

    if (!pControl)
        return;

	// Add to the list
	mControls[pControl->mID] = pControl;
}


//--------------------------------------------------------------------------------------
void GLUFDialog::InitControl(GLUFControlPtr& pControl)
{
	//GLUFResult hr;

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
GLUFControlPtr GLUFDialog::GetControl(GLUFControlIndex ID, GLUFControlType controlType) const
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
GLUFControlPtr GLUFDialog::GetNextControl(GLUFControlPtr control)
{
    GLUFDialog& dialog = control->mDialog;


    auto indexIt = dialog.mControls.find(control->mID);

    //get the 'next' one
    ++indexIt;

    //is this still a valid control?
    if (indexIt != dialog.mControls.end())
        return indexIt->second;//yes

    //if not, get the next dialog
    GLUFDialogPtr nextDlg = dialog.mNextDialog;

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
GLUFControlPtr GLUFDialog::GetPrevControl(GLUFControlPtr control)
{
    GLUFDialog& dialog = control->mDialog;

    auto indexIt = dialog.mControls.find(control->mID);

    //get the 'previous' one
    --indexIt;

    //is this still a valid control?
    if (indexIt != dialog.mControls.end())
        return indexIt->second;

    //if not get the previous dialog
    GLUFDialogPtr prevDlg = dialog.mPrevDialog;

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

/*


Ended Here July 25 2015




*/



//--------------------------------------------------------------------------------------
void GLUFDialog::ClearRadioButtonGroup(GLUFRadioButtonGroup buttonGroup)
{
	// Find all radio buttons with the given group number
	for (auto it : mControls)
	{
		if (it.second->GetType() == GLUF_CONTROL_RADIOBUTTON)
		{
			GLUFRadioButtonPtr radioButton = std::dynamic_pointer_cast<GLUFRadioButton>(it.second);

            if (radioButton->GetButtonGroup() == buttonGroup)
                radioButton->SetChecked(false, false);
		}
	}
}


//--------------------------------------------------------------------------------------
void GLUFDialog::ClearComboBox(GLUFControlIndex ID)
{
	GLUFComboBoxPtr comboBox = GetControl<GLUFComboBox>(ID);
	if (!comboBox)
		return;

	comboBox->RemoveAllItems();
}


//--------------------------------------------------------------------------------------
void GLUFDialog::RequestFocus(GLUFControlPtr& control)
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
void GLUFDialog::DrawRect(const GLUFRect& rect, const Color& color)
{
	GLUFRect rcScreen = rect;
	GLUFOffsetRect(rcScreen, mRegion.x - long(g_WndWidth / 2), mCaptionHeight + mRegion.y - long(g_WndHeight / 2));

	//if (m_bCaption)
	//	GLUFOffsetRect(rcScreen, 0, m_nCaptionHeight);

	//rcScreen = GLUFScreenToClipspace(rcScreen);

    auto thisSprite = GLUFSpriteVertexStruct::MakeMany(4);
    thisSprite[0] = 
    {
        glm::vec3(rcScreen.left, rcScreen.top, GLUF_NEAR_BUTTON_DEPTH),
        GLUFColorToFloat(color),
        glm::vec2() 
    };

    thisSprite[1] =
    {
        glm::vec3(rcScreen.right, rcScreen.top, GLUF_NEAR_BUTTON_DEPTH),
        GLUFColorToFloat(color),
        glm::vec2()
    };

    thisSprite[2] =
    {
        glm::vec3(rcScreen.left, rcScreen.bottom, GLUF_NEAR_BUTTON_DEPTH),
        GLUFColorToFloat(color),
        glm::vec2()
    };

    thisSprite[3] =
    {
        glm::vec3(rcScreen.right, rcScreen.bottom, GLUF_NEAR_BUTTON_DEPTH),
        GLUFColorToFloat(color),
        glm::vec2()
    };

    mDialogManager->mSpriteBuffer.BufferData(thisSprite);

	// Why are we drawing the sprite every time?  This is very inefficient, but the sprite workaround doesn't have support for sorting now, so we have to
	// draw a sprite every time to keep the order correct between sprites and text.
    mDialogManager->EndSprites(nullptr, false);//render in untextured mode
}


//--------------------------------------------------------------------------------------

void GLUFDialog::DrawSprite(const GLUFElementPtr& element, const GLUF::GLUFRect& rect, float depth, bool textured)
{
	// No need to draw fully transparent layers
    if (element->mTextureColor.mCurrentColor.a == 0)
        return;

    if (element->mTextureColor.mCurrentColor == element->mTextureColor.mStates[GLUF_STATE_HIDDEN])
		return;


    GLUFRectf uvRect = element->mUVRect;

	GLUFRect rcScreen = rect;

	GLUFOffsetRect(rcScreen, mRegion.x - long(g_WndWidth / 2), mCaptionHeight + mRegion.y - long(g_WndHeight / 2));
    
	/*GLUFTextureNodePtr textureNode = GetTexture(pElement->iTexture);
    if (!textureNode)
		return;*/

    auto thisSprite = GLUFSpriteVertexStruct::MakeMany(4);

    thisSprite[0] =
    {
        glm::vec3(rcScreen.left, rcScreen.top, depth),
        GLUFColorToFloat(element->mTextureColor.mCurrentColor),
        glm::vec2(uvRect.left, uvRect.top)
    };

    thisSprite[1] =
    {
        glm::vec3(rcScreen.right, rcScreen.top, depth),
        GLUFColorToFloat(element->mTextureColor.mCurrentColor),
        glm::vec2(uvRect.right, uvRect.top)
    };

    thisSprite[2] =
    {
        glm::vec3(rcScreen.left, rcScreen.bottom, depth),
        GLUFColorToFloat(element->mTextureColor.mCurrentColor),
        glm::vec2(uvRect.left, uvRect.bottom)
    };

    thisSprite[3] =
    {
        glm::vec3(rcScreen.right, rcScreen.bottom, depth),
        GLUFColorToFloat(element->mTextureColor.mCurrentColor),
        glm::vec2(uvRect.right, uvRect.bottom)
    };

	// Why are we drawing the sprite every time?  This is very inefficient, but the sprite workaround doesn't have support for sorting now, so we have to
	// draw a sprite every time to keep the order correct between sprites and text.
	mDialogManager->EndSprites(element, textured);
}


//--------------------------------------------------------------------------------------
void GLUFDialog::DrawText(const std::wstring& text, const GLUFElementPtr& element, const GLUF::GLUFRect& rect, bool shadow, bool hardRect)
{
	// No need to draw fully transparent layers
    if (element->mFontColor.mCurrentColor.a == 0)
		return;

	GLUFRect screen = rect;
    GLUFOffsetRect(screen, mRegion.x, mRegion.y);


	GLUFOffsetRect(screen, 0, mCaptionHeight);

	/*if (bShadow)
	{
		GLUFRect rcShadow = rcScreen;
		GLUFOffsetRect(rcShadow, 1 / m_pManager->GetWindowSize().x, 1 / m_pManager->GetWindowSize().y);

		Color vShadowColor(0, 0, 0, 255);
		DrawTextGLUF(*m_pManager->GetFontNode(pElement->iFont), strText, rcShadow, vShadowColor, bCenter, bHardRect);

	}*/

    Color vFontColor = element->mFontColor.mCurrentColor;
    DrawTextGLUF(mDialogManager->GetFontNode(element->mFontIndex), text, screen, element->mFontColor.mCurrentColor, element->mTextFormatFlags, hardRect);
}


//--------------------------------------------------------------------------------------
void GLUFDialog::CalcTextRect(const std::wstring& text, const GLUFElementPtr& element, GLUF::GLUFRect& rect) const
{
	GLUFFontNodePtr pFontNode = GetFont(pElement->iFont);
	if (!pFontNode)
		return;

    GLUF_UNREFERENCED_PARAMETER(text);
    GLUF_UNREFERENCED_PARAMETER(element);
    GLUF_UNREFERENCED_PARAMETER(rect);
	// TODO -

}

//--------------------------------------------------------------------------------------
void GLUFDialog::SetNextDialog(GLUFDialogPtr nextDialog)
{
    if (!nextDialog)
        mNextDialog = shared_from_this();
    else
    {
        mNextDialog = nextDialog;
        nextDialog->mPrevDialog = shared_from_this();
    }
}


//--------------------------------------------------------------------------------------
void GLUFDialog::ClearFocus()
{
	if (sControlFocus)
	{
		sControlFocus->OnFocusOut();
		sControlFocus = nullptr;
	}
}


//--------------------------------------------------------------------------------------
void GLUFDialog::FocusDefaultControl()
{
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
}


//--------------------------------------------------------------------------------------
bool GLUFDialog::OnCycleFocus(bool forward)
{
	GLUFControlPtr pControl = nullptr;
	GLUFDialogPtr pDialog = nullptr; // pDialog and pLastDialog are used to track wrapping of
	GLUFDialogPtr pLastDialog;    // focus from first control to last or vice versa.

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
		GLUF_ASSERT(pControl != 0);

        //this is safe to assume that the dialog is 'this' because of the line 'else if (&sControlFocus->mDialog != this)'
        pLastDialog = shared_from_this();

		pControl = (forward) ? GetNextControl(sControlFocus) : GetPrevControl(sControlFocus);

        //this is kind of inefficient
        if (!(pDialog = mDialogManager->GetDialogPtrFromRef(pControl->mDialog)))
            pDialog = shared_from_this();//not sure if this is what to do if the dialog is not found, but its the best thing I could think of
	}

	GLUF_ASSERT(pControl != 0);

	// If we just wrapped from last control to first or vice versa,
	// set the focused control to nullptr. This state, where no control
	// has focus, allows the camera to work.
	int nLastDialogIndex = -1;
    for (int i = 0; i < mDialogManager->mDialogs.size(); ++i)
    {
        if (mDialogManager->mDialogs[i] == pLastDialog)
        {
            nLastDialogIndex = i;
            break;
        }
    }

    int nDialogIndex = -1;
    for (int i = 0; i < mDialogManager->mDialogs.size(); ++i)
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
	GLUF_ERROR("GLUFDialog: Multiple dialogs are improperly chained together");
	return false;
}

GLUFFontPtr g_ArialDefault = nullptr;
//--------------------------------------------------------------------------------------
void GLUFDialog::InitDefaultElements()
{
	//this makes it more efficient
	int fontIndex = 0;
	if (g_DefaultFont == nullptr)
	{
		if (g_ArialDefault == nullptr)
		{

            std::vector<char> rawData;
            GLUFLoadFileIntoMemory(L"Arial.ttf", rawData);
			GLUFLoadFont(g_ArialDefault, rawData, 15L);
		}

		fontIndex = mDialogManager->AddFont(g_ArialDefault, 1.15f, FONT_WEIGHT_NORMAL);
	}
	else
	{
        fontIndex = mDialogManager->AddFont(g_DefaultFont, 1.15f, FONT_WEIGHT_NORMAL);
	}

	SetFont(0, fontIndex);

	GLUFElement Element;
	GLUFRectf rcTexture;

	//-------------------------------------
	// Element for the caption
	//-------------------------------------
	mCapElement->SetFont(0);
	GLUFSetRect(rcTexture, 0.0f, 0.078125f, 0.4296875f, 0.0f);//blank part of the texture
    mCapElement->SetTexture(0, rcTexture);
    mCapElement->mTextureColor.Init(Color(255, 255, 255, 255));
    mCapElement->mFontColor.Init(Color(0, 0, 0, 255));
    mCapElement->SetFont(0, Color(0, 0, 0, 255), GT_LEFT | GT_VCENTER);
	// Pre-blend as we don't need to transition the state
    mCapElement->mTextureColor.Blend(GLUF_STATE_NORMAL, 10.0f);
    mCapElement->mFontColor.Blend(GLUF_STATE_NORMAL, 10.0f);

	mDlgElement->SetFont(0);
	GLUFSetRect(rcTexture, 0.0f, 0.078125f, 0.4296875f, 0.0f);//blank part of the texture
	//GLUFSetRect(rcTexture, 0.0f, 1.0f, 1.0f, 0.0f);//blank part of the texture
	mDlgElement->SetTexture(0, rcTexture);
	mDlgElement->mTextureColor.Init(Color(255, 0, 0, 128));
	mDlgElement->mFontColor.Init(Color(0, 0, 0, 255));
	mDlgElement->SetFont(0, Color(0, 0, 0, 255), GT_LEFT | GT_VCENTER);
	// Pre-blend as we don't need to transition the state
	mDlgElement->mTextureColor.Blend(GLUF_STATE_NORMAL, 10.0f);
	mDlgElement->mFontColor.Blend(GLUF_STATE_NORMAL, 10.0f);


	//Element.mFontColor.mStates[GLUF_STATE_NORMAL]		= Color(0, 0, 0, 255);
	//Element.mFontColor.mStates[GLUF_STATE_DISABLED]	= Color(0, 0, 0, 255);
	//Element.mFontColor.mStates[GLUF_STATE_HIDDEN]		= Color(0, 0, 0, 255);
	//Element.mFontColor.mStates[GLUF_STATE_FOCUS]		= Color(0, 0, 0, 255);
	//Element.mFontColor.mStates[GLUF_STATE_MOUSEOVER]	= Color(0, 0, 0, 255);
	//Element.mFontColor.mStates[GLUF_STATE_PRESSED]	= Color(0, 0, 0, 255);

	//-------------------------------------
	// GLUFStatic
	//-------------------------------------
	Element.SetFont(0);
	Element.mTextFormatFlags = GT_LEFT | GT_VCENTER;
	Element.mFontColor.mStates[GLUF_STATE_DISABLED] = Color(200, 200, 200, 200);

	// Assign the Element
	SetDefaultElement(GLUF_CONTROL_STATIC, 0, std::make_shared<GLUFElement>(Element));


	//-------------------------------------
	// GLUFButton - Button
	//-------------------------------------
	GLUFSetRect(rcTexture, 0.0f, 1.0f, 0.53125f, 0.7890625f);
	//GLUFSetRect(rcTexture, 0.53125f, 1.0f, 0.984375f, 0.7890625f);
	Element.SetTexture(0, rcTexture);
	Element.SetFont(0);
	Element.mTextureColor.mStates[GLUF_STATE_NORMAL] = Color(255, 255, 255, 0);
	Element.mTextureColor.mStates[GLUF_STATE_PRESSED] = Color(255, 255, 255, 30);
	Element.mFontColor.mStates[GLUF_STATE_MOUSEOVER] = Color(0, 0, 0, 255);
	Element.mFontColor.mStates[GLUF_STATE_NORMAL] = Color(0, 0, 0, 255);

	// Assign the Element
	SetDefaultElement(GLUF_CONTROL_BUTTON, 0, std::make_shared<GLUFElement>(Element));


	//-------------------------------------
	// GLUFButton - Fill layer
	//-------------------------------------
	GLUFSetRect(rcTexture, 0.53125f, 1.0f, 0.984375f, 0.7890625f);
	Element.SetTexture(0, rcTexture, Color(255, 255, 255, 0));
	Element.mTextureColor.mStates[GLUF_STATE_MOUSEOVER] = Color(200, 200, 200, 10);
	Element.mTextureColor.mStates[GLUF_STATE_PRESSED] = Color(0, 0, 0, 8);
	Element.mTextureColor.mStates[GLUF_STATE_FOCUS] = Color(255, 255, 255, 10);


	// Assign the Element
	SetDefaultElement(GLUF_CONTROL_BUTTON, 1, std::make_shared<GLUFElement>(Element));


	//-------------------------------------
	// GLUFCheckBox - Box
	//-------------------------------------
	GLUFSetRect(rcTexture, 0.0f, 0.7890625f, 0.10546875f, 0.68359375f);
	Element.SetTexture(0, rcTexture);
	Element.SetFont(0, Color(0, 0, 0, 255), GT_LEFT | GT_VCENTER);
	Element.mFontColor.mStates[GLUF_STATE_DISABLED] = Color(80, 80, 80, 100);
	Element.mTextureColor.mStates[GLUF_STATE_NORMAL] = Color(255, 255, 255, 20);
	Element.mTextureColor.mStates[GLUF_STATE_FOCUS] = Color(255, 255, 255, 30);
	Element.mTextureColor.mStates[GLUF_STATE_PRESSED] = Color(255, 255, 255, 127);

	// Assign the Element
	SetDefaultElement(GLUF_CONTROL_CHECKBOX, 0, std::make_shared<GLUFElement>(Element));


	//-------------------------------------
	// GLUFCheckBox - Check
	//-------------------------------------
	GLUFSetRect(rcTexture, 0.10546875f, 0.7890625f, 0.2109375f, 0.68359375f);
	Element.SetTexture(0, rcTexture, Color(255, 255, 255, 0));

	// Assign the Element
	SetDefaultElement(GLUF_CONTROL_CHECKBOX, 1, std::make_shared<GLUFElement>(Element));


	//-------------------------------------
	// GLUFRadioButton - Box
	//-------------------------------------
	GLUFSetRect(rcTexture, 0.2109375f, 0.7890625f, 0.31640625f, 0.68359375f);
	Element.SetTexture(0, rcTexture);
	Element.SetFont(0, Color(0, 0, 0, 255), GT_LEFT | GT_VCENTER);
	Element.mFontColor.mStates[GLUF_STATE_DISABLED] = Color(0, 0, 0, 255);
	Element.mTextureColor.mStates[GLUF_STATE_NORMAL] = Color(255, 255, 255, 75);
	Element.mTextureColor.mStates[GLUF_STATE_FOCUS] = Color(255, 255, 255, 100);
	Element.mTextureColor.mStates[GLUF_STATE_PRESSED] = Color(255, 255, 255, 127);

	// Assign the Element
	SetDefaultElement(GLUF_CONTROL_RADIOBUTTON, 0, std::make_shared<GLUFElement>(Element));


	//-------------------------------------
	// GLUFRadioButton - Check
	//-------------------------------------
	GLUFSetRect(rcTexture, 0.31640625f, 0.7890625f, 0.421875f, 0.68359375f);
	Element.SetTexture(0, rcTexture, Color(255, 255, 255, 0));
	//Element.mTextureColor.mStates[GLUF_STATE_HIDDEN] = Color(255, 255, 255, 255);

	// Assign the Element
	SetDefaultElement(GLUF_CONTROL_RADIOBUTTON, 1, std::make_shared<GLUFElement>(Element));


	//-------------------------------------
	// GLUFComboBox - Main
	//-------------------------------------
	GLUFSetRect(rcTexture, 0.02734375f, 0.5234375f, 0.96484375f, 0.3671875f);
	Element.SetTexture(0, rcTexture);
	Element.SetFont(0, Color(0, 0, 0, 255), GT_LEFT | GT_VCENTER);
	Element.mTextureColor.mStates[GLUF_STATE_NORMAL] = Color(200, 200, 200, 150);
	Element.mTextureColor.mStates[GLUF_STATE_FOCUS] = Color(230, 230, 230, 170);
	Element.mTextureColor.mStates[GLUF_STATE_DISABLED] = Color(200, 200, 200, 70);
	Element.mFontColor.mStates[GLUF_STATE_MOUSEOVER] = Color(0, 0, 0, 255);
	Element.mFontColor.mStates[GLUF_STATE_PRESSED] = Color(0, 0, 0, 255);
	Element.mFontColor.mStates[GLUF_STATE_DISABLED] = Color(200, 200, 200, 200);


	// Assign the Element
	SetDefaultElement(GLUF_CONTROL_COMBOBOX, 0, std::make_shared<GLUFElement>(Element));


	//-------------------------------------
	// GLUFComboBox - Button
	//-------------------------------------
	GLUFSetRect(rcTexture, 0.3828125f, 0.26171875f, 0.58984375f, 0.0703125f);
	Element.SetTexture(0, rcTexture);
	Element.mTextureColor.mStates[GLUF_STATE_NORMAL] = Color(255, 255, 255, 0);
	Element.mTextureColor.mStates[GLUF_STATE_MOUSEOVER] = Color(255, 255, 255, 50);
	Element.mTextureColor.mStates[GLUF_STATE_PRESSED] = Color(100, 100, 100, 100);
	Element.mTextureColor.mStates[GLUF_STATE_FOCUS] = Color(255, 255, 255, 20);
	Element.mTextureColor.mStates[GLUF_STATE_DISABLED] = Color(255, 255, 255, 50);

	// Assign the Element
	SetDefaultElement(GLUF_CONTROL_COMBOBOX, 1, std::make_shared<GLUFElement>(Element));


	//-------------------------------------
	// GLUFComboBox - Dropdown
	//-------------------------------------
	GLUFSetRect(rcTexture, 0.05078125f, 0.51953125f, 0.94140625f, 0.37109375f);
	Element.SetTexture(0, rcTexture, Color(0, 0, 0, 0));
	Element.SetFont(0, Color(0, 0, 0, 255), GT_LEFT | GT_TOP);

	// Assign the Element
	SetDefaultElement(GLUF_CONTROL_COMBOBOX, 2, std::make_shared<GLUFElement>(Element));


	//-------------------------------------
	// GLUFComboBox - Selection
	//-------------------------------------
	GLUFSetRect(rcTexture, 0.046875f, 0.36328125f, 0.93359375f, 0.28515625f);
	Element.SetTexture(0, rcTexture);
	Element.SetFont(0, Color(255, 255, 255, 255), GT_LEFT | GT_TOP);

	// Assign the Element
	SetDefaultElement(GLUF_CONTROL_COMBOBOX, 3, std::make_shared<GLUFElement>(Element));


	//-------------------------------------
	// GLUFSlider - Track
	//-------------------------------------
	GLUFSetRect(rcTexture, 0.00390625f, 0.26953125f, 0.36328125f, 0.109375f);
	Element.SetTexture(0, rcTexture);
	Element.mTextureColor.mStates[GLUF_STATE_NORMAL] = Color(255, 255, 255, 75);
	Element.mTextureColor.mStates[GLUF_STATE_FOCUS] = Color(255, 255, 255, 100);
	Element.mTextureColor.mStates[GLUF_STATE_DISABLED] = Color(255, 255, 255, 35);

	// Assign the Element
	SetDefaultElement(GLUF_CONTROL_SLIDER, 0, std::make_shared<GLUFElement>(Element));

	//-------------------------------------
	// GLUFSlider - Button
	//-------------------------------------
	GLUFSetRect(rcTexture, 0.58984375f, 0.24609375f, 0.75f, 0.0859375f);
	Element.SetTexture(0, rcTexture);

	// Assign the Element
	SetDefaultElement(GLUF_CONTROL_SLIDER, 1, std::make_shared<GLUFElement>(Element));

	//-------------------------------------
	// GLUFScrollBar - Track
	//-------------------------------------
	float nScrollBarStartX = 0.76470588f;
	float nScrollBarStartY = 0.046875f;
	GLUFSetRect(rcTexture, nScrollBarStartX + 0.0f, nScrollBarStartY + 0.12890625f, nScrollBarStartX + 0.09076287f, nScrollBarStartY + 0.125f);
	Element.SetTexture(0, rcTexture);
	Element.mTextureColor.mStates[GLUF_STATE_DISABLED] = Color(200, 200, 200, 255);

	// Assign the Element
	SetDefaultElement(GLUF_CONTROL_SCROLLBAR, 0, std::make_shared<GLUFElement>(Element));

	//-------------------------------------
	// GLUFScrollBar - Down Arrow
	//-------------------------------------
	GLUFSetRect(rcTexture, nScrollBarStartX + 0.0f, nScrollBarStartY + 0.08203125f, nScrollBarStartX + 0.09076287f, nScrollBarStartY + 0.00390625f);
	Element.SetTexture(0, rcTexture);
	Element.mTextureColor.mStates[GLUF_STATE_DISABLED] = Color(200, 200, 200, 100);


	// Assign the Element
	SetDefaultElement(GLUF_CONTROL_SCROLLBAR, 2, std::make_shared<GLUFElement>(Element));

	//-------------------------------------
	// GLUFScrollBar - Up Arrow
	//-------------------------------------
	GLUFSetRect(rcTexture, nScrollBarStartX + 0.0f, nScrollBarStartY + 0.20703125f, nScrollBarStartX + 0.09076287f, nScrollBarStartY + 0.125f);
	Element.SetTexture(0, rcTexture);
	Element.mTextureColor.mStates[GLUF_STATE_DISABLED] = Color(180, 180, 180, 150);


	// Assign the Element
	SetDefaultElement(GLUF_CONTROL_SCROLLBAR, 1, std::make_shared<GLUFElement>(Element));

	//-------------------------------------
	// GLUFScrollBar - Button
	//-------------------------------------
	GLUFSetRect(rcTexture, 0.859375f, 0.25f, 0.9296875f, 0.0859375f);
	Element.SetTexture(0, rcTexture);

	// Assign the Element
	SetDefaultElement(GLUF_CONTROL_SCROLLBAR, 3, std::make_shared<GLUFElement>(Element));

	//-------------------------------------
	// GLUFEditBox
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

	Element.SetFont(0, Color(0, 0, 0, 255), GT_LEFT | GT_TOP);
	//TODO: this
	// Assign the style
	GLUFSetRect(rcTexture, 0.0546875f, 0.6484375f, 0.94140625f, 0.55859375f);
	Element.SetTexture(0, rcTexture);
	SetDefaultElement(GLUF_CONTROL_EDITBOX, 0, std::make_shared<GLUFElement>(Element));

	GLUFSetRect(rcTexture, 0.03125f, 0.6796875f, 0.0546875f, 0.6484375f);
	Element.SetTexture(0, rcTexture);
	SetDefaultElement(GLUF_CONTROL_EDITBOX, 1, std::make_shared<GLUFElement>(Element));

	GLUFSetRect(rcTexture, 0.0546875f, 0.6796875f, 0.94140625f, 0.6484375f);
	Element.SetTexture(0, rcTexture);
	SetDefaultElement(GLUF_CONTROL_EDITBOX, 2, std::make_shared<GLUFElement>(Element));

	GLUFSetRect(rcTexture, 0.94140625f, 0.6796875f, 0.9609375f, 0.6484375f);
	Element.SetTexture(0, rcTexture);
	SetDefaultElement(GLUF_CONTROL_EDITBOX, 3, std::make_shared<GLUFElement>(Element));

	GLUFSetRect(rcTexture, 0.03125f, 0.6484375f, 0.0546875f, 0.55859375f);
	Element.SetTexture(0, rcTexture);
	SetDefaultElement(GLUF_CONTROL_EDITBOX, 4, std::make_shared<GLUFElement>(Element));

	GLUFSetRect(rcTexture, 0.94140625f, 0.6484375f, 0.9609375f, 0.55859375f);
	Element.SetTexture(0, rcTexture);
	SetDefaultElement(GLUF_CONTROL_EDITBOX, 5, std::make_shared<GLUFElement>(Element));

	GLUFSetRect(rcTexture, 0.03125f, 0.55859375f, 0.0546875f, 0.52734375f);
	Element.SetTexture(0, rcTexture);
	SetDefaultElement(GLUF_CONTROL_EDITBOX, 6, std::make_shared<GLUFElement>(Element));

	GLUFSetRect(rcTexture, 0.0546875f, 0.55859375f, 0.94140625f, 0.52734375f);
	Element.SetTexture(0, rcTexture);
	SetDefaultElement(GLUF_CONTROL_EDITBOX, 7, std::make_shared<GLUFElement>(Element));

	GLUFSetRect(rcTexture, 0.94140625f, 0.55859375f, 0.9609375f, 0.52734375f);
	Element.SetTexture(0, rcTexture);
	SetDefaultElement(GLUF_CONTROL_EDITBOX, 8, std::make_shared<GLUFElement>(Element));

	//-------------------------------------
	// GLUFListBox - Main
	//-------------------------------------
	GLUFSetRect(rcTexture, 0.05078125f, 0.51953125f, 0.94140625f, 0.375f);
	Element.SetTexture(0, rcTexture);
	Element.SetFont(0, Color(0, 0, 0, 255), GT_LEFT | GT_TOP);

	// Assign the Element
	SetDefaultElement(GLUF_CONTROL_LISTBOX, 0, std::make_shared<GLUFElement>(Element));

	//-------------------------------------
	// GLUFListBox - Selection
	//-------------------------------------

	GLUFSetRect(rcTexture, 0.0625f, 0.3515625f, 0.9375f, 0.28515625f);
	Element.SetTexture(0, rcTexture);
	Element.SetFont(0, Color(255, 255, 255, 255), GT_LEFT | GT_TOP);

	// Assign the Element
	SetDefaultElement(GLUF_CONTROL_LISTBOX, 1, std::make_shared<GLUFElement>(Element));
}




/*
======================================================================================================================================================================================================
GLUFDialogResourceManager Functions


*/

//--------------------------------------------------------------------------------------
GLUFDialogResourceManager::GLUFDialogResourceManager() :
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

    mSpriteBuffer.AddVertexAttrib({ 4, 3, 0, GL_FLOAT, 0 });
    mSpriteBuffer.AddVertexAttrib({ 4, 4, 1, GL_FLOAT, 0 });
    mSpriteBuffer.AddVertexAttrib({ 4, 2, 2, GL_FLOAT, 0 });

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
GLUFDialogResourceManager::~GLUFDialogResourceManager()
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
bool GLUFDialogResourceManager::MsgProc(GLUFMessageType msg, int32_t param1, int32_t param2, int32_t param3, int32_t param4)
{
	GLUF_UNREFERENCED_PARAMETER(msg);
	//GLUF_UNREFERENCED_PARAMETER(param1);
	//GLUF_UNREFERENCED_PARAMETER(param2);
	GLUF_UNREFERENCED_PARAMETER(param3);
	GLUF_UNREFERENCED_PARAMETER(param4);

	switch (msg)
	{
	case GM_RESIZE:
		mWndSize.width = 0L;
		mWndSize.height = 0L;
		GetWindowSize();

		//refresh the fonts to the new window size
		/*for (auto it : m_FontCache)
			if (it != nullptr)
				it->m_pFontType->Refresh();*/
	}

	return false;
}

//--------------------------------------------------------------------------------------
void GLUFDialogResourceManager::ApplyRenderUI()
{
	// Shaders
	/*glEnableVertexAttribArray(g_UIShaderLocations.position);
	glEnableVertexAttribArray(g_UIShaderLocations.color);
	glEnableVertexAttribArray(g_UIShaderLocations.uv);*/
	GLUFSHADERMANAGER.UseProgram(g_UIProgram);

	ApplyOrtho();
}


//--------------------------------------------------------------------------------------
void GLUFDialogResourceManager::ApplyRenderUIUntex()
{
	/*glEnableVertexAttribArray(g_UIShaderLocationsUntex.position);
	glEnableVertexAttribArray(g_UIShaderLocationsUntex.color);*/
	GLUFSHADERMANAGER.UseProgram(g_UIProgramUntex);
	
	ApplyOrtho();
}

glm::mat4 GLUFDialogResourceManager::GetOrthoMatrix()
{
	GLUFPoint pt = GetWindowSize();
	float x2 = (float)pt.x / 2.0f;
	float y2 = (float)pt.y / 2.0f;
	return glm::ortho((float)-x2, (float)x2, (float)-y2, (float)y2);
}

GLUFDialogPtr GLUFDialogResourceManager::GetDialogPtrFromRef(const GLUFDialog& ref)
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

void GLUFDialogResourceManager::ApplyOrtho()
{
	glm::mat4 mat = GetOrthoMatrix();
	glUniformMatrix4fv(g_UIShaderLocations.ortho, 1, GL_FALSE, &mat[0][0]);
}

//--------------------------------------------------------------------------------------
void GLUFDialogResourceManager::BeginSprites()
{
}


//--------------------------------------------------------------------------------------

void GLUFDialogResourceManager::EndSprites(GLUFElementPtr element, bool textured)
{
	if (textured)
	{
        mSpriteBuffer.EnableVertexAttribute(2);
	}
	else
	{
        mSpriteBuffer.DisableVertexAttribute(2);
	}

	
	if (textured && element)
	{
		ApplyRenderUI();

		GLUFTextureNodePtr pTexture = GetTextureNode(element->mTextureIndex);

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
void GLUFDialogResourceManager::RegisterDialog(const GLUFDialogPtr& dialog)
{
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
}


//--------------------------------------------------------------------------------------
void GLUFDialogResourceManager::UnregisterDialog(const GLUFDialogPtr& pDialog)
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
void GLUFDialogResourceManager::EnableKeyboardInputForAllDialogs()
{
	// Enable keyboard input for all registered dialogs
	for (auto it : mDialogs)
		it->EnableKeyboardInput(true);
}

//--------------------------------------------------------------------------------------
GLUFPoint GLUFDialogResourceManager::GetWindowSize()
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
GLUFFontIndex GLUFDialogResourceManager::AddFont(const GLUFFontPtr& font, GLUFFontSize leading, GLUFFontWeight weight)
{
	// See if this font already exists (this is simple)
	for (size_t i = 0; i < mFontCache.size(); ++i)
	{
		GLUFFontNodePtr node = mFontCache[i];
		if (node->mFontType == font && node->mWeight == weight && node->mLeading == leading)
			return i;
	}

	// Add a new font and try to create it
    auto newFontNode = std::make_shared<GLUFFontNode>();

	//wcscpy_s(pNewFontNode->strFace, MAX_PATH, strFaceName);
    newFontNode->mFontType = font;
    newFontNode->mLeading = leading;
	//pNewFontNode->mSize = height;
    newFontNode->mWeight = weight;
    mFontCache.push_back(newFontNode);

    return mFontCache.size() - 1;
}


//--------------------------------------------------------------------------------------
GLUFTextureIndex GLUFDialogResourceManager::AddTexture(GLuint texture)
{
	// See if this texture already exists
	for (size_t i = 0; i < mTextureCache.size(); ++i)
	{
		GLUFTextureNodePtr pTextureNode = mTextureCache[i];
		if (texture == pTextureNode->mTextureElement)
			return i;
	}

	// Add a new texture and try to create it
    auto newTextureNode = std::make_shared<GLUFTextureNode>();

    newTextureNode->mTextureElement = texture;
    mTextureCache.push_back(newTextureNode);

	return mTextureCache.size() - 1;
}


/*
======================================================================================================================================================================================================
GLUFControl Functions


*/

GLUFControl::GLUFControl(GLUFDialog& dialog) : mDialog(dialog)
{
	mType = GLUF_CONTROL_BUTTON;
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
GLUFControl::~GLUFControl()
{
}


//--------------------------------------------------------------------------------------
void GLUFControl::SetTextColor(const GLUF::Color& color)
{
	GLUFElementPtr element = mElements[0];

    if (element)
        element->mFontColor.mStates[GLUF_STATE_NORMAL] = color;
}


//--------------------------------------------------------------------------------------
void GLUFControl::SetElement(GLUFElementIndex elementId, const GLUFElementPtr& element)
{
    mElements[elementId] = element;
}


//--------------------------------------------------------------------------------------
void GLUFControl::Refresh()
{
	mMouseOver = false;
	mHasFocus = false;

	for (auto it : mElements)
	{
		it.second->Refresh();
	}
}


/*
======================================================================================================================================================================================================
GLUFStatic Functions


*/

//--------------------------------------------------------------------------------------
GLUFStatic::GLUFStatic(const GLUFBitfield& textFlags, GLUFDialog& dialog) : GLUFControl(dialog), mTextFlags(textFlags)
{
	mType = GLUF_CONTROL_STATIC;
}


//--------------------------------------------------------------------------------------
void GLUFStatic::Render(float elapsedTime)
{
	if (!mVisible)
		return;

	GLUFControlState state = GLUF_STATE_NORMAL;

	if (mEnabled == false)
        state = GLUF_STATE_DISABLED;

	GLUFElementPtr element = mElements[0];
    element->mTextFormatFlags = mTextFlags;

    element->mFontColor.Blend(state, elapsedTime);

    mDialog.DrawText(mText, element, mRegion, false, false);
}


/*
======================================================================================================================================================================================================
GLUFButton Functions


*/

GLUFButton::GLUFButton(GLUFDialog& dialog) : GLUFStatic(GT_CENTER | GT_VCENTER, dialog)
{
	mType = GLUF_CONTROL_BUTTON;

	mPressed = false;
}

//--------------------------------------------------------------------------------------
void GLUFButton::OnHotkey()
{
    if (mDialog.IsKeyboardInputEnabled())
    {
        mDialog.RequestFocus(shared_from_this());
        mDialog.SendEvent(GLUF_EVENT_BUTTON_CLICKED, true, shared_from_this());
    }
}

//--------------------------------------------------------------------------------------
bool GLUFButton::MsgProc(GLUFMessageType msg, int32_t param1, int32_t param2, int32_t param3, int32_t param4)
{
	if (!mEnabled || !mVisible)
		return false;

	GLUFPoint mousePos = mDialog.GetMousePositionDialogSpace();

	switch (msg)
	{

		case GM_CURSOR_POS:

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
		case GM_MB && param1 == GLFW_MOUSE_BUTTON_LEFT:
		{
			if (param2 == GLFW_PRESS)
			{
				if (ContainsPoint(mousePos))
				{
					// Pressed while inside the control
					mPressed = true;
					//SetCapture(GLUFGetHWND());

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
						mDialog.SendEvent(GLUF_EVENT_BUTTON_CLICKED, true, shared_from_this());

					return true;
				}

			}
		}
		break;

		case GM_KEY:
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

					mDialog.SendEvent(GLUF_EVENT_BUTTON_CLICKED, true, shared_from_this());
				}

				return true;
			}

			return true;
		}
	};

	return false;
}

//--------------------------------------------------------------------------------------
void GLUFButton::Render(float elapsedTime)
{
	int nOffsetX = 0;
	int nOffsetY = 0;

	GLUFPoint wndSize = mDialog.GetManager()->GetWindowSize();

	GLUFControlState iState = GLUF_STATE_NORMAL;

	if (mVisible == false)
	{
		iState = GLUF_STATE_HIDDEN;
	}
	else if (mEnabled == false)
	{
		iState = GLUF_STATE_DISABLED;
	}
	else if (mPressed)
	{
		iState = GLUF_STATE_PRESSED;

		nOffsetX = 1;
		nOffsetY = 2;
	}
	else if (mMouseOver)
	{
		iState = GLUF_STATE_MOUSEOVER;

		nOffsetX = -1;
		nOffsetY = -2;
	}
	else if (mHasFocus)
	{
		iState = GLUF_STATE_FOCUS;
	}

	float fBlendRate = (iState == GLUF_STATE_PRESSED) ? 0.0f : 0.8f;

	GLUFRect rcWindow = mRegion;
	GLUFOffsetRect(rcWindow, nOffsetX, nOffsetY);


	// Background fill layer
	GLUFElementPtr pElement = mElements[0];

	// Blend current color
	pElement->mTextureColor.Blend(iState, elapsedTime, fBlendRate);
	pElement->mFontColor.Blend(iState, elapsedTime, fBlendRate);

	mDialog.DrawSprite(pElement, rcWindow, GLUF_FAR_BUTTON_DEPTH);
	//m_pDialog->DrawText(m_strText, pElement, rcWindow, false, true);

	// Main button
	pElement = mElements[1];

	// Blend current color
	pElement->mTextureColor.Blend(iState, elapsedTime, fBlendRate);
	pElement->mFontColor.Blend(iState, elapsedTime, fBlendRate);

	m_pDialog->DrawSprite(pElement, rcWindow, GLUF_NEAR_BUTTON_DEPTH);
	m_pDialog->DrawText(mText, pElement, rcWindow, false, true);
}


/*
======================================================================================================================================================================================================
GLUFButton Functions


*/

GLUFCheckBox::GLUFCheckBox(const bool& checked, GLUFDialog& dialog) : GLUFButton(dialog)
{
	mType = GLUF_CONTROL_CHECKBOX;

    mChecked = checked;
}

//--------------------------------------------------------------------------------------

bool GLUFCheckBox::MsgProc(GLUFMessageType msg, int32_t param1, int32_t param2, int32_t param3, int32_t param4)
{
	if (!mEnabled || !mVisible)
		return false;

	GLUFPoint mousePos = mDialog.GetMousePositionDialogSpace();

	switch (msg)
	{

	case GM_CURSOR_POS:

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
	case GM_MB && param1 == GLFW_MOUSE_BUTTON_LEFT:
	{
		if (param2 == GLFW_PRESS)
		{
			if (ContainsPoint(mousePos))
			{
				// Pressed while inside the control
				mPressed = true;
				//SetCapture(GLUFGetHWND());

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
		break;
	}

	case GM_KEY:
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
	}
	};

	return false;
}


//--------------------------------------------------------------------------------------
void GLUFCheckBox::SetCheckedInternal(bool checked, bool fromInput)
{
    mChecked = checked;

    mDialog.SendEvent(GLUF_EVENT_CHECKBOXCHANGED, fromInput, shared_from_this());
}


//--------------------------------------------------------------------------------------
bool GLUFCheckBox::ContainsPoint(const GLUF::GLUFPoint& pt) const
{
	return (GLUFPtInRect(mRegion, pt) ||
		GLUFPtInRect(mButtonRegion, pt));
}


//--------------------------------------------------------------------------------------
void GLUFCheckBox::UpdateRects()
{
	GLUFButton::UpdateRects();

    mButtonRegion = mRegion;
    mButtonRegion.right = mButtonRegion.left + GLUFRectHeight(mButtonRegion);

    mTextRegion = mRegion;
    GLUFOffsetRect(mTextRegion, GLUFRectWidth(mButtonRegion)/*1.25f*/, 0L);
}


//--------------------------------------------------------------------------------------
void GLUFCheckBox::Render(float elapsedTime)
{
	GLUFControlState iState = GLUF_STATE_NORMAL;

	if (mVisible == false)
		iState = GLUF_STATE_HIDDEN;
	else if (mEnabled == false)
		iState = GLUF_STATE_DISABLED;
	else if (mPressed)
		iState = GLUF_STATE_PRESSED;
	else if (mMouseOver)
		iState = GLUF_STATE_MOUSEOVER;
	else if (mHasFocus)
		iState = GLUF_STATE_FOCUS;

	GLUFElementPtr pElement = mElements[0];

	float fBlendRate = (iState == GLUF_STATE_PRESSED) ? 0.0f : 0.8f;

	pElement->mTextureColor.Blend(iState, elapsedTime, fBlendRate);
	pElement->mFontColor.Blend(iState, elapsedTime, fBlendRate);

	mDialog.DrawSprite(pElement, mButtonRegion, GLUF_FAR_BUTTON_DEPTH);
	mDialog.DrawText(mText, pElement, mTextRegion, false, false);

	if (mChecked)
	{
		pElement = mElements[1];

		pElement->mTextureColor.Blend(iState, elapsedTime, fBlendRate);
		mDialog.DrawSprite(pElement, mButtonRegion, GLUF_NEAR_BUTTON_DEPTH);
	}
}


/*


Ended Here July 26 2015




*/

//======================================================================================
// GLUFRadioButton class
//======================================================================================

GLUFRadioButton::GLUFRadioButton( GLUFDialog* pDialog)
{
	m_Type = GLUF_CONTROL_RADIOBUTTON;
	m_pDialog = pDialog;
}

//--------------------------------------------------------------------------------------
bool GLUFRadioButton::MsgProc(GLUFMessageType msg, int32_t param1, int32_t param2, int32_t param3, int32_t param4)
{
	if (!m_bEnabled || !m_bVisible)
		return false;

	GLUFPoint mousePos = m_pDialog->m_MousePositionDialogSpace;

	switch (msg)
	{

	case GM_CURSOR_POS:

		if (m_bPressed)
		{
			//if the button is pressed and the mouse is moved off, then unpress it
			if (!ContainsPoint(mousePos))
			{
				m_bPressed = false;

				//ContainsPoint(mousePos);

				if (!m_pDialog->m_bKeyboardInput)
					m_pDialog->ClearFocus();
			}
		}

		break;
	case GM_MB && param1 == GLFW_MOUSE_BUTTON_LEFT:
	{
		if (param2 == GLFW_PRESS)
		{
			if (ContainsPoint(mousePos))
			{
				// Pressed while inside the control
				m_bPressed = true;
				//SetCapture(GLUFGetHWND());

				if (!m_bHasFocus)
					m_pDialog->RequestFocus(this);

				return true;

			}
		}
		else if (param2 == GLFW_RELEASE)
		{
			if (m_bPressed || !m_bChecked)
			{
				if (ContainsPoint(mousePos))
				{
					m_bPressed = false;
					//ReleaseCapture();

					SetCheckedInternal(true, true, true);
					return true;
				}
			}

		}
	}
		break;

	case GM_KEY:
	{

		if (param1 = GLFW_KEY_SPACE)
		{
			if (param3 == GLFW_PRESS)
			{
				m_bPressed = true;
			}
			else if (param3 == GLFW_RELEASE)
			{
				if (m_bChecked)
					return false;

				m_bPressed = false;

				SetCheckedInternal(true, true, true);
			}
		}


		return true;

		break;
	}
	};

	return false;
}

//--------------------------------------------------------------------------------------

void GLUFRadioButton::SetCheckedInternal(bool bChecked, bool bClearGroup, bool bFromInput)
{
	if (bChecked && bClearGroup)
		m_pDialog->ClearRadioButtonGroup(m_nButtonGroup);

	m_bChecked = bChecked;
	m_pDialog->SendEvent(GLUF_EVENT_RADIOBUTTON_CHANGED, bFromInput, this);
}


//======================================================================================
// GLUFComboBox class
//======================================================================================

GLUFComboBox::GLUFComboBox( GLUFDialog* pDialog) : m_ScrollBar(pDialog)
{
	m_Type = GLUF_CONTROL_COMBOBOX;
	m_pDialog = pDialog;

	m_fDropHeight = 100L;

	m_fSBWidth = 16L;
	m_bOpened = false;
	m_iSelected = -1;
	m_iFocused = -1;
}


//--------------------------------------------------------------------------------------
GLUFComboBox::~GLUFComboBox()
{
	RemoveAllItems();
}


//--------------------------------------------------------------------------------------
void GLUFComboBox::SetTextColor(Color Color)
{
	GLUFElement* pElement = m_Elements[0];

	if (pElement)
		pElement->FontColor.States[GLUF_STATE_NORMAL] = Color;

	pElement = m_Elements[2];

	if (pElement)
		pElement->FontColor.States[GLUF_STATE_NORMAL] = Color;
}


//--------------------------------------------------------------------------------------
void GLUFComboBox::UpdateRects()
{

	GLUFButton::UpdateRects();

	m_rcButton = m_rcBoundingBox;
	m_rcButton.left = m_rcButton.right - GLUFRectHeight(m_rcButton);

	m_rcText = m_rcBoundingBox;
	m_rcText.right = m_rcButton.left;

	m_rcDropdown.left = long(m_rcText.left * 1.019f);
	m_rcDropdown.top = long(1.02f * m_rcText.bottom);
	m_rcDropdown.right = m_rcText.right;
	m_rcDropdown.bottom = m_rcDropdown.top - m_fDropHeight;
	//GLUFOffsetRect(m_rcDropdown, 0, -GLUFRectHeight(m_rcText));

	m_rcDropdownText = m_rcDropdown;
	m_rcDropdownText.left += long(0.1f * GLUFRectWidth(m_rcDropdown));
	m_rcDropdownText.right -= long(0.1f * GLUFRectWidth(m_rcDropdown));
	m_rcDropdownText.top += long(0.05f * GLUFRectHeight(m_rcDropdown));
	m_rcDropdownText.bottom -= long(0.1f * GLUFRectHeight(m_rcDropdown));

	// Update the scrollbar's rects
	m_ScrollBar.SetLocation(m_rcDropdown.right, m_rcDropdown.bottom);
	m_ScrollBar.SetSize(m_fSBWidth, abs(m_rcButton.bottom - m_rcDropdown.bottom));
	m_ScrollBar.m_y = m_rcText.top;
	GLUFFontNode* pFontNode = m_pDialog->GetManager()->GetFontNode(m_Elements[2]->iFont);
	if (pFontNode/* && pFontNode->mSize*/)
	{
		m_ScrollBar.SetPageSize(int(GLUFRectHeight(m_rcDropdownText) / pFontNode->m_pFontType->mHeight));

		// The selected item may have been scrolled off the page.
		// Ensure that it is in page again.
		m_ScrollBar.ShowItem(m_iSelected);
	}

}

//--------------------------------------------------------------------------------------
void GLUFComboBox::UpdateItemRects()
{
	GLUFFontNode* pFont = m_pDialog->GetFont(GetElement(2)->iFont);
	if (pFont)
	{
		int curY = m_rcText.bottom - 4;// +((m_ScrollBar.GetTrackPos() - 1) * pFont->mSize);
		int fRemainingHeight = GLUFRectHeight(m_rcDropdownText) - pFont->m_Leading;//subtract the font size initially too, because we do not want it hanging off the edge


		for (size_t i = m_ScrollBar.GetTrackPos(); i < m_Items.size(); i++)
		{
			GLUFComboBoxItem* pItem = m_Items[i];

			// Make sure there's room left in the dropdown
			fRemainingHeight -= pFont->m_Leading;
			if (fRemainingHeight <= 0.0f)
			{
				pItem->bVisible = false;
				continue;
			}

			pItem->bVisible = true;

			GLUFSetRect(pItem->rcActive, m_rcDropdownText.left, curY, m_rcDropdownText.right, curY - pFont->m_pFontType->mHeight);
			curY -= pFont->m_Leading;
		}
	}
}

//--------------------------------------------------------------------------------------
void GLUFComboBox::OnFocusOut()
{
	GLUFButton::OnFocusOut();

	m_bOpened = false;
}



//--------------------------------------------------------------------------------------
bool GLUFComboBox::MsgProc(GLUFMessageType msg, int32_t param1, int32_t param2, int32_t param3, int32_t param4)
{
	if (!m_bEnabled || !m_bVisible)
		return false;

	// Let the scroll bar handle it first.
	if (m_ScrollBar.MsgProc(msg, param1, param2, param3, param4))
		return true;

	GLUFPoint pt = m_pDialog->m_MousePositionDialogSpace;

	switch (msg)
	{
		case GM_CURSOR_POS:
		{
			/*if (m_bPressed)
			{
				//if the button is pressed and the mouse is moved off, then unpress it
				if (!ContainsPoint(pt))
				{
					m_bPressed = false;

					ContainsPoint(pt);

					if (!m_pDialog->m_bKeyboardInput)
						m_pDialog->ClearFocus();
				}
			}*/

			if (m_bOpened && GLUFPtInRect(m_rcDropdown, pt))
			{
				// Determine which item has been selected
				for (size_t i = 0; i < m_Items.size(); i++)
				{
					GLUFComboBoxItem* pItem = m_Items[i];
					if (pItem->bVisible &&
						GLUFPtInRect(pItem->rcActive, pt))
					{
						m_iFocused = static_cast<int>(i);
					}
				}
				return true;
			}
			break;
		}

		case GM_MB && param1 == GLFW_MOUSE_BUTTON_LEFT:
			if (param2 == GLFW_PRESS)
			{
				if (ContainsPoint(pt))
				{
					// Pressed while inside the control
					m_bPressed = true;
					//SetCapture(GLUFGetHWND());

					if (!m_bHasFocus)
						m_pDialog->RequestFocus(this);

					return true;
				}
				
				// Perhaps this click is within the dropdown
				if (m_bOpened && GLUFPtInRect(m_rcDropdown, pt))
				{
					// Determine which item has been selected
					for (size_t i = m_ScrollBar.GetTrackPos(); i < m_Items.size(); i++)
					{
						GLUFComboBoxItem* pItem = m_Items[i];
						if (pItem->bVisible &&
							GLUFPtInRect(pItem->rcActive, pt))
						{
							m_iFocused = m_iSelected = static_cast<int>(i);
							m_pDialog->SendEvent(GLUF_EVENT_COMBOBOX_SELECTION_CHANGED, true, this);
							m_bOpened = false;

							if (!m_pDialog->m_bKeyboardInput)
								m_pDialog->ClearFocus();

							break;
						}
					}

					return true;
				}

				// Mouse click not on main control or in dropdown, fire an event if needed
				if (m_bOpened)
				{
					m_iFocused = m_iSelected;

					m_pDialog->SendEvent(GLUF_EVENT_COMBOBOX_SELECTION_CHANGED, true, this);
					m_bOpened = false;
				}
					

				break;
			}
			else if (param2 == GLFW_RELEASE)
			{
				if (m_bPressed && ContainsPoint(pt))
				{
					// Button click
					m_bPressed = false;

					// Toggle dropdown
					if (m_bHasFocus)
					{
						m_bOpened = !m_bOpened;

						if (!m_bOpened)
						{
							if (!m_pDialog->m_bKeyboardInput)
								m_pDialog->ClearFocus();

							m_iFocused = m_iSelected;
						}
						
						//setup the scroll bar to the correct position (if it is still within the range, it looks better to keep its old position)
						int pageMin, pageMax;
						pageMin = m_ScrollBar.GetTrackPos();
						pageMax = m_ScrollBar.GetTrackPos() + m_ScrollBar.GetPageSize() - 2;
						if (m_iFocused > pageMax || m_iFocused < pageMin)
							m_ScrollBar.SetTrackPos(m_iFocused);
					}

					//ReleaseCapture();
					return true;
				}

				break;
			}

		case GM_SCROLL:
		{
			int zDelta = (param2) / WHEEL_DELTA;
			if (m_bOpened)
			{
				//UINT uLines = 0;
				//if (!SystemParametersInfo(SPI_GETWHEELSCROLLLINES, 0, &uLines, 0))
				//	uLines = 0;
				m_ScrollBar.Scroll(-zDelta/* * uLines*/);
				
				//if it is scroll, then make sure to also send a mouse move event to select the newly hovered item
				UpdateItemRects();
				this->MsgProc(GM_CURSOR_POS, 0, 0, 0, 0);//all blank params may be sent because it retrieves the mouse position from the old message
				//TODO: make this work, but for now:
				
				/*if (GLUFPtInRect(m_rcDropdown, pt))
				{
					// Determine which item has been selected
					for (size_t i = 0; i < m_Items.size(); i++)
					{
						GLUFComboBoxItem* pItem = m_Items[i];
						GLUFRect oldRect = pItem->rcActive;

						GLUFOffsetRect(oldRect, 0, float(m_ScrollBar.GetTrackPos() - oldValue) * GLUFRectHeight(pItem->rcActive));
						if (pItem->bVisible &&
							GLUFPtInRect(oldRect, pt))
						{
							m_iFocused = static_cast<int>(i);
						}
					}
				}*/

			}
			else
			{
				if (zDelta > 0)
				{
					if (m_iFocused > 0)
					{
						m_iFocused--;
						m_iSelected = m_iFocused;

						if (!m_bOpened)
							m_pDialog->SendEvent(GLUF_EVENT_COMBOBOX_SELECTION_CHANGED, true, this);
					}
				}
				else
				{
					if (m_iFocused + 1 < (int)GetNumItems())
					{
						m_iFocused++;
						m_iSelected = m_iFocused;

						if (!m_bOpened)
							m_pDialog->SendEvent(GLUF_EVENT_COMBOBOX_SELECTION_CHANGED, true, this);
					}
				}

			}
			return true;
		}
		case GM_KEY:
		{
			if (param3 != GLFW_RELEASE)
				return true;

			switch (param1)
			{
			case GLFW_KEY_ENTER:
				if (m_bOpened)
				{
					if (m_iSelected != m_iFocused)
					{
						m_iSelected = m_iFocused;
						m_pDialog->SendEvent(GLUF_EVENT_COMBOBOX_SELECTION_CHANGED, true, this);
					}
					m_bOpened = false;

					if (!m_pDialog->m_bKeyboardInput)
						m_pDialog->ClearFocus();

					return true;
				}
				break;

			case GLFW_KEY_F4:
				// Filter out auto-repeats
				/*if (param3 == GLFW_REPEAT)
					return true;*/

				m_bOpened = !m_bOpened;

				if (!m_bOpened)
				{
					m_pDialog->SendEvent(GLUF_EVENT_COMBOBOX_SELECTION_CHANGED, true, this);

					if (!m_pDialog->m_bKeyboardInput)
						m_pDialog->ClearFocus();
				}

				return true;

			case GLFW_KEY_UP:
			case GLFW_KEY_LEFT:
				if (m_iFocused > 0)
				{
					m_iFocused--;
					m_iSelected = m_iFocused;

					if (!m_bOpened)
						m_pDialog->SendEvent(GLUF_EVENT_COMBOBOX_SELECTION_CHANGED, true, this);
				}

				return true;

			case GLFW_KEY_RIGHT:
			case GLFW_KEY_DOWN:
				if (m_iFocused + 1 < (int)GetNumItems())
				{
					m_iFocused++;
					m_iSelected = m_iFocused;

					if (!m_bOpened)
						m_pDialog->SendEvent(GLUF_EVENT_COMBOBOX_SELECTION_CHANGED, true, this);
				}

				return true;
			}
			break;
		}
	};

	return false;
}

//--------------------------------------------------------------------------------------
void GLUFComboBox::OnHotkey()
{
	if (m_bOpened)
		return;

	if (m_iSelected == -1)
		return;

	if (m_pDialog->IsKeyboardInputEnabled())
		m_pDialog->RequestFocus(this);

	m_iSelected++;

	if (m_iSelected >= (int)m_Items.size())
		m_iSelected = 0;

	m_iFocused = m_iSelected;
	m_pDialog->SendEvent(GLUF_EVENT_COMBOBOX_SELECTION_CHANGED, true, this);
}


//--------------------------------------------------------------------------------------
void GLUFComboBox::Render( float elapsedTime)
{
	if (m_bVisible == false)
		return;
	GLUF_CONTROL_STATE iState = GLUF_STATE_NORMAL;

	if (!m_bOpened)
		iState = GLUF_STATE_HIDDEN;

	// Dropdown box
	GLUFElement* pElement = m_Elements[2];

	// If we have not initialized the scroll bar page size,
	// do that now.
	static bool bSBInit;
	if (!bSBInit)
	{
		// Update the page size of the scroll bar
		if (m_pDialog->GetManager()->GetFontNode(pElement->iFont)->m_pFontType->mHeight)
			m_ScrollBar.SetPageSize(int(GLUFRectHeight(m_rcDropdownText) /
			(m_pDialog->GetManager()->GetFontNode(pElement->iFont)->m_Leading)));
		else
			m_ScrollBar.SetPageSize(0);
		bSBInit = true;
	}

	// Scroll bar --EDITED, only render any of this stuff if OPENED
	if (m_bOpened)
	{
		m_ScrollBar.Render(elapsedTime);

		// Blend current color
		pElement->TextureColor.Blend(iState, elapsedTime);
		pElement->FontColor.Blend(iState, elapsedTime);

		m_pDialog->DrawSprite(pElement, m_rcDropdown, GLUF_NEAR_BUTTON_DEPTH);


		// Selection outline
		GLUFElement* pSelectionElement = m_Elements[3];
		pSelectionElement->TextureColor.Current = pElement->TextureColor.Current;
		pSelectionElement->FontColor.SetCurrent(/*pSelectionElement->FontColor.States[GLUF_STATE_NORMAL]*/Color(0,0,0,255));

		GLUFFontNode* pFont = m_pDialog->GetFont(pElement->iFont);
		if (pFont)
		{
			//float curY = m_rcDropdownText.top - 0.02f;
			//float fRemainingHeight = GLUFRectHeight(m_rcDropdownText) - pFont->mSize;//subtract the font size initially too, because we do not want it hanging off the edge
			//WCHAR strDropdown[4096] = {0};

			UpdateItemRects();

			for (size_t i = m_ScrollBar.GetTrackPos(); i < m_Items.size(); i++)
			{
				GLUFComboBoxItem* pItem = m_Items[i];
				GLUFRect active = pItem->rcActive;

				active.top = active.bottom + pFont->m_Leading;

				// Make sure there's room left in the dropdown
				
				if (!pItem->bVisible)
					continue;
				//GLUFSetRect(pItem->rcActive, m_rcDropdownText.left, curY, m_rcDropdownText.right, curY - pFont->mSize);
				//curY -= pFont->mSize;

				//debug
				//int blue = 50 * i;
				//m_pDialog->DrawRect( &pItem->rcActive, 0xFFFF0000 | blue );

				//pItem->bVisible = true;

				//GLUFSetRect(rc, m_rcDropdown.left, pItem->rcActive.top - (2 / m_pDialog->GetManager()->GetWindowSize().y), m_rcDropdown.right,
				//	pItem->rcActive.bottom + (2 / m_pDialog->GetManager()->GetWindowSize().y));
				//GLUFSetRect(rc, m_rcDropdown.left + GLUFRectWidth(m_rcDropdown) / 12.0f, m_rcDropdown.top - (GLUFRectHeight(pItem->rcActive) * i), m_rcDropdown.right,
				//	m_rcDropdown.top - (GLUFRectHeight(pItem->rcActive) * (i + 1)));

				if ((int)i == m_iFocused)
				{
					//GLUFSetRect(rc, m_rcDropdown.left, pItem->rcActive.top - (2 / m_pDialog->GetManager()->GetWindowSize().y), m_rcDropdown.right,
					//	pItem->rcActive.bottom + (2 / m_pDialog->GetManager()->GetWindowSize().y));
					/*GLUFSetRect(rc, m_rcDropdown.left, m_rcDropdown.top - (GLUFRectHeight(pItem->rcActive) * i), m_rcDropdown.right,
						m_rcDropdown.top - (GLUFRectHeight(pItem->rcActive) * (i + 1)));*/
					//m_pDialog->DrawText(pItem->strText, pSelectionElement, rc);
					m_pDialog->DrawSprite(pSelectionElement, active, GLUF_NEAR_BUTTON_DEPTH);
					m_pDialog->DrawText(pItem->strText, pSelectionElement, pItem->rcActive);
				}
				else
				{
					m_pDialog->DrawText(pItem->strText, pElement, pItem->rcActive);
				}
			}
		}
	}

	int OffsetX = 0;
	int OffsetY = 0;

	iState = GLUF_STATE_NORMAL;

	if (m_bVisible == false)
		iState = GLUF_STATE_HIDDEN;
	else if (m_bEnabled == false)
		iState = GLUF_STATE_DISABLED;
	else if (m_bPressed)
	{
		iState = GLUF_STATE_PRESSED;

		OffsetX = 1;
		OffsetY = 2;
	}
	else if (m_bMouseOver)
	{
		iState = GLUF_STATE_MOUSEOVER;

		OffsetX = -1;
		OffsetY = -2;
	}
	else if (m_bHasFocus)
		iState = GLUF_STATE_FOCUS;

	float fBlendRate = (iState == GLUF_STATE_PRESSED) ? 0.0f : 0.8f;

	// Button
	pElement = m_Elements[1];

	// Blend current color
	pElement->TextureColor.Blend(iState, elapsedTime, fBlendRate);

	GLUFRect rcWindow = m_rcButton;
	GLUFOffsetRect(rcWindow, OffsetX, OffsetY);
	m_pDialog->DrawSprite(pElement, rcWindow, GLUF_FAR_BUTTON_DEPTH);

	if (m_bOpened)
		iState = GLUF_STATE_PRESSED;


	// Main text box
	pElement = m_Elements[0];

	// Blend current color
	pElement->TextureColor.Blend(iState, elapsedTime, fBlendRate);
	pElement->FontColor.Blend(iState, elapsedTime, fBlendRate);


	m_pDialog->DrawSprite(pElement, m_rcText, GLUF_NEAR_BUTTON_DEPTH);

	if (m_iSelected >= 0 && m_iSelected < (int)m_Items.size())
	{
		GLUFComboBoxItem* pItem = m_Items[m_iSelected];
		if (pItem)
		{
			m_pDialog->DrawText(pItem->strText, pElement, m_rcText, false, true);

		}
	}

}


//--------------------------------------------------------------------------------------

GLUFResult GLUFComboBox::AddItem(std::wstring strText, void* pData)
{
	// Validate parameters
	/*if (!strText)
	{
		return E_INVALIDARG;
	}*/

	// Create a new item and set the data
	GLUFComboBoxItem* pItem = new (std::nothrow) GLUFComboBoxItem;
	if (!pItem)
	{
		return GLUFTRACE_ERR("new", GR_OUTOFMEMORY);
	}

	//ZeroMemory(pItem, sizeof(GLUFComboBoxItem));
	//wcscpy_s(pItem->strText, 256, strText);
	pItem->strText = strText;
	pItem->pData = pData;

	m_Items.push_back(pItem);

	// Update the scroll bar with new range
	m_ScrollBar.SetTrackRange(0, (int)m_Items.size());

	// If this is the only item in the list, it's selected
	if (GetNumItems() == 1)
	{
		m_iSelected = 0;
		m_iFocused = 0;
		m_pDialog->SendEvent(GLUF_EVENT_COMBOBOX_SELECTION_CHANGED, false, this);
	}

	return GR_SUCCESS;
}


//--------------------------------------------------------------------------------------
void GLUFComboBox::RemoveItem( unsigned int index)
{
	auto it = m_Items.begin() + index;
	GLUFComboBoxItem* pItem = *it;
	GLUF_SAFE_DELETE(pItem);
	m_Items.erase(it);
	m_ScrollBar.SetTrackRange(0, (int)m_Items.size());
	if (m_iSelected >= (int)m_Items.size())
		m_iSelected = (int)m_Items.size() - 1;
}


//--------------------------------------------------------------------------------------
void GLUFComboBox::RemoveAllItems()
{
	for (auto it = m_Items.begin(); it != m_Items.end(); ++it)
	{
		GLUFComboBoxItem* pItem = *it;
		GLUF_SAFE_DELETE(pItem);
	}

	m_Items.clear();
	m_ScrollBar.SetTrackRange(0, 1);
	m_iFocused = m_iSelected = -1;
}


//--------------------------------------------------------------------------------------
bool GLUFComboBox::ContainsItem(std::wstring strText, unsigned int iStart)
{
	return (-1 != FindItem(strText, iStart));
}


//--------------------------------------------------------------------------------------
int GLUFComboBox::FindItem(std::wstring strText, unsigned int iStart)
{
	/*if (!strText)
		return -1;*/

	for (size_t i = iStart; i < m_Items.size(); i++)
	{
		GLUFComboBoxItem* pItem = m_Items[i];

		if (0 != pItem->strText.compare(strText))//REMEMBER if this returns 0, they are the same
		{
			return static_cast<int>(i);
		}
	}

	return -1;
}


//--------------------------------------------------------------------------------------
void* GLUFComboBox::GetSelectedData()
{
	if (m_iSelected < 0)
		return nullptr;

	GLUFComboBoxItem* pItem = m_Items[m_iSelected];
	return pItem->pData;
}


//--------------------------------------------------------------------------------------
GLUFComboBoxItem* GLUFComboBox::GetSelectedItem()
{
	if (m_iSelected < 0)
		return nullptr;

	return m_Items[m_iSelected];
}


//--------------------------------------------------------------------------------------
void* GLUFComboBox::GetItemData(std::wstring strText) 
{
	int index = FindItem(strText);
	if (index == -1)
	{
		return nullptr;
	}

	GLUFComboBoxItem* pItem = m_Items[index];
	if (!pItem)
	{
		GLUFTRACE_ERR("GLUFComboBox::GetItemData", GR_FAILURE);
		return nullptr;
	}

	return pItem->pData;
}


//--------------------------------------------------------------------------------------
void* GLUFComboBox::GetItemData( int nIndex)
{
	if (nIndex < 0 || nIndex >= (int)m_Items.size())
		return nullptr;

	return m_Items[nIndex]->pData;
}


//--------------------------------------------------------------------------------------
GLUFResult GLUFComboBox::SetSelectedByIndex( unsigned int index)
{
	if (index >= GetNumItems())
		return GR_INVALIDARG;

	m_iFocused = m_iSelected = index;
	m_pDialog->SendEvent(GLUF_EVENT_COMBOBOX_SELECTION_CHANGED, false, this);

	return GR_SUCCESS;
}



//--------------------------------------------------------------------------------------
GLUFResult GLUFComboBox::SetSelectedByText(std::wstring strText)
{
	/*if (!strText)
		return E_INVALIDARG;*/

	int index = FindItem(strText);
	if (index == -1)
		return GR_FAILURE;

	m_iFocused = m_iSelected = index;
	m_pDialog->SendEvent(GLUF_EVENT_COMBOBOX_SELECTION_CHANGED, false, this);

	return GR_SUCCESS;
}



//--------------------------------------------------------------------------------------
GLUFResult GLUFComboBox::SetSelectedByData( void* pData)
{
	for (size_t i = 0; i < m_Items.size(); i++)
	{
		GLUFComboBoxItem* pItem = m_Items[i];

		if (pItem->pData == pData)
		{
			m_iFocused = m_iSelected = static_cast<int>(i);
			m_pDialog->SendEvent(GLUF_EVENT_COMBOBOX_SELECTION_CHANGED, false, this);
			return GR_SUCCESS;
		}
	}

	return GR_FAILURE;
}


//======================================================================================
// GLUFSlider class
//======================================================================================

GLUFSlider::GLUFSlider( GLUFDialog* pDialog)
{
	m_Type = GLUF_CONTROL_SLIDER;
	m_pDialog = pDialog;

	m_nMin = 0;
	m_nMax = 100;
	m_nValue = 0;
	m_bPressed = false;
}


//--------------------------------------------------------------------------------------
bool GLUFSlider::ContainsPoint(GLUFPoint pt)
{
	return (GLUFPtInRect(m_rcBoundingBox, pt) ||
		GLUFPtInRect(m_rcButton, pt));
}


//--------------------------------------------------------------------------------------
void GLUFSlider::UpdateRects()
{
	GLUFControl::UpdateRects();

	m_rcButton = m_rcBoundingBox;
	m_rcButton.right = m_rcButton.left + GLUFRectHeight(m_rcButton);
	GLUFOffsetRect(m_rcButton, -GLUFRectWidth(m_rcButton) / 2, 0);

	m_nButtonX = (int)((float(m_nValue - m_nMin) / float(m_nMax - m_nMin)) * GLUFRectWidth(m_rcBoundingBox));
	GLUFOffsetRect(m_rcButton, m_nButtonX, 0);
}


//--------------------------------------------------------------------------------------
int GLUFSlider::ValueFromPos(long x)
{
	float fValuePerPixel = (float)(m_nMax - m_nMin) / GLUFRectWidth(m_rcBoundingBox);
	float fPixelPerValue2 = 1.0f / (2.0f * fValuePerPixel);//use this to get it to change locations at the half way mark instead of using truncate int methods
	return int(((x - m_x + fPixelPerValue2) * fValuePerPixel) + m_nMin);
}


//--------------------------------------------------------------------------------------
/*
bool GLUFSlider::HandleKeyboard(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);

	if (!m_bEnabled || !m_bVisible)
		return false;

	switch (uMsg)
	{
	case WM_KEYDOWN:
	{
		switch (wParam)
		{
		case VK_HOME:
			SetValueInternal(m_nMin, true);
			return true;

		case VK_END:
			SetValueInternal(m_nMax, true);
			return true;

		case VK_LEFT:
		case VK_DOWN:
			SetValueInternal(m_nValue - 1, true);
			return true;

		case VK_RIGHT:
		case VK_UP:
			SetValueInternal(m_nValue + 1, true);
			return true;

		case VK_NEXT:
			SetValueInternal(m_nValue - (10 > (m_nMax - m_nMin) / 10 ? 10 : (m_nMax - m_nMin) / 10),
				true);
			return true;

		case VK_PRIOR:
			SetValueInternal(m_nValue + (10 > (m_nMax - m_nMin) / 10 ? 10 : (m_nMax - m_nMin) / 10),
				true);
			return true;
		}
		break;
	}
	}


	return false;
}


//--------------------------------------------------------------------------------------

bool GLUFSlider::HandleMouse(UINT uMsg, const POINT& pt, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);

	if (!m_bEnabled || !m_bVisible)
		return false;

	switch (uMsg)
	{
	case WM_LBUTTONDOWN:
	case WM_LBUTTONDBLCLK:
	{
		if (PtInRect(&m_rcButton, pt))
		{
			// Pressed while inside the control
			m_bPressed = true;
			SetCapture(GLUFGetHWND());

			m_nDragX = pt.x;
			//m_nDragY = pt.y;
			m_nDragOffset = m_nButtonX - m_nDragX;

			//m_nDragValue = m_nValue;

			if (!m_bHasFocus)
				m_pDialog->RequestFocus(this);

			return true;
		}

		if (PtInRect(&m_rcBoundingBox, pt))
		{
			m_nDragX = pt.x;
			m_nDragOffset = 0;
			m_bPressed = true;

			if (!m_bHasFocus)
				m_pDialog->RequestFocus(this);

			if (pt.x > m_nButtonX + m_x)
			{
				SetValueInternal(m_nValue + 1, true);
				return true;
			}

			if (pt.x < m_nButtonX + m_x)
			{
				SetValueInternal(m_nValue - 1, true);
				return true;
			}
		}

		break;
	}

	case WM_LBUTTONUP:
	{
		if (m_bPressed)
		{
			m_bPressed = false;
			ReleaseCapture();
			m_pDialog->SendEvent(GLUF_EVENTSLIDER_VALUE_CHANGED_UP, true, this);

			return true;
		}

		break;
	}

	case WM_MOUSEMOVE:
	{
		if (m_bPressed)
		{
			SetValueInternal(ValueFromPos(m_x + pt.x + m_nDragOffset), true);
			return true;
		}

		break;
	}

	case WM_MOUSEWHEEL:
	{
		int nScrollAmount = int((short)HIWORD(wParam)) / WHEEL_DELTA;
		SetValueInternal(m_nValue - nScrollAmount, true);
		return true;
	}
	};

	return false;
}
*/


//--------------------------------------------------------------------------------------
bool GLUFSlider::MsgProc(GLUFMessageType msg, int32_t param1, int32_t param2, int32_t param3, int32_t param4)
{
	if (!m_bEnabled || !m_bVisible)
		return false;

	GLUFPoint pt = m_pDialog->m_MousePositionDialogSpace;

	switch (msg)
	{
		case GM_MB && param1 == GLFW_MOUSE_BUTTON_LEFT:
			if (param2 == GLFW_PRESS)
			{
				if (GLUFPtInRect(m_rcButton, pt))
				{
					// Pressed while inside the control
					m_bPressed = true;
					//SetCapture(GLUFGetHWND());

					m_nDragX = pt.x;
					//m_nDragY = pt.y;
					m_nDragOffset = m_nButtonX - m_nDragX;

					//m_nDragValue = m_nValue;

					if (!m_bHasFocus)
						m_pDialog->RequestFocus(this);

					return true;
				}

				if (GLUFPtInRect(m_rcBoundingBox, pt))
				{

					if (!m_bHasFocus)
						m_pDialog->RequestFocus(this);

					SetValueInternal(ValueFromPos(pt.x), true);

					return true;
				}
			}
			else if (param2 == GLFW_RELEASE)
			{
				if (m_bPressed)
				{
					m_bPressed = false;
					//ReleaseCapture();
					m_pDialog->SendEvent(GLUF_EVENT_SLIDER_VALUE_CHANGED_UP, true, this);

					return true;
				}

				break;

			}
			break;
	
		case GM_CURSOR_POS:
		{

			if (m_bPressed)
			{
				SetValueInternal(ValueFromPos(m_x + pt.x + m_nDragOffset), true);
				return true;
			}

			break;
		}

		case GM_SCROLL:
		{
			int nScrollAmount = param2 / WHEEL_DELTA;
			SetValueInternal(m_nValue - nScrollAmount, true);
			return true;
		}
		case GM_KEY:
		{
			if (param3 == GLFW_RELEASE)
				break;

			switch (param1)
			{
			case GLFW_KEY_HOME:
				SetValueInternal(m_nMin, true);
				return true;

			case GLFW_KEY_END:
				SetValueInternal(m_nMax, true);
				return true;

			case GLFW_KEY_LEFT:
			case GLFW_KEY_DOWN:
				SetValueInternal(m_nValue - 3, true);
				return true;

			case GLFW_KEY_RIGHT:
			case GLFW_KEY_UP:
				SetValueInternal(m_nValue + 3, true);
				return true;

			case GLFW_KEY_PAGE_DOWN:
				SetValueInternal(m_nValue - (10 > (m_nMax - m_nMin) / 10 ? 10 : (m_nMax - m_nMin) / 10),
					true);
				return true;

			case GLFW_KEY_PAGE_UP:
				SetValueInternal(m_nValue + (10 > (m_nMax - m_nMin) / 10 ? 10 : (m_nMax - m_nMin) / 10),
					true);
				return true;
			}
			break;
		}
	};

	return false;
}

//--------------------------------------------------------------------------------------
void GLUFSlider::SetRange(int nMin, int nMax)
{
	m_nMin = nMin;
	m_nMax = nMax;

	SetValueInternal(m_nValue, false);
}


//--------------------------------------------------------------------------------------
void GLUFSlider::SetValueInternal(int nValue, bool bFromInput)
{
	// Clamp to range
	nValue = std::clamp(nValue, m_nMin, m_nMax);


	if (nValue == m_nValue)
		return;

	m_nValue = nValue;

	UpdateRects();

	m_pDialog->SendEvent(GLUF_EVENT_SLIDER_VALUE_CHANGED, bFromInput, this);
}


//--------------------------------------------------------------------------------------
void GLUFSlider::Render( float elapsedTime)
{
	if (m_bVisible == false)
		return;

	int OffsetX = 0;
	int OffsetY = 0;

	GLUF_CONTROL_STATE iState = GLUF_STATE_NORMAL;

	if (m_bVisible == false)
	{
		iState = GLUF_STATE_HIDDEN;
	}
	else if (m_bEnabled == false)
	{
		iState = GLUF_STATE_DISABLED;
	}
	else if (m_bPressed)
	{
		iState = GLUF_STATE_PRESSED;

		OffsetX = 1;
		OffsetY = 2;
	}
	else if (m_bMouseOver)
	{
		iState = GLUF_STATE_MOUSEOVER;

		OffsetX = -1;
		OffsetY = -2;
	}
	else if (m_bHasFocus)
	{
		iState = GLUF_STATE_FOCUS;
	}

	float fBlendRate = (iState == GLUF_STATE_PRESSED) ? 0.0f : 0.8f;

	GLUFElement* pElement = m_Elements[0];

	// Blend current color
	pElement->TextureColor.Blend(iState, elapsedTime, fBlendRate);
	m_pDialog->DrawSprite(pElement, m_rcBoundingBox, GLUF_FAR_BUTTON_DEPTH);

	pElement = m_Elements[1];

	// Blend current color
	pElement->TextureColor.Blend(iState, elapsedTime, fBlendRate);
	m_pDialog->DrawSprite(pElement, m_rcButton, GLUF_NEAR_BUTTON_DEPTH);
}


//======================================================================================
// GLUFScrollBar class
//======================================================================================

GLUFScrollBar::GLUFScrollBar( GLUFDialog* pDialog)
{
	m_Type = GLUF_CONTROL_SCROLLBAR;
	m_pDialog = pDialog;

	m_bShowThumb = true;
	m_bDrag = false;

	GLUFSetRect(m_rcUpButton, 0, 0, 0, 0);
	GLUFSetRect(m_rcDownButton, 0, 0, 0, 0);
	GLUFSetRect(m_rcTrack, 0, 0, 0, 0);
	GLUFSetRect(m_rcThumb, 0, 0, 0, 0);
	m_nPosition = 0;
	m_nPageSize = 1;
	m_nStart = 0;
	m_nEnd = 1;
	m_Arrow = CLEAR;
	m_dArrowTS = 0.0;
}


//--------------------------------------------------------------------------------------
GLUFScrollBar::~GLUFScrollBar()
{
}


//--------------------------------------------------------------------------------------
void GLUFScrollBar::UpdateRects()
{
	GLUFControl::UpdateRects();

	// Make the buttons square

	GLUFSetRect(m_rcUpButton, m_rcBoundingBox.left, m_rcBoundingBox.top,
		m_rcBoundingBox.right, m_rcBoundingBox.top - GLUFRectWidth(m_rcBoundingBox));
	//GLUFOffsetRect(m_rcDownButton, 0.0f, GLUFRectHeight(m_rcDownButton));

	GLUFSetRect(m_rcDownButton, m_rcBoundingBox.left, m_rcBoundingBox.bottom + GLUFRectWidth(m_rcBoundingBox),
		m_rcBoundingBox.right, m_rcBoundingBox.bottom);
	//GLUFOffsetRect(m_rcDownButton, 0.0f, GLUFRectHeight(m_rcDownButton));

	GLUFSetRect(m_rcTrack, m_rcUpButton.left, m_rcUpButton.bottom,
		m_rcDownButton.right, m_rcDownButton.top);
	//GLUFOffsetRect(m_rcDownButton, 0.0f, GLUFRectHeight(m_rcDownButton));

	m_rcThumb.left = m_rcUpButton.left;
	m_rcThumb.right = m_rcUpButton.right;

	UpdateThumbRect();
}


//--------------------------------------------------------------------------------------
// Compute the dimension of the scroll thumb
void GLUFScrollBar::UpdateThumbRect()
{
	//TODO: fix bug where the icon can go just below the max it should
	if (m_nEnd - m_nStart > m_nPageSize)
	{
		int nThumbHeight = std::clamp((int)GLUFRectHeight(m_rcTrack) * m_nPageSize / (m_nEnd - m_nStart),
			SCROLLBAR_MINTHUMBSIZE, (int)GLUFRectHeight(m_rcTrack));
		int nMaxPosition = m_nEnd - m_nStart - m_nPageSize + 1;
		m_rcThumb.top = m_rcTrack.top - (m_nPosition - m_nStart) * (GLUFRectHeight(m_rcTrack) - nThumbHeight)
			/ nMaxPosition;
		m_rcThumb.bottom = m_rcThumb.top - nThumbHeight;
		m_bShowThumb = true;

	}
	else
	{
		// No content to scroll
		m_rcThumb.bottom = m_rcThumb.top;
		m_bShowThumb = false;
	}
}


//--------------------------------------------------------------------------------------
// Scroll() scrolls by nDelta items.  A positive value scrolls down, while a negative
// value scrolls up.
void GLUFScrollBar::Scroll( int nDelta)
{
	// Perform scroll
	m_nPosition += nDelta;

	// Cap position
	Cap();

	// Update thumb position
	UpdateThumbRect();
}


//--------------------------------------------------------------------------------------
void GLUFScrollBar::ShowItem( int nIndex)
{
	// Cap the index

	nIndex = std::clamp(nIndex, 0, m_nEnd - 1);

	// Adjust position

	if (m_nPosition > nIndex)
		m_nPosition = nIndex;
	else if (m_nPosition + m_nPageSize <= nIndex)
		m_nPosition = nIndex - m_nPageSize + 1;

	UpdateThumbRect();
}

//--------------------------------------------------------------------------------------

bool GLUFScrollBar::MsgProc(GLUFMessageType msg, int32_t param1, int32_t param2, int32_t param3, int32_t param4)
{
	//UNREFERENCED_PARAMETER(wParam);

	if (GM_FOCUS == msg && param1 == GL_FALSE)
	{
		// The application just lost mouse capture. We may not have gotten
		// the WM_MOUSEUP message, so reset m_bDrag here.
		//if ((HWND)lParam != GLUFGetHWND())
		m_bDrag = false;
	}
	static int nThumbYOffset;

	GLUFPoint pt = m_pDialog->m_MousePositionDialogSpace;
	m_LastMouse = pt;


	//if the mousebutton is NOT PRESSED, the stop scrolling(since this does not get the mouse release button if it is not over the control)
	//int mbPressed = glfwGetMouseButton(g_pGLFWWindow, GLFW_MOUSE_BUTTON_LEFT);
	//if (mbPressed == GLFW_RELEASE)
	//	m_bDrag = false;

	switch (msg)
	{
	case GM_MB && param1 == GLFW_MOUSE_BUTTON_LEFT:
		if (param2 == GLFW_PRESS)
		{
			// Check for click on up button

			if (GLUFPtInRect(m_rcUpButton, pt))
			{
				//SetCapture(GLUFGetHWND());
				if (m_nPosition > m_nStart)
					--m_nPosition;

				UpdateThumbRect();
				m_Arrow = CLICKED_UP;
				m_dArrowTS = GLUFGetTime();
				return true;
			}

			// Check for click on down button

			if (GLUFPtInRect(m_rcDownButton, pt))
			{
				//SetCapture(GLUFGetHWND());
				if (m_nPosition + m_nPageSize <= m_nEnd)
					++m_nPosition;

				UpdateThumbRect();
				m_Arrow = CLICKED_DOWN;
				m_dArrowTS = GLUFGetTime();
				return true;
			}

			// Check for click on thumb

			if (GLUFPtInRect(m_rcThumb, pt))
			{
				//SetCapture(GLUFGetHWND());
				m_bDrag = true;
				nThumbYOffset = m_rcThumb.top - pt.y;
				return true;
			}

			// Check for click on track

			if (m_rcThumb.left <= pt.x &&
				m_rcThumb.right > pt.x)
			{
				//SetCapture(GLUFGetHWND());
				if (m_rcThumb.top > pt.y &&
					m_rcTrack.top <= pt.y)
				{
					Scroll(-(m_nPageSize - 1));
					return true;
				}
				else if (m_rcThumb.bottom <= pt.y &&
					m_rcTrack.bottom > pt.y)
				{
					Scroll(m_nPageSize - 1);
					return true;
				}
			}

			break;
		}
		else if (param2 == GLFW_RELEASE)
		{
			m_bDrag = false;
			//ReleaseCapture();
			UpdateThumbRect();
			m_Arrow = CLEAR;
			break;
		}

		case GM_CURSOR_POS:
		{
			if (m_bDrag)
			{
				static int nThumbHeight;
				nThumbHeight = GLUFRectHeight(m_rcThumb);

				m_rcThumb.top = pt.y + nThumbYOffset;
				m_rcThumb.bottom = m_rcThumb.top - nThumbHeight;
				if (m_rcThumb.top > m_rcTrack.top)
					GLUFOffsetRect(m_rcThumb, 0, -(m_rcThumb.top - m_rcTrack.top));
				else if (m_rcThumb.bottom < m_rcTrack.bottom)
					GLUFOffsetRect(m_rcThumb, 0, -(m_rcThumb.bottom - m_rcTrack.bottom));
				
				// Compute first item index based on thumb position

				int nMaxFirstItem = m_nEnd - m_nStart - m_nPageSize + 1;  // Largest possible index for first item
				int nMaxThumb = GLUFRectHeight(m_rcTrack) - GLUFRectHeight(m_rcThumb);  // Largest possible thumb position from the top

				m_nPosition = m_nStart + int(((m_rcTrack.top - m_rcThumb.top) * (float)nMaxFirstItem) / nMaxThumb);
					/*(m_rcTrack.top - m_rcThumb.top +
					nMaxThumb / (nMaxFirstItem * 2)) * // Shift by half a row to avoid last row covered by only one pixel
					nMaxFirstItem / nMaxThumb;*/

				return true;
			}

			break;
		}
	}

	return false;
}


//--------------------------------------------------------------------------------------
void GLUFScrollBar::Render( float elapsedTime)
{
	if (m_bVisible == false)
		return;

	// Check if the arrow button has been held for a while.
	// If so, update the thumb position to simulate repeated
	// scroll.
	if (m_Arrow != CLEAR)
	{
		double dCurrTime = GLUFGetTime();
		if (GLUFPtInRect(m_rcUpButton, m_LastMouse))
		{
			switch (m_Arrow)
			{
			case CLICKED_UP:
				if (SCROLLBAR_ARROWCLICK_DELAY < dCurrTime - m_dArrowTS)
				{
					Scroll(-1);
					m_Arrow = HELD_UP;
					m_dArrowTS = dCurrTime;
				}
				break;
			case HELD_UP:
				if (SCROLLBAR_ARROWCLICK_REPEAT < dCurrTime - m_dArrowTS)
				{
					Scroll(-1);
					m_dArrowTS = dCurrTime;
				}
				break;
			}
		}
		else if (GLUFPtInRect(m_rcDownButton, m_LastMouse))
		{
			switch (m_Arrow)
			{
			case CLICKED_DOWN:
				if (SCROLLBAR_ARROWCLICK_DELAY < dCurrTime - m_dArrowTS)
				{
					Scroll(1);
					m_Arrow = HELD_DOWN;
					m_dArrowTS = dCurrTime;
				}
				break;
			case HELD_DOWN:
				if (SCROLLBAR_ARROWCLICK_REPEAT < dCurrTime - m_dArrowTS)
				{
					Scroll(1);
					m_dArrowTS = dCurrTime;
				}
				break;
			}
		}
	}

	GLUF_CONTROL_STATE iState = GLUF_STATE_NORMAL;

	if (m_bVisible == false)
		iState = GLUF_STATE_HIDDEN;
	else if (m_bEnabled == false || m_bShowThumb == false)
		iState = GLUF_STATE_DISABLED;
	else if (m_bMouseOver)
		iState = GLUF_STATE_MOUSEOVER;
	else if (m_bHasFocus)
		iState = GLUF_STATE_FOCUS;


	float fBlendRate = (iState == GLUF_STATE_PRESSED) ? 0.0f : 0.8f;

	// Background track layer
	GLUFElement* pElement = m_Elements[0];

	// Blend current color
	pElement->TextureColor.Blend(iState, elapsedTime, fBlendRate);
	m_pDialog->DrawSprite(pElement, m_rcTrack, GLUF_FAR_BUTTON_DEPTH);

	
	GLUF_CONTROL_STATE iArrowState = iState;
	//if it is all the way at the top, then disable
	if (m_nPosition == 0 && iState != GLUF_STATE_HIDDEN)
		iArrowState = GLUF_STATE_DISABLED;
	
	// Up Arrow
	pElement = m_Elements[1];

	// Blend current color
	pElement->TextureColor.Blend(iArrowState, elapsedTime, fBlendRate);
	m_pDialog->DrawSprite(pElement, m_rcUpButton, GLUF_NEAR_BUTTON_DEPTH);


	//similar with the bottom
	iArrowState = iState;
	if ((m_nPosition + m_nPageSize - 1 == m_nEnd && iState != GLUF_STATE_HIDDEN) || m_nEnd == 1/*when no scrolling is necesary*/)
		iArrowState = GLUF_STATE_DISABLED;

	// Down Arrow
	pElement = m_Elements[2];

	// Blend current color
	pElement->TextureColor.Blend(iArrowState, elapsedTime, fBlendRate);
	m_pDialog->DrawSprite(pElement, m_rcDownButton, GLUF_NEAR_BUTTON_DEPTH);

	// Thumb button
	pElement = m_Elements[3];

	// Blend current color
	pElement->TextureColor.Blend(iState, elapsedTime, fBlendRate);
	m_pDialog->DrawSprite(pElement, m_rcThumb, GLUF_NEAR_BUTTON_DEPTH);

}


//--------------------------------------------------------------------------------------
void GLUFScrollBar::SetTrackRange( int nStart,  int nEnd)
{
	m_nStart = nStart; m_nEnd = nEnd;
	Cap();
	UpdateThumbRect();
}


//--------------------------------------------------------------------------------------
void GLUFScrollBar::Cap()  // Clips position at boundaries. Ensures it stays within legal range.
{
	if (m_nPosition < m_nStart ||
		m_nEnd - m_nStart <= m_nPageSize)
	{
		m_nPosition = m_nStart;
	}
	else if (m_nPosition + m_nPageSize > m_nEnd)
		m_nPosition = m_nEnd - m_nPageSize + 1;
}


//======================================================================================
// GLUFListBox class
//======================================================================================

GLUFListBox::GLUFListBox( GLUFDialog* pDialog) : m_ScrollBar(pDialog)
{
	m_Type = GLUF_CONTROL_LISTBOX;
	m_pDialog = pDialog;



	GLUFPoint pt = m_pDialog->GetManager()->GetWindowSize();

	m_dwStyle = 0;
	m_fSBWidth = 16;
	m_Selected.push_back(-1);
	m_bDrag = false;
	m_fBorder = 6;
	m_fMargin = 5;
	m_fTextHeight = 20;
}


//--------------------------------------------------------------------------------------
GLUFListBox::~GLUFListBox()
{
	RemoveAllItems();
}

//--------------------------------------------------------------------------------------
void GLUFListBox::UpdateRects()
{
	GLUFControl::UpdateRects();

	GLUFFontNode* pFont = m_pDialog->GetFont(GetElement(0)->iFont);
	m_fTextHeight = pFont->m_Leading;

	m_rcSelection = m_rcBoundingBox;
	m_rcSelection.right -= m_fSBWidth;
	GLUFInflateRect(m_rcSelection, -m_fBorder, -m_fBorder);
	m_rcText = m_rcSelection;
	GLUFInflateRect(m_rcText, -m_fMargin, 0);
	//m_fBorder += -(GLUFRectHeight(m_rcText) % (long)pFont->m_Leading) / 2;

	// Update the scrollbar's rects
	//m_ScrollBar.SetLocation(m_rcBoundingBox.right - m_fSBWidth, m_rcBoundingBox.top);
	//m_ScrollBar.SetSize(m_fSBWidth, m_height);

	m_ScrollBar.SetLocation(m_rcBoundingBox.right, m_rcBoundingBox.bottom);
	m_ScrollBar.SetSize(m_fSBWidth, GLUFRectHeight(m_rcBoundingBox));
	m_ScrollBar.m_y = m_rcText.top;
	GLUFFontNode* pFontNode = m_pDialog->GetManager()->GetFontNode(m_Elements[0]->iFont);
	if (pFontNode && pFontNode->m_pFontType->mHeight)
	{
		m_ScrollBar.SetPageSize(int(GLUFRectHeight(m_rcText) / pFontNode->m_Leading));

		// The selected item may have been scrolled off the page.
		// Ensure that it is in page again.
		m_ScrollBar.ShowItem(m_Selected[m_Selected.size()-1]);
	}

	UpdateItemRects();
}


//--------------------------------------------------------------------------------------

GLUFResult GLUFListBox::AddItem(std::wstring wszText, void* pData)
{
	GLUFListBoxItem* pNewItem = new (std::nothrow) GLUFListBoxItem;
	if (!pNewItem)
		return GR_OUTOFMEMORY;

	//clear the selection vector
	m_Selected.clear();//this makes it so we do not have to offset the selection
	m_Selected.push_back(-1);

	//wcscpy_s(pNewItem->strText, 256, wszText);
	pNewItem->strText = wszText;
	pNewItem->pData = pData;
	GLUFSetRect(pNewItem->rcActive, 0, 0, 0, 0);
	//pNewItem->bSelected = false;

	m_Items.push_back(pNewItem);
	m_ScrollBar.SetTrackRange(0, (int)m_Items.size());

	return GR_SUCCESS;
}


//--------------------------------------------------------------------------------------

GLUFResult GLUFListBox::InsertItem(int nIndex, std::wstring wszText, void* pData)
{
	GLUFListBoxItem* pNewItem = new (std::nothrow) GLUFListBoxItem;
	if (!pNewItem)
		return GR_OUTOFMEMORY;

	//clear the selection vector
	m_Selected.clear();//this makes it so we do not have to offset the selection
	m_Selected.push_back(-1);

	//wcscpy_s(pNewItem->strText, 256, wszText);
	pNewItem->strText = wszText;
	pNewItem->pData = pData;
	GLUFSetRect(pNewItem->rcActive, 0, 0, 0, 0);
	//pNewItem->bSelected = false;


	m_Items[nIndex] = pNewItem;
	m_ScrollBar.SetTrackRange(0, (int)m_Items.size());

	return GR_SUCCESS;
}


//--------------------------------------------------------------------------------------
void GLUFListBox::RemoveItem( int nIndex)
{
	if (nIndex < 0 || nIndex >= (int)m_Items.size())
		return;

	auto it = m_Items.begin() + nIndex;
	GLUFListBoxItem* pItem = *it;
	delete pItem;
	m_Items.erase(it);
	m_ScrollBar.SetTrackRange(0, (int)m_Items.size());
	if (m_Selected[0] >= (int)m_Items.size())
		m_Selected[0] = int(m_Items.size()) - 1;

	m_pDialog->SendEvent(GLUF_EVENT_LISTBOX_SELECTION, true, this);
}


//--------------------------------------------------------------------------------------
void GLUFListBox::RemoveAllItems()
{
	for (auto it = m_Items.begin(); it != m_Items.end(); ++it)
	{
		GLUFListBoxItem* pItem = *it;
		delete pItem;
	}

	m_Items.clear();
	m_ScrollBar.SetTrackRange(0, 1);
	m_Selected.clear();
	m_Selected.push_back(-1);
}


//--------------------------------------------------------------------------------------
GLUFListBoxItem* GLUFListBox::GetItem( int nIndex)
{
	if (nIndex < 0 || nIndex >= (int)m_Items.size())
		return nullptr;

	return m_Items[nIndex];
}


//--------------------------------------------------------------------------------------
// For single-selection listbox, returns the index of the selected item.
// For multi-selection, returns the first selected item after the nPreviousSelected position.
// To search for the first selected item, the app passes -1 for nPreviousSelected.  For
// subsequent searches, the app passes the returned index back to GetSelectedIndex as.
// nPreviousSelected.
// Returns -1 on error or if no item is selected.
int GLUFListBox::GetSelectedIndex( int nPreviousSelected)
{
	if (nPreviousSelected < -1)
		return -1;

	if (m_dwStyle & MULTISELECTION)
	{
		// Multiple selection enabled. Search for the next item with the selected flag.
		/*for (int i = nPreviousSelected + 1; i < (int)m_Items.size(); ++i)
		{
			GLUFListBoxItem* pItem = m_Items[i];

			if (pItem->bSelected)
				return i;
		}*/

		//TODO: does this work?
		int in = *(std::find(m_Selected.begin(), m_Selected.end(), nPreviousSelected) + 1);

		return -1;
	}
	else
	{
		// Single selection
		return m_Selected[0];
	}
}


//--------------------------------------------------------------------------------------
void GLUFListBox::SelectItem( int nNewIndex)
{
	// If no item exists, do nothing.
	if (m_Items.size() == 0)
		return;

	int nOldSelected = m_Selected[0];

	// Adjust m_nSelected
	m_Selected[0] = nNewIndex;

	// Perform capping
	if (m_Selected[0] < 0)
		m_Selected[0] = 0;
	if (m_Selected[0] >= (int)m_Items.size())
		m_Selected[0] = int(m_Items.size()) - 1;

	if (nOldSelected != m_Selected[0])
	{
		/*if (m_dwStyle & MULTISELECTION)
		{
			//m_Items[m_Selected[0]]->bSelected = true;
		}*/

		nNewIndex = m_Selected[0];
		m_Selected.clear();
		m_Selected.push_back(nNewIndex);
		// Update selection start
		//m_nSelStart = m_nSelected;

		// Adjust scroll bar
		m_ScrollBar.ShowItem(m_Selected[m_Selected.size() - 1]);
	}

	m_pDialog->SendEvent(GLUF_EVENT_LISTBOX_SELECTION, true, this);
}


//--------------------------------------------------------------------------------------
/*
bool GLUFListBox::HandleKeyboard(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (!m_bEnabled || !m_bVisible)
		return false;

	// Let the scroll bar have a chance to handle it first
	if (m_ScrollBar.HandleKeyboard(uMsg, wParam, lParam))
		return true;

	switch (uMsg)
	{
	case WM_KEYDOWN:
		switch (wParam)
		{
		case VK_UP:
		case VK_DOWN:
		case VK_NEXT:
		case VK_PRIOR:
		case VK_HOME:
		case VK_END:
		{
			// If no item exists, do nothing.
			if (m_Items.size() == 0)
				return true;

			int nOldSelected = m_nSelected;

			// Adjust m_nSelected
			switch (wParam)
			{
			case VK_UP:
				--m_nSelected; break;
			case VK_DOWN:
				++m_nSelected; break;
			case VK_NEXT:
				m_nSelected += m_ScrollBar.GetPageSize() - 1; break;
			case VK_PRIOR:
				m_nSelected -= m_ScrollBar.GetPageSize() - 1; break;
			case VK_HOME:
				m_nSelected = 0; break;
			case VK_END:
				m_nSelected = int(m_Items.size()) - 1; break;
			}

			// Perform capping
			if (m_nSelected < 0)
				m_nSelected = 0;
			if (m_nSelected >= (int)m_Items.size())
				m_nSelected = int(m_Items.size()) - 1;

			if (nOldSelected != m_nSelected)
			{
				if (m_dwStyle & MULTISELECTION)
				{
					// Multiple selection

					// Clear all selection
					for (int i = 0; i < (int)m_Items.size(); ++i)
					{
						GLUFListBoxItem* pItem = m_Items[i];
						pItem->bSelected = false;
					}

					if (GetKeyState(VK_SHIFT) < 0)
					{
						// Select all items from m_nSelStart to
						// m_nSelected
						int nEnd = std::max(m_nSelStart, m_nSelected);

						for (int n = std::min(m_nSelStart, m_nSelected); n <= nEnd; ++n)
							m_Items[n]->bSelected = true;
					}
					else
					{
						m_Items[m_nSelected]->bSelected = true;

						// Update selection start
						m_nSelStart = m_nSelected;
					}
				}
				else
					m_nSelStart = m_nSelected;

				// Adjust scroll bar

				m_ScrollBar.ShowItem(m_nSelected);

				// Send notification

				m_pDialog->SendEvent(GLUF_EVENTLISTBOX_SELECTION, true, this);
			}
			return true;
		}

			// Space is the hotkey for double-clicking an item.
			//
		case VK_SPACE:
			m_pDialog->SendEvent(GLUF_EVENTLISTBOX_ITEM_DBLCLK, true, this);
			return true;
		}
		break;
	}

	return false;
}


//--------------------------------------------------------------------------------------

bool GLUFListBox::HandleMouse(UINT uMsg, const POINT& pt, WPARAM wParam, LPARAM lParam)
{
	if (!m_bEnabled || !m_bVisible)
		return false;

	// First acquire focus
	if (WM_LBUTTONDOWN == uMsg)
		if (!m_bHasFocus)
			m_pDialog->RequestFocus(this);

	// Let the scroll bar handle it first.
	if (m_ScrollBar.HandleMouse(uMsg, pt, wParam, lParam))
		return true;

	switch (uMsg)
	{
	case WM_LBUTTONDOWN:
	case WM_LBUTTONDBLCLK:
		// Check for clicks in the text area
		if (!m_Items.empty() && PtInRect(&m_rcSelection, pt))
		{
			// Compute the index of the clicked item

			int nClicked;
			if (m_nTextHeight)
				nClicked = m_ScrollBar.GetTrackPos() + (pt.y - m_rcText.top) / m_nTextHeight;
			else
				nClicked = -1;

			// Only proceed if the click falls on top of an item.

			if (nClicked >= m_ScrollBar.GetTrackPos() &&
				nClicked < (int)m_Items.size() &&
				nClicked < m_ScrollBar.GetTrackPos() + m_ScrollBar.GetPageSize())
			{
				SetCapture(GLUFGetHWND());
				m_bDrag = true;

				// If this is a double click, fire off an event and exit
				// since the first click would have taken care of the selection
				// updating.
				if (uMsg == WM_LBUTTONDBLCLK)
				{
					m_pDialog->SendEvent(GLUF_EVENTLISTBOX_ITEM_DBLCLK, true, this);
					return true;
				}

				m_nSelected = nClicked;
				if (!(wParam & MK_SHIFT))
					m_nSelStart = m_nSelected;

				// If this is a multi-selection listbox, update per-item
				// selection data.

				if (m_dwStyle & MULTISELECTION)
				{
					// Determine behavior based on the state of Shift and Ctrl

					GLUFListBoxItem* pSelItem = m_Items[m_nSelected];
					if ((wParam & (MK_SHIFT | MK_CONTROL)) == MK_CONTROL)
					{
						// Control click. Reverse the selection of this item.

						pSelItem->bSelected = !pSelItem->bSelected;
					}
					else if ((wParam & (MK_SHIFT | MK_CONTROL)) == MK_SHIFT)
					{
						// Shift click. Set the selection for all items
						// from last selected item to the current item.
						// Clear everything else.

						int nBegin = std::min(m_nSelStart, m_nSelected);
						int nEnd = std::max(m_nSelStart, m_nSelected);

						for (int i = 0; i < nBegin; ++i)
						{
							GLUFListBoxItem* pItem = m_Items[i];
							pItem->bSelected = false;
						}

						for (int i = nEnd + 1; i < (int)m_Items.size(); ++i)
						{
							GLUFListBoxItem* pItem = m_Items[i];
							pItem->bSelected = false;
						}

						for (int i = nBegin; i <= nEnd; ++i)
						{
							GLUFListBoxItem* pItem = m_Items[i];
							pItem->bSelected = true;
						}
					}
					else if ((wParam & (MK_SHIFT | MK_CONTROL)) == (MK_SHIFT | MK_CONTROL))
					{
						// Control-Shift-click.

						// The behavior is:
						//   Set all items from m_nSelStart to m_nSelected to
						//     the same state as m_nSelStart, not including m_nSelected.
						//   Set m_nSelected to selected.

						int nBegin = std::min(m_nSelStart, m_nSelected);
						int nEnd = std::max(m_nSelStart, m_nSelected);

						// The two ends do not need to be set here.

						bool bLastSelected = m_Items[m_nSelStart]->bSelected;
						for (int i = nBegin + 1; i < nEnd; ++i)
						{
							GLUFListBoxItem* pItem = m_Items[i];
							pItem->bSelected = bLastSelected;
						}

						pSelItem->bSelected = true;

						// Restore m_nSelected to the previous value
						// This matches the Windows behavior

						m_nSelected = m_nSelStart;
					}
					else
					{
						// Simple click.  Clear all items and select the clicked
						// item.


						for (int i = 0; i < (int)m_Items.size(); ++i)
						{
							GLUFListBoxItem* pItem = m_Items[i];
							pItem->bSelected = false;
						}

						pSelItem->bSelected = true;
					}
				}  // End of multi-selection case

				m_pDialog->SendEvent(GLUF_EVENTLISTBOX_SELECTION, true, this);
			}

			return true;
		}
		break;

	case WM_LBUTTONUP:
	{
		ReleaseCapture();
		m_bDrag = false;

		if (m_nSelected != -1)
		{
			// Set all items between m_nSelStart and m_nSelected to
			// the same state as m_nSelStart
			int nEnd = std::max(m_nSelStart, m_nSelected);

			for (int n = std::min(m_nSelStart, m_nSelected) + 1; n < nEnd; ++n)
				m_Items[n]->bSelected = m_Items[m_nSelStart]->bSelected;
			m_Items[m_nSelected]->bSelected = m_Items[m_nSelStart]->bSelected;

			// If m_nSelStart and m_nSelected are not the same,
			// the user has dragged the mouse to make a selection.
			// Notify the application of this.
			if (m_nSelStart != m_nSelected)
				m_pDialog->SendEvent(GLUF_EVENTLISTBOX_SELECTION, true, this);

			m_pDialog->SendEvent(GLUF_EVENTLISTBOX_SELECTION_END, true, this);
		}
		return false;
	}

	case WM_MOUSEMOVE:
		if (m_bDrag)
		{
			// Compute the index of the item below cursor

			int nItem;
			if (m_nTextHeight)
				nItem = m_ScrollBar.GetTrackPos() + (pt.y - m_rcText.top) / m_nTextHeight;
			else
				nItem = -1;

			// Only proceed if the cursor is on top of an item.

			if (nItem >= (int)m_ScrollBar.GetTrackPos() &&
				nItem < (int)m_Items.size() &&
				nItem < m_ScrollBar.GetTrackPos() + m_ScrollBar.GetPageSize())
			{
				m_nSelected = nItem;
				m_pDialog->SendEvent(GLUF_EVENTLISTBOX_SELECTION, true, this);
			}
			else if (nItem < (int)m_ScrollBar.GetTrackPos())
			{
				// User drags the mouse above window top
				m_ScrollBar.Scroll(-1);
				m_nSelected = m_ScrollBar.GetTrackPos();
				m_pDialog->SendEvent(GLUF_EVENTLISTBOX_SELECTION, true, this);
			}
			else if (nItem >= m_ScrollBar.GetTrackPos() + m_ScrollBar.GetPageSize())
			{
				// User drags the mouse below window bottom
				m_ScrollBar.Scroll(1);
				m_nSelected = std::min((int)m_Items.size(), m_ScrollBar.GetTrackPos() +
					m_ScrollBar.GetPageSize()) - 1;
				m_pDialog->SendEvent(GLUF_EVENTLISTBOX_SELECTION, true, this);
			}
		}
		break;

	case WM_MOUSEWHEEL:
	{
		UINT uLines = 0;
		if (!SystemParametersInfo(SPI_GETWHEELSCROLLLINES, 0, &uLines, 0))
			uLines = 0;
		int nScrollAmount = int((short)HIWORD(wParam)) / WHEEL_DELTA * uLines;
		m_ScrollBar.Scroll(-nScrollAmount);
		return true;
	}
	}

	return false;
}
*/

//--------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------
bool GLUFListBox::MsgProc(GLUFMessageType msg, int32_t param1, int32_t param2, int32_t param3, int32_t param4)
{

	if (GM_FOCUS == msg && param1 == GL_FALSE)
	{
		// The application just lost mouse capture. We may not have gotten
		// the WM_MOUSEUP message, so reset m_bDrag here.
		m_bDrag = false;
	}

	GLUFPoint pt = m_pDialog->m_MousePositionDialogSpace;

	if (!m_bEnabled || !m_bVisible)
		return false;

	// First acquire focus
	if (GM_MB == msg && param1 == GLFW_MOUSE_BUTTON_LEFT && param2 == GLFW_PRESS)
		if (!m_bHasFocus)
			m_pDialog->RequestFocus(this);

	// Let the scroll bar have a chance to handle it first
	if (m_ScrollBar.MsgProc(GLUF_PASS_CALLBACK_PARAM))
		return true;

	switch (msg)
	{
	case GM_KEY:
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
				if (m_Items.size() == 0)
					return true;

				int nOldSelected = m_Selected[0];
				m_Selected.clear();
				m_Selected.push_back(nOldSelected);

				// Adjust m_nSelected
				switch (param1)
				{
				case GLFW_KEY_UP:
					--m_Selected[0]; break;
				case GLFW_KEY_DOWN:
					++m_Selected[0]; break;
				case GLFW_KEY_PAGE_DOWN:
					m_Selected[0] += m_ScrollBar.GetPageSize() - 1; break;
				case GLFW_KEY_PAGE_UP:
					m_Selected[0] -= m_ScrollBar.GetPageSize() - 1; break;
				case GLFW_KEY_HOME:
					m_Selected[0] = 0; break;
				case GLFW_KEY_END:
					m_Selected[0] = int(m_Items.size()) - 1; break;
				}

				// Perform capping
				if (m_Selected[0] < 0)
					m_Selected[0] = 0;
				if (m_Selected[0] >= (int)m_Items.size())
					m_Selected[0] = int(m_Items.size()) - 1;

				if (nOldSelected != m_Selected[0])
				{
					if (m_dwStyle & MULTISELECTION)
					{
						// Multiple selection

						// Clear all selection
						/*for (int i = 0; i < (int)m_Items.size(); ++i)
						{
							GLUFListBoxItem* pItem = m_Items[i];
							pItem->bSelected = false;
						}

						m_Selected.clear();

						if (param4 & GLFW_MOD_SHIFT)
						{
							// Select all items from m_nSelStart to
							// m_nSelected
							int nEnd = m_Selected[m_Selected.size() - 1];//std::max(m_nSelStart, m_nSelected);

							for (int n = m_Selected[0]; n <= nEnd; ++n)
								m_Items[n]->bSelected = true;
						}
						else
						{
							m_Items[m_Selected[0]]->bSelected = true;

							// Update selection start
							//m_nSelStart = m_nSelected;
						}*/

						//TODO: key callback
					}
					else;
						//m_nSelStart = m_nSelected;

					// Adjust scroll bar

					m_ScrollBar.ShowItem(m_Selected[0]);

					// Send notification

					m_pDialog->SendEvent(GLUF_EVENT_LISTBOX_SELECTION, true, this);
				}
				return true;
			}

				// Space is the hotkey for double-clicking an item.
				//
			case GLFW_KEY_SPACE:
				m_pDialog->SendEvent(GLUF_EVENT_LISTBOX_ITEM_DBLCLK, true, this);
				return true;
			}
		}
		break;
	//case WM_LBUTTONDOWN:
	//case WM_LBUTTONDBLCLK:
	case GM_MB:
		if (param2 == GLFW_PRESS)
		{
			// Check for clicks in the text area
			if (!m_Items.empty() && GLUFPtInRect(m_rcSelection, pt))
			{
				// Compute the index of the clicked item

				//int nClicked;
				//m_pDialog->GetManager()->GetFontNode(p
				//nClicked = int(m_ScrollBar.GetTrackPos() + (pt.y - m_rcText.top) / m_fTextHeight);

				// Only proceed if the click falls on top of an item.

				/*if (nClicked >= m_ScrollBar.GetTrackPos() &&
					nClicked < (int)m_Items.size() &&
					nClicked < m_ScrollBar.GetTrackPos() + m_ScrollBar.GetPageSize())
				{
					//SetCapture(GLUFGetHWND());
					m_bDrag = true;

					// If this is a double click, fire off an event and exit
					// since the first click would have taken care of the selection
					// updating.
					//TODO: handle doubleclicking
					if (uMsg == WM_LBUTTONDBLCLK)
					{
						m_pDialog->SendEvent(GLUF_EVENT_LISTBOX_ITEM_DBLCLK, true, this);
						return true;
					}

					m_nSelected = nClicked;
					if (!(param3 & GLFW_MOD_SHIFT))
						m_nSelStart = m_nSelected;*/

					// If this is a multi-selection listbox, update per-item
					// selection data.

				int currSelectedIndex = -1;

				//the easy way
				for (unsigned int it = 0; it < m_Items.size(); ++it)
				{
					if (GLUFPtInRect(m_Items[it]->rcActive, pt))
					{
						currSelectedIndex = it;
						break;
					}
				}

				if (m_dwStyle & MULTISELECTION)
				{
					// Determine behavior based on the state of Shift and Ctrl

					//GLUFListBoxItem* pSelItem = m_Items[currSelectedIndex];
					if (param3 & GLFW_MOD_CONTROL)
					{
						// Control click. Reverse the selection of this item.

						//pSelItem->bSelected = !pSelItem->bSelected;
						std::vector<int>::iterator it = std::find(m_Selected.begin(), m_Selected.end(), currSelectedIndex);
						if (it == m_Selected.end())
						{
							m_Selected.push_back(currSelectedIndex);
						}							
						else
						{
							m_Selected.erase(it);//this should never fail

							//make sure that if it is the last one, then add the -1
							if (m_Selected.size() == 0)
							{
								m_Selected.push_back(-1);
							}
						}
					}
					else if (param3 & GLFW_MOD_SHIFT)
					{
						// Shift click. Set the selection for all items
						// from last selected item to the current item.
						// Clear everything else.

						if (m_Selected[0] == -1)
							m_Selected[0] = 0;//this just fixes any issues with accidently keeping -1 in here

						int nBegin = m_Selected[0];
						int nEnd = currSelectedIndex;

						m_Selected.clear();

						/*for (int i = 0; i < nBegin; ++i)
						{
							GLUFListBoxItem* pItem = m_Items[i];
							pItem->bSelected = false;
						}

						for (int i = nEnd + 1; i < (int)m_Items.size(); ++i)
						{
							GLUFListBoxItem* pItem = m_Items[i];
							pItem->bSelected = false;
						}*/

						if (nBegin < nEnd)
						{
							for (int i = nBegin; i <= nEnd; ++i)
							{
								/*GLUFListBoxItem* pItem = m_Items[i];
								pItem->bSelected = true;
								*/
								m_Selected.push_back(i);
							}
						}
						else if (nBegin > nEnd)
						{
							for (int i = nBegin; i >= nEnd; --i)
							{
								/*GLUFListBoxItem* pItem = m_Items[i];
								pItem->bSelected = true;
								*/
								m_Selected.push_back(i);
							}
						}
						else
						{
							m_Selected.push_back(-1);
						}
					}
					else if (param3 & (GLFW_MOD_SHIFT | GLFW_MOD_CONTROL))
					{
						//No one uses shift control anyway (i see no use in it

						// Control-Shift-click.

						// The behavior is:
						//   Set all items from m_nSelStart to m_nSelected to
						//     the same state as m_nSelStart, not including m_nSelected.
						//   Set m_nSelected to selected.

						/*int nBegin = m_Selected[0];
						int nEnd = currSelectedIndex;
						m_Selected.clear();

						// The two ends do not need to be set here.

						bool bLastSelected = m_Items[nBegin]->bSelected;
						for (int i = nBegin + 1; i < nEnd; ++i)
						{
							GLUFListBoxItem* pItem = m_Items[i];
							pItem->bSelected = bLastSelected;

							m_Selected.push_back(i);
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


						/*for (int i = 0; i < (int)m_Items.size(); ++i)
						{
							GLUFListBoxItem* pItem = m_Items[i];
							pItem->bSelected = false;
						}

						pSelItem->bSelected = true;
						*/

						m_Selected.clear();
						m_Selected.push_back(currSelectedIndex);

						//NOTE: clicking not on an item WILL lead to a clearing of the selection
					}
				}  // End of multi-selection case
				else
				{
					m_Selected[0] = currSelectedIndex;
				}

				//always make sure we have one
				if (m_Selected.size() == 0)
					m_Selected.push_back(-1);

				//sort it for proper functionality when using shift-clicking (NOT HELPFUL)
				//std::sort(m_Selected.begin(), m_Selected.end());

				m_pDialog->SendEvent(GLUF_EVENT_LISTBOX_SELECTION, true, this);
				

				return true;
			}
			break;
		}
		else
		{
			//TODO: drag click
			//ReleaseCapture();
			/*m_bDrag = false;

			if (m_Selected[0] != -1)
			{
				// Set all items between m_nSelStart and m_nSelected to
				// the same state as m_nSelStart
				int nEnd = m_Selected[m_Selected.size() - 1];

				for (int n = m_Selected[0] + 1; n < nEnd; ++n)
					m_Items[n]->bSelected = m_Items[m_Selected[0]]->bSelected;
				m_Items[m_Selected[0]]->bSelected = m_Items[m_Selected[0]]->bSelected;

				// If m_nSelStart and m_nSelected are not the same,
				// the user has dragged the mouse to make a selection.
				// Notify the application of this.
				if (m_Selected[0] != m_Selected[m_Selected.size() - 1])
					m_pDialog->SendEvent(GLUF_EVENT_LISTBOX_SELECTION, true, this);

				m_pDialog->SendEvent(GLUF_EVENT_LISTBOX_SELECTION_END, true, this);
			}*/
			return false;
		}

	case GM_CURSOR_POS:
		/*if (m_bDrag)
		{
			// Compute the index of the item below cursor

			int nItem = -1;
			for (unsigned int it = 0; it < m_Items.size(); ++it)
			{
				if (GLUFPtInRect(m_Items[it]->rcActive, pt))
				{
					nItem = it;
					break;
				}
			}

			// Only proceed if the cursor is on top of an item.

			if (nItem >= (int)m_ScrollBar.GetTrackPos() &&
				nItem < (int)m_Items.size() &&
				nItem < m_ScrollBar.GetTrackPos() + m_ScrollBar.GetPageSize())
			{
				m_Selected[0] = nItem;
				m_pDialog->SendEvent(GLUF_EVENT_LISTBOX_SELECTION, true, this);
			}
			else if (nItem < (int)m_ScrollBar.GetTrackPos())
			{
				// User drags the mouse above window top
				m_ScrollBar.Scroll(-1);
				m_Selected[0] = m_ScrollBar.GetTrackPos();
				m_pDialog->SendEvent(GLUF_EVENT_LISTBOX_SELECTION, true, this);
			}
			else if (nItem >= m_ScrollBar.GetTrackPos() + m_ScrollBar.GetPageSize())
			{
				// User drags the mouse below window bottom
				m_ScrollBar.Scroll(1);
				m_Selected[0] = std::min((int)m_Items.size(), m_ScrollBar.GetTrackPos() +
					m_ScrollBar.GetPageSize()) - 1;
				m_pDialog->SendEvent(GLUF_EVENT_LISTBOX_SELECTION, true, this);
			}
		}*/
		break;

	case GM_SCROLL:
		//UINT uLines = 0;
		//if (!SystemParametersInfo(SPI_GETWHEELSCROLLLINES, 0, &uLines, 0))
		//	uLines = 0;
		//int nScrollAmount = int((short)HIWORD(wParam)) / WHEEL_DELTA * uLines;
		m_ScrollBar.Scroll(-(param2 / WHEEL_DELTA));
		return true;
	}

	return false;
}

//--------------------------------------------------------------------------------------
void GLUFListBox::UpdateItemRects()
{
	GLUFFontNode* pFont = m_pDialog->GetFont(GetElement(0)->iFont);
	if (pFont)
	{
		int curY = m_rcText.top - pFont->m_Leading / 2;// +((m_ScrollBar.GetTrackPos() - 1) * pFont->mSize);
		int nRemainingHeight = GLUFRectHeight(m_rcBoundingBox) - pFont->m_Leading;


		//int nRemainingHeight = GLUFRectHeight(m_rcBoundingBox) - pFont->m_Leading;

		//for all of the ones before the displayed, just set them to something impossible
		for (size_t i = 0; i < (size_t)m_ScrollBar.GetTrackPos(); ++i)
		{
			GLUFSetRect(m_Items[i]->rcActive, 0, 0, 0, 0);
		}
		for (size_t i = m_ScrollBar.GetTrackPos(); i < m_Items.size(); i++)
		{
			GLUFListBoxItem* pItem = m_Items[i];

			// Make sure there's room left in the box
			nRemainingHeight -= pFont->m_Leading;
			if (nRemainingHeight - m_fBorder <= 0)
			{
				pItem->bVisible = false;
				continue;
			}

			pItem->bVisible = true;

			GLUFSetRect(pItem->rcActive, m_rcBoundingBox.left + m_fMargin, curY, m_rcBoundingBox.right - m_fMargin, curY - pFont->m_pFontType->mHeight);
			curY -= pFont->m_Leading;
		}
	}
}

//--------------------------------------------------------------------------------------
void GLUFListBox::Render( float elapsedTime)
{
	if (m_bVisible == false)
		return;

	GLUFElement* pElement = m_Elements[0];
	pElement->TextureColor.Blend(GLUF_STATE_NORMAL, elapsedTime);
	pElement->FontColor.Blend(GLUF_STATE_NORMAL, elapsedTime);

	GLUFElement* pSelElement = m_Elements[1];
	pSelElement->TextureColor.Blend(GLUF_STATE_NORMAL, elapsedTime);
	pSelElement->FontColor.Blend(GLUF_STATE_NORMAL, elapsedTime);

	m_pDialog->DrawSprite(pElement, m_rcBoundingBox, GLUF_FAR_BUTTON_DEPTH);

		GLUFFontNode* pFont = m_pDialog->GetManager()->GetFontNode(pElement->iFont);
	// Render the text
	if (!m_Items.empty() && pFont)
	{

		UpdateItemRects();

		static bool bSBInit;
		if (!bSBInit)
		{
			// Update the page size of the scroll bar
			if (m_fTextHeight > 0)
				m_ScrollBar.SetPageSize(int((GLUFRectHeight(m_rcBoundingBox) - (2 * m_fBorder)) / m_fTextHeight) + 1);
			else
				m_ScrollBar.SetPageSize(0);
			bSBInit = true;
		}

		for (int i = m_ScrollBar.GetTrackPos(); i < (int)m_Items.size(); ++i)
		{

			GLUFListBoxItem* pItem = m_Items[i];

			if (!pItem->bVisible)
				continue;

			// Determine if we need to render this item with the
			// selected element.
			bool bSelectedStyle = false;

			//m_Items[i]->bSelected = false;

			if (!(m_dwStyle & MULTISELECTION) && i == m_Selected[0])
				bSelectedStyle = true;
			else if (m_dwStyle & MULTISELECTION)
			{
				for (auto it : m_Selected)
					if (i == it)
						bSelectedStyle = true;

					
				/*if (m_bDrag &&
					((i >= m_Selected[0] && i < m_Selected[0]) ||
					(i <= m_Selected[0] && i > m_Selected[0])))
					bSelectedStyle = m_Items[m_Selected[0]]->bSelected;
				else if (pItem->bSelected)
					bSelectedStyle = true;*/
			}

			//bSelectedStyle = m_Items[i]->bSelected;
			if (bSelectedStyle)
			{
				//rcSel.top = rc.top; rcSel.bottom = rc.bottom;
				m_pDialog->DrawSprite(pSelElement, pItem->rcActive, GLUF_NEAR_BUTTON_DEPTH);
				m_pDialog->DrawText(pItem->strText, pSelElement, pItem->rcActive);
			}
			else
				m_pDialog->DrawText(pItem->strText, pElement, pItem->rcActive);

			//GLUFOffsetRect(rc, 0, m_fTextHeight);
		}

	}

	// Render the scroll bar

	m_ScrollBar.Render(elapsedTime);
}


const wchar_t *g_Charsets[] = { 
	L" !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~",
	L" !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~ÄÅÇÉÑÖÜáàâäãåçéèêëíìîïñóòôöõúùûü†°¢£§•¶ß®©™´¨≠ÆØ∞±≤≥¥µ∂∑∏π∫ªºΩæø¿¡¬√ƒ≈∆«»… ÀÃÕŒœ–—“”‘’÷◊ÿŸ⁄€‹›ﬁﬂ‡·‚„‰ÂÊÁËÈÍÎÏÌÓÔÒÚÛÙıˆ˜¯˘˙˚¸˝˛ˇ",
	L"0123456789",
	L"abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ",
	L"abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789"};

const unsigned int g_CharsetLengths[] = { 96, 223, 10, 52, 62 };

bool CharsetContains(unsigned int codepoint, Charset charset)
{
	switch (charset)
	{
	case Unicode:
		return true;
	default:
		for (unsigned int i = 0; i < g_CharsetLengths[charset]; ++i)
		{
			if (g_Charsets[charset][i] == codepoint)
				return true;
		}
		return false;
	}
}


//======================================================================================
// GLUFEditBox class
//======================================================================================

// Static member initialization
bool GLUFEditBox::s_bHideCaret;   // If true, we don't render the caret.

// When scrolling, EDITBOX_SCROLLEXTENT is reciprocal of the amount to scroll.
// If EDITBOX_SCROLLEXTENT = 4, then we scroll 1/4 of the control each time.
#define EDITBOX_SCROLLEXTENT 4

//--------------------------------------------------------------------------------------
GLUFEditBox::GLUFEditBox(Charset charset, bool isMultiline, GLUFDialog* pDialog) : GLUFControl(pDialog), m_ScrollBar(pDialog), m_bMultiline(isMultiline), m_Charset(charset)
{
	m_Type = GLUF_CONTROL_EDITBOX;
	m_pDialog = pDialog;

	m_fBorder  = 5;  // Default border width
	m_fSpacing = 4;  // Default spacing

	m_bCaretOn = true;
	m_dfBlink = GetCaretBlinkTime() * 0.001f;
	m_dfLastBlink = glfwGetTime();
	s_bHideCaret = false;
	m_nFirstVisible = 0;
	m_TextColor = Color(16, 16, 16, 255);
	m_SelTextColor = Color(255, 255, 255, 255);
	m_SelBkColor = Color(40, 50, 92, 255);
	m_CaretColor = Color(0, 0, 0, 255);
	m_nCaret = m_nSelStart = 0;
	m_bInsertMode = true;

	m_fSBWidth = 16;
	m_bMouseDrag = false;

	m_bAnalyseRequired = true;
}


//--------------------------------------------------------------------------------------
GLUFEditBox::~GLUFEditBox()
{
}


//--------------------------------------------------------------------------------------
// PlaceCaret: Set the caret to a character position, and adjust the scrolling if
//             necessary.
//--------------------------------------------------------------------------------------
void GLUFEditBox::PlaceCaret( int nCP)
{
	if (m_bMultiline)
	{
		if (nCP == -1)
		{
			m_ScrollBar.SetTrackPos(0);//top
			m_nCaret = nCP;
		}
		else if (nCP >= GetTextLength())
		{
			//anything past this, set to the max
			m_nCaret = GetTextLength() - 1;
		}
		else
		{
			//GLUF_ASSERT(nCP >= 0 && nCP <= GetTextLength());
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
				m_ScrollBar.Scroll(m_ScrollBar.GetPageSize() - 1);
				Analyse();

				rendCaret = GetStrRenderIndexFromStrIndex(nCP);
			}

			int line = GetLineNumberFromCharPos(rendCaret);

			if (line < 0 || line >= m_ScrollBar.GetPageSize())
			{
				m_ScrollBar.SetTrackPos(line);
			}
			/*if (line < m_ScrollBar.GetTrackPos() || line > m_ScrollBar.GetTrackPos() + m_ScrollBar.GetPageSize() - 1)
				m_ScrollBar.SetTrackPos(line);*/
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
			/*m_ScrollBar.SetTrackRange(0, GetTextLength());
			if (nCP >= m_ScrollBar.GetTrackPos() + m_ScrollBar.GetPageSize())
			m_ScrollBar.Scroll(delta);
			else if (nCP < m_ScrollBar.GetTrackPos())
			m_ScrollBar.SetTrackPos(nCP);*/

			m_nCaret = nCP;

			//Analyse();
			/*
			if (m_nCaret >= m_ScrollBar.GetTrackPos() + m_ScrollBar.GetPageSize())
			m_ScrollBar.Scroll(-((m_ScrollBar.GetTrackPos() + m_ScrollBar.GetPageSize()) - m_nCaret));
			else if (m_nCaret < m_ScrollBar.GetTrackPos())
			m_ScrollBar.SetTrackPos(m_nCaret);*/
		}

		//int rendCaret = GetStrRenderIndexFromStrIndex(m_nCaret);
		Analyse();
	}
}

void GLUFEditBox::PlaceCaretRndBuffer(int nRndCp)
{
	m_nCaret = GetStrIndexFromStrRenderIndex(nRndCp);
	PlaceCaret(m_nCaret);
}

int GLUFEditBox::GetLineNumberFromCharPos(unsigned int nCP)
{
	int nCPModified = nCP;
	int lin = 0;
	for (auto it : m_strInsertedNewlineLocations)
		if (nCP >= it)
			--nCPModified;
	
	for (unsigned int i = 0; i < nCPModified + m_strRenderBufferOffset; ++i)
		if (m_strBuffer[i] == '\n')
			++lin;

	/*for (auto it : m_strInsertedNewlineLocations)
		if (nCP >= it)
			++lin;*/
	//add the difference
	lin += nCP - nCPModified;

	if (m_strRenderBuffer[nCP] == '\n')
		++lin;

	return lin;
}



//--------------------------------------------------------------------------------------
void GLUFEditBox::ClearText()
{
	m_strBuffer.clear();
	m_nFirstVisible = 0;
	PlaceCaret(0);
	m_nSelStart = 0;

	m_bAnalyseRequired = true;
}


//--------------------------------------------------------------------------------------
void GLUFEditBox::SetText( std::wstring wszText,  bool bSelected)
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
void GLUFEditBox::DeleteSelectionText()
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
void GLUFEditBox::UpdateRects()
{
	GLUFControl::UpdateRects();

	// Update the text rectangle
	m_rcText = m_rcBoundingBox;
	// First inflate by m_nBorder to compute render rects
	GLUFInflateRect(m_rcText, -m_fBorder, -m_fBorder);
	
	m_rcText.right -= m_fSBWidth;

	// Update the render rectangles
	m_rcRender[0] = m_rcText;
	GLUFSetRect(m_rcRender[1], m_rcBoundingBox.left, m_rcBoundingBox.top, m_rcText.left, m_rcText.top);
	GLUFSetRect(m_rcRender[2], m_rcText.left, m_rcBoundingBox.top, m_rcText.right, m_rcText.top);
	GLUFSetRect(m_rcRender[3], m_rcText.right, m_rcBoundingBox.top, m_rcBoundingBox.right, m_rcText.top);
	GLUFSetRect(m_rcRender[4], m_rcBoundingBox.left, m_rcText.top, m_rcText.left, m_rcText.bottom);
	GLUFSetRect(m_rcRender[5], m_rcText.right, m_rcText.top, m_rcBoundingBox.right, m_rcText.bottom);
	GLUFSetRect(m_rcRender[6], m_rcBoundingBox.left, m_rcText.bottom, m_rcText.left, m_rcBoundingBox.bottom);
	GLUFSetRect(m_rcRender[7], m_rcText.left, m_rcText.bottom, m_rcText.right, m_rcBoundingBox.bottom);
	GLUFSetRect(m_rcRender[8], m_rcText.right, m_rcText.bottom, m_rcBoundingBox.right, m_rcBoundingBox.bottom);

	GLUFFontNode* pFontNode = m_pDialog->GetManager()->GetFontNode(m_Elements[0]->iFont);

	// Inflate further by m_nSpacing
	GLUFInflateRect(m_rcText, -m_fSpacing, -m_fSpacing);	

	m_ScrollBar.SetLocation(m_rcText.right, m_rcBoundingBox.bottom);
	m_ScrollBar.SetSize(m_fSBWidth, GLUFRectHeight(m_rcBoundingBox));
	m_ScrollBar.m_y = m_rcText.top;


	//Analyse();
	m_bAnalyseRequired = true;
}

#pragma warning(disable : 4018)
#pragma warning(push)
#pragma warning( disable : 4616 6386 )
void GLUFEditBox::CopyToClipboard()
{
	// Copy the selection text to the clipboard
	if (m_nCaret != m_nSelStart/* && OpenClipboard(nullptr)*/)
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
			GlobalFree(hBlock);*/

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

void GLUFEditBox::PasteFromClipboard()
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
				}*/

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


void GLUFEditBox::InsertString(int pos, std::wstring str)
{
	for (auto it : str)
	{
		if (!CharsetContains(it, m_Charset))
			return;
	}
	//GLUF_ASSERT(pos < GetTextLength() - 1);
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

void GLUFEditBox::RemoveString(int pos, int len)
{
	//GLUF_ASSERT(pos + len < GetTextLength() - 1);

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

void GLUFEditBox::InsertChar(int pos, wchar_t ch)
{
	if (!CharsetContains(ch, m_Charset))
		return;
	//GLUF_ASSERT(pos < GetTextLength() - 1);
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

void GLUFEditBox::RemoveChar(int pos)
{
	//GLUF_ASSERT(pos < GetTextLength() - 1);
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

void GLUFEditBox::GetNextItemPos(int pos, int& next)
{
	if (pos == m_strBuffer.size())
		next = pos;
	else
		next = pos + 1;
}

void GLUFEditBox::GetPriorItemPos(int pos, int& prior)
{
	if (pos == 0)
		prior = pos;
	else
		prior = pos - 1;
}

int GLUFEditBox::GetNumNewlines()
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

int GLUFEditBox::GetStrIndexFromStrRenderIndex(int strRenderIndex)
{
	GLUF_ASSERT(strRenderIndex < m_strRenderBuffer.length());

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

int GLUFEditBox::GetStrRenderIndexFromStrIndex(int strIndex)
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
	for (unsigned int i = 0; i+offset < m_strRenderBuffer.length()/*this is usually hit first*/; ++i)
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
/*
bool GLUFEditBox::HandleKeyboard(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);

	if (!m_bEnabled || !m_bVisible)
		return false;

	bool bHandled = false;

	switch (uMsg)
	{
	case WM_KEYDOWN:
	{
		switch (wParam)
		{
		case VK_TAB:
			// We don't process Tab in case keyboard input is enabled and the user
			// wishes to Tab to other controls.
			break;

		case VK_HOME:
			PlaceCaret(0);
			if (GetKeyState(VK_SHIFT) >= 0)
				// Shift is not down. Update selection
				// start along with the caret.
				m_nSelStart = m_nCaret;
			ResetCaretBlink();
			bHandled = true;
			break;

		case VK_END:
			PlaceCaret(m_Buffer.GetTextSize());
			if (GetKeyState(VK_SHIFT) >= 0)
				// Shift is not down. Update selection
				// start along with the caret.
				m_nSelStart = m_nCaret;
			ResetCaretBlink();
			bHandled = true;
			break;

		case VK_INSERT:
			if (GetKeyState(VK_CONTROL) < 0)
			{
				// Control Insert. Copy to clipboard
				CopyToClipboard();
			}
			else if (GetKeyState(VK_SHIFT) < 0)
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

		case VK_DELETE:
			// Check if there is a text selection.
			if (m_nCaret != m_nSelStart)
			{
				DeleteSelectionText();
				m_pDialog->SendEvent(GLUF_EVENTEDITBOX_CHANGE, true, this);
			}
			else
			{
				// Deleting one character
				if (m_Buffer.RemoveChar(m_nCaret))
					m_pDialog->SendEvent(GLUF_EVENTEDITBOX_CHANGE, true, this);
			}
			ResetCaretBlink();
			bHandled = true;
			break;

		case VK_LEFT:
			if (GetKeyState(VK_CONTROL) < 0)
			{
				// Control is down. Move the caret to a new item
				// instead of a character.
				m_Buffer.GetPriorItemPos(m_nCaret, &m_nCaret);
				PlaceCaret(m_nCaret);
			}
			else if (m_nCaret > 0)
				PlaceCaret(m_nCaret - 1);
			if (GetKeyState(VK_SHIFT) >= 0)
				// Shift is not down. Update selection
				// start along with the caret.
				m_nSelStart = m_nCaret;
			ResetCaretBlink();
			bHandled = true;
			break;

		case VK_RIGHT:
			if (GetKeyState(VK_CONTROL) < 0)
			{
				// Control is down. Move the caret to a new item
				// instead of a character.
				m_Buffer.GetNextItemPos(m_nCaret, &m_nCaret);
				PlaceCaret(m_nCaret);
			}
			else if (m_nCaret < m_Buffer.GetTextSize())
				PlaceCaret(m_nCaret + 1);
			if (GetKeyState(VK_SHIFT) >= 0)
				// Shift is not down. Update selection
				// start along with the caret.
				m_nSelStart = m_nCaret;
			ResetCaretBlink();
			bHandled = true;
			break;

		case VK_UP:
		case VK_DOWN:
			// Trap up and down arrows so that the dialog
			// does not switch focus to another control.
			bHandled = true;
			break;

		default:
			bHandled = wParam != VK_ESCAPE;  // Let the application handle Esc.
		}
	}
	}
	return bHandled;
}


//--------------------------------------------------------------------------------------

bool GLUFEditBox::HandleMouse(UINT uMsg, const POINT& pt, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(wParam);
	UNREFERENCED_PARAMETER(lParam);

	if (!m_bEnabled || !m_bVisible)
		return false;

	switch (uMsg)
	{
	case WM_LBUTTONDOWN:
	case WM_LBUTTONDBLCLK:
	{
		if (!m_bHasFocus)
			m_pDialog->RequestFocus(this);

		if (!ContainsPoint(pt))
			return false;

		m_bMouseDrag = true;
		SetCapture(GLUFGetHWND());
		// Determine the character corresponding to the coordinates.
		int nCP, nTrail, nX1st;
		m_Buffer.CPtoX(m_nFirstVisible, FALSE, &nX1st);  // X offset of the 1st visible char
		if (m_Buffer.XtoCP(pt.x - m_rcText.left + nX1st, &nCP, &nTrail))
		{
			// Cap at the nul character.
			if (nTrail && nCP < m_Buffer.GetTextSize())
				PlaceCaret(nCP + 1);
			else
				PlaceCaret(nCP);
			m_nSelStart = m_nCaret;
			ResetCaretBlink();
		}
		return true;
	}

	case WM_LBUTTONUP:
		ReleaseCapture();
		m_bMouseDrag = false;
		break;

	case WM_MOUSEMOVE:
		if (m_bMouseDrag)
		{
			// Determine the character corresponding to the coordinates.
			int nCP, nTrail, nX1st;
			m_Buffer.CPtoX(m_nFirstVisible, FALSE, &nX1st);  // X offset of the 1st visible char
			if (m_Buffer.XtoCP(pt.x - m_rcText.left + nX1st, &nCP, &nTrail))
			{
				// Cap at the nul character.
				if (nTrail && nCP < m_Buffer.GetTextSize())
					PlaceCaret(nCP + 1);
				else
					PlaceCaret(nCP);
			}
		}
		break;
	}

	return false;
}
*/

//--------------------------------------------------------------------------------------
void GLUFEditBox::OnFocusIn()
{
	GLUFControl::OnFocusIn();

	ResetCaretBlink();

	m_bAnalyseRequired = true;
}


//--------------------------------------------------------------------------------------

bool GLUFEditBox::MsgProc(GLUFMessageType msg, int32_t param1, int32_t param2, int32_t param3, int32_t param4)
{

	// Let the scroll bar have a chance to handle it first
	if (m_ScrollBar.MsgProc(GLUF_PASS_CALLBACK_PARAM) && m_bMultiline)//if we are single line, the scroll bar is only here to give us auto scroll
	{
		m_bAnalyseRequired = true;
		return true;
	}
	
	//UNREFERENCED_PARAMETER(lParam);

	if (!m_bEnabled || !m_bVisible)
		return false;

	GLUFPoint pt = m_pDialog->m_MousePositionDialogSpace;

	bool bHandled = false;

	switch (msg)
	{
	case GM_MB:
		//case WM_LBUTTONDBLCLK:

		if (!m_bHasFocus)
			m_pDialog->RequestFocus(this);

		if (param2 == GLFW_PRESS && m_strRenderBuffer.length())
		{

			if (!ContainsPoint(pt))
				return false;
			m_bAnalyseRequired = true;

			m_bMouseDrag = true;
			//SetCapture(GLUFGetHWND());
			// Determine the character corresponding to the coordinates.
			int nCP;
			bool bTrail;
			//m_Buffer.CPtoX(m_nFirstVisible, false, &nX1st);  // X offset of the 1st visible char
			if (PttoCP(GLUFPoint(pt.x - m_rcText.left, pt.y - m_rcText.bottom), &nCP, &bTrail))
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
			else if (   pt.y - m_rcText.bottom < m_CharBoundingBoxes[m_strRenderBuffer.length() - 1].bottom || (
						pt.x - m_rcText.left   > m_CharBoundingBoxes[m_strRenderBuffer.length() - 1].right && 
						pt.y - m_rcText.bottom < m_CharBoundingBoxes[m_strRenderBuffer.length() - 1].top && 
						pt.y - m_rcText.bottom > m_CharBoundingBoxes[m_strRenderBuffer.length() - 1].bottom))
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
	case GM_CURSOR_POS:
		if (m_bMouseDrag)
		{
			// Determine the character corresponding to the coordinates.
			//int nCP, nX1st;
			//bool bTrail;
			/*m_Buffer.CPtoX(m_nFirstVisible, false, &nX1st);  // X offset of the 1st visible char
			if (m_Buffer.XtoCP(pt.x - m_rcText.left + nX1st, &nCP, &bTrail))
			{
				// Cap at the nul character.
				if (bTrail && nCP < m_Buffer.GetTextSize())
					PlaceCaret(nCP + 1);
				else
					PlaceCaret(nCP);
			}*/

			//m_bAnalyseRequired = true;
		}
		break;
	case GM_SCROLL:
		if (!m_bMultiline)
			break;

		if (!m_bHasFocus)
			m_pDialog->RequestFocus(this);

		m_ScrollBar.Scroll(-(param2 / WHEEL_DELTA) / 2);
		m_bAnalyseRequired = true;

		bHandled = true;
		break;
	case GM_UNICODE_CHAR:

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
		m_pDialog->SendEvent(GLUF_EVENT_EDITBOX_CHANGE, true, this);

		bHandled = true;
		break;
	case GM_KEY:
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
					m_pDialog->SendEvent(GLUF_EVENT_EDITBOX_CHANGE, true, this);
				}
				else
				{
					// Deleting one character
					if (m_nCaret != m_strBuffer.length() - 1)
					{
						RemoveChar(m_nCaret + 1);
						m_pDialog->SendEvent(GLUF_EVENT_EDITBOX_CHANGE, true, this);
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
				GLUFRect rc = { 0L, 0L, 0L, 0L };


				GLUFFontNode* pFont = m_pDialog->GetFont(GetElement(0)->iFont);

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

						*/

						//THERE IS NOT CURRENTLY SUPPORT FOR THIS FEATURE

						/*std::vector<std::wstring> lines = GLUFSplitStr(m_strRenderBuffer, '\n', true);
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
								width += pFont->m_pFontType->GetCharAdvance(m_strRenderBuffer[it]);
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
							
						}*/

					}

					if (CPtoRC(rndCP, &rc))
					{
						//is this the top line?
						if ((bIsKeyUp ? rc.top == GLUFRectHeight(m_rcText) : rc.bottom < (long)pFont->m_Leading))
						{
							m_ScrollBar.Scroll(bIsKeyUp ? -1 : 1);
							Analyse();
							rndCP = GetStrRenderIndexFromStrIndex(m_nCaret);
							if (!CPtoRC(rndCP, &rc))
								break;
						}

						//we have scrolled up if necisary, now to determine what is the character above this one
						int newCP = 0;
						bool trail = false;
						if (!PttoCP(GLUFPoint(rc.right, rc.bottom + long((bIsKeyUp ? 1.5 : -0.5) * (long)pFont->m_Leading)), &newCP, &trail))
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
					m_pDialog->SendEvent(GLUF_EVENT_EDITBOX_CHANGE, true, this);
				}
				else if (m_nCaret >= 0)
				{
					// Move the caret, then delete the char.
					RemoveChar(m_nCaret);
					PlaceCaret(m_nCaret - 1);
					m_nSelStart = m_nCaret;
					m_pDialog->SendEvent(GLUF_EVENT_EDITBOX_CHANGE, true, this);
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
						m_pDialog->SendEvent(GLUF_EVENT_EDITBOX_CHANGE, true, this);

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
					m_pDialog->SendEvent(GLUF_EVENT_EDITBOX_CHANGE, true, this);

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
				//m_pDialog->SendEvent(GLUF_EVENT_EDITBOX_STRING, true, this);
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
}


//--------------------------------------------------------------------------------------
void GLUFEditBox::Render( float elapsedTime)
{
	//UpdateRects();
	if (m_bAnalyseRequired)
		Analyse();

	//TODO: don't render scrollbar UNLESS there is necessity to scroll, change this on ALL controls



	if (m_bVisible == false)
		return;


	//
	// Blink the caret
	//
	if (glfwGetTime() - m_dfLastBlink >= m_dfBlink)
	{
		m_bCaretOn = !m_bCaretOn;
		m_dfLastBlink = glfwGetTime();
	}

	GLUFElement* pElement = GetElement(0);

	// Render the control graphics
	for (int e = 0; e < 9; ++e)
	{
		pElement = m_Elements[e];
		pElement->TextureColor.Blend(GLUF_STATE_NORMAL, elapsedTime);

		m_pDialog->DrawSprite(pElement, m_rcRender[e], GLUF_FAR_BUTTON_DEPTH);
	}

	//render the scrollbar
	if (m_bMultiline)
		m_ScrollBar.Render(elapsedTime);

	GLUFFontNode* pFontNode = m_pDialog->GetManager()->GetFontNode(m_Elements[0]->iFont);
	if (pElement)
	{
		if (m_bHasFocus && m_bCaretOn && !s_bHideCaret)
		{
			// Start the rectangle with insert mode caret
			GLUFRect rcCaret;
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
					GLUFSetRect(rcCaret, 0, GLUFRectHeight(m_rcText), 2, GLUFRectHeight(m_rcText) - pFontNode->m_Leading);

				GLUFOffsetRect(rcCaret, m_rcText.left, m_rcText.bottom);
				m_pDialog->DrawRect(rcCaret, m_CaretColor);
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

						GLUFSetRect(rcCaret, m_CharBoundingBoxes[0].left - 2, m_CharBoundingBoxes[rndCaret].top, 2, m_CharBoundingBoxes[rndCaret].bottom);
					}
					else if (m_strRenderBuffer[rndCaret] == '\n')
						rndCaret++;
					
					if (rndCaret == m_CharBoundingBoxes.size())
						rndCaret--;
					GLUFSetRect(rcCaret,
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

					GLUFOffsetRect(rcCaret, m_rcText.left, m_rcText.bottom);

					m_pDialog->DrawRect(rcCaret, m_CaretColor/*GLUF::Color(0, 0, 0, 255)*/);
				}
#pragma warning(default : 4018)
			}
		}
	}

	//debugging
	/*Color color = Color(255, 0, 255, 128);
	for (auto it : m_CharBoundingBoxes)
	{
		GLUFRect copy = it;
		GLUFOffsetRect(copy, m_rcText.left, m_rcText.bottom);
		m_pDialog->DrawRect(copy, color);

		if (color.r == 5)
			color.g += 10;
		else
			color.r -= 10;
	}*/

	m_Elements[0]->FontColor.SetCurrent(m_TextColor);
	m_pDialog->DrawText(m_strRenderBuffer.c_str(), m_Elements[0], m_rcText);


}


//--------------------------------------------------------------------------------------
void GLUFEditBox::ResetCaretBlink()
{
	m_bCaretOn = true;
	m_dfLastBlink = glfwGetTime();
}

//--------------------------------------------------------------------------------------

bool GLUFEditBox::CPtoRC(int nCP, GLUFRect *pRc)
{
	GLUF_ASSERT(pRc);
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

bool GLUFEditBox::PttoCP(GLUFPoint pt, int* pCP, bool* bTrail)
{
	GLUF_ASSERT(pCP && bTrail);
	*pCP = 0; *bTrail = false;  // Default

	if (m_bAnalyseRequired)
		Analyse();

	GLUFRect charRect = { 0L, 0L, 0L, 0L };

	for (auto it : m_CharBoundingBoxes)
	{
		charRect = it;

		//this deals with the bounding boxes leaving space between characters
		if (it != m_CharBoundingBoxes.back())
			charRect.right = m_CharBoundingBoxes[*pCP + 1].left;

		if (GLUFPtInRect(charRect, pt))
		{
			if (GLUFPtInRect({ charRect.left, charRect.top, charRect.right - GLUFRectWidth(it) / 2, charRect.bottom }, pt))
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

void GLUFEditBox::Analyse()
{
	GLUFFontNode* pFontNode = m_pDialog->GetManager()->GetFontNode(m_Elements[0]->iFont);
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
		long TextWidth = GLUFRectWidth(m_rcText);
		long currXValue = 0;
		int charIndex = 0;
		int addedCharactersCount = 0;

		std::vector<std::wstring> strings = GLUFSplitStr(m_strRenderBuffer, ' ', true);
		m_strInsertedNewlineLocations.clear();
		for (auto it : strings)
		{


			long WordWidth = pFontNode->m_pFontType->GetStringWidth(it);

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
					GLUFFontSize nCharWidth;
					nCharWidth = pFontNode->m_pFontType->GetCharAdvance(itch);
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

			if (currXValue/* + 0.0125f/*a little buffer*/ > TextWidth)
			{

				//add a "newline" (since there is always a trailing space, hack this a little bit so the space will always be at the end

				//blank strings i.e. double spaces ( or more ) then we just keep those at the end of the line
				/*if (it == L" ")
				{

				}*/
				
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
	if (pFontNode && pFontNode->m_pFontType->mHeight)
	{
		if (m_bMultiline)
		{
			m_ScrollBar.SetPageSize(int(GLUFRectHeight(m_rcText) / pFontNode->m_pFontType->mHeight));
			m_ScrollBar.SetTrackRange(0, GetNumNewlines() + 1);
		}
		else
		{
			m_ScrollBar.SetTrackRange(0, (int)m_strBuffer.length()-1);

			GLUFFontSize strWidth = 0;
			unsigned int count = 0;
			unsigned long textWidth = GLUFRectWidth(m_rcText);
			for (unsigned int i = m_ScrollBar.GetTrackPos(); i < m_strBuffer.length(); ++i)
				if (strWidth < textWidth)
				{
					strWidth += pFontNode->m_pFontType->GetCharAdvance(m_strBuffer[i]);
					++count;
				}
				else
				{
					//strWidth -= pFontNode->m_pFontType->GetCharAdvance(m_strBuffer[i]);
					--count;
					break;
				}

			m_ScrollBar.SetPageSize(count);
			m_ScrollBar.SetTrackPos(m_nCaret);
		}
		// The selected item may have been scrolled off the page.
		// Ensure that it is in page again.
		//m_ScrollBar.ShowItem(GetLineNumberFromCharPos(m_nCaret));
	}


	//now get the char bounding boxes (this ALSO culls the characters that would go off screen, as well as offset for the scrollbar)

	m_CharBoundingBoxes.clear();
	m_strRenderBufferOffset = 0;

	//for each character, get the width, and add that to the width of the previous, also add initial 0
	//m_CalcXValues.push_back(0);
	if (m_bMultiline)
	{
		int currXValue = 0;
		int distanceFromBottom = GLUFRectHeight(m_rcText);
		int   lineNum = 0;

		int thisCharWidth = 0;
		int fontHeight = pFontNode->m_Leading;
		int scrollbarOffset = m_ScrollBar.GetTrackPos() * pFontNode->m_Leading;
		GLUFRect rc;

		bool topCulled = false;

		std::wstring strRenderBufferTmp = L"";
		unsigned int i = 0;
		for (auto it : m_strRenderBuffer)
		{



			//cull the characters that will not fit on the page
			if (lineNum > m_ScrollBar.GetTrackPos() + m_ScrollBar.GetPageSize() || lineNum < m_ScrollBar.GetTrackPos())
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
				thisCharWidth = pFontNode->m_pFontType->GetCharAdvance(it);
#pragma warning(default : 4244)

				rc = pFontNode->m_pFontType->GetCharRect(it);
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
					rc.right = rc.left + pFontNode->m_pFontType->GetCharWidth(it);//use the char width for nice carrot position
					GLUFOffsetRect(rc, currXValue, 0);
					//GLUFSetRect(rc, currXValue, distanceFromBottom - pFontNode->m_pFontType->Get, currXValue + thisCharWidth, distanceFromBottom - fontHeight);
				}


				//offset the whole block by the scroll level
				GLUFOffsetRect(rc, 0, scrollbarOffset);

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
				GLUFSetRect(rc, 0.0f, distanceFromBottom, 0.0f, distanceFromBottom - fontHeight);
				m_CharBoundingBoxes.push_back();*/
			}

			++i;
		}
		m_strRenderBuffer = strRenderBufferTmp;
	}
	else
	{
		//for single line, we do HORIZONTAL culling
		int currXValue = 0;
		long top = GLUFRectHeight(m_rcText), bottom = top - pFontNode->m_Leading;

		bool leftCulled = false;
		int thisCharWidth = 0;
		int scrollbarOffset = m_ScrollBar.GetTrackPos();
		GLUFRect rc;

		std::wstring strRenderBufferTmp = L"";
		int i = 0;
		for (auto it : m_strRenderBuffer)
		{

			//cull the characters that will not fit on the page
			if (i < m_ScrollBar.GetTrackPos() || i > m_ScrollBar.GetTrackPos() + m_ScrollBar.GetPageSize() - 1)
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
				thisCharWidth = pFontNode->m_pFontType->GetCharAdvance(it);
#pragma warning(default : 4244)

				rc = pFontNode->m_pFontType->GetCharRect(it);
				rc.top = top;
				rc.bottom = bottom;
				rc.right = rc.left + pFontNode->m_pFontType->GetCharWidth(it);//use the char width for nice carrot position
				GLUFOffsetRect(rc, currXValue, 0);

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
	m_ScrollBar.ShowItem(m_nCaret);

}



//======================================================================================
// GLUF Text Operations
//======================================================================================

glm::mat4 g_TextOrtho;
glm::mat4 g_TextModelMatrix;

//--------------------------------------------------------------------------------------
void BeginText(glm::mat4 orthoMatrix)
{
	g_TextOrtho = orthoMatrix;
	g_TextVerticies.clear();
}

//--------------------------------------------------------------------------------------
void DrawTextGLUF(GLUFFontNode font, std::wstring strText, GLUFRect rcScreen, Color vFontColor, unsigned int dwTextFlags, bool bHardRect)
{

	if ((long)font.m_pFontType->mHeight > GLUFRectHeight(rcScreen) && bHardRect)
		return;//no sense rendering if it is too big

	//rcScreen = GLUFScreenToClipspace(rcScreen);

	GLUFFontSize tmpSize = font.m_pFontType->mHeight; // GLUF_FONT_HEIGHT_NDC(font.m_pFontType->mHeight);

	GLUFRectf UV;

	GLUFOffsetRect(rcScreen, -long(g_WndWidth / 2), -long(g_WndHeight / 2));

	long CurX = rcScreen.left;
	long CurY = rcScreen.top;

	//calc widths
	long strWidth = font.m_pFontType->GetStringWidth(strText);
	unsigned int centerOffset = (GLUFRectWidth(rcScreen) - strWidth) / 2;
	if (dwTextFlags & GT_CENTER)
	{		
		CurX = rcScreen.left + centerOffset;
	}
	else if (dwTextFlags & GT_RIGHT)
	{
		CurX = rcScreen.left + centerOffset * 2;
	}

	int numLines = 1;//always have one to get the GT_VCENTER correct
	for (auto it : strText)
	{
		if (it == L'\n')
			numLines++;
	}

	if (dwTextFlags & GT_VCENTER)
	{
		long value = GLUFRectHeight(rcScreen);
		value = value - numLines * font.m_pFontType->mHeight;
		value /= 2;
		CurY -= (GLUFRectHeight(rcScreen) - (long)numLines * (long)font.m_pFontType->mHeight) / 2;
	}
	else if (dwTextFlags & GT_BOTTOM)
	{
		CurY -= GLUFRectHeight(rcScreen) - numLines * font.m_pFontType->mHeight;
	}

	//glBegin(GL_QUADS);
	float z = GLUF_NEAR_BUTTON_DEPTH;
	for (auto ch : strText)
	{
		int widthConverted = font.m_pFontType->GetCharAdvance(ch);//(font.m_pFontType->CellX * tmpSize) / font.m_pFontType->mAtlasWidth;

		//lets support newlines :) (or if the next char will go outside the rect)
		if (ch == '\n' || (CurX + widthConverted > rcScreen.right && bHardRect))
		{
			if (dwTextFlags & GT_CENTER)
				CurX = rcScreen.left + centerOffset;
			else if (dwTextFlags & GT_LEFT)
				CurX = rcScreen.left;
			else if (dwTextFlags & GT_RIGHT)
				CurX = rcScreen.left + centerOffset * 2;

			CurY -= font.m_Leading;// *1.1f;//assume a reasonible leding

			//if the next line will go off of the page, then don't draw it
			if ((CurY - (long)font.m_Leading < rcScreen.bottom) && bHardRect)
				break;

			if (ch == '\n')
			{
				continue;
			}
		}

		//Row = (ch - font.m_pFontType->Base) / font.m_pFontType->RowPitch;
		//Col = (ch - font.m_pFontType->Base) - Row*font.m_pFontType->RowPitch;

		//U = Col*font.m_pFontType->ColFactor;
		//V = Row*font.m_pFontType->RowFactor;
		//U1 = U + font.m_pFontType->ColFactor;
		//V1 = V + font.m_pFontType->RowFactor;

		//U = font.m_pFontType->GetTextureXOffset(ch);
		//V = 0.0f;
		//U1 = U + font.m_pFontType->GetCharWidth(ch);
		//V1 = 1.0f;
		UV = font.m_pFontType->GetCharTexRect(ch);

		//glTexCoord2f(U, V1);  glVertex2i(CurX, CurY);
		//glTexCoord2f(U1, V1);  glVertex2i(CurX + font.m_pFontType->CellX, CurY);
		//glTexCoord2f(U1, V); glVertex2i(CurX + font.m_pFontType->CellX, CurY + font.m_pFontType->CellY);
		//glTexCoord2f(U, V); glVertex2i(CurX, CurY + font.m_pFontType->CellY);

		GLUFRect glyph = font.m_pFontType->GetCharRect(ch);

		//remember to expand for this
		//glyph.right = GLUF_FONT_HEIGHT_NDC(glyph.right);
		//glyph.top   = GLUF_FONT_HEIGHT_NDC(glyph.right);

		GLUFOffsetRect(glyph, CurX, CurY - (tmpSize));

		//glyph.left = CurX;
		//glyph.right = CurX + widthConverted;
		//glyph.top = CurY;
		//glyph.bottom = CurY - tmpSize;


		//triangle 1
		g_TextVerticies.push_back(
			glm::vec3(GLUFGetVec2FromRect(glyph, false, true), z),
			GLUFGetVec2FromRect(UV, false, true));
		//glm::vec2(z, 1.0f));

		g_TextVerticies.push_back(
			glm::vec3(GLUFGetVec2FromRect(glyph, false, false), z),
			GLUFGetVec2FromRect(UV, false, false));
		//glm::vec2(z, z));

		g_TextVerticies.push_back(
			glm::vec3(GLUFGetVec2FromRect(glyph, true, true), z),
			GLUFGetVec2FromRect(UV, true, true));
		//glm::vec2(1.0f, 1.0f));


		//triangle 2

		g_TextVerticies.push_back(
			glm::vec3(GLUFGetVec2FromRect(glyph, false, false), z),
			GLUFGetVec2FromRect(UV, false, false));
		//glm::vec2(z, z));

		g_TextVerticies.push_back(
			glm::vec3(GLUFGetVec2FromRect(glyph, true, false), z),
			GLUFGetVec2FromRect(UV, true, false));
		//glm::vec2(1.0f, z));

		g_TextVerticies.push_back(
			glm::vec3(GLUFGetVec2FromRect(glyph, true, true), z),
			GLUFGetVec2FromRect(UV, true, true));
		//glm::vec2(1.0f, 1.0f));


		CurX += widthConverted;
		//CurX += 0.05;

		//z += 0.00005f;//to solve the depth problem
		
	}
	//glEnd();
	
	g_TextVerticies.set_color(vFontColor);

	//g_TextModelMatrix = glm::translate(glm::mat4(), glm::vec3(0.5f, 1.5f, 0.0f));//glm::mat4(1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.3f, 0.3f, 0.0f, 1.0f);
	
	EndText(font.m_pFontType);

}


void EndText(GLUFFontPtr font)
{
	
	//get the currently bound texture to rebind later
	//GLint tmpTexId;
	//glGetIntegerv(GL_TEXTURE_BINDING_2D, &tmpTexId);
	//GLUF_ASSERT(tmpTexId >= 0);

	//buffer the data
	glBindVertexArray(g_TextVAO);
	glBindBuffer(GL_ARRAY_BUFFER, g_TextPos);
	glBufferData(GL_ARRAY_BUFFER, g_TextVerticies.size() * sizeof(glm::vec3), g_TextVerticies.data_pos(), GL_STREAM_DRAW);


	glBindBuffer(GL_ARRAY_BUFFER, g_TextTexCoords);
	glBufferData(GL_ARRAY_BUFFER, g_TextVerticies.size() * sizeof(glm::vec2), g_TextVerticies.data_tex(), GL_STREAM_DRAW);

	GLUFSHADERMANAGER.UseProgram(g_TextProgram);
	
	//first uniform: model-view matrix
	glm::mat4 mv = g_TextOrtho;
	glUniformMatrix4fv(g_TextShaderLocations.ortho, 1, GL_FALSE, glm::value_ptr(mv));

	//second, the sampler
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, font->mTexId);
	glUniform1i(g_TextShaderLocations.sampler, 0);


	//third, the color
	Color4f color = g_TextVerticies.get_color();
	glUniform4f(g_TextShaderLocations.color, color.r, color.g, color.b, color.a);

	glEnableVertexAttribArray(g_TextShaderLocations.position);//positions
	glEnableVertexAttribArray(g_TextShaderLocations.uv);//uvs

	//make sure to enable this with text
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDrawArrays(GL_TRIANGLES, 0, g_TextVerticies.size());

	g_TextVerticies.clear();
	glBindVertexArray(0);

	//lastly, rebind the old texture
	//glBindTexture(GL_TEXTURE_BINDING_2D, tmpTexId);
}


//GLUFTextHelper
GLUFTextHelper::GLUFTextHelper(GLUFDialogResourceManager* pManager) : 
m_pManager(pManager), m_clr(0, 0, 0, 255), m_pt(0L, 0L), 
m_fLineHeight(20L), m_nFont(0), m_fFontSize(15L), m_Weight(FONT_WEIGHT_NORMAL)
{
	GLUF_ASSERT(pManager);
}

void GLUFTextHelper::Init(GLUFFontSize fLineHeight)
{
	m_fLineHeight = fLineHeight;
	m_clr = Color(0, 0, 0, 255);
	m_pt = { 0L, 0L };
	m_nFont = 0;
}

void GLUFTextHelper::Begin(GLUFFontIndex fontToUse, GLUF_FONT_WEIGHT weight)
{
	m_nFont = fontToUse;

	m_Weight = weight;

	BeginText(m_pManager->GetOrthoMatrix());
}

GLUFResult GLUFTextHelper::DrawFormattedTextLine(const wchar_t* strMsg, size_t strLen, ...)
{
	va_list param;
	wchar_t* Msg = new wchar_t[strLen];

	va_start(param, strMsg);

	//let sprintf handle all of the formatting
	swprintf(Msg, strLen, strMsg, param);//TODO: if this fails?

	return DrawTextLine(Msg);
}

GLUFResult GLUFTextHelper::DrawTextLine(const wchar_t* strMsg)
{
	m_pManager->GetFontNode(m_nFont)->m_Leading = m_fLineHeight;
	std::wstring sMsg = strMsg;

	DrawTextGLUF(*m_pManager->GetFontNode(m_nFont), sMsg, { m_pt.x, m_pt.y, m_pt.x + 50L, m_pt.y - 50L }, m_clr, GT_LEFT | GT_TOP);

	//set the point down however many lines were drawn
	for (auto it : sMsg)
	{
		if (it == '\n')
			m_pt.y += m_fLineHeight;
	}
	m_pt.y -= m_fLineHeight;//once no matter what because we are drawing a LINE of text

	return GR_SUCCESS;
}

GLUFResult GLUFTextHelper::DrawFormattedTextLine(const GLUFRect& rc, unsigned int dwFlags, const wchar_t* strMsg, size_t strLen, ...)
{
	va_list param;
	wchar_t *Msg = new wchar_t[strLen];

	va_start(param, strMsg);

	swprintf(Msg, strLen, strMsg, param);

	return DrawTextLine(rc, dwFlags, Msg);
}

GLUFResult GLUFTextHelper::DrawTextLine(const GLUFRect& rc, unsigned int dwFlags, const wchar_t* strMsg)
{
	m_pManager->GetFontNode(m_nFont)->m_Leading = m_fLineHeight;
	DrawTextGLUF(*m_pManager->GetFontNode(m_nFont), strMsg, rc, m_clr, dwFlags, true);

	return GR_SUCCESS;
}

void GLUFTextHelper::End()
{
	EndText(m_pManager->GetFontNode(m_nFont)->m_pFontType);
}

}
/*

Copyright (C) 2015  Kevin Mackenzie

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License http://www.gnu.org/licenses/gpl-3.0.en.html 
for more details.

*/


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
    glm::mat4 g_TextOrtho;


    void BeginText(const glm::mat4& orthoMatrix);


    void DrawText(const FontNodePtr& font, const std::wstring& text, const Rect& rect, const Color& color, Bitfield textFlags, bool hardRect = false);
    void EndText(const FontPtr& font, const VertexArrayPtr& data, const Color& textColor, const glm::mat4& projMatrix = g_TextOrtho);

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
        mCharAtlas[p].mTexYOffset = static_cast<float>(y) / static_cast<float>(mAtlasSize.y);

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
        return{ { 0 }, 0, 0, { 0 } };
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
        mStates[static_cast<ControlState>(i)] = color;
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

    if (msg == UNICODE_CHAR)
    {
        int i = 0;
    }

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
		/*if (sControlFocus &&
			&sControlFocus->mDialog == this &&
			sControlFocus->GetEnabled())
			for (auto it : mControls)     --> Not Quite sure what this loop was here for
			{
			    if (sControlFocus->MsgProc(msg, param1, param2, param3, param4))
				    return true;
			}*/

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
void Dialog::AddEditBox(ControlIndex ID, const std::wstring& strText, const Rect& region, Charset charset, GLbitfield textFlags, bool isDefault, std::shared_ptr<EditBoxPtr> ctrlPtr)
{
	auto pEditBox = CreateEditBox(*this, (textFlags & GT_MULTI_LINE) == GT_MULTI_LINE);

	if (ctrlPtr)
		*ctrlPtr = pEditBox;

	pEditBox->GetElement(0).mTextFormatFlags = textFlags;

	// Set the ID and position
	pEditBox->SetID(ID);
    pEditBox->SetRegion(region);
	pEditBox->mIsDefault = isDefault;
    pEditBox->SetCharset(charset);
	pEditBox->SetText(strText);

	AddControl(std::dynamic_pointer_cast<Control>(pEditBox));
}


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
void Dialog::DrawRect(const Rect& rect, const Color& color, bool transform)
{
	Rect rcScreen = rect;

    if (transform)
        ScreenSpaceToGLSpace(rcScreen);

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

void Dialog::ScreenSpaceToGLSpace(Rect& rc) noexcept
{
    OffsetRect(rc, mRegion.left - long(g_WndWidth / 2), mRegion.bottom - long(g_WndHeight / 2));
}

void Dialog::ScreenSpaceToGLSpace(Point& pt) noexcept
{
    pt.x += mRegion.left - long(g_WndWidth / 2);
    pt.y += mRegion.bottom - long(g_WndHeight / 2);
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
    SetRect(rcTexture, 0.0507812f, 0.6484375f, 0.9375f, 0.55859375f);
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
    mSpriteBuffer.AddVertexAttrib({ 4, 2, g_UIShaderLocations.uv, GL_FLOAT, 0 }, 28);

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
	SHADERMANAGER.GLUniformMatrix4f(g_UIShaderLocations.ortho, mat);
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
        mDialog.SendEvent(EVENT_BUTTON_CLICKED, true, shared_from_this());
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
                        mDialog.SendEvent(EVENT_BUTTON_CLICKED, true, shared_from_this());

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

                mDialog.SendEvent(EVENT_BUTTON_CLICKED, true, shared_from_this());
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

    mDialog.SendEvent(EVENT_CHECKBOXCHANGED, fromInput, shared_from_this());
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
	mDialog.SendEvent(EVENT_RADIOBUTTON_CHANGED, fromInput, shared_from_this());
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
    InflateRect(mTextRegion, -(int32_t)mHorizontalMargin, -(int32_t)mVerticalMargin);

    // Update the scrollbar's rects
    //mScrollBar->SetLocation(mRegion.right - mSBWidth, mRegion.top);
    //mScrollBar->SetSize(mSBWidth, m_height);

    mScrollBar->SetLocation(mRegion.right, mRegion.bottom);
    mScrollBar->SetSize(mSBWidth, RectHeight(mRegion));
    FontNodePtr pFontNode = mDialog.GetFont(mElements[0].mFontIndex);
    if (pFontNode && pFontNode->mFontType->mHeight)
    {
        mScrollBar->SetPageSize(int(RectHeight(mTextRegion) / pFontNode->mLeading));

        // The selected item may have been scrolled off the page.
        // Ensure that it is in page again.
        mScrollBar->ShowItem(mSelected[mSelected.size() - 1]);
    }

    mScrollBar->UpdateRects();
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
    SetRect(newItem->mTextRegion, 0, 0, 0, 0);
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

    mDialog.SendEvent(EVENT_LISTBOX_SELECTION, true, shared_from_this());
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

    mDialog.SendEvent(EVENT_LISTBOX_SELECTION, true, shared_from_this());
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

                    mDialog.SendEvent(EVENT_LISTBOX_SELECTION, true, shared_from_this());
                }
                return true;
            }

            // Space is the hotkey for double-clicking an item.
            //
            case GLFW_KEY_SPACE:
                mDialog.SendEvent(EVENT_LISTBOX_ITEM_DBLCLK, true, shared_from_this());
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
                mDialog.SendEvent(EVENT_LISTBOX_ITEM_DBLCLK, true, this);
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
                    if (PtInRect(mItems[it]->mTextRegion, pt))
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

                mDialog.SendEvent(EVENT_LISTBOX_SELECTION, true, shared_from_this());


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
            mDialog.SendEvent(EVENT_LISTBOX_SELECTION, true, this);

            mDialog.SendEvent(EVENT_LISTBOX_SELECTION_END, true, this);
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
        mDialog.SendEvent(EVENT_LISTBOX_SELECTION, true, this);
        }
        else if (nItem < (int)mScrollBar->GetTrackPos())
        {
        // User drags the mouse above window top
        mScrollBar->Scroll(-1);
        mSelected[0] = mScrollBar->GetTrackPos();
        mDialog.SendEvent(EVENT_LISTBOX_SELECTION, true, this);
        }
        else if (nItem >= mScrollBar->GetTrackPos() + mScrollBar->GetPageSize())
        {
        // User drags the mouse below window bottom
        mScrollBar->Scroll(1);
        mSelected[0] = std::min((int)mItems.size(), mScrollBar->GetTrackPos() +
        mScrollBar->GetPageSize()) - 1;
        mDialog.SendEvent(EVENT_LISTBOX_SELECTION, true, this);
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
        int curY = mTextRegion.top - mVerticalMargin;
        int nRemainingHeight = RectHeight(mRegion) - 2 * mVerticalMargin;


        //int nRemainingHeight = RectHeight(mRegion) - pFont->mLeading;

        //for all of the ones before the displayed, just set them to something impossible
        for (size_t i = 0; i < (size_t)mScrollBar->GetTrackPos(); ++i)
        {
            SetRect(mItems[i]->mTextRegion, 0, 0, 0, 0);
        }
        for (size_t i = mScrollBar->GetTrackPos(); i < mItems.size(); i++)
        {
            ListBoxItemPtr pItem = mItems[i];

            // Make sure there's room left in the box
            nRemainingHeight -= pFont->mLeading;
            if (nRemainingHeight - (int)mVerticalMargin < 0)
            {
                pItem->mVisible = false;
                continue;
            }

            pItem->mVisible = true;

            SetRect(pItem->mTextRegion, mRegion.left + mHorizontalMargin, curY, mRegion.right - mHorizontalMargin, curY - pFont->mFontType->mHeight);
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
                Rect activeRect = pItem->mTextRegion;

                //add 1 here in order to make SURE the selection will be seamless
                InflateRect(activeRect, 0, pFont->mLeading / 4 + 1);
                mDialog.DrawSprite(pSelElement, activeRect, _NEAR_BUTTON_DEPTH);
                mDialog.DrawText(pItem->mText, pSelElement, pItem->mTextRegion);
            }
            else
                mDialog.DrawText(pItem->mText, *pElement, pItem->mTextRegion);

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
    //Rect tmpRect = mScrollBar->GetRegion();
    //tmpRect.y = mTextRegion.top;
    //mScrollBar->SetRegion(tmpRect);
    FontNodePtr pFontNode = mDialog.GetFont(mElements[2].mFontIndex);
    if (pFontNode/* && pFontNode->mSize*/)
    {
        mScrollBar->SetPageSize(int(RectHeight(mDropdownTextRegion) / pFontNode->mFontType->mHeight));

        // The selected item may have been scrolled off the page.
        // Ensure that it is in page again.
        mScrollBar->ShowItem(mSelected);
    }

    mScrollBar->UpdateRects();
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

            SetRect(pItem->mTextRegion, mDropdownTextRegion.left, curY, mDropdownTextRegion.right, curY - pFont->mFontType->mHeight);
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
                    PtInRect(pItem->mTextRegion, pt))
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
                            PtInRect(pItem->mTextRegion, pt))
                        {
                            mFocused = mSelected = static_cast<int>(i);
                            mDialog.SendEvent(EVENT_COMBOBOX_SELECTION_CHANGED, true, shared_from_this());
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

                    mDialog.SendEvent(EVENT_COMBOBOX_SELECTION_CHANGED, true, shared_from_this());
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
                        mDialog.SendEvent(EVENT_COMBOBOX_SELECTION_CHANGED, true, shared_from_this());
                }
            }
            else
            {
                if (mFocused + 1 < (int)GetNumItems())
                {
                    mFocused++;
                    mSelected = mFocused;

                    if (!mOpened)
                        mDialog.SendEvent(EVENT_COMBOBOX_SELECTION_CHANGED, true, shared_from_this());
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
                    mDialog.SendEvent(EVENT_COMBOBOX_SELECTION_CHANGED, true, shared_from_this());
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
                mDialog.SendEvent(EVENT_COMBOBOX_SELECTION_CHANGED, true, shared_from_this());

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
                    mDialog.SendEvent(EVENT_COMBOBOX_SELECTION_CHANGED, true, shared_from_this());
            }

            return true;

        case GLFW_KEY_RIGHT:
        case GLFW_KEY_DOWN:
            if (mFocused + 1 < (int)GetNumItems())
            {
                mFocused++;
                mSelected = mFocused;

                if (!mOpened)
                    mDialog.SendEvent(EVENT_COMBOBOX_SELECTION_CHANGED, true, shared_from_this());
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
    mDialog.SendEvent(EVENT_COMBOBOX_SELECTION_CHANGED, true, shared_from_this());
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
                Rect active = pItem->mTextRegion;

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
                    mDialog.DrawText(pItem->mText, *pSelectionElement, pItem->mTextRegion);
                }
                else
                {
                    mDialog.DrawText(pItem->mText, *pElement, pItem->mTextRegion);
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
        mDialog.SendEvent(EVENT_COMBOBOX_SELECTION_CHANGED, false, shared_from_this());
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
	mDialog.SendEvent(EVENT_COMBOBOX_SELECTION_CHANGED, false, shared_from_this());
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
	mDialog.SendEvent(EVENT_COMBOBOX_SELECTION_CHANGED, false, shared_from_this());
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
                    mDialog.SendEvent(EVENT_SLIDER_VALUE_CHANGED_UP, true, shared_from_this());

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

    mDialog.SendEvent(EVENT_SLIDER_VALUE_CHANGED, bFromInput, shared_from_this());

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

//--------------------------------------------------------------------------------------
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

//--------------------------------------------------------------------------------------
bool CharsetContains(const std::wstring& codepoint, Charset charset)
{
    switch (charset)
    {
    case Unicode:
        return true;
    default:
        for (auto it : codepoint)
        {
            if (!CharsetContains(it, charset))
                return false;
        }
return false;
    }
}

/*
======================================================================================================================================================================================================
ModificationStack Functions


*/


//--------------------------------------------------------------------------------------
void ModificationStack::FlattenRedoStack() noexcept
{
    while (!mRedoStack.empty())
    {
        mRedoStack.pop();
    }
}

//--------------------------------------------------------------------------------------
void ModificationStack::FlattenUndoStack() noexcept
{
    while (!mUndoStack.empty())
    {
        mUndoStack.pop();
    }
}

//--------------------------------------------------------------------------------------
ModificationStack::ModificationStack(const std::wstring& initialText) :
    mText(initialText)
{
}

//--------------------------------------------------------------------------------------
void ModificationStack::PushAddition(const std::wstring& text, uint32_t loc)
{
    ModificationStackInternal::ModificationAddition m;
    m.mInsertLocation = loc;
    m.mNewText = text;
    PushGeneric(m);
}

//--------------------------------------------------------------------------------------
void ModificationStack::PushRemoval(uint32_t start, uint32_t end)
{
    ModificationStackInternal::ModificationRemoval m;
    m.mStartIndex = start;
    m.mEndIndex = end;
    PushGeneric(m);
}

//--------------------------------------------------------------------------------------
/*void ModificationStack::PushRemovalAndAddition(uint32_t startIndex, uint32_t endIndex, const std::wstring& addition)
{
    ModificationStackInternal::ModificationRemovalAndAddition m;
    m.mStartIndex = startIndex;
    m.mInsertLocation = startIndex;
    m.mEndIndex = endIndex;
    m.mNewText = addition;
    PushGeneric(m);
}*/

//--------------------------------------------------------------------------------------
void ModificationStack::ApplyPartialModifications() noexcept
{
    if (mPartialMod.Empty())
        return;

    PushGeneric(mPartialMod);
    mPartialMod.ClearParts();
}

//--------------------------------------------------------------------------------------
void ModificationStack::PushPartialAddition(const std::wstring& text, uint32_t loc)
{
    ModificationStackInternal::ModificationAddition m;
    m.mInsertLocation = loc;
    m.mNewText = text;
    mPartialMod.PushPart(m);
}

//--------------------------------------------------------------------------------------
void ModificationStack::PushPartialRemoval(uint32_t start, uint32_t end)
{
    ModificationStackInternal::ModificationRemoval m;
    m.mStartIndex = start;
    m.mEndIndex = end;
    mPartialMod.PushPart(m);
}

//--------------------------------------------------------------------------------------
void ModificationStack::UndoNextItem() noexcept
{
    if (mUndoStack.empty())
    return;

    auto top = mUndoStack.top();
    top->RemoveModificationToString(mText);
    mUndoStack.pop();
    mRedoStack.push(top);
}

//--------------------------------------------------------------------------------------
void ModificationStack::RedoNextItem() noexcept
{
    if (mRedoStack.empty())
    return;

    auto top = mRedoStack.top();
    top->ApplyModificationToString(mText);
    mRedoStack.pop();
    mUndoStack.push(top);
}

//--------------------------------------------------------------------------------------
void ModificationStack::FlattenStack() noexcept
{
    FlattenUndoStack();
    FlattenRedoStack();
}

//--------------------------------------------------------------------------------------
void ModificationStack::SetText(const std::wstring& text) noexcept
{
    mText = text;
    FlattenStack();
}

/*

Subclass functions

*/

//--------------------------------------------------------------------------------------
ModificationStackInternal::ModificationAddition& ModificationStackInternal::ModificationAddition::operator=(const ModificationAddition& str)
{
    mInsertLocation = str.mInsertLocation;
    mNewText = str.mNewText;

    return *this;
}

//--------------------------------------------------------------------------------------
void ModificationStackInternal::ModificationAddition::ApplyModificationToString(std::wstring& str) const
{
    auto begin = std::begin(str) + std::clamp(mInsertLocation, (uint32_t)0, static_cast<uint32_t>(str.size()));
    str.insert(begin, std::begin(mNewText), std::end(mNewText));
}

//--------------------------------------------------------------------------------------
void ModificationStackInternal::ModificationAddition::RemoveModificationToString(std::wstring& str) const
{
    auto begin = std::begin(str) + std::clamp(mInsertLocation, (uint32_t)0, static_cast<uint32_t>(str.size()));
    auto end = std::begin(str) + std::clamp(mInsertLocation + mNewText.size(), (uint32_t)0, static_cast<uint32_t>(str.size()));

    str.erase(begin, end);
}

//--------------------------------------------------------------------------------------
ModificationStackInternal::ModificationRemoval& ModificationStackInternal::ModificationRemoval::operator=(const ModificationRemoval& str)
{
    mStartIndex = str.mStartIndex;
    mEndIndex = str.mEndIndex;
    mRemovedText = str.mRemovedText;

    return *this;
}

//--------------------------------------------------------------------------------------
void ModificationStackInternal::ModificationRemoval::ApplyModificationToString(std::wstring& str) const
{
    auto begin = std::begin(str) + std::clamp(mStartIndex, (uint32_t)0, static_cast<uint32_t>(str.size()));
    auto end = std::begin(str) + std::clamp(mEndIndex, (uint32_t)0, static_cast<uint32_t>(str.size()));

    mRemovedText.assign(begin, end);
    str.erase(begin, end);
}

//--------------------------------------------------------------------------------------
void ModificationStackInternal::ModificationRemoval::RemoveModificationToString(std::wstring& str) const
{
    auto begin = std::begin(str) + std::clamp(mStartIndex, (uint32_t)0, static_cast<uint32_t>(str.size()));

    str.insert(begin, mRemovedText.begin(), mRemovedText.end());
    mRemovedText.clear();
}

//--------------------------------------------------------------------------------------
void ModificationStackInternal::ModificationRemovalAndAddition::ApplyModificationToString(std::wstring& str) const
{
    ModificationRemoval::ApplyModificationToString(str);
    ModificationAddition::ApplyModificationToString(str);
}

//--------------------------------------------------------------------------------------
void ModificationStackInternal::ModificationRemovalAndAddition::RemoveModificationToString(std::wstring& str) const
{
    ModificationAddition::RemoveModificationToString(str);
    ModificationRemoval::RemoveModificationToString(str);
}

ModificationStackInternal::ModificationRemovalAndAddition& ModificationStackInternal::ModificationRemovalAndAddition::operator=(const ModificationRemovalAndAddition& str)
{
    mStartIndex = str.mStartIndex;
    mEndIndex = str.mEndIndex;
    mInsertLocation = str.mInsertLocation;
    mNewText = str.mNewText;
    mRemovedText = str.mRemovedText;

    return *this;
}

//--------------------------------------------------------------------------------------
ModificationStackInternal::ModificationRemovalAndAddition::ModificationRemovalAndAddition(const ModificationRemovalAndAddition& other) : ModificationRemoval(other), ModificationAddition(other)
{
}

//--------------------------------------------------------------------------------------
ModificationStackInternal::ModificationRemovalAndAddition::ModificationRemovalAndAddition() : ModificationRemoval(), ModificationAddition()
{
}



//--------------------------------------------------------------------------------------
void ModificationStackInternal::GenericCompositeModification::ApplyModificationToString(std::wstring& str) const
{
    for (auto& it : mParts)
    {
        it->ApplyModificationToString(str);
    }
}

//--------------------------------------------------------------------------------------
void ModificationStackInternal::GenericCompositeModification::RemoveModificationToString(std::wstring& str) const
{
    for (auto it = mParts.crbegin(); it != mParts.crend(); ++it)
    {
        (*it)->RemoveModificationToString(str);
    }
}

//--------------------------------------------------------------------------------------
void ModificationStackInternal::GenericCompositeModification::ClearParts()
{
    mParts.clear();
}

/*
======================================================================================================================================================================================================
Edit Box Functions


*/

//--------------------------------------------------------------------------------------
EditBox::EditBox(Dialog& dialog, bool isMultiline) : Control(dialog), mTextDataBuffer(std::make_shared<VertexArray>(GL_TRIANGLES, GL_STREAM_DRAW, true)), mMultiline(isMultiline), mTextHistoryKeeper(L"")
{
    mType = CONTROL_EDITBOX;

    mScrollBar = CreateScrollBar(dialog);

    mDialog.InitControl(std::dynamic_pointer_cast<Control>(mScrollBar));

    mTextDataBuffer->AddVertexAttrib({ 4, 3, g_TextShaderLocations.position, GL_FLOAT, 0 });
    mTextDataBuffer->AddVertexAttrib({ 4, 2, g_TextShaderLocations.uv, GL_FLOAT, 12 });

    mCaretColor.SetAll({ 0, 0, 0, 255 });
    mCaretColor.SetState(STATE_DISABLED, { 0, 0, 0, 0 });
    mCaretColor.SetState(STATE_HIDDEN, { 0, 0, 0, 0 });

    mSelBkColor.SetAll({ 0, 0, 200, 128 });
    mSelBkColor.SetState(STATE_HIDDEN, { 0, 0, 0, 0 });

    mSelTextColor.SetAll({ 0, 0, 0, 255 });
    mSelTextColor.SetState(STATE_HIDDEN, { 0, 0, 0, 0 });

    if (isMultiline)
    {
        mUpdateRectsFunction = &EditBox::UpdateRectsMultiline;
        mUpdateCharRectsFunction = &EditBox::UpdateCharRectsMultiline;
        mRenderFunction = &EditBox::RenderMultiline;
    }
    else
    {
        mUpdateRectsFunction = &EditBox::UpdateRectsSingleline;
        mUpdateCharRectsFunction = &EditBox::UpdateCharRectsSingleline;
        mRenderFunction = &EditBox::RenderSingleline;
    }
}

//--------------------------------------------------------------------------------------
EditBox::~EditBox()
{
}

//--------------------------------------------------------------------------------------
void EditBox::InvalidateRects() noexcept
{
    mUpdateRequired = true;
}

#pragma region Setters and Getters

//--------------------------------------------------------------------------------------
std::wstring EditBox::GetSelectedText() noexcept
{
    if (mSelStart == -2)
        return L"";

    std::wstring text = GetText();
    std::wstring::iterator begin, end;
    long len = 0;
    if (mSelStart < mCaretPos)
    {
        begin = std::begin(text) + (mSelStart + 1);
        end = std::begin(text) + (mCaretPos + 1);
        len = mCaretPos + 1 - (mSelStart + 1);
    }
    else
    {
        begin = std::begin(text) + (mCaretPos + 1);
        end = std::begin(text) + (mSelStart + 1);
        len = mSelStart + 1 - (mCaretPos + 1);
    }

    std::wstring ret(len, L' ');
    std::copy(begin, end, ret.begin());

    return ret;
}

//--------------------------------------------------------------------------------------
void EditBox::SetText(const std::wstring& text)
{
    if (!CharsetContains(text, mCharset))
        GLUF_CRITICAL_EXCEPTION(StringContainsInvalidCharacters());

    mTextHistoryKeeper.SetText(text);
    InvalidateRects();
}

//--------------------------------------------------------------------------------------
void EditBox::SetCharset(Charset chSet) noexcept
{
    NOEXCEPT_REGION_START

    mCharset = chSet;
    std::wstring text = GetText();

    //remove all characters from the string that are not in 'chSet'
    for (auto it = text.begin(); it != text.end(); ++it)
    {
        if (!CharsetContains(*it, chSet))
            text.erase(it);
    }

    mTextHistoryKeeper.SetText(text);

    NOEXCEPT_REGION_END
}

//--------------------------------------------------------------------------------------
void EditBox::SetBlinkPeriod(double period) noexcept
{
    mBlinkPeriod = period;
}

//--------------------------------------------------------------------------------------
void EditBox::SetCaretState(bool state) noexcept
{
    mHideCaret = !state;
}

//--------------------------------------------------------------------------------------
void EditBox::SetCaretPosition(Value pos) noexcept
{
    NOEXCEPT_REGION_START

    if (mIsEmpty)
        mCaretPos = -1;

    mCaretPos = glm::clamp(pos, -1, (int32_t)GetText().length());

    NOEXCEPT_REGION_END
}

//--------------------------------------------------------------------------------------
void EditBox::SetInsertMode(bool insertMode) noexcept
{
    mInsertMode = insertMode;
}

//--------------------------------------------------------------------------------------
void EditBox::SetSelectionStart(Value pos) noexcept
{
    mSelStart = pos;
}

//--------------------------------------------------------------------------------------
void EditBox::SetSelectionEmpty() noexcept
{
	SetSelectionStart(-2);
	SetCaretPosition(-1);
}

//--------------------------------------------------------------------------------------
void EditBox::SetHorizontalMargin(Size marg) noexcept
{
    mHorizontalMargin = marg;
   
    InvalidateRects();
}

//--------------------------------------------------------------------------------------
void EditBox::SetVerticalMargin(Size marg) noexcept
{
    mVerticalMargin = marg;

    InvalidateRects();
}

//--------------------------------------------------------------------------------------
void EditBox::SetSelectedTextBlendColor(const BlendColor& col) noexcept
{
    mSelTextColor = col;
}

//--------------------------------------------------------------------------------------
void EditBox::SetSelectedBackgroundBlendColor(const BlendColor& col) noexcept
{
    mSelBkColor = col;
}

//--------------------------------------------------------------------------------------
void EditBox::SetCaretBlendColor(const BlendColor& col) noexcept
{
    mCaretColor = col;
}

//--------------------------------------------------------------------------------------
void EditBox::SetTextBlendColor(const BlendColor& col) noexcept
{ 
    NOEXCEPT_REGION_START
    
    mElements[0].mFontColor = col;

    NOEXCEPT_REGION_END
}


#pragma endregion

//--------------------------------------------------------------------------------------
void EditBox::InsertString(const std::wstring& str, Value pos) noexcept
{
    NOEXCEPT_REGION_START


    //start by removing ALL newlines
    std::wstring newStr = str;
    {
        std::wstring::iterator it = std::find(newStr.begin(), newStr.end(), '\n');

        while (it != newStr.end())
        {
            newStr.erase(it);
            it = std::find(newStr.begin(), newStr.end(), '\n');
        }
    }

    pos = glm::clamp(pos, -1, static_cast<Value>(GetText().size()));

    //since a space is used as a placeholder when there is no text, make sure to clear when we get text
    if (mIsEmpty)
    {
        mTextHistoryKeeper.PushPartialRemoval(0, 1);
        mIsEmpty = false;
    }
    mTextHistoryKeeper.PushPartialAddition(newStr, pos);

    if (pos <= mCaretPos)
        mCaretPos += newStr.size();

    mSelStart = -2;

    InvalidateRects();

    NOEXCEPT_REGION_END
}

//--------------------------------------------------------------------------------------
void EditBox::InsertChar(wchar_t ch, Value pos) noexcept
{
    NOEXCEPT_REGION_START

    pos = glm::clamp(pos, -1, static_cast<Value>(GetText().size()));

    //since a space is used as a placeholder when there is no text, make sure to clear when we get text
    if (mIsEmpty)
    {
        mTextHistoryKeeper.PushPartialRemoval(0, 1);
        mIsEmpty = false;
    }
    mTextHistoryKeeper.PushPartialAddition(std::wstring() + ch, pos + 1);

    if (pos <= mCaretPos)
        ++mCaretPos;

    mSelStart = -2;

    InvalidateRects();

    NOEXCEPT_REGION_END
}

void EditBox::DeleteChar(Value pos) noexcept
{
    NOEXCEPT_REGION_START

    if (pos < 0 || pos >= GetText().size())
        return;

    if (GetText().size() == 1)
    {
        if (!mIsEmpty)
        {
            mTextHistoryKeeper.PushPartialRemoval(0, 1);
            mTextHistoryKeeper.PushPartialAddition(L" ", 0);
            mIsEmpty = true;
        }
    }
    else
    {
        mTextHistoryKeeper.PushPartialRemoval(pos, pos + 1);
    }

    InvalidateRects();

    NOEXCEPT_REGION_END
}

//--------------------------------------------------------------------------------------
bool EditBox::MsgProc(MessageType msg, int32_t param1, int32_t param2, int32_t param3, int32_t param4) noexcept
{
    NOEXCEPT_REGION_START

    auto mousePos = mDialog.GetMousePositionDialogSpace();

    if (mScrollBar->MsgProc(_PASS_CALLBACK_PARAM)/* || mScrollBar->ContainsPoint(mousePos)*/)
    {
        InvalidateRects();
        return true;
    }

    switch (msg)
    {
    case MB:
    {
        if (param1 == GLFW_MOUSE_BUTTON_LEFT)
        {
            if (param2 == GLFW_PRESS)
            {
                if (PtInRect(mRegion, mousePos))
                {
                    if (!mHasFocus)
                        mDialog.RequestFocus(shared_from_this());

					mSelStart = -2;
					SetCaretPosition(PointToCharPos(mousePos));
					mMouseDrag = true;
                }
            }
            else if (param2 == GLFW_RELEASE)
            {
                //is the scroll bar pressed
                /*if (mScrollBar->IsPressed())
                {
                    break;
                }*/

                //if they occupy the same index, no sense in rendering a blank selection region
                if (mSelStart == mCaretPos)
                    mSelStart = -2;

                mMouseDrag = false;
            }
        }

        break;
    }
    case CURSOR_POS:
    {
        //if there is a selection, make sure to update which parts are selected
        if (mMouseDrag)
        {
            if (mSelStart == -2)
            {
                mSelStart = PointToCharPos(mousePos);
                SetCaretPosition(mSelStart);
            }
            else
            {
                SetCaretPosition(PointToCharPos(mousePos));
            }

        }
        break;
    }
    case SCROLL:
    {
        mScrollBar->Scroll(-(param2 / WHEEL_DELTA));
        InvalidateRects();
        /*if (mMultiline)
        {
            mScrollBar->Scroll(-(param2 / WHEEL_DELTA));
        }
        else
        {
            mScrollBar->Scroll(param2 > 0 ? 1 : -1);
        }*/
        break;
    }
    case UNICODE_CHAR:
    {

        //is there a selection?
        if (mSelStart != -2)
        {
            //if so, delete it
            RemoveSelectedRegion();
        }

        InsertChar(static_cast<wchar_t>(param1), mCaretPos);

        break;
    }
    case KEY:
    {
        switch(param1)
        {
        case GLFW_KEY_BACKSPACE:
            if (param3 != GLFW_RELEASE)
            {
                if (mSelStart != -2)
                {
                    RemoveSelectedRegion();
                }
                else
                {
                    DeleteChar(mCaretPos);
                    SetCaretPosition(mCaretPos - 1);
                }
            }

            break;
        case GLFW_KEY_DELETE:
            if (param3 != GLFW_RELEASE)
            {
                if (mSelStart != -2)
                {
                    RemoveSelectedRegion();
                }
                else
                {
                    DeleteChar(mCaretPos + 1);
                }
            }

            break;
        case GLFW_KEY_ENTER:
            if (param3 != GLFW_RELEASE)
            {
                if (mSelStart != -2)
                {
                    RemoveSelectedRegion();
                }

                InsertChar(L'\n', mCaretPos);

            }
            break;
        case GLFW_KEY_C:
            if (param4 == GLFW_MOD_CONTROL && param3 == GLFW_PRESS)
            {
                //get selected text
                auto selText = GetSelectedText();

                glfwSetClipboardString(g_pGLFWWindow, WStringToString(selText).c_str());
            }
            break;
        case GLFW_KEY_V:
            if (param4 == GLFW_MOD_CONTROL && param3 != GLFW_RELEASE)
            {
                //insert clipboard
                std::string clipboardString = glfwGetClipboardString(g_pGLFWWindow);

                InsertString(StringToWString(clipboardString), mCaretPos + 1);
            }
            break;
        case GLFW_KEY_X:
            if (param4 == GLFW_MOD_CONTROL && param3 == GLFW_PRESS)
            {
                auto selText = GetSelectedText();

                glfwSetClipboardString(g_pGLFWWindow, WStringToString(selText).c_str());

                RemoveSelectedRegion();
            }
            break;
        case GLFW_KEY_A:
            if (param4 == GLFW_MOD_CONTROL && param3 == GLFW_PRESS)
            {
                mSelStart = -1;
                mCaretPos = GetText().size() - 1;
            }
            break;
        case GLFW_KEY_Z:
            if (param4 == GLFW_MOD_CONTROL && param3 != GLFW_RELEASE)
            {
                mTextHistoryKeeper.UndoNextItem();
                InvalidateRects();
            }
            break;
        case GLFW_KEY_Y:
            if (param4 == GLFW_MOD_CONTROL && param3 != GLFW_RELEASE)
            {
                mTextHistoryKeeper.RedoNextItem();
                InvalidateRects();
            }
            break;
        }
        break;
    }
	case FOCUS:
		if (param1 == GL_TRUE)
		{
			SetSelectionEmpty();
		}
		break;
    default:
        break;
    }

    ApplyCompositeModifications();

    return false;

    NOEXCEPT_REGION_END
}

//--------------------------------------------------------------------------------------
void EditBox::UpdateRectsMultiline() noexcept
{
    NOEXCEPT_REGION_START

    mTextRegion.right -= mSBWidth;

    mScrollBar->SetRegion({ { mTextRegion.right }, mSubRegions[0].top, mSubRegions[0].right, { mSubRegions[0].bottom } });
    mScrollBar->SetPageSize(static_cast<int>(RectHeight(mTextRegion) / mDialog.GetFont(mElements[0].mFontIndex)->mLeading));

    //TODO: finish setting up the scroll bar page size/everything else for the scroll bar update
    mScrollBar->UpdateRects();

    
    NOEXCEPT_REGION_END
}

//--------------------------------------------------------------------------------------
void EditBox::UpdateRectsSingleline() noexcept
{
    NOEXCEPT_REGION_START
    
    NOEXCEPT_REGION_END
}

//--------------------------------------------------------------------------------------
void EditBox::UpdateRects() noexcept
{
    NOEXCEPT_REGION_START

    if (GetText().size() == 0)
    {
        mIsEmpty = true;
    }

    Control::UpdateRects();

    mTextRegion = mRegion;
    InflateRect(mTextRegion, -static_cast<int32_t>(2 * mHorizontalMargin), -static_cast<int32_t>(2 * mVerticalMargin));

    mSubRegions[0] = mRegion;
    InflateRect(mSubRegions[0], -14, -14);
    SetRect(mSubRegions[1], mRegion.left, mRegion.top, mSubRegions[0].left, mSubRegions[0].top);
    SetRect(mSubRegions[2], mSubRegions[0].left, mRegion.top, mSubRegions[0].right, mSubRegions[0].top);
    SetRect(mSubRegions[3], mSubRegions[0].right, mRegion.top, mRegion.right, mSubRegions[0].top);
    SetRect(mSubRegions[4], mRegion.left, mSubRegions[0].top, mSubRegions[0].left, mSubRegions[0].bottom);
    SetRect(mSubRegions[5], mSubRegions[0].right, mSubRegions[0].top, mRegion.right, mSubRegions[0].bottom);
    SetRect(mSubRegions[6], mRegion.left, mSubRegions[0].bottom, mSubRegions[0].left, mRegion.bottom);
    SetRect(mSubRegions[7], mSubRegions[0].left, mSubRegions[0].bottom, mSubRegions[0].right, mRegion.bottom);
    SetRect(mSubRegions[8], mSubRegions[0].right, mSubRegions[0].bottom, mRegion.right, mRegion.bottom);

    (this->*mUpdateRectsFunction)();

    UpdateCharRects();

    mUpdateRequired = false;

    NOEXCEPT_REGION_END
}

//--------------------------------------------------------------------------------------
void EditBox::RenderMultiline(float elapsedTime) noexcept
{
    NOEXCEPT_REGION_START

    //debug rendering
    /*unsigned int color = 0x80000000;
    for (auto it : mCharacterBBs)
    {
        mDialog.DrawRect(it, *reinterpret_cast<Color*>(&color), false);

        color += 0x0000000F;
    }*/   

    mScrollBar->Render(elapsedTime);

    NOEXCEPT_REGION_END
}

//--------------------------------------------------------------------------------------
void EditBox::RenderSingleline(float elapsedTime) noexcept
{
    NOEXCEPT_REGION_START

    NOEXCEPT_REGION_END
}

//--------------------------------------------------------------------------------------
void EditBox::Render(float elapsedTime) noexcept
{
    NOEXCEPT_REGION_START

    if (mUpdateRequired)
        UpdateRects();

    auto state = STATE_NORMAL;
    if (!mVisible)
        state = STATE_HIDDEN;
    else if (!mEnabled)
        state = STATE_DISABLED;
    else if (mMouseOver)
        state = STATE_MOUSEOVER;
    else if (mHasFocus)
        state = STATE_FOCUS;

    unsigned int i = 0;
    for (auto& it : mElements)
    {
        it.second.mTextureColor.Blend(state, elapsedTime);

        if (i == 0)
        {
            it.second.mFontColor.Blend(state, elapsedTime);
        }

        mDialog.DrawSprite(it.second, mSubRegions[i], _NEAR_BUTTON_DEPTH);

        ++i;
    }

    mSelBkColor.Blend(state, elapsedTime);
    mSelTextColor.Blend(state, elapsedTime);
    mCaretColor.Blend(state, elapsedTime);

    //draw the text
    RenderText(elapsedTime);

    //draw the selected region
    if (mSelStart != -2 && mCharacterBBs.size() > 0 && mSelStart != mCaretPos)
    {

        Value begin = 0;
        Value end = 0;
        if (mCaretPos > mSelStart)
        {
            begin = mSelStart;
            end = mCaretPos;
        }
        else if (mCaretPos < mSelStart)
        {
            begin = mCaretPos;
            end = mSelStart;
        }

        begin++;

        begin = glm::clamp(begin, static_cast<Value>(mRenderOffset), static_cast<Value>(mRenderOffset + mRenderCount));
        end = glm::clamp(end, static_cast<Value>(mRenderOffset), static_cast<Value>(mRenderOffset + mRenderCount));

        //if caret position and selection start are equal, it won't render anything

        for (int32_t i = begin; i <= end; ++i)
        {
            mDialog.DrawRect(mCharacterBBs[i], mSelBkColor.GetCurrent(), false);
        }
    }

    //draw the caret
    if (!mHideCaret)
    {

        if (mCaretOn && ShouldRenderCaret())
        {
            //get the caret rect
            Rect caretRect = CharPosToRect(mCaretPos);
            mDialog.DrawRect(caretRect, mCaretColor.GetCurrent(), false);
        }

        if (GetTime() - mPreviousBlinkTime >= mBlinkPeriod)
        {
            bFlip(mCaretOn);
            mPreviousBlinkTime = GetTime();
        }
    }

    (this->*mRenderFunction)(elapsedTime);

    NOEXCEPT_REGION_END
}

//--------------------------------------------------------------------------------------
void EditBox::RenderText(float elapsedTime) noexcept
{
    NOEXCEPT_REGION_START

    auto& element = mElements[0];

    SHADERMANAGER.UseProgram(g_TextProgram);

    //first uniform: model-view matrix
    SHADERMANAGER.GLUniformMatrix4f(g_TextShaderLocations.ortho, Text::g_TextOrtho);

    //second, the sampler
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, mDialog.GetFont(element.mFontIndex)->mFontType->mTexId);
    SHADERMANAGER.GLUniform1i(g_TextShaderLocations.sampler, 0);


    //third, the color
    SHADERMANAGER.GLUniform4f(g_TextShaderLocations.color, ColorToFloat(element.mFontColor.GetCurrent()));

    //make sure to enable this with text
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    if (mSelStart == -2 || mSelStart == mCaretPos)
    {
        mTextDataBuffer->DrawRange(mRenderOffset * 6, mRenderCount * 6);
    }
    else
    {
        //get the begin and end of the selected text region
        GLuint begin = 6;
        GLuint end = 6;
        
        if (mSelStart > mCaretPos)
        {
            begin *= mCaretPos;
            end *= mSelStart;
        }
        else if (mSelStart < mCaretPos)
        {
            begin *= mSelStart;
            end *= mCaretPos;
        }

        //account for the one character offset to the left of the 'beginning' of the selection
        begin += 6;

        //get the values for the start and end of the preselection, the selection, and the postselection regions

        GLuint rndOffsetIndex = mRenderOffset * 6;
        GLuint rndEndIndex = rndOffsetIndex + mRenderCount * 6;

        GLuint selectionStartIndex = glm::clamp(begin, rndOffsetIndex, rndEndIndex);
        GLuint selectionSize = glm::clamp(end, rndOffsetIndex, rndEndIndex) - selectionStartIndex;

        GLuint preSelectionStartIndex = rndOffsetIndex;
        GLuint preSelectionSize = selectionStartIndex - preSelectionStartIndex;

        GLuint postSelectionStartIndex = glm::clamp(selectionStartIndex + selectionSize, rndOffsetIndex, rndEndIndex);
        GLuint postSelectionSize = rndEndIndex - postSelectionStartIndex;


        /*
        
        The Pre-selection region
        
        */
        //SHADERMANAGER.GLUniform4f(g_TextShaderLocations.color, ColorToFloat(elemenet.mFontColor.GetCurrent()));
        mTextDataBuffer->DrawRange(preSelectionStartIndex, preSelectionSize);

        /*
        
        The Selection Region
        
        */
        SHADERMANAGER.GLUniform4f(g_TextShaderLocations.color, ColorToFloat(mSelTextColor.GetCurrent()));
        mTextDataBuffer->DrawRange(selectionStartIndex, selectionSize);

        /*
        
        The Post-Selection Region
        
        */
        SHADERMANAGER.GLUniform4f(g_TextShaderLocations.color, ColorToFloat(element.mFontColor.GetCurrent()));
        mTextDataBuffer->DrawRange(postSelectionStartIndex, postSelectionSize);
    }

    NOEXCEPT_REGION_END
}

//--------------------------------------------------------------------------------------
void EditBox::RemoveSelectedRegion() noexcept
{
    NOEXCEPT_REGION_START

    if (mSelStart != -2)
    {
        if (mSelStart > mCaretPos)
            mTextHistoryKeeper.PushPartialRemoval(mCaretPos + 1, mSelStart + 1);
        else
            mTextHistoryKeeper.PushPartialRemoval(mSelStart + 1, mCaretPos + 1);

        if (mSelStart == -1 && mCaretPos == GetText().size() - 1)
            mTextHistoryKeeper.PushPartialAddition(L" ", 0);

        mCaretPos = glm::min(mCaretPos, mSelStart);
        mSelStart = -2;
    }

    InvalidateRects();

    NOEXCEPT_REGION_END
}

//--------------------------------------------------------------------------------------
Value EditBox::PointToCharPos(const Point& pt) noexcept
{
    NOEXCEPT_REGION_START

    //convert the point into the right space
    Point newPt = pt;
    mDialog.ScreenSpaceToGLSpace(newPt);

    //first see if it intersects a character
    for (unsigned int i = 0; i < mCharacterBBs.size(); ++i)
    {
        auto thisRect = mCharacterBBs[i];
        if (PtInRect(thisRect, newPt))
        {
            //see which side of the rect it is on
            if (newPt.x > thisRect.left + RectWidth(thisRect) / 2 || !mInsertMode)
            {
                //left side:
                return i;
            }
            else
            {
                //right side:
                return i - 1;
            }
        }
    }

    //if it didn't hit a character, get the nearest character (TODO:) 

    //which line is the cursor on?
    {
        bool foundLine = false;
        bool onCurrentLine = false;
        Rect regionRndSpace = mRegion;
        mDialog.ScreenSpaceToGLSpace(regionRndSpace);
        Rect textRegionRndSpace = mTextRegion; 
        mDialog.ScreenSpaceToGLSpace(textRegionRndSpace);
        for (int i = 0; i < mCharacterBBs.size(); ++i)
        {
            if (!onCurrentLine && foundLine)
                return i - 2;

            auto rightOf = mCharacterBBs[i];
            auto leftOf = mCharacterBBs[i];

            leftOf.left = regionRndSpace.left;
            leftOf.right = textRegionRndSpace.left;

            //use the left as the left of the text region, because if the cursor is to the left of the text region, we want the FIRST character on the line, not the last
            rightOf.left = textRegionRndSpace.left;
            rightOf.right = regionRndSpace.right;
            if (PtInRect(rightOf, newPt))
            {
                foundLine = true;
                onCurrentLine = true;
            }
            else if (PtInRect(leftOf, newPt))
            {
                //this can only possibly occur the first time on the line
                return i;
            }
            else
            {
                //the first character on the next line is hit, so activate that flag
                onCurrentLine = false;
            }
        }
    }

    NOEXCEPT_REGION_END

    return -1;

}

//--------------------------------------------------------------------------------------
Value EditBox::RenderTextToText(Value rndIndex)
{
    return -1;
}

//--------------------------------------------------------------------------------------
Rect EditBox::CharPosToRect(Value charPos) noexcept
{
    NOEXCEPT_REGION_START

    charPos = glm::clamp(charPos, -1, static_cast<Value>(mCharacterBBs.size() - 1));

    //if it is the first position
    if (charPos == -1)
    {
        auto firstRect = mCharacterBBs[0];
        
        if (mInsertMode)
        {
            firstRect.right = firstRect.left;
            firstRect.left -= mCaretSize;
        }

        return firstRect;
    }

    auto thisRect = mCharacterBBs[charPos];

    if (mInsertMode)
    {
        thisRect.right = mCharacterRects[charPos].right + mCaretSize;
        thisRect.left = thisRect.right - mCaretSize;
    }

    return thisRect;

    NOEXCEPT_REGION_END
}

//--------------------------------------------------------------------------------------
bool EditBox::ShouldRenderCaret() noexcept
{
    NOEXCEPT_REGION_START

    auto caretPosRect = CharPosToRect(mCaretPos);
    auto textRenderRect = mTextRegion;
    mDialog.ScreenSpaceToGLSpace(textRenderRect);


    //is the caret within the visible window
    if (PtInRect(textRenderRect, { caretPosRect.x, caretPosRect.y }) ||
        PtInRect(textRenderRect, { caretPosRect.right, caretPosRect.y }))
        return true;


    return false;

    NOEXCEPT_REGION_END
}

void EditBox::ApplyCompositeModifications() noexcept
{
    NOEXCEPT_REGION_START

    mTextHistoryKeeper.ApplyPartialModifications();

    NOEXCEPT_REGION_END
}

//--------------------------------------------------------------------------------------
Value EditBox::TextToRenderText(Value txtIndex)
{
    return -1;
}

//--------------------------------------------------------------------------------------
void EditBox::OnFocusIn() noexcept
{
    NOEXCEPT_REGION_START

    Control::OnFocusIn();
    mScrollBar->OnFocusIn();

	MsgProc(FOCUS, GL_TRUE, 0, 0, 0);

    NOEXCEPT_REGION_END
}

//--------------------------------------------------------------------------------------
void EditBox::OnFocusOut() noexcept
{
    NOEXCEPT_REGION_START

    Control::OnFocusOut();
    mScrollBar->OnFocusOut();

	MsgProc(FOCUS, GL_FALSE, 0, 0, 0);

    NOEXCEPT_REGION_END
}

//--------------------------------------------------------------------------------------
void EditBox::OnMouseEnter() noexcept
{
    NOEXCEPT_REGION_START

    Control::OnMouseEnter();
    mScrollBar->OnMouseEnter();

    NOEXCEPT_REGION_END
}

//--------------------------------------------------------------------------------------
void EditBox::OnMouseLeave() noexcept
{
    NOEXCEPT_REGION_START

    Control::OnMouseLeave();
    mScrollBar->OnMouseLeave();

    NOEXCEPT_REGION_END
}

//--------------------------------------------------------------------------------------
void EditBox::OnInit() noexcept
{
    NOEXCEPT_REGION_START

    Control::OnInit();

    mScrollBar->OnInit();

    NOEXCEPT_REGION_END
}

//--------------------------------------------------------------------------------------
void EditBox::UpdateCharRects() noexcept
{
    NOEXCEPT_REGION_START

    if (mIsEmpty)
    {
        //mText = L" ";
    }

    (this->*mUpdateCharRectsFunction)();

    BufferCharRects();

    NOEXCEPT_REGION_END
}

//--------------------------------------------------------------------------------------
void EditBox::UpdateCharRectsMultiline() noexcept
{
    NOEXCEPT_REGION_START

    std::wstring text = GetText();

    Rect rcScreen = mTextRegion;
    auto dlgRect = mDialog.GetRegion();
    mDialog.ScreenSpaceToGLSpace(rcScreen);

    auto &element = mElements[0];
    auto textFlags = element.mTextFormatFlags;
    auto font = mDialog.GetFont(element.mFontIndex);
    auto fontHeight = font->mFontType->mHeight;
    auto leading = font->mLeading;

    std::vector<std::wstring> textLines = { L"" };
    std::vector<bool> whichLinesCausedByNewlines = { 0 };

    mCharacterRects.resize(text.size());
    mCharacterBBs.resize(text.size());

    {
        auto textRegionWidth = RectWidth(rcScreen);
        int lineWidth = 0;

        /*
        
        Get Each Line of Text
        
        */
        for (unsigned int i = 0; i < text.size(); ++i)
        {
            auto thisChar = text[i];
            auto thisCharWidth = 0;

            int potentialNewLineWidth = 0;

            /*
            
            this bool is a workaround for newlines not being in the character atlas
            
            */
            bool execNewlineCode = thisChar == '\n';
            if (!execNewlineCode)
            {
                thisCharWidth = font->mFontType->GetCharAdvance(thisChar);
                potentialNewLineWidth = lineWidth + thisCharWidth;
                lineWidth += thisCharWidth;

                execNewlineCode = potentialNewLineWidth > textRegionWidth;
            }

            if (execNewlineCode)
            {
                textLines.push_back(L"");

                lineWidth = 0;

                if (thisChar == '\n')
                {
                    whichLinesCausedByNewlines.push_back(true);
                    continue;
                }
                whichLinesCausedByNewlines.push_back(false);
            }
            

            textLines[textLines.size() - 1] += thisChar;
        }

        mScrollBar->SetTrackRange(0, textLines.size() - 1);

        /*
        
        Get the Rects for each line
        
        */
        unsigned charIndex = 0;
        long currY = rcScreen.top + mScrollBar->GetTrackPos() * leading;
        long lineXOffset = 0;
        for (unsigned int i = 0; i < textLines.size() + 1; ++i)
        {
            //do this at the beginning of the loop to put the newline character in the right place for the caret
            if (i != 0 && i < whichLinesCausedByNewlines.size())
            {
                if (whichLinesCausedByNewlines[i])
                {
                    //make the bounding box invalid so the newline character can never truly be 'clicked on'
                    SetRect(mCharacterRects[charIndex], rcScreen.left, currY, rcScreen.left - 1, currY - leading);
                    mCharacterBBs[charIndex] = mCharacterRects[charIndex];
                    charIndex++;//add one at the end of each line
                }
            }

            //there is ONLY a newline on the next line, OR text is over
            if (i == textLines.size())
                break;

            auto thisLine = textLines[i];
            auto lineWidth = font->mFontType->GetStringWidth(thisLine);
            lineXOffset = rcScreen.left;



            if (textFlags & GT_CENTER)
                lineXOffset += static_cast<long>((static_cast<float>(textRegionWidth) / 2.0f) - (static_cast<float>(lineWidth) / 2.0f));
            else if (textFlags & GT_RIGHT)
                lineXOffset += textRegionWidth - lineWidth;
            //else if(textFlags & GT_LEFT)
            //  lineXOffset = 0;


            //get the rects for this line
            for (unsigned int j = 0; j < thisLine.size(); ++j)
            {
                auto thisCharRect = font->mFontType->GetCharRect(thisLine[j]);
                OffsetRect(thisCharRect, lineXOffset, currY - fontHeight);

                mCharacterRects[charIndex] = thisCharRect;

                long left = j != 0 ? mCharacterBBs[charIndex - 1].right : thisCharRect.left;
                SetRect(mCharacterBBs[charIndex], left, currY, left + font->mFontType->GetCharAdvance(thisLine[j]), currY - leading);

                lineXOffset += font->mFontType->GetCharAdvance(thisLine[j]);
                charIndex++;
            }



            currY -= leading;
        }
    }



    /*
    
    Get Rendering offset and count based on scroll bar position
    
    */
    auto sbPos = mScrollBar->GetTrackPos();
    auto lastLine = mScrollBar->GetPageSize() + sbPos;
    mRenderOffset = 0;
    mRenderCount = 0;
    for (auto it = 0; it < textLines.size(); ++it)
    {
        if (it < sbPos)
            mRenderOffset += textLines[it].size();
        else if (it < lastLine)
            mRenderCount += textLines[it].size();
    }

    NOEXCEPT_REGION_END
}

//--------------------------------------------------------------------------------------
void EditBox::UpdateCharRectsSingleline() noexcept
{
    NOEXCEPT_REGION_START

    Rect rcScreen = mTextRegion;
    auto dlgRect = mDialog.GetRegion();
    mDialog.ScreenSpaceToGLSpace(rcScreen);

    auto &element = mElements[0];
    auto textFlags = element.mTextFormatFlags;
    auto font = mDialog.GetFont(element.mFontIndex);
    auto fontHeight = font->mFontType->mHeight;
    auto leading = font->mLeading;

    std::wstring str = GetText();

    mCharacterRects.resize(str.size());
    mCharacterBBs.resize(str.size());

    //get the vertical alignment
    long yPos = rcScreen.top;
    if (textFlags | GT_VCENTER)
    {
        yPos = rcScreen.top - (RectHeight(rcScreen) - fontHeight) / 2;
    }
    else if (textFlags | GT_BOTTOM)
    {
        yPos = rcScreen.bottom + fontHeight;
    }

    //update the scroll bar
    mScrollBar->SetTrackRange(0, str.size());

    //get the horizontal offset based on the scroll bar position
    long currX = rcScreen.left;
    {
        long offset = 0;
        for (Value i = 0; i < mScrollBar->GetTrackPos(); ++i)
        {
            offset -= font->mFontType->GetCharAdvance(str[i]);
        }

        currX += offset;
    }

    //no horizontal alignment on single-line edit boxes (TODO)

    //get the character rects
    unsigned int i = 0;
    for (auto it : str)
    {
        auto thisCharRect = font->mFontType->GetCharRect(it);
        OffsetRect(thisCharRect, currX, yPos - fontHeight);

        mCharacterRects[i] = thisCharRect;

        long left = i != 0 ? mCharacterBBs[i - 1].right : thisCharRect.left;
        SetRect(mCharacterBBs[i], left, yPos, left + font->mFontType->GetCharAdvance(it), yPos - leading);

        ++i;
        currX += font->mFontType->GetCharAdvance(it);
    }

    //get the rendering offset based on the scroll bar position
    mRenderOffset = mScrollBar->GetTrackPos();
    mRenderCount = 0;
    {
        long tmpWidth = 0;
        long textRectWidth = RectWidth(mTextRegion);
        for (i = mScrollBar->GetTrackPos(); i < str.size(); ++i)
        {
            tmpWidth += font->mFontType->GetCharAdvance(str[i]);
            if (tmpWidth > textRectWidth)
                break;

            ++mRenderCount;
        }
    }

    NOEXCEPT_REGION_END
}

//--------------------------------------------------------------------------------------
void EditBox::BufferCharRects() noexcept
{
    NOEXCEPT_REGION_START

    auto str = GetText();

    //make sure there are characters to load
    if (str.size() == 0)
        return;

    auto font = mDialog.GetFont(GetElement(0).mFontIndex);
    /*
    
    Buffer data into OpenGL
    
    */
    GLVector<TextVertexStruct> textVertices = TextVertexStruct::MakeMany(str.size() * 4);
    std::vector<glm::u32vec3> indices;
    indices.resize(str.size() * 2);

    float z = _NEAR_BUTTON_DEPTH;
    for (unsigned int i = 0; i < str.size(); ++i)
    {
        auto &glyph = mCharacterRects[i];
        auto ch = str[i];


        auto UV = font->mFontType->GetCharTexRect(ch);

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
    }

    mTextDataBuffer->BufferData(textVertices);
    mTextDataBuffer->BufferIndices(indices);

    NOEXCEPT_REGION_END
}

/*
======================================================================================================================================================================================================
 Text Functions


*/
namespace Text
{

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

void TextHelper::DrawTextLine(const std::wstring& text)
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

void TextHelper::DrawTextLineBase(const Rect& rc, Bitfield flags, const std::wstring& text)
{
    mManager->GetFontNode(mFontIndex)->mLeading = mLeading;
    Text::DrawText(mManager->GetFontNode(mFontIndex), text, rc, mColor, flags, true);
}

void TextHelper::End() noexcept
{
    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_CLAMP);//set this back because it is the default
}
}
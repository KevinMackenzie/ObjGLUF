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

#include "GLUFGui.h"
#include <cassert>


namespace GLUF {

/*

Random Text Helping functions

================================================= TODO: ==================================================
    Relocate these methods into a more sensible spot/container/namespace/whatever
        Probably in a source file instead of a header file
*/
namespace Text {
glm::mat4 g_TextOrtho;


void BeginText(const glm::mat4 &orthoMatrix);


void DrawText(const FontNodePtr &font, const std::wstring &text, const Rect &rect, const Color &color, Bitfield textFlags, bool hardRect = false);
void EndText(const FontPtr &font, const VertexArrayPtr &data, const Color &textColor, const glm::mat4 &projMatrix = g_TextOrtho);

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

#define _WHEEL_DELTA 400//TODO:

//this is just a constant to be a little bit less windows api dependent (TODO: make this a setting)
unsigned int GetCaretBlinkTime() {
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
void GLFWWindowPosCallback(GLFWwindow *, int x, int y) {
    MessageProcedure(POS, x, y, 0, 0);
}

//--------------------------------------------------------------------------------------
void GLFWWindowSizeCallback(GLFWwindow *, int width, int height) {
    MessageProcedure(RESIZE, width, height, 0, 0);
}

//--------------------------------------------------------------------------------------
void GLFWWindowCloseCallback(GLFWwindow *) {
    MessageProcedure(CLOSE, 0, 0, 0, 0);
}
/*
//--------------------------------------------------------------------------------------
void GLFWWindowRefreshCallback(GLFWwindow*)
{
    MessageProcedure(REFRESH, 0, 0, 0, 0);
}*/

//--------------------------------------------------------------------------------------
void GLFWWindowFocusCallback(GLFWwindow *, int focused) {
    MessageProcedure(FOCUS, focused, 0, 0, 0);
}

//--------------------------------------------------------------------------------------
void GLFWWindowIconifyCallback(GLFWwindow *, int iconified) {
    MessageProcedure(ICONIFY, iconified, 0, 0, 0);
}

//--------------------------------------------------------------------------------------
void GLFWFrameBufferSizeCallback(GLFWwindow *, int width, int height) {
    MessageProcedure(FRAMEBUFFER_SIZE, width, height, 0, 0);
}

/*
===================================================================================================
GLFW Input Callback

*/

//--------------------------------------------------------------------------------------
void GLFWMouseButtonCallback(GLFWwindow *, int button, int action, int mods) {
    MessageProcedure(MB, button, action, mods, 0);
}

//--------------------------------------------------------------------------------------
void GLFWCursorPosCallback(GLFWwindow *, double xPos, double yPos) {
    MessageProcedure(CURSOR_POS, (int) xPos, (int) yPos, 0, 0);
}

//--------------------------------------------------------------------------------------
void GLFWCursorEnterCallback(GLFWwindow *, int entered) {
    MessageProcedure(CURSOR_ENTER, entered, 0, 0, 0);
}

//--------------------------------------------------------------------------------------
void GLFWScrollCallback(GLFWwindow *, double xoffset, double yoffset) {
    MessageProcedure(SCROLL, (int) (xoffset * 1000.0), (int) (yoffset * 1000.0), 0, 0);
}

//--------------------------------------------------------------------------------------
void GLFWKeyCallback(GLFWwindow *, int key, int scancode, int action, int mods) {
    MessageProcedure(KEY, key, scancode, action, mods);
}

//--------------------------------------------------------------------------------------
void GLFWCharCallback(GLFWwindow *, unsigned int codepoint) {
    MessageProcedure(UNICODE_CHAR, (int) codepoint, 0, 0, 0);
}


CallbackFuncPtr g_pCallback;

//--------------------------------------------------------------------------------------
void MessageProcedure(MessageType msg, int param1, int param2, int param3, int param4) {
    if (g_pCallback(msg, param1, param2, param3, param4)) {

    }

    //todo: anything else to do?
}


/*
======================================================================================================================================================================================================
Various Structs Used For UI


*/
struct ScreenVertex {
    glm::vec3 pos;
    Color color;
    glm::vec2 uv;
};

struct ScreenVertexUntex {
    glm::vec3 pos;
    Color color;
};


/*
======================================================================================================================================================================================================
Initialization and Globals


*/



unsigned short g_WndWidth = 0;
unsigned short g_WndHeight = 0;

FontPtr g_DefaultFont = nullptr;
ProgramPtr g_UIProgram = nullptr;
ProgramPtr g_UIProgramUntex = nullptr;


GLFWwindow *g_pGLFWWindow;
GLuint g_pControlTexturePtr;
int g_ControlTextureResourceManLocation = -1;


//uniform locations
struct UIShaderLocations_t {
    GLuint position = 0;
    GLuint color = 0;
    GLuint uv = 0;
    GLuint ortho = 0;
    GLuint sampler = 0;

} g_UIShaderLocations;

struct UIShaderLocationsUntex_t {
    GLuint position = 0;
    GLuint color = 0;
    GLuint ortho = 0;

} g_UIShaderLocationsUntex;


/*
======================================================================================================================================================================================================
Shaders for UI Elements


*/

std::string g_UIShaderVert =
        "#version 120                                                        \n"\
"attribute vec3 _Position;                                            \n"\
"attribute vec2 _UV;                                                \n"\
"attribute vec4 _Color;                                                \n"\
"uniform   mat4 _Ortho;                                                \n"\
"varying vec4 Color;                                                \n"\
"varying vec2 uvCoord;                                                \n"\
"void main(void)                                                    \n"\
"{                                                                    \n"\
"    gl_Position = vec4(_Position, 1.0f) * _Ortho;                    \n"\
"    Color = _Color;                                                    \n"\
"   uvCoord = abs(vec2(0.0f, 1.0f) - _UV);                            \n"\
"}                                                                    \n";
/*the V's are inverted because the texture is loaded bottom to top*/


std::string g_UIShaderFrag =
        "#version 120                                                        \n"\
"varying vec4 Color;                                                \n"\
"varying vec2 uvCoord;                                                \n"\
"uniform sampler2D _TS;                                                \n"\
"void main(void)                                                    \n"\
"{                                                                    \n"\
"    //Color = vec4(1.0f, 0.0, 0.0f, 1.0f);                            \n"\
"    //Color = fs_in.Color;                                            \n"\
"   vec4 oColor = texture2D(_TS, uvCoord);                            \n"\
"    oColor = vec4(                                                  \n"\
"       oColor.r * Color.r,                                         \n"\
"       oColor.g * Color.g,                                         \n"\
"       oColor.b * Color.b,                                         \n"\
"       oColor.a * Color.a);                                        \n"\
"    gl_FragColor = oColor;                                            \n"\
"}                                                                    \n";

std::string g_UIShaderFragUntex =
        "#version 120                                                        \n"\
"varying vec4 Color;                                                \n"\
"varying vec2 uvCoord;                                                \n"\
"void main(void)                                                    \n"\
"{                                                                    \n"\
"    gl_FragColor = Color;                                            \n"\
"}                                                                    \n";


/*
======================================================================================================================================================================================================
Initialization Functions


*/

//--------------------------------------------------------------------------------------
bool InitGui(GLFWwindow *pInitializedGLFWWindow, CallbackFuncPtr callback, GLuint controltex) {
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
    sources.insert({SH_VERTEX_SHADER, g_UIShaderVert});
    sources.insert({SH_FRAGMENT_SHADER, g_UIShaderFrag});
    SHADERMANAGER.CreateProgram(g_UIProgram, sources);
    sources.clear();

    sources.insert({SH_VERTEX_SHADER, g_UIShaderVert});
    sources.insert({SH_FRAGMENT_SHADER, g_UIShaderFragUntex});
    SHADERMANAGER.CreateProgram(g_UIProgramUntex, sources);
    sources.clear();



    //load the locations
    g_UIShaderLocations.position = SHADERMANAGER.GetShaderVariableLocation(g_UIProgram, GLT_ATTRIB, "_Position");
    g_UIShaderLocations.uv = SHADERMANAGER.GetShaderVariableLocation(g_UIProgram, GLT_ATTRIB, "_UV");
    g_UIShaderLocations.color = SHADERMANAGER.GetShaderVariableLocation(g_UIProgram, GLT_ATTRIB, "_Color");
    g_UIShaderLocations.ortho = SHADERMANAGER.GetShaderVariableLocation(g_UIProgram, GLT_UNIFORM, "_Ortho");
    g_UIShaderLocations.sampler = SHADERMANAGER.GetShaderVariableLocation(g_UIProgram, GLT_UNIFORM, "_TS");

    g_UIShaderLocationsUntex.position = SHADERMANAGER.GetShaderVariableLocation(g_UIProgram, GLT_ATTRIB, "_Position");
    g_UIShaderLocationsUntex.color = SHADERMANAGER.GetShaderVariableLocation(g_UIProgram, GLT_ATTRIB, "_Color");
    g_UIShaderLocationsUntex.ortho = SHADERMANAGER.GetShaderVariableLocation(g_UIProgram, GLT_UNIFORM, "_Ortho");

    //create the text arrrays
    /*glGenVertexArrayBindVertexArray(&g_TextVAO);
    glGenBuffers(1, &g_TextPos);
    glGenBuffers(1, &g_TextTexCoords);

    glBindBuffer(GL_ARRAY_BUFFER, g_TextPos);
    glVertexAttribPointer(g_TextShaderLocations.position, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

    glBindBuffer(GL_ARRAY_BUFFER, g_TextTexCoords);
    glVertexAttribPointer(g_TextShaderLocations.uv, 2, GL_FLOAT, GL_FALSE, 0, nullptr);

    glBindVertexArray(0);*/


    /*static std::vector<glm::u32vec3> indices =
    {
        { 3, 0, 2 },
        { 2, 0, 1 }
    };

    g_TextVertexArray->BufferIndices(indices);*/

    //load the texture for the controls
    g_pControlTexturePtr = controltex;


    int w, h;
    glfwGetWindowSize(g_pGLFWWindow, &w, &h);
    g_WndHeight = h;
    g_WndWidth = w;

    return true;
}

//--------------------------------------------------------------------------------------
CallbackFuncPtr ChangeCallbackFunc(CallbackFuncPtr newCallback) {
    CallbackFuncPtr tmp = g_pCallback;
    g_pCallback = newCallback;
    return tmp;
}

//--------------------------------------------------------------------------------------
void Terminate() {
    FT_Done_FreeType(g_FtLib);
}


const std::wstring g_Charsets[] =
        {
                L" !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~",
                L"0123456789",
                L"abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ",
                L"abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789"
        };

//--------------------------------------------------------------------------------------
bool CharsetContains(int codepoint, Charset charset) {
    switch (charset) {
        case Unicode:
            return true;
        default:
            for (unsigned int i = 0; i < g_Charsets[charset].size(); ++i) {
                if (g_Charsets[charset][i] == codepoint)
                    return true;
            }
            return false;
    }
}

//--------------------------------------------------------------------------------------
bool CharsetContains(const std::wstring &codepoint, Charset charset) {
    switch (charset) {
        case Unicode:
            return true;
        default:
            for (auto it : codepoint) {
                if (!CharsetContains(it, charset))
                    return false;
            }
            return false;
    }
}

}

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

// ObjGLUFUF.cpp : Defines the exported functions for the DLL application.
//

#define USING_ASSIMP
#include "ObjGLUF.h"
#include <fstream>
#include <sstream>
#include <GLFW/glfw3.h>


/*

Internal Macros

*/
#define RETHROW throw

namespace GLUF
{

/*
GLExtensions

    Data Members:
        'mExtensionList': a map of the extensions.  Maps are used for fast lookups O(log n), however only the key is needed
        'mBufferedExtensionList': a map of extensions that have been requested, and true or false depending on whether they are supported
        'mCachedExtensionVector': a variable whose sole purpose is act as a point of data to reference to when calling 'GetGLExtensions()'

    Note:
        even though 'std::map' may have a O(log n) lookup speed, keeping a separate map for extensions that are actually requested only slightly slows down
            randomly accessed extension support

*/
class GLExtensions
{
    std::map<std::string, bool> mExtensionList;

    std::map<std::string, bool> mBufferedExtensionList;

    std::vector<std::string> mCachedExtensionVector;

    //--------------------------------------------------------------------------------------
    void Init(const std::vector<std::string>& extensions)
    {
        for (auto it : extensions)
        {
            mExtensionList.insert({ it, true });
        }

        mCachedExtensionVector = extensions;
    }

    friend bool InitOpenGLExtensions();
public:

    //--------------------------------------------------------------------------------------
    operator const std::vector<std::string>&() const
    {
        return mCachedExtensionVector;
    }

    //--------------------------------------------------------------------------------------
    bool HasExtension(const std::string& str)
    {
        //first look in the buffered list
        auto buffIt = mBufferedExtensionList.find(str);
        if (buffIt != mBufferedExtensionList.end())
            return buffIt->second;

        bool success = false;
        auto it = mExtensionList.find(str);
        if (it == mExtensionList.end())
        {
            //success = false; //defaults to false
        }
        else
        {
            success = true;
        }

        //buffer this one
        mBufferedExtensionList.insert({ str, success });
        
        //then return
        return success;
    }
};

class UnsupportedExtensionException : public Exception
{
    const std::string mExt;
public:
    virtual const char* what() const noexcept override
    {
        std::stringstream ss;
        ss << "Unsupported Extension: \"" << mExt << "\"";
        return ss.str().c_str();
    }

    UnsupportedExtensionException(const std::string& ext) : mExt(ext)
    {
        EXCEPTION_CONSTRUCTOR_BODY
    }
};

#define EXT_TO_TEXT(ext) #ext
#define ASSERT_EXTENTION(ext) if(!gExtensions.HasExtension(#ext)) GLUF_CRITICAL_EXCEPTION(UnsupportedExtensionException(#ext));

//for switch statements based on opengl extensions being present
#define SWITCH_GL_EXT(ext) if(gExtensions.HasExtension(#ext))

//for switch statements based on opengl version
#define SWITCH_GL_VERSION if(false){}
#define GL_VERSION_GREATER(val) else if(gGLVersion2Digit > val)
#define GL_VERSION_GREATER_EQUAL(val) else if(gGLVersion2Digit >= val)
#define GL_VERSION_LESS(val) else if(gGLVersion2Digit < val)
#define GL_VERSION_LESS_EQUAL(val) else if(gGLVersion2Digit <= val)

GLExtensions gExtensions;

/*
=======================================================================================================================================================================================================
Global Instances


*/

ErrorMethod g_ErrorMethod;
//BufferManager g_BufferManager;
ShaderManager g_ShaderManager;


/*
=======================================================================================================================================================================================================
Premade Attribute Info's which comply with Assimp capibilities, but are not exclusive to them


*/

//initialize the standard vertex attributes
//                            Name            bytes,    count,    location,                    type,     offset
const VertexAttribInfo    g_attribPOS     = { 4,        3,        GLUF_VERTEX_ATTRIB_POSITION, GL_FLOAT, 0 };
const VertexAttribInfo    g_attribNORM    = { 4,        3,        GLUF_VERTEX_ATTRIB_NORMAL,   GL_FLOAT, 0 };
const VertexAttribInfo    g_attribUV0     = { 4,        2,        GLUF_VERTEX_ATTRIB_UV0,      GL_FLOAT, 0 };
const VertexAttribInfo    g_attribUV1     = { 4,        2,        GLUF_VERTEX_ATTRIB_UV1,      GL_FLOAT, 0 };
const VertexAttribInfo    g_attribUV2     = { 4,        2,        GLUF_VERTEX_ATTRIB_UV2,      GL_FLOAT, 0 };
const VertexAttribInfo    g_attribUV3     = { 4,        2,        GLUF_VERTEX_ATTRIB_UV3,      GL_FLOAT, 0 };
const VertexAttribInfo    g_attribUV4     = { 4,        2,        GLUF_VERTEX_ATTRIB_UV4,      GL_FLOAT, 0 };
const VertexAttribInfo    g_attribUV5     = { 4,        2,        GLUF_VERTEX_ATTRIB_UV5,      GL_FLOAT, 0 };
const VertexAttribInfo    g_attribUV6     = { 4,        2,        GLUF_VERTEX_ATTRIB_UV6,      GL_FLOAT, 0 };
const VertexAttribInfo    g_attribUV7     = { 4,        2,        GLUF_VERTEX_ATTRIB_UV7,      GL_FLOAT, 0 };
const VertexAttribInfo    g_attribCOLOR0  = { 4,        4,        GLUF_VERTEX_ATTRIB_COLOR0,   GL_FLOAT, 0 };
const VertexAttribInfo    g_attribCOLOR1  = { 4,        4,        GLUF_VERTEX_ATTRIB_COLOR1,   GL_FLOAT, 0 };
const VertexAttribInfo    g_attribCOLOR2  = { 4,        4,        GLUF_VERTEX_ATTRIB_COLOR2,   GL_FLOAT, 0 };
const VertexAttribInfo    g_attribCOLOR3  = { 4,        4,        GLUF_VERTEX_ATTRIB_COLOR3,   GL_FLOAT, 0 };
const VertexAttribInfo    g_attribCOLOR4  = { 4,        4,        GLUF_VERTEX_ATTRIB_COLOR4,   GL_FLOAT, 0 };
const VertexAttribInfo    g_attribCOLOR5  = { 4,        4,        GLUF_VERTEX_ATTRIB_COLOR5,   GL_FLOAT, 0 };
const VertexAttribInfo    g_attribCOLOR6  = { 4,        4,        GLUF_VERTEX_ATTRIB_COLOR6,   GL_FLOAT, 0 };
const VertexAttribInfo    g_attribCOLOR7  = { 4,        4,        GLUF_VERTEX_ATTRIB_COLOR7,   GL_FLOAT, 0 };
const VertexAttribInfo    g_attribTAN     = { 4,        3,        GLUF_VERTEX_ATTRIB_TAN,      GL_FLOAT, 0 };
const VertexAttribInfo    g_attribBITAN   = { 4,        3,        GLUF_VERTEX_ATTRIB_BITAN,    GL_FLOAT, 0 };


VertexAttribMap g_stdAttrib;

/*

Helpful OpenGL Constants

*/

GLuint gGLVersionMajor = 0;
GLuint gGLVersionMinor = 0;
GLuint gGLVersion2Digit = 0;

//--------------------------------------------------------------------------------------
GLuint GetGLVersionMajor()
{
    return gGLVersionMajor;
}

//--------------------------------------------------------------------------------------
GLuint GetGLVersionMinor()
{
    return gGLVersionMinor;
}

//--------------------------------------------------------------------------------------
GLuint GetGLVersion2Digit()
{
    return gGLVersion2Digit;
}


/*
======================================================================================================================================================================================================
Debugging Macros and Setup Functions

*/

//--------------------------------------------------------------------------------------
void RegisterErrorMethod(ErrorMethod method)
{
    g_ErrorMethod = method;
}

//--------------------------------------------------------------------------------------
void GLFWErrorMethod(int error, const char* description)
{
    std::stringstream ss;
    ss << "GLFW ERROR: Error Code:" << error << "; " << description << std::endl;
    GLUF_ERROR(ss.str());
}


//--------------------------------------------------------------------------------------
ErrorMethod GetErrorMethod()
{
    return g_ErrorMethod;
}

    /*
    const char* StandardVertexShader =
    "#version 430 core    layout(std140, binding = 0) uniform MatrixTransformations    {    mat4 m;    mat4 v; mat4 p; mat4 mv; mat4 mvp;};    out VS_OUT    {    vec2 uvCoord;    } vs_out; ";

    const char* StandardFragmentShader =
    "#version 430 core    layout(std140, binding = 0) uniform MatrixTransformations    {    mat4 m;    mat4 v; mat4 p; mat4 mv; mat4 mvp;};    in VS_OUT    {    vec2 uvCoord;    } fs_in; layout(location = 0) out vec4 color; layout(location = 5) uniform sampler2D TextureSampler;";
    */


/*
======================================================================================================================================================================================================
 API Core Controller Methods

*/
//--------------------------------------------------------------------------------------
bool Init()
{
    glfwSetErrorCallback(GLFWErrorMethod);
    if (!glfwInit())
    {
        GLUF_ERROR("GLFW Initialization Failed!");
        return false;
    }


    return true;
}

//--------------------------------------------------------------------------------------
bool InitOpenGLExtensions()
{
    int err = gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
    if (err == 0)
    {
        GLUF_ERROR("Failed to initialize OpenGL Extensions using GLAD");
        return false;
    }

    //get the list of extensions
    const char* extensions = (const char*)glGetString(GL_EXTENSIONS);
    gExtensions.Init(SplitStr(extensions, ' '));


    //setup global openGL version
    const char* version = (const char*)glGetString(GL_VERSION);

    std::vector<std::string> vsVec;
    vsVec = SplitStr((const char*)version, '.');//TODO: global openGL version
    gGLVersionMajor = std::stoi(vsVec[0]);
    gGLVersionMinor = std::stoi(vsVec[1]);
    gGLVersion2Digit = gGLVersionMajor * 10 + gGLVersionMinor;

    //if the version is less than 2.1, do not allow to continue
    if ((gGLVersionMajor == 2 && gGLVersionMinor < 1) || gGLVersionMajor < 2)
    {
        GLUF_ERROR("OpenGL Version To Low!");
        return false;
    }

    g_stdAttrib.insert(VertexAttribPair(GLUF_VERTEX_ATTRIB_POSITION, g_attribPOS));
    g_stdAttrib.insert(VertexAttribPair(GLUF_VERTEX_ATTRIB_NORMAL, g_attribNORM));
    g_stdAttrib.insert(VertexAttribPair(GLUF_VERTEX_ATTRIB_UV0, g_attribPOS));
    g_stdAttrib.insert(VertexAttribPair(GLUF_VERTEX_ATTRIB_COLOR0, g_attribCOLOR0));
    g_stdAttrib.insert(VertexAttribPair(GLUF_VERTEX_ATTRIB_TAN, g_attribTAN));
    g_stdAttrib.insert(VertexAttribPair(GLUF_VERTEX_ATTRIB_BITAN, g_attribBITAN));

    g_stdAttrib.insert(VertexAttribPair(GLUF_VERTEX_ATTRIB_UV1, g_attribUV1));
    g_stdAttrib.insert(VertexAttribPair(GLUF_VERTEX_ATTRIB_UV2, g_attribUV2));
    g_stdAttrib.insert(VertexAttribPair(GLUF_VERTEX_ATTRIB_UV3, g_attribUV3));
    g_stdAttrib.insert(VertexAttribPair(GLUF_VERTEX_ATTRIB_UV4, g_attribUV4));
    g_stdAttrib.insert(VertexAttribPair(GLUF_VERTEX_ATTRIB_UV5, g_attribUV5));
    g_stdAttrib.insert(VertexAttribPair(GLUF_VERTEX_ATTRIB_UV6, g_attribUV6));
    g_stdAttrib.insert(VertexAttribPair(GLUF_VERTEX_ATTRIB_UV7, g_attribUV7));

    g_stdAttrib.insert(VertexAttribPair(GLUF_VERTEX_ATTRIB_COLOR1, g_attribCOLOR1));
    g_stdAttrib.insert(VertexAttribPair(GLUF_VERTEX_ATTRIB_COLOR2, g_attribCOLOR2));
    g_stdAttrib.insert(VertexAttribPair(GLUF_VERTEX_ATTRIB_COLOR3, g_attribCOLOR3));
    g_stdAttrib.insert(VertexAttribPair(GLUF_VERTEX_ATTRIB_COLOR4, g_attribCOLOR4));
    g_stdAttrib.insert(VertexAttribPair(GLUF_VERTEX_ATTRIB_COLOR5, g_attribCOLOR5));
    g_stdAttrib.insert(VertexAttribPair(GLUF_VERTEX_ATTRIB_COLOR6, g_attribCOLOR6));
    g_stdAttrib.insert(VertexAttribPair(GLUF_VERTEX_ATTRIB_COLOR7, g_attribCOLOR7));

    return true;
}


//--------------------------------------------------------------------------------------
const std::vector<std::string>& GetGLExtensions()
{
    return gExtensions;
}

//void Terminate()
//{
//
//}
//See Gui for this function; it destroys the font library

/*
======================================================================================================================================================================================================
Statistics

*/

namespace FrameStats
{
    StatsData g_StatsData;
}

//--------------------------------------------------------------------------------------
void Stats_func()
{
    using namespace FrameStats;

    ++g_StatsData.mFrameCount;

    double thisFrame = GetTime();
    if (thisFrame - g_StatsData.mPreviousFrame < g_StatsData.mUpdateInterval)
        return;//don't update statistics

    double deltaTime = thisFrame - g_StatsData.mPreviousFrame;

    g_StatsData.mCurrFPS = (float)((double)g_StatsData.mFrameCount / deltaTime);

    g_StatsData.mFrameCount = 0;//reset the frame count

    std::wstringstream wss;
    //for now, only use whole FPS's
    wss << (unsigned int)g_StatsData.mCurrFPS << " fps";

    //update device statistics
    //TODO:

    g_StatsData.mPreviousFrame = thisFrame;
}

//--------------------------------------------------------------------------------------
const std::wstring& GetFrameStatsString()
{
    return FrameStats::g_StatsData.mFormattedStatsData;
}

//--------------------------------------------------------------------------------------
std::wstring g_DeviceStatusCache = L"WIP";
const std::wstring& GetDeviceStatus()
{
    return g_DeviceStatusCache;
}

//--------------------------------------------------------------------------------------
const StatsData& GetFrameStats()
{
    return FrameStats::g_StatsData;
}

/*
======================================================================================================================================================================================================
IO and Stream Utilities

*/

//--------------------------------------------------------------------------------------
void LoadFileIntoMemory(const std::string& path, std::string& textMemory)
{
    //try to open file
    std::ifstream inFile;
    inFile.exceptions(std::ios_base::failbit | std::ifstream::badbit);
    try
    {
        inFile.open(path, std::ios_base::in);
    }
    catch (std::ios_base::failure e)
    {
        GLUF_ERROR_LONG("Failed to Open File: " << e.what());
        RETHROW;
    }

    //delete anything already in here
    textMemory.clear();

    //get file length
    inFile.seekg(0, std::ios::end);
    unsigned int rawSize = static_cast<unsigned int>(inFile.tellg());
    inFile.seekg(0, std::ios::beg);

    //resize the vector
    textMemory.reserve(rawSize);//is this line needed?
    textMemory.resize(rawSize);

    //try reading the memory
    try
    {
        inFile.read(&textMemory[0], rawSize);
        inFile.close();
    }
    catch (std::ios_base::failure e)
    {
        if (inFile.is_open())
            inFile.close();
        GLUF_ERROR_LONG("Failed to load file into memory:" << e.what());
        RETHROW;
    }
}

//--------------------------------------------------------------------------------------
void LoadFileIntoMemory(const std::string& path, std::vector<char>& binMemory)
{
    //try to open file
    std::ifstream inFile;
    inFile.exceptions(std::ios_base::failbit | std::ifstream::badbit);
    try
    {
        inFile.open(path, std::ios::binary | std::ios_base::in);
    }
    catch (std::ios_base::failure e)
    {
        GLUF_ERROR_LONG("Failed to Open File: " << e.what());
        RETHROW;
    }

    // TODO: we don't need to delete it if we're going to resize it below
    //delete anything already in here
    binMemory.clear();

    //get file length
    inFile.seekg(0, std::ios::end);
    unsigned int rawSize = static_cast<unsigned int>(inFile.tellg());
    inFile.seekg(0, std::ios::beg);

    //resize the vector
    binMemory.reserve(rawSize);//is this line needed?
    binMemory.resize(rawSize);

    //try reading the memory
    try
    {
        inFile.read(&binMemory[0], rawSize);
        inFile.close();
    }
    catch (std::ios_base::failure e)
    {
        if (inFile.is_open())
            inFile.close();
        GLUF_ERROR_LONG("Failed to load file into memory:" << e.what());
        RETHROW;
    }
}

//--------------------------------------------------------------------------------------
void LoadBinaryArrayIntoString(char* rawMemory, std::size_t size, std::string& outString)
{
    if (rawMemory == nullptr)
    {
        GLUF_ERROR("Cannot Load Null Ptr Into String!");
        GLUF_CRITICAL_EXCEPTION(std::invalid_argument("Cannot Load Null Ptr Into String!"));
    }
    
    //for automatic deletion if exception thrown
    auto streamBuf = std::make_shared<MemStreamBuf>(rawMemory, size);
   
    std::istream indata(streamBuf.get());
    indata.exceptions(std::ifstream::badbit);
    char ch = ' ';

    try
    {
        while (indata.get(ch))
            outString += ch;
    }
    catch (std::ios_base::failure e)
    {
        outString.clear();
        GLUF_ERROR_LONG("Failed to load binary array into string: " << e.what());
        RETHROW;
    }
}


//--------------------------------------------------------------------------------------
void LoadBinaryArrayIntoString(const std::vector<char>& rawMemory, std::string& outString)
{
    //const cast is OK to use here, because we know 'rawMemory' will not be modified
    LoadBinaryArrayIntoString(const_cast<char*>(&rawMemory[0]), rawMemory.size(), outString);
}


/*
======================================================================================================================================================================================================
Misc.  Classes


*/

glm::mat4 MatrixStack::mIdentity = glm::mat4();

//--------------------------------------------------------------------------------------
void MatrixStack::Push(const glm::mat4& matrix)
{
    if (mStack.size() != 0)
    {
        //if there are already things on the stack, instead of multiplying through EVERY TIME TOP IS CALLED, make it more efficient 
        //so the top is ALWAYS a concatenation
        glm::mat4 transformed = matrix * Top();

        mStack.push(transformed);
    }
    else
    {
        mStack.push(matrix);
    }
}

//--------------------------------------------------------------------------------------
void MatrixStack::Pop(void)
{
    mStack.pop();
}

//--------------------------------------------------------------------------------------
const glm::mat4& MatrixStack::Top(void) const
{
    //if it is empty, then we want to return the identity
    if (mStack.size() == 0)
        return mIdentity;

    return mStack.top();
}

//--------------------------------------------------------------------------------------
size_t MatrixStack::Size(void) const
{
    return mStack.size();
}

//--------------------------------------------------------------------------------------
bool MatrixStack::Empty(void) const
{
    return mStack.empty();
}


/*
======================================================================================================================================================================================================
OpenGL Basic Data Structures and Operators

*/

//--------------------------------------------------------------------------------------
bool PtInRect(const Rect& rect, const Point& pt)
{
    //for the first comparison, it is impossible for both statements to be false, 
    //because if the y is greater than the top, it is automatically greater than the bottom, and vise versa
    return    (pt.y >= rect.bottom && pt.y <= rect.top) &&
        (pt.x <= rect.right && pt.x >= rect.left);
}

//--------------------------------------------------------------------------------------
void SetRectEmpty(Rect& rect)
{
    rect.top = rect.bottom = rect.left = rect.right = 0;
}

//--------------------------------------------------------------------------------------
void SetRect(Rect& rect, long left, long top, long right, long bottom)
{
    rect.top = top;
    rect.bottom = bottom;
    rect.left = left;
    rect.right = right;
}

//--------------------------------------------------------------------------------------
void SetRect(Rectf& rect, float left, float top, float right, float bottom)
{
    rect.top = top;
    rect.bottom = bottom;
    rect.left = left;
    rect.right = right;
}

//--------------------------------------------------------------------------------------
void OffsetRect(Rect& rect, long x, long y)
{
    rect.top += y;
    rect.bottom += y;
    rect.left += x;
    rect.right += x;
}

//--------------------------------------------------------------------------------------
void RepositionRect(Rect& rect, long newX, long newY)
{
    long deltaX = newX - rect.left;
    long deltaY = newY - rect.bottom;

    rect.left = newX; 
    rect.right += deltaX;
    rect.bottom = newY;
    rect.top += deltaY;
}

//--------------------------------------------------------------------------------------
long RectHeight(const Rect& rect)
{
    return rect.top - rect.bottom;
}

//--------------------------------------------------------------------------------------
long RectWidth(const Rect& rect)
{
    return rect.right - rect.left;
}


//--------------------------------------------------------------------------------------
void InflateRect(Rect& rect, long dx, long dy)
{
    long dx2 = dx / 2;
    long dy2 = dy / 2;
    rect.left -= dx2;
    rect.right += dx2;//remember to have opposites

    rect.top += dy2;
    rect.bottom -= dy2;
}

//--------------------------------------------------------------------------------------
void ResizeRect(Rect& rect, long newWidth, long newHeight)
{
    rect.top = rect.bottom + newHeight;
    rect.right = rect.left + newWidth;
}

//--------------------------------------------------------------------------------------
bool IntersectRect(const Rect& rect0, const Rect& rect1, Rect& rectIntersect)
{

    //Left
    if (rect0.left > rect1.left)
    {
        rectIntersect.left = rect0.left;
    }
    else
    {
        rectIntersect.left = rect1.left;
    }

    //Right
    if (rect0.right < rect1.right)
    {
        rectIntersect.right = rect0.right;
    }
    else
    {
        rectIntersect.right = rect1.right;
    }


    //Top
    if (rect0.top < rect1.top)
    {
        rectIntersect.top = rect0.top;
    }
    else
    {
        rectIntersect.top = rect1.top;
    }

    //Bottom
    if (rect0.bottom > rect1.bottom)
    {
        rectIntersect.bottom = rect0.bottom;
    }
    else
    {
        rectIntersect.bottom = rect1.bottom;
    }

    //this will ONLY happen if the do NOT intersect
    if (rectIntersect.left > rectIntersect.right || rectIntersect.top < rectIntersect.bottom)
    {
        SetRectEmpty(rectIntersect);
        return false;
    }

    return true;
}


/*
======================================================================================================================================================================================================
Datatype Conversion Functions

*/

//--------------------------------------------------------------------------------------
Color4f ColorToFloat(const Color& color)
{
    Color4f col;
    col.x = glm::clamp((float)color.x / 255.0f, 0.0f, 1.0f);
    col.y = glm::clamp((float)color.y / 255.0f, 0.0f, 1.0f);
    col.z = glm::clamp((float)color.z / 255.0f, 0.0f, 1.0f);
    col.w = glm::clamp((float)color.w / 255.0f, 0.0f, 1.0f);
    return col;
}

//--------------------------------------------------------------------------------------
Color3f ColorToFloat3(const Color& color)
{
    Color3f col;
    col.x = glm::clamp((float)color.x / 255.0f, 0.0f, 1.0f);
    col.y = glm::clamp((float)color.y / 255.0f, 0.0f, 1.0f);
    col.z = glm::clamp((float)color.z / 255.0f, 0.0f, 1.0f);
    return col;
}

//--------------------------------------------------------------------------------------
Point MultPoints(const Point& pt0, const Point& pt1)
{
    Point retPt;
    retPt.x = pt1.x * pt0.x;
    retPt.y = pt1.y * pt1.y;

    return retPt;
}


//--------------------------------------------------------------------------------------
glm::vec2 GetVec2FromRect(const Rect& rect, bool x, bool y)
{
    if (x)
        if (y)
            return glm::vec2(rect.right, rect.top);
        else
            return glm::vec2(rect.right, rect.bottom);
    else
        if (y)
            return glm::vec2(rect.left, rect.top);
        else
            return glm::vec2(rect.left, rect.bottom);
}

//--------------------------------------------------------------------------------------
glm::vec2 GetVec2FromRect(const Rectf& rect, bool x, bool y)
{
    if (x)
        if (y)
            return glm::vec2(rect.right, rect.top);
        else
            return glm::vec2(rect.right, rect.bottom);
    else
        if (y)
            return glm::vec2(rect.left, rect.top);
        else
            return glm::vec2(rect.left, rect.bottom);
}

#pragma warning (disable : 4244)
std::string WStringToString(const std::wstring& str) noexcept
{
    std::string temp(str.length(), ' ');
    std::copy(str.begin(), str.end(), temp.begin());
    return temp;
}

std::wstring StringToWString(const std::string& str) noexcept
{
    std::wstring temp(str.length(), L' ');
    std::copy(str.begin(), str.end(), temp.begin());
    return temp;
}

#pragma warning (default : 4244)



}

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

#include "ObjGLUF.h"
#include "Ext/Assimp.h"
#include "GLExtensions.h"
#include <fstream>
#include <sstream>
#include <GLFW/glfw3.h>


namespace GLUF
{

/*
=======================================================================================================================================================================================================
Global Instances


*/

ErrorMethod g_ErrorMethod;
//BufferManager g_BufferManager;





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




}

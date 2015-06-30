// ObjGLUFUF.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#define USING_ASSIMP
#include "ObjGLUF.h"
#include <fstream>
#include <sstream>
#include <GLFW/glfw3.h>
#include <vadefs.h>


/*

Internal Macros

*/
#define RETHROW throw;

namespace GLUF
{

/*
GLExtensions

    Data Members:
        'mExtensionList': a map of the extensions.  Maps are used for fast lookups O(log n), however only the key is needed
        'mBufferedExtensionList': a map of extensions that have been requested, and true or false depending on whether they are supported
        'mCachedExtensionVector': a variable whose sole purpose is act as a point of data to reference to when calling 'GLUFGetGLExtensions()'

    Note:
        even though 'std::map' may have a O(log n) lookup speed, keeping a separate map for extensions that are actually requested only slightly slows down
            randomly accessed extension support

*/
class GLExtensions
{
    std::map<std::string, bool> mExtensionList;

    std::map<std::string, bool> mBufferedExtensionList;

    std::vector<std::string> mCachedExtensionVector;

    void Init(const std::vector<std::string>& extensions)
    {
        for (auto it : extensions)
        {
            mExtensionList.insert({ it, true });
        }

        mCachedExtensionVector = extensions;
    }

    friend bool GLUFInitOpenGLExtensions();
public:

    operator const std::vector<std::string>&() const
    {
        return mCachedExtensionVector;
    }

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

class UnsupportedExtensionException : public GLUFException
{
    const std::string mExt;
public:
    virtual const char* what() const override
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

GLUFErrorMethod ErrorMethod;
//GLUFBufferManager g_BufferManager;
GLUFShaderManager g_ShaderManager;


/*
=======================================================================================================================================================================================================
Premade Attribute Info's which comply with Assimp capibilities, but are not exclusive to them


*/

//initialize the standard vertex attributes
//							Name				bytes,	count,	location,				    type,     offset
const GLUFVertexAttribInfo	g_attribPOS		= { 4,		3,		GLUF_VERTEX_ATTRIB_POSITION,GL_FLOAT, 0 };
const GLUFVertexAttribInfo	g_attribNORM	= { 4,		3,		GLUF_VERTEX_ATTRIB_NORMAL,	GL_FLOAT, 0 };
const GLUFVertexAttribInfo	g_attribUV0		= { 4,		2,		GLUF_VERTEX_ATTRIB_UV0,		GL_FLOAT, 0 };
const GLUFVertexAttribInfo	g_attribUV1		= { 4,		2,		GLUF_VERTEX_ATTRIB_UV1,		GL_FLOAT, 0 };
const GLUFVertexAttribInfo	g_attribUV2		= { 4,		2,      GLUF_VERTEX_ATTRIB_UV2,		GL_FLOAT, 0 };
const GLUFVertexAttribInfo	g_attribUV3		= { 4,		2,		GLUF_VERTEX_ATTRIB_UV3,		GL_FLOAT, 0 };
const GLUFVertexAttribInfo	g_attribUV4		= { 4,		2,		GLUF_VERTEX_ATTRIB_UV4,		GL_FLOAT, 0 };
const GLUFVertexAttribInfo	g_attribUV5		= { 4,		2,		GLUF_VERTEX_ATTRIB_UV5,		GL_FLOAT, 0 };
const GLUFVertexAttribInfo	g_attribUV6		= { 4,		2,		GLUF_VERTEX_ATTRIB_UV6,		GL_FLOAT, 0 };
const GLUFVertexAttribInfo	g_attribUV7		= { 4,		2,		GLUF_VERTEX_ATTRIB_UV7,		GL_FLOAT, 0 };
const GLUFVertexAttribInfo	g_attribCOLOR0	= { 4,		4,		GLUF_VERTEX_ATTRIB_COLOR0,	GL_FLOAT, 0 };
const GLUFVertexAttribInfo	g_attribCOLOR1	= { 4,		4,		GLUF_VERTEX_ATTRIB_COLOR1,	GL_FLOAT, 0 };
const GLUFVertexAttribInfo	g_attribCOLOR2	= { 4,		4,		GLUF_VERTEX_ATTRIB_COLOR2,	GL_FLOAT, 0 };
const GLUFVertexAttribInfo	g_attribCOLOR3	= { 4,		4,		GLUF_VERTEX_ATTRIB_COLOR3,	GL_FLOAT, 0 };
const GLUFVertexAttribInfo	g_attribCOLOR4	= { 4,		4,		GLUF_VERTEX_ATTRIB_COLOR4,	GL_FLOAT, 0 };
const GLUFVertexAttribInfo	g_attribCOLOR5	= { 4,		4,		GLUF_VERTEX_ATTRIB_COLOR5,	GL_FLOAT, 0 };
const GLUFVertexAttribInfo	g_attribCOLOR6	= { 4,		4,		GLUF_VERTEX_ATTRIB_COLOR6,	GL_FLOAT, 0 };
const GLUFVertexAttribInfo	g_attribCOLOR7	= { 4,		4,		GLUF_VERTEX_ATTRIB_COLOR7,	GL_FLOAT, 0 };
const GLUFVertexAttribInfo	g_attribTAN		= { 4,		3,		GLUF_VERTEX_ATTRIB_TAN,		GL_FLOAT, 0 };
const GLUFVertexAttribInfo	g_attribBITAN	= { 4,		3,		GLUF_VERTEX_ATTRIB_BITAN,	GL_FLOAT, 0 };


GLUFVertexAttribMap g_stdAttrib;

/*

Helpful OpenGL Constants

*/

GLuint gGLVersionMajor = 0;
GLuint gGLVersionMinor = 0;
GLuint gGLVersion2Digit = 0;


/*
======================================================================================================================================================================================================
Debugging Macros and Setup Functions

*/

void GLUFRegisterErrorMethod(GLUFErrorMethod method)
{
	ErrorMethod = method;
}

void GLFWErrorMethod(int error, const char* description)
{
	std::stringstream ss;
	ss << "GLFW ERROR: Error Code:" << error << "; " << description << std::endl;
	GLUF_ERROR(ss.str());
}


GLUFErrorMethod GLUFGetErrorMethod()
{
	return ErrorMethod;
}

	/*
	const char* StandardVertexShader =
	"#version 430 core	layout(std140, binding = 0) uniform MatrixTransformations	{	mat4 m;	mat4 v; mat4 p; mat4 mv; mat4 mvp;};	out VS_OUT	{	vec2 uvCoord;	} vs_out; ";

	const char* StandardFragmentShader =
	"#version 430 core	layout(std140, binding = 0) uniform MatrixTransformations	{	mat4 m;	mat4 v; mat4 p; mat4 mv; mat4 mvp;};	in VS_OUT	{	vec2 uvCoord;	} fs_in; layout(location = 0) out vec4 color; layout(location = 5) uniform sampler2D TextureSampler;";
	*/


/*
======================================================================================================================================================================================================
GLUF API Core Controller Methods

*/
bool GLUFInit()
{
	glfwSetErrorCallback(GLFWErrorMethod);
	if (!glfwInit())
	{
		GLUF_ERROR("GLFW Initialization Failed!");
		return false;
	}


	return true;
}

bool GLUFInitOpenGLExtensions()
{
    GLenum err = glewInit();
    if (err != GLEW_OK)
    {
        GLUF_ERROR("Failed to initialize OpenGL Extensions using GLEW");
        return false;
    }

    //get the list of extensions
    const char* extensions = (const char*)glGetString(GL_EXTENSIONS);
    gExtensions.Init(GLUFSplitStr(extensions, ' '));


    //setup global openGL version
    const char* version = (const char*)glGetString(GL_VERSION);

    std::vector<std::string> vsVec;
    vsVec = GLUFSplitStr((const char*)version, L'.');//TODO: global openGL version
    gGLVersionMajor = std::stoi(vsVec[0]);
    gGLVersionMinor = std::stoi(vsVec[1]);
    gGLVersion2Digit = gGLVersionMajor * 10 + gGLVersionMinor;

    //if the version is less than 2.1, do not allow to continue
    if ((gGLVersionMajor == 2 && gGLVersionMinor < 1) || gGLVersionMajor < 2)
    {
        GLUF_ERROR("OpenGL Version To Low!");
        return false;
    }

    g_stdAttrib.insert(GLUFVertexAttribPair(GLUF_VERTEX_ATTRIB_POSITION, g_attribPOS));
    g_stdAttrib.insert(GLUFVertexAttribPair(GLUF_VERTEX_ATTRIB_NORMAL, g_attribNORM));
    g_stdAttrib.insert(GLUFVertexAttribPair(GLUF_VERTEX_ATTRIB_UV0, g_attribPOS));
    g_stdAttrib.insert(GLUFVertexAttribPair(GLUF_VERTEX_ATTRIB_COLOR0, g_attribCOLOR0));
    g_stdAttrib.insert(GLUFVertexAttribPair(GLUF_VERTEX_ATTRIB_TAN, g_attribTAN));
    g_stdAttrib.insert(GLUFVertexAttribPair(GLUF_VERTEX_ATTRIB_BITAN, g_attribBITAN));

    g_stdAttrib.insert(GLUFVertexAttribPair(GLUF_VERTEX_ATTRIB_UV1, g_attribUV1));
    g_stdAttrib.insert(GLUFVertexAttribPair(GLUF_VERTEX_ATTRIB_UV2, g_attribUV2));
    g_stdAttrib.insert(GLUFVertexAttribPair(GLUF_VERTEX_ATTRIB_UV3, g_attribUV3));
    g_stdAttrib.insert(GLUFVertexAttribPair(GLUF_VERTEX_ATTRIB_UV4, g_attribUV4));
    g_stdAttrib.insert(GLUFVertexAttribPair(GLUF_VERTEX_ATTRIB_UV5, g_attribUV5));
    g_stdAttrib.insert(GLUFVertexAttribPair(GLUF_VERTEX_ATTRIB_UV6, g_attribUV6));
    g_stdAttrib.insert(GLUFVertexAttribPair(GLUF_VERTEX_ATTRIB_UV7, g_attribUV7));

    g_stdAttrib.insert(GLUFVertexAttribPair(GLUF_VERTEX_ATTRIB_COLOR1, g_attribCOLOR1));
    g_stdAttrib.insert(GLUFVertexAttribPair(GLUF_VERTEX_ATTRIB_COLOR2, g_attribCOLOR2));
    g_stdAttrib.insert(GLUFVertexAttribPair(GLUF_VERTEX_ATTRIB_COLOR3, g_attribCOLOR3));
    g_stdAttrib.insert(GLUFVertexAttribPair(GLUF_VERTEX_ATTRIB_COLOR4, g_attribCOLOR4));
    g_stdAttrib.insert(GLUFVertexAttribPair(GLUF_VERTEX_ATTRIB_COLOR5, g_attribCOLOR5));
    g_stdAttrib.insert(GLUFVertexAttribPair(GLUF_VERTEX_ATTRIB_COLOR6, g_attribCOLOR6));
    g_stdAttrib.insert(GLUFVertexAttribPair(GLUF_VERTEX_ATTRIB_COLOR7, g_attribCOLOR7));

    return true;
}


const std::vector<std::string>& GLUFGetGLExtensions()
{
    return gExtensions;
}

//void GLUFTerminate()
//{
//
//}
//See GLUFGui for this function; it destroys the font library

/*
======================================================================================================================================================================================================
Statistics

*/

namespace Stats
{
    GLUFStatsData g_StatsData;
}

void GLUFStats_func()
{
    using namespace Stats;

    ++g_StatsData.mFrameCount;

	double thisFrame = GLUFGetTime();
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

const std::wstring& GLUFGetFrameStatsString()
{
    return Stats::g_StatsData.mFormattedStatsData;
}

std::wstring g_DeviceStatusCache = L"WIP";
const std::wstring& GLUFGetDeviceStatus()
{
    return g_DeviceStatusCache;
}

const GLUFStatsData& GLUFGetFrameStats()
{
    return Stats::g_StatsData;
}

/*
======================================================================================================================================================================================================
IO and Stream Utilities

*/

void GLUFLoadFileIntoMemory(const std::wstring& path, std::vector<char>& binMemory)
{
    //try to open file
    std::ifstream inFile;
    inFile.exceptions(std::ios_base::failbit | std::ifstream::badbit);
    try
    {
        inFile.open(path, std::ios::binary);
    }
    catch (std::ios_base::failure e)
    {
        GLUF_ERROR_LONG("Failed to Open File: " << e.what());
        RETHROW;
    }

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
	}
    catch (std::ios_base::failure e)
	{
		GLUF_ERROR_LONG("Failed to load file into memory:" << e.what());
        RETHROW;
	}
}

void GLUFLoadFileIntoMemory(const std::string& path, std::vector<char>& binMemory)
{
    //try to open file
    std::ifstream inFile;
    inFile.exceptions(std::ios_base::failbit | std::ifstream::badbit);
    try
    {
        inFile.open(path, std::ios::binary);
    }
    catch (std::ios_base::failure e)
    {
        GLUF_ERROR_LONG("Failed to Open File: " << e.what());
        RETHROW;
    }

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
    }
    catch (std::ios_base::failure e)
    {
        GLUF_ERROR_LONG("Failed to load file into memory: " << e.what());
        RETHROW;
    }
}

void GLUFLoadBinaryArrayIntoString(char* rawMemory, std::size_t size, std::string& outString)
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


void GLUFLoadBinaryArrayIntoString(const std::vector<char>& rawMemory, std::string& outString)
{
    //const cast is OK to use here, because we know 'rawMemory' will not be modified
    GLUFLoadBinaryArrayIntoString(const_cast<char*>(&rawMemory[0]), rawMemory.size(), outString);
}


/*
======================================================================================================================================================================================================
Misc. GLUF Classes


*/

glm::mat4 GLUFMatrixStack::mIdentity = glm::mat4();

void GLUFMatrixStack::Push(const glm::mat4& matrix)
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

void GLUFMatrixStack::Pop(void)
{
	mStack.pop();
}

const glm::mat4& GLUFMatrixStack::Top(void) const
{
	//if it is empty, then we want to return the identity
	if (mStack.size() == 0)
		return mIdentity;

	return mStack.top();
}

size_t GLUFMatrixStack::Size(void) const
{
	return mStack.size();
}

bool GLUFMatrixStack::Empty(void) const
{
	return mStack.empty();
}


/*
======================================================================================================================================================================================================
OpenGL Basic Data Structures and Operators

*/

bool GLUFPtInRect(GLUFRect rect, GLUFPoint pt)
{
	//for the first comparison, it is impossible for both statements to be false, 
	//because if the y is greater than the top, it is automatically greater than the bottom, and vise versa
	return	(pt.y >= rect.bottom && pt.y <= rect.top) &&
		(pt.x <= rect.right && pt.x >= rect.left);
}

void GLUFSetRectEmpty(GLUFRect& rect)
{
	rect.top = rect.bottom = rect.left = rect.right = 0;
}

void GLUFSetRect(GLUFRect& rect, long left, long top, long right, long bottom)
{
	rect.top = top;
	rect.bottom = bottom;
	rect.left = left;
	rect.right = right;
}

void GLUFSetRect(GLUFRectf& rect, float left, float top, float right, float bottom)
{
	rect.top = top;
	rect.bottom = bottom;
	rect.left = left;
	rect.right = right;
}

void GLUFOffsetRect(GLUFRect& rect, long x, long y)
{
	rect.top += y;
	rect.bottom += y;
	rect.left += x;
	rect.right += x;
}

long GLUFRectHeight(GLUFRect rect)
{
	return rect.top - rect.bottom;
}

long GLUFRectWidth(GLUFRect rect)
{
	return rect.right - rect.left;
}


void GLUFInflateRect(GLUFRect& rect, long dx, long dy)
{
	long dx2 = dx / 2;
	long dy2 = dy / 2;
	rect.left -= dx2;
	rect.right += dx2;//remember to have opposites

	rect.top += dy2;
	rect.bottom -= dy2;
}

bool GLUFIntersectRect(GLUFRect rect0, GLUFRect rect1, GLUFRect& rectIntersect)
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
		GLUFSetRectEmpty(rectIntersect);
		return false;
	}

	return true;
}


/*
======================================================================================================================================================================================================
Datatype Conversion Functions

*/

Color4f GLUFColorToFloat(Color color)
{
	Color4f col;
	col.x = glm::clamp((float)color.x / 255.0f, 0.0f, 1.0f);
	col.y = glm::clamp((float)color.y / 255.0f, 0.0f, 1.0f);
	col.z = glm::clamp((float)color.z / 255.0f, 0.0f, 1.0f);
	col.w = glm::clamp((float)color.w / 255.0f, 0.0f, 1.0f);
	return col;
}

Color3f GLUFColorToFloat3(Color color)
{
	Color3f col;
	col.x = glm::clamp((float)color.x / 255.0f, 0.0f, 1.0f);
	col.y = glm::clamp((float)color.y / 255.0f, 0.0f, 1.0f);
	col.z = glm::clamp((float)color.z / 255.0f, 0.0f, 1.0f);
	return col;
}

GLUFPoint GLUFMultPoints(GLUFPoint pt0, GLUFPoint pt1)
{
	pt0.x *= pt1.x;
	pt0.y *= pt1.y;

	return pt0;
}


glm::vec2 GLUFGetVec2FromRect(GLUFRect rect, bool x, bool y)
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

glm::vec2 GLUFGetVec2FromRect(GLUFRectf rect, bool x, bool y)
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

/*
======================================================================================================================================================================================================
Shader API (Alpha)

Note:
    Attaching shaders to programs and programs to ppo's does not check if shader compilation failed, or if program linking failed respectively

*/


class CompileShaderException : public GLUFException
{
public:
    virtual const char* what() const override
    {
        return "Failed to Compile Shader!";
    }

    EXCEPTION_CONSTRUCTOR(CompileShaderException)
};

class CreateGLShaderException : public GLUFException
{
public:
    virtual const char* what() const override
    {
        return "OpenGL Failed to Create Shader Instance!";
    }

    EXCEPTION_CONSTRUCTOR(CreateGLShaderException)
};

class LinkProgramException : public GLUFException
{
public:
    virtual const char* what() const override
    {
        return "Failed to Link Program!";
    }

    EXCEPTION_CONSTRUCTOR(LinkProgramException)
};

class CreateGLProgramException : public GLUFException
{
public:
    virtual const char* what() const override
    {
        return "OpenGL Failed to Create Program Instance!";
    }

    EXCEPTION_CONSTRUCTOR(CreateGLProgramException)
};

class CreateGLPPOException : public GLUFException
{
public:
    virtual const char* what() const override
    {
        return "OpenGL Failed to Create PPO Instance!";
    }

    EXCEPTION_CONSTRUCTOR(CreateGLPPOException)
};



/*

GLUFShaderTypeToProgramStage

*/


GLUFProgramStage GLUFShaderTypeToProgramStage(GLUFShaderType type)
{
    switch (type)
    {
    case SH_VERTEX_SHADER:
        return PPO_VERTEX_SHADER_BIT;
    case SH_TESS_CONTROL_SHADER:
        return PPO_TESS_CONTROL_SHADER_BIT;
    case SH_TESS_EVALUATION_SHADER:
        return PPO_TESS_EVALUATION_SHADER_BIT;
    case SH_GEOMETRY_SHADER:
        return PPO_GEOMETRY_SHADER_BIT;
    case SH_FRAGMENT_SHADER:
        return PPO_FRAGMENT_SHADER_BIT;
    default:
        return PPO_INVALID_SHADER_BIT;
    }
};

/*
GLUFShader

    Data Members:
        'mShaderId': OpenGL-Assigned Shader id
        'mTmpShaderText': shader text cache kept if shaders are appending eachother
        'mShaderType': what shader type is it (i.e. GL_FRAGMENT_SHADER)

*/
class GLUFShader
{
	friend GLUFShaderManager;
	friend GLUFProgram;

	GLuint mShaderId;

	std::string mTmpShaderText;

	GLUFShaderType mShaderType;

    //disallow copy and assign, because these must always be refered to by pointers
    GLUFShader(const GLUFShader& other) = delete;
    GLUFShader& operator=(const GLUFShader& other) = delete;
public:

    /*
    Constructor
        Throws:
            no-throw guarantee
    
    */
	GLUFShader() noexcept;
	~GLUFShader() noexcept;

	//common shader is if the shader will not be deleted after building into a program
	//this is used for things like lighting functions

    /*
    Init

        Parameters:
            'shaderType': which shader type it is

        Throws:
            no-throw guarantee
    */
    void Init(GLUFShaderType shaderType) noexcept;


    /*
    Load
        
        Parameters:
            'shaderText': text to be added
            'append': append 'shaderText' to 'mTmpShaderText' or clear 'mTmpShaderText'

        Throws:
            may throw 'std::bad_alloc' if string allocation fails
    */
	void Load(const std::string& shaderText, bool append = false);

    /*
    LoadFromMemory

        Parameters:
            'shaderData': raw memory to read shader text from
            'append': append text loaded from 'shaderData' to 'mTmpShaderText' or clear 'mTmpShaderText'

        Throws:
            may throw 'std::bad_alloc' if string allocation fails            
    */
	//void LoadFromMemory(const std::vector<char>& shaderData, bool append = false);

    /*
    LoadFromFile

        Parameters:
            'filePath': path of the file to open
            'append': append 'filePath' to 'MTmpShaderText' or clear 'mTmpShaderText'

        Throws:
            'std::ios_base::failure': if file fails to open or read
    
    */
	//bool LoadFromFile(const std::wstring& filePath, bool append = false);

    /*
    FlushText

        Throws:
            no-throw guarantee
    */
    void FlushText(void) noexcept { mTmpShaderText.clear(); }


    /*
    Compile
        
        -Take all of the loaded text and compile into a shader

        Parameters:
            'retStruct': the returned information about shader compilation.

        Throws:
            'CompileShaderException': if shader compilation fails
            'CreateGLShaderException': if shader creaetion failes (because shader creation is stalled until compilation)
    
    */
	void Compile(GLUFShaderInfoStruct& retStruct);


    /*
    Destroy

        -equivilent to destructor, except object itself is not destroyed (i.e. it can be reused)

        Throws:
            no-throw guarantee
        
    */
    void Destroy() noexcept;

};

using GLUFShaderP = std::pair<GLUFShaderType, GLUFShaderPtr>;

//this is a special instance
class GLUFComputeShader
{
	friend GLUFShaderManager;
	//TODO: low priority
};


/*
GLUFProgram

    Data Members:
        'mProgramId': OpenGL assigned id
        'mShaderBuff': buffer of shaders before linking
        'mAttributeLocations': location of vertex attributes in program
        'mUniformLocations': location of the uniforms in programs
        'mStages': the stages which this program uses; initialized if separable set to true

*/
class GLUFProgram
{
	friend GLUFShaderManager;

	//GLuint mUniformBuffId;
	GLuint mProgramId;
	std::map<GLUFShaderType, GLUFShaderPtr > mShaderBuff;
	GLUFVariableLocMap mAttributeLocations;
	GLUFVariableLocMap mUniformLocations;
    GLbitfield mStages;

public:

    /*
    Constructor
        Throws:
            no-throw guarantee

    */
    GLUFProgram() noexcept;
    ~GLUFProgram() noexcept;

    /*
    Init
        Throws:
            'CreateGLProgramException': OpenGL failed to create program using glCreateProgram
    */
	void Init();

    /*
    AttachShader
        
        Note:
            if a shader already exists for that stage, it will be overwritten

        Parameters:
            'shader': pointer to shader which is being attached

        Throws:
            'std::invalid_argument': if 'shader == nullptr' or 'shader' is invalid
    */
	void AttachShader(GLUFShaderPtr shader);

    /*
    FlushShaders

        -Flush shader cache, but does not explicitly delete shaders

        Throws:
            no-throw guarantee
    */
    void FlushShaders(void) noexcept;


    /*
    Build
        
        Parameters:
            'retStruct': the returned information about program linking.  i.e. 'mSuccess'
            'separate': whether this program will be used 'separately' (see OpenGL docs for definition)

        Throws:
            'LinkProgramException': if program linking fails
    */
	void Build(GLUFShaderInfoStruct& retStruct, bool separate);

    /*
    GetId

        Returns:
            OpenGL Id of the program
    */
	GLuint GetId(){ return mProgramId; }


    /*
    Destroy

        -equivilent to destructor, except object itself is not destroyed (i.e. it can be reused)

        Throws:
            no-throw guarantee

    */
    void Destroy() noexcept;

    /*
    GetBitfield()

        Returns:
            GLbitfield of the program's stages; this is used when initializing and adding to PPO's
    */
    GLbitfield GetBitfield() const noexcept;
};

/*
GLUFSeparateProgram


    Data Members:
        'mPPOId': id of programmible pipeline object
        'mPrograms': list of programs used; kept here so they do not delete themselves while this pipeline object exists
        'mActiveProgram': the currently active program which is having uniforms buffered

*/
class GLUFSeparateProgram
{
	friend GLUFShaderManager;
	GLuint mPPOId;

	GLUFProgramPtrList mPrograms;//so the programs don't go deleting themselves until the PPO is destroyed

    GLUFProgramPtr mActiveProgram;//this is used as the 'active program' when assigning uniforms

public:
    GLUFSeparateProgram();
	~GLUFSeparateProgram();

    /*
    Init

        Throws:
            'CreateGLPPOException': if opengl creation of PPO failed
    */
	void Init();
	
    /*
    AttachProgram

        Parameters:
            'program': program to attach

        Throws:
            no-throw guarantee

    */
	void AttachProgram(const GLUFProgramPtr& program);

    /*
    ClearStages

        -Removes the give stages

        Parameters:
            'stages': bitfield of stages, default value is all of them

        Throws:
            no-throw guarantee
    
    */
    void ClearStages(GLbitfield stages = GL_ALL_SHADER_BITS);

    /*
    SetActiveShaderProgram

        Parameters:
            'stage': the stage of which program will be active

        Throws:
            no-throw guarantee
    
    */
    void SetActiveShaderProgram(GLUFProgramStage stage);


    /*
    GetId

        Returns:
            id of ppo object
    
    */
    GLuint GetId()const noexcept{ return mPPOId; }

    /*
    GetActiveProgram

        Returns:
            the active program

        Throws:
            'std::exception' if there is no active program

    */
    const GLUFProgramPtr& GetActiveProgram() const;

};



/*
===================================================================================================
GLUFSeparateProgram Methods


*/

GLUFSeparateProgram::GLUFSeparateProgram()
{
    //prevents from attempting to create separate shaders w/o the extension
    ASSERT_EXTENTION(GL_ARB_separate_shader_objects);
}

GLUFSeparateProgram::~GLUFSeparateProgram()
{
    NOEXCEPT_REGION_START

	glDeleteProgramPipelines(1, &mPPOId); 

    NOEXCEPT_REGION_END
}

void GLUFSeparateProgram::Init()
{
	glGenProgramPipelines(1, &mPPOId);

    if (mPPOId == 0)
    {
        GLUF_CRITICAL_EXCEPTION(CreateGLPPOException());
    }
}

void GLUFSeparateProgram::AttachProgram(const GLUFProgramPtr& program)
{
    NOEXCEPT_REGION_START

    glBindProgramPipeline(mPPOId);
	mPrograms.push_back(program); 
	glUseProgramStages(mPPOId, program->GetBitfield(), program->GetId());

    NOEXCEPT_REGION_END
}

void GLUFSeparateProgram::ClearStages(GLbitfield stages)
{
    NOEXCEPT_REGION_START

    glUseProgramStages(mPPOId, stages, 0);

    //remove the programs that are affected by this clear
    for (auto it = mPrograms.begin(); it != mPrograms.end(); ++it)
    {
        //if any of the bits of this bitfield are the same, then remove the program.  NOTE: IT IS BAD PRACTICE TO HAVE PROGRAMS HAVE MULTIPLE STAGES, AND ONLY REMOVE ONE OF THEM
        if (((*it)->GetBitfield() & stages) != 0)
            mPrograms.erase(it);
    }

    NOEXCEPT_REGION_END
}

void GLUFSeparateProgram::SetActiveShaderProgram(GLUFProgramStage stage)
{
    NOEXCEPT_REGION_START

    glBindProgramPipeline(mPPOId);

    //does the currently bound program have this stage?
    if (mActiveProgram)
        if ((mActiveProgram->GetBitfield() & stage) != 0)
            return;//do nothing
    
    //find which program contains this stage
    bool success = false;
    for (auto it : mPrograms)
    {
        if ((it->GetBitfield() & stage) != 0)
        {
            mActiveProgram = it;
            success = true;
            break;
        }
    }

    if (!success)
    {
        //if the stage does not exist, reset to default (null)
        mActiveProgram = nullptr;
        glActiveShaderProgram(mPPOId, 0);

    }
    else
    {
        glActiveShaderProgram(mPPOId, mActiveProgram->GetId());
    }

    NOEXCEPT_REGION_END
}

const GLUFProgramPtr& GLUFSeparateProgram::GetActiveProgram() const
{
    if (mActiveProgram == nullptr)
    {
        GLUF_NON_CRITICAL_EXCEPTION(NoActiveProgramUniformException());
    }

    return mActiveProgram;
}


/*
===================================================================================================
GLUFShader Methods


*/


GLUFShader::GLUFShader()
{
	mShaderId = 0;
}

GLUFShader::~GLUFShader()
{
	if (mShaderId != 0)
	{
		glDeleteShader(mShaderId);
	}
	mTmpShaderText.clear();
}

void GLUFShader::Init(GLUFShaderType shaderType)
{
	mShaderType = shaderType;
	mShaderId = 0;
}

void GLUFShader::Load(const std::string& shaderText, bool append)
{
	if (!append)
		mTmpShaderText.clear();

	mTmpShaderText.append(shaderText);
}

void GLUFShader::Destroy()
{
    NOEXCEPT_REGION_START

    if (mShaderId != 0)
    {
        glDeleteShader(mShaderId);
        mShaderId = 0;
    }
    mTmpShaderText.clear();

    NOEXCEPT_REGION_END
}

/*void GLUFShader::LoadFromMemory(const std::vector<char>& shaderData, bool append)
{
	if (!append)
		mTmpShaderText.clear();

    std::string loadedText;

    try
    {
        GLUFLoadBinaryArrayIntoString(shaderData, loadedText);
    }
    catch (...)
    {
        GLUF_ERROR("(GLUFShader): Failed to load binary memory into shader string");
        throw;
    }

    
    mTmpShaderText += loadedText;

}*/

/*bool GLUFShader::LoadFromFile(const std::wstring& filePath, bool append)
{
    if (!append)
        mTmpShaderText.clear();

    std::ifstream inFile(filePath);
    inFile.exceptions(std::ios::badbit | std::ios::failbit);

    try
    {
#pragma warning (disable : 4244)
        std::string newString;

        inFile.seekg(0, std::ios::end);
        newString.resize(inFile.tellg());
        inFile.seekg(0, std::ios::beg);

        //TODO: DOES THIS NEED TO HAVE A +1 SOMEWHERE TO ACCOUNT FOR NULL CHARACTER
        inFile.read(&newString[0], newString.size());
        inFile.close();
    }
    catch (...)
    {
        GLUF_ERROR("(GLUFShader): Failed to load shader text from file!");
        throw;
    }

    return true;
}*/

#define FAILED_COMPILE 'F'
#define FAILED_LINK    'F'


void GLUFShader::Compile(GLUFShaderInfoStruct& returnStruct)
{
    //if the shader id is not 0, this means the previous compile attempt was successful, because if it was not, the program is deleted and reset to 0
	if (mShaderId != 0)
	{
		returnStruct.mSuccess = false;
		returnStruct.mLog = "F";
		return;
	}

    //create the shader
	mShaderId = glCreateShader(mShaderType);

    //if shader creation failed, throw an exception
    if (mShaderId == 0)
        GLUF_CRITICAL_EXCEPTION(CreateGLShaderException());

	//start by adding the strings to glShader Source.  This is done right before the compile
	//process becuase it is hard to remove it if there is any reason to flush the text

	std::string tmpText = mTmpShaderText;

	GLint tmpSize = (GLuint)mTmpShaderText.length();
	tmpSize--; /*BECAUSE OF NULL TERMINATED STRINGS*/

	const GLchar* text = tmpText.c_str();
	glShaderSource(mShaderId, 1, &text, &tmpSize);

    //flush text upon compile no matter what (if compile failed, why would we want to keep the error-ridden code)
	FlushText();

	glCompileShader(mShaderId);

	GLint isCompiled = 0;
	glGetShaderiv(mShaderId, GL_COMPILE_STATUS, &isCompiled);
	returnStruct.mSuccess = (isCompiled == GL_FALSE) ? false : true;

	GLint maxLength = 0;
	glGetShaderiv(mShaderId, GL_INFO_LOG_LENGTH, &maxLength);

	//The maxLength includes the NULL character
	returnStruct.mLog.resize(maxLength);
	glGetShaderInfoLog(mShaderId, maxLength, &maxLength, &returnStruct.mLog[0]);

	//Provide the infolog in whatever manor you deem best.
	//Exit with failure.

	//if it failed, delete the shader, to have a universal way of determining failure
	if (returnStruct.mSuccess == false)
	{
		glDeleteShader(mShaderId);
		mShaderId = 0;
        GLUF_CRITICAL_EXCEPTION(CompileShaderException());
	}
	return;
}





/*
===================================================================================================
GLUFProgram Methods


*/

GLUFProgram::GLUFProgram()
{
	mProgramId = 0;
}

GLUFProgram::~GLUFProgram()
{
	glDeleteProgram(mProgramId);
}

void GLUFProgram::Init()
{
	//unlike with the shader, this will be created during initialization
	mProgramId = glCreateProgram();

    if (mProgramId == 0)
        GLUF_CRITICAL_EXCEPTION(CreateGLProgramException());
}

void GLUFProgram::AttachShader(GLUFShaderPtr shader)
{
    GLUFShaderP toInsert{ shader->mShaderType, shader };
    
    //does this stage already have a shader assigned to it?
    auto exists = mShaderBuff.find(shader->mShaderType);
    if (exists != mShaderBuff.end())
    {
        //if it does, delete it
        mShaderBuff.erase(exists);
    }

    //finally, insert the new one
    mShaderBuff.insert(toInsert);

    //and add it to OpenGL
	glAttachShader(mProgramId, shader->mShaderId);
}

void GLUFProgram::FlushShaders(void)
{
    //simply go through all of the shaders, remove them, and clear the shader buffer
	for (auto it : mShaderBuff)
	{
		glDetachShader(mProgramId, it.second->mShaderId);
	}
	mShaderBuff.clear();
}

void GLUFProgram::Build(GLUFShaderInfoStruct& retStruct, bool separate)
{
	//make sure we enable separate shading
	if (separate)
    { 
        mStages = 0;

        glProgramParameteri(mProgramId, GL_PROGRAM_SEPARABLE, GL_TRUE); 


        //also create the 'mStages' list

        for (auto it : mShaderBuff)
        {
            mStages |= GLUFShaderTypeToProgramStage(it.first);
        }
    }

	//Link our program
	glLinkProgram(mProgramId);

	//Note the different functions here: glGetProgram* instead of glGetShader*.
	GLint isLinked = 0;
	glGetProgramiv(mProgramId, GL_LINK_STATUS, &isLinked);
	retStruct.mSuccess = (isLinked == GL_FALSE) ? false : true;

	GLint maxLength = 0;
	glGetProgramiv(mProgramId, GL_INFO_LOG_LENGTH, &maxLength);

	//The maxLength includes the NULL character
	retStruct.mLog.resize(maxLength);
	glGetProgramInfoLog(mProgramId, maxLength, &maxLength, &retStruct.mLog[0]);

	if (!retStruct.mSuccess)
	{
		//in the case of failure, DO NOT DELETE ANYTHING, but do throw an error
        GLUF_CRITICAL_EXCEPTION(LinkProgramException());
	}
	else
	{
		FlushShaders();//this removes the references to them from the program, but they will still exist unless this is the last reference to them
		
        /*
        
        This region loads uniform and attribute locations

        */
		
        
        //Load the variable names
		GLint attribCount = 0;
		glGetProgramiv(mProgramId, GL_ACTIVE_ATTRIBUTES, &attribCount);

		GLint maxLength;
		glGetProgramiv(mProgramId, GL_ACTIVE_ATTRIBUTE_MAX_LENGTH, &maxLength);

        //TODO: right now, these are just placeholders, but perhaps a more in depth variable information structure is needed
		GLenum type;
		GLint written, size;

        //this is used to trim the null charactors from the end of the string
        const auto TrimString = [](std::string& toTrim)
        {
            for (unsigned int i = 0; i < toTrim.size(); ++i)
            {
                if (toTrim[i] == '\0')
                {
                    toTrim.erase(toTrim.begin() + i, toTrim.end());
                    break;
                }

            }
        };


        std::string data;
        for (int i = 0; i < attribCount; ++i)
        {
            //resize and clear every time is a bit messy, but is the only way to make sure the string is the right length
            data.resize(maxLength);

            glGetActiveAttrib(mProgramId, i, maxLength, &written, &size, &type, &data[0]);
            TrimString(data);
            mAttributeLocations.insert(GLUFVariableLocPair(data, glGetAttribLocation(mProgramId, &data[0])));

            data.clear();
        }

		GLint uniformCount = 0;
		glGetProgramiv(mProgramId, GL_ACTIVE_UNIFORMS, &uniformCount);

		for (int i = 0; i < uniformCount; ++i)
        {
            //resize and clear every time is a bit messy, but is the only way to make sure the string is the right length
            data.resize(maxLength);

            glGetActiveUniform(mProgramId, i, maxLength, &written, &size, &type, &data[0]);
            TrimString(data);
			mUniformLocations.insert(GLUFVariableLocPair(data, glGetUniformLocation(mProgramId, &data[0])));

            data.clear();
		}
	}
}

void GLUFProgram::Destroy()
{ 
    NOEXCEPT_REGION_START

    if (mProgramId != 0)
    {
        glDeleteProgram(mProgramId);
        mProgramId = 0;
    }
    mShaderBuff.clear();
    mAttributeLocations.clear();
    mUniformLocations.clear();

    NOEXCEPT_REGION_END
}

GLbitfield GLUFProgram::GetBitfield() const
{
    NOEXCEPT_REGION_START

    return mStages;

    NOEXCEPT_REGION_END

    return 0;
}


/*
===================================================================================================
GLUFShaderManager Methods


*/

/*
GLUFShaderPtr GLUFShaderManager::CreateShader(std::wstring shad, GLUFShaderType type, bool file, bool separate)
{
	GLUFShaderPtr shader(new GLUFShader());
	shader->Init(type);

	(file) ? shader->LoadFromFile(shad.c_str()) : shader->Load(shad.c_str());

	GLUFShaderInfoStruct output;
	shader->Compile(output);
	mCompileLogs.insert(std::pair<GLUFShaderPtr, GLUFShaderInfoStruct>(shader, output));

	//log it if it failed
	if (!output)
	{
		std::wstringstream ss;
		ss << "Shader Compilation Failed: \n" << output.mLog;
		GLUF_ERROR(ss.str().c_str());
	}

	return shader;
}
*/

void GLUFShaderManager::AddCompileLog(const GLUFShaderPtr& shader, const GLUFShaderInfoStruct& log)
{
    //TODO: vs2013 does not support shared locking
    GLUF_TSAFE_SCOPE(mCompLogMutex);

    mCompileLogs.insert(std::pair<GLUFShaderPtr, GLUFShaderInfoStruct>(shader, log));
}

void GLUFShaderManager::AddLinkLog(const GLUFProgramPtr& program, const GLUFShaderInfoStruct& log)
{
    GLUF_TSAFE_SCOPE(mLinkLogMutex);

    mLinklogs.insert(std::pair<GLUFProgramPtr, GLUFShaderInfoStruct>(program, log));
}

GLuint GLUFShaderManager::GetUniformIdFromName(const GLUFSepProgramPtr& ppo, const std::string& name) const
{
    auto activeProgram = ppo->GetActiveProgram();
    auto it = activeProgram->mUniformLocations.find(name);
    if (it == activeProgram->mUniformLocations.end())
    {
        GLUF_NON_CRITICAL_EXCEPTION(std::invalid_argument("Uniform Name Not Found!"));
        return 0;//if we are in release mode, and the name does not exist, default to the 0th, however this MAY CREATE UNDESIRED RESULTS
    }

    return it->second;
}

GLuint GLUFShaderManager::GetUniformIdFromName(const GLUFProgramPtr& prog, const std::string& name) const
{
    auto it = prog->mUniformLocations.find(name);
    if (it == prog->mUniformLocations.end())
    {
        GLUF_NON_CRITICAL_EXCEPTION(std::invalid_argument("Uniform Name Not Found!"));
        return 0;//if we are in release mode, and the name does not exist, default to the 0th, however this MAY CREATE UNDESIRED RESULTS
    }

    return it->second;
}


void GLUFShaderManager::CreateShaderFromFile(GLUFShaderPtr& outShader, const std::wstring& filePath, GLUFShaderType type)
{
    //create the shader
    outShader = std::make_shared<GLUFShader>();

    //file to open
    std::ifstream inFile;
    inFile.exceptions(std::ios::badbit | std::ios::failbit);

    GLUFShaderInfoStruct output;
    try
    {
        //Load the text from the file
        std::string newString;

        //open the file
        inFile.open(filePath);

        //get the file length
        inFile.seekg(0, std::ios::end);
        newString.resize(static_cast<unsigned int>(inFile.tellg()));
        inFile.seekg(0, std::ios::beg);

        //TODO: DOES THIS NEED TO HAVE A +1 SOMEWHERE TO ACCOUNT FOR NULL CHARACTER
        inFile.read(&newString[0], newString.size());
        inFile.close();

        //load it from the file
        outShader->Load(newString);

        //compile it
        outShader->Compile(output);
        AddCompileLog(outShader, output);
    }
    catch (const std::ios_base::failure& e)
    {
        GLUF_ERROR_LONG("(GLUFShaderManager): Shader File Load Failed: " << e.what());
        RETHROW;//rethrow here, because if file loading failed, the it never got to compilation
    }
    catch (const CompileShaderException& e)
    {
        GLUF_ERROR_LONG("(GLUFShaderManager): " << e.what());
        //add the log if file load failed
        AddCompileLog(outShader, output);

        GLUF_CRITICAL_EXCEPTION(MakeShaderException());
    }
    catch (const CreateGLShaderException& e)
    {
        GLUF_ERROR_LONG("(GLUFShaderManager): " << e.what());
        GLUF_CRITICAL_EXCEPTION(MakeShaderException());//don't add compile log if the shader could not be created
    }

}

void GLUFShaderManager::CreateShaderFromText(GLUFShaderPtr& outShader, const std::string& text, GLUFShaderType type)
{
    outShader = std::make_shared<GLUFShader>();

    GLUFShaderInfoStruct out;
    try
    {
        //initialize the shader
        outShader->Init(type);

        //load from the text
        outShader->Load(text);

        //compile it
        outShader->Compile(out);
        AddCompileLog(outShader, out);
    }
    catch (const CompileShaderException& e)
    {
        GLUF_ERROR_LONG("(GLUFShaderManager): " << e.what() << "\n ========Log======== \n" << out.mLog);
        //add the log if file load failed
        AddCompileLog(outShader, out);

        GLUF_CRITICAL_EXCEPTION(MakeShaderException());
    }
    catch (const CreateGLShaderException& e)
    {
        GLUF_ERROR_LONG("(GLUFShaderManager): " << e.what());
        GLUF_CRITICAL_EXCEPTION(MakeShaderException());//don't add compile log if the shader could not be created
    }
}

void GLUFShaderManager::CreateShaderFromMemory(GLUFShaderPtr& outShader, const std::vector<char>& memory, GLUFShaderType type)
{
    //load the string from the memory
    std::string outString;
    GLUFLoadBinaryArrayIntoString(memory, outString);

    //newline here is needed, because end of loaded file might not have one, which is required for the end of a shader for some reason
    return CreateShaderFromText(outShader, outString + "\n", type);
}

void GLUFShaderManager::CreateProgram(GLUFProgramPtr& outProgram, GLUFShaderPtrList shaders, bool separate)
{
    outProgram = std::make_shared<GLUFProgram>();

    GLUFShaderInfoStruct out;
    try
    {
        //initialize
        outProgram->Init();

        //add the shaders
        for (auto it : shaders)
        {
            outProgram->AttachShader(it);
        }

        //build the program
        outProgram->Build(out, separate);
        AddLinkLog(outProgram, out);
    }
    catch (const CreateGLProgramException& e)
    {
        GLUF_ERROR_LONG("(GLUFShaderManager): " << e.what());
        GLUF_CRITICAL_EXCEPTION(MakeShaderException());
    }
    catch (const LinkProgramException& e)
    {
        GLUF_ERROR_LONG("(GLUFShaderManager): " << e.what() << "\n ========Log======== \n" << out.mLog);
        AddLinkLog(outProgram, out);//if linking failed, still add the log
    }
    catch (const std::invalid_argument& e)
    {
        GLUF_ERROR_LONG("(GLUFShaderManager): " << e.what());
        outProgram->FlushShaders();//if any of the shaders failed to add, flush so it is in a valid state
        GLUF_CRITICAL_EXCEPTION(MakeShaderException());
    }
}


void GLUFShaderManager::CreateProgram(GLUFProgramPtr& outProgram, GLUFShaderSourceList shaderSources, bool separate)
{
	GLUFShaderPtrList shaders;
	for (auto it : shaderSources)
	{
        //create the shader from the text
        std::shared_ptr<GLUFShader> nowShader;
        CreateShaderFromText(nowShader, it.second, it.first);

        //add the shader to the list
		shaders.push_back(nowShader);

        //the exception hierarchy handles all errors for this method
	}

    CreateProgram(outProgram, shaders, separate);
}


void GLUFShaderManager::CreateProgram(GLUFProgramPtr& outProgram, GLUFShaderPathList shaderPaths, bool separate)
{
	GLUFShaderPtrList shaders;
	for (auto it : shaderPaths)
	{
        //create the shader from the text
        auto nowShader = std::make_shared<GLUFShader>();
        CreateShaderFromFile(nowShader, it.second, it.first);

        //add the shader to the list
        shaders.push_back(nowShader);

        //the exception hierarchy handles all errors for this method
	}

	CreateProgram(outProgram, shaders, separate);
}

//for removing things

void GLUFShaderManager::DeleteShader(GLUFShaderPtr& shader)
{
    //make sure it exists
    if (shader)
        shader->Destroy();
}

void GLUFShaderManager::DeleteProgram(GLUFProgramPtr& program)
{
    //make sure it exists
    if (program)
        program->Destroy();
}

void GLUFShaderManager::FlushLogs()
{
	mLinklogs.clear();
	mCompileLogs.clear();
}


//for accessing things

const GLuint GLUFShaderManager::GetShaderId(const GLUFShaderPtr& shader) const
{
    GLUF_NULLPTR_CHECK(shader);

	return shader->mShaderId;
}


const GLUFShaderType GLUFShaderManager::GetShaderType(const GLUFShaderPtr& shader) const
{
    GLUF_NULLPTR_CHECK(shader);

	return shader->mShaderType;
}


const GLuint GLUFShaderManager::GetProgramId(const GLUFProgramPtr& program) const
{
    GLUF_NULLPTR_CHECK(program);

	return program->mProgramId;
}

const GLUFCompileOutputStruct GLUFShaderManager::GetShaderLog(const GLUFShaderPtr& shaderPtr) const
{
    GLUF_NULLPTR_CHECK(shaderPtr);
    GLUF_TSAFE_SCOPE(mCompLogMutex);

	return mCompileLogs.find(shaderPtr)->second;
}


const GLUFLinkOutputStruct GLUFShaderManager::GetProgramLog(const GLUFProgramPtr& programPtr) const
{
    GLUF_NULLPTR_CHECK(programPtr);
    GLUF_TSAFE_SCOPE(mLinkLogMutex);

	return mLinklogs.find(programPtr)->second;
}

//for using things

void GLUFShaderManager::UseProgram(const GLUFProgramPtr& program) const
{
    GLUF_NULLPTR_CHECK(program);

    //binding a null program means the program is uninitialized or broken, which is an error
    if (program->GetId() == 0)
        GLUF_NON_CRITICAL_EXCEPTION(UseProgramException());

	glUseProgram(program->mProgramId);
}

void GLUFShaderManager::UseProgramNull() const
{
	glUseProgram(0);
	glBindProgramPipeline(0);//juse in case we are using pipelines
}

void GLUFShaderManager::CreateSeparateProgram(GLUFSepProgramPtr& ppo, const GLUFProgramPtrList& programs) const
{
    ppo = std::make_shared<GLUFSeparateProgram>();

    try
    {
        //initialize the PPO
        ppo->Init();

        for (auto it : programs)
        {
            GLUF_NULLPTR_CHECK(it);
            if (it->GetId() == 0)
                GLUF_CRITICAL_EXCEPTION(std::invalid_argument("Uninitialized Program Attempted to be Added to a PPO"));

            ppo->AttachProgram(it);
        }
    }
    catch (const CreateGLPPOException& e)
    {
        GLUF_ERROR_LONG("(GLUFShaderManager): " << e.what());
        GLUF_CRITICAL_EXCEPTION(MakePPOException());
    }
    catch (const std::invalid_argument& e)
    {
        GLUF_ERROR_LONG("(GLUFShaderManager): " << e.what());
        RETHROW;
    }
}

const GLuint GLUFShaderManager::GetShaderVariableLocation(const GLUFProgramPtr& program, GLUFLocationType locType, const std::string& varName) const
{
    GLUF_NULLPTR_CHECK(program);

	std::map<std::string, GLuint>::iterator it;

	if (locType == GLT_ATTRIB)
	{
        it = program->mAttributeLocations.find(varName);
	}
	else
	{
        it = program->mUniformLocations.find(varName);
	}

    if (it == program->mAttributeLocations.end())
        GLUF_NON_CRITICAL_EXCEPTION(std::invalid_argument("\"varName\" Could not be found when searching program attributes/uniforms!"));

	return it->second;
}

const GLUFVariableLocMap& GLUFShaderManager::GetShaderAttribLocations(const GLUFProgramPtr& program) const
{
    GLUF_NULLPTR_CHECK(program);
    return program->mAttributeLocations;
}

const GLUFVariableLocMap& GLUFShaderManager::GetShaderUniformLocations(const GLUFProgramPtr& program) const
{
    GLUF_NULLPTR_CHECK(program);
    return program->mUniformLocations;
}

const GLUFVariableLocMap GLUFShaderManager::GetShaderAttribLocations(const GLUFSepProgramPtr& program) const
{
    GLUF_NULLPTR_CHECK(program);
	GLUFVariableLocMap ret;

    for (auto it : program->mPrograms)
	{
		ret.insert(it->mAttributeLocations.begin(), it->mAttributeLocations.end());
	}

	return ret;
}

const GLUFVariableLocMap GLUFShaderManager::GetShaderUniformLocations(const GLUFSepProgramPtr& program) const
{
    GLUF_NULLPTR_CHECK(program);

	GLUFVariableLocMap ret;

    for (auto it : program->mPrograms)
	{
		ret.insert(it->mUniformLocations.begin(), it->mUniformLocations.end());
	}

	return ret;
}

void GLUFShaderManager::AttachProgram(GLUFSepProgramPtr& ppo, const GLUFProgramPtr& program) const
{
    GLUF_NULLPTR_CHECK(ppo);
    GLUF_NULLPTR_CHECK(program);

    if (program->GetId() == 0)
        GLUF_CRITICAL_EXCEPTION(std::invalid_argument("Uninitialized Program Attempted to be Added to a PPO"));

	ppo->AttachProgram(program);
}

void GLUFShaderManager::AttachPrograms(GLUFSepProgramPtr& ppo, const GLUFProgramPtrList& programs) const
{
    GLUF_NULLPTR_CHECK(ppo);

	for (auto it : programs)
    {
        GLUF_NULLPTR_CHECK(it);
        if (it->GetId() == 0)
            GLUF_CRITICAL_EXCEPTION(std::invalid_argument("Uninitialized Program Attempted to be Added to a PPO"));

		ppo->AttachProgram(it);
	}
}

void GLUFShaderManager::ClearPrograms(GLUFSepProgramPtr& ppo, GLbitfield stages) const
{
    GLUF_NULLPTR_CHECK(ppo);
    ppo->ClearStages(stages);
}

void GLUFShaderManager::UseProgram(const GLUFSepProgramPtr& ppo) const
{
    GLUF_NULLPTR_CHECK(ppo);
	glUseProgram(0);
	glBindProgramPipeline(ppo->GetId());
}

void GLUFShaderManager::GLActiveShaderProgram(GLUFSepProgramPtr& ppo, GLUFShaderType stage) const
{
    GLUF_NULLPTR_CHECK(ppo);
    ppo->SetActiveShaderProgram(GLUFShaderTypeToProgramStage(stage));
}

/*

GLUniform*

*/


/*

float's

*/
void GLUFShaderManager::GLUniform1f(GLuint loc, const GLfloat& value) const
{
    glUniform1f(loc, value);
}

void GLUFShaderManager::GLUniform2f(GLuint loc, const glm::vec2& value) const
{
    glUniform2fv(loc, 1, &value[0]);
}

void GLUFShaderManager::GLUniform3f(GLuint loc, const glm::vec3& value) const
{
    glUniform3fv(loc, 1, &value[0]);
}

void GLUFShaderManager::GLUniform4f(GLuint loc, const glm::vec4& value) const
{
    glUniform4fv(loc, 1, &value[0]);
}

void GLUFShaderManager::GLUniform1f(const GLUFProgramPtr& prog, const std::string& name, const GLfloat& value) const
{    
    glUniform1f(GetUniformIdFromName(prog, name), value);
}

void GLUFShaderManager::GLUniform2f(const GLUFProgramPtr& prog, const std::string& name, const glm::vec2& value) const
{
    glUniform2fv(GetUniformIdFromName(prog, name), 1, &value[0]);
}

void GLUFShaderManager::GLUniform3f(const GLUFProgramPtr& prog, const std::string& name, const glm::vec3& value) const
{
    glUniform3fv(GetUniformIdFromName(prog, name), 1, &value[0]);
}

void GLUFShaderManager::GLUniform4f(const GLUFProgramPtr& prog, const std::string& name, const glm::vec4& value) const
{
    glUniform4fv(GetUniformIdFromName(prog, name), 1, &value[0]);
}

/*

int's

*/

void GLUFShaderManager::GLUniform1i(GLuint loc, const GLint& value) const
{
    glUniform1i(loc, value);
}

void GLUFShaderManager::GLUniform2i(GLuint loc, const glm::i32vec2& value) const
{
    glUniform2iv(loc, 1, &value[0]);
}

void GLUFShaderManager::GLUniform3i(GLuint loc, const glm::i32vec3& value) const
{
    glUniform3iv(loc, 1, &value[0]);
}

void GLUFShaderManager::GLUniform4i(GLuint loc, const glm::i32vec4& value) const
{
    glUniform4iv(loc, 1, &value[0]);
}

void GLUFShaderManager::GLUniform1i(const GLUFProgramPtr& prog, const std::string& name, const GLint& value) const
{
    glUniform1i(GetUniformIdFromName(prog, name), value);
}

void GLUFShaderManager::GLUniform2i(const GLUFProgramPtr& prog, const std::string& name, const glm::i32vec2& value) const
{
    glUniform2iv(GetUniformIdFromName(prog, name), 1, &value[0]);
}

void GLUFShaderManager::GLUniform3i(const GLUFProgramPtr& prog, const std::string& name, const glm::i32vec3& value) const
{
    glUniform3iv(GetUniformIdFromName(prog, name), 1, &value[0]);
}

void GLUFShaderManager::GLUniform4i(const GLUFProgramPtr& prog, const std::string& name, const glm::i32vec4& value) const
{
    glUniform4iv(GetUniformIdFromName(prog, name), 1, &value[0]);
}


/*

uint's

*/

void GLUFShaderManager::GLUniform1ui(GLuint loc, const GLuint& value) const
{
    glUniform1ui(loc, value);
}

void GLUFShaderManager::GLUniform2ui(GLuint loc, const glm::u32vec2& value) const
{
    glUniform2uiv(loc, 1, &value[0]);
}

void GLUFShaderManager::GLUniform3ui(GLuint loc, const glm::u32vec3& value) const
{
    glUniform3uiv(loc, 1, &value[0]);
}

void GLUFShaderManager::GLUniform4ui(GLuint loc, const glm::u32vec4& value) const
{
    glUniform4uiv(loc, 1, &value[0]);
}

void GLUFShaderManager::GLUniform1ui(const GLUFProgramPtr& prog, const std::string& name, const GLuint& value) const
{
    glUniform1ui(GetUniformIdFromName(prog, name), value);
}

void GLUFShaderManager::GLUniform2ui(const GLUFProgramPtr& prog, const std::string& name, const glm::u32vec2& value) const
{
    glUniform2uiv(GetUniformIdFromName(prog, name), 1, &value[0]);
}

void GLUFShaderManager::GLUniform3ui(const GLUFProgramPtr& prog, const std::string& name, const glm::u32vec3& value) const
{
    glUniform3uiv(GetUniformIdFromName(prog, name), 1, &value[0]);
}

void GLUFShaderManager::GLUniform4ui(const GLUFProgramPtr& prog, const std::string& name, const glm::u32vec4& value) const
{
    glUniform4uiv(GetUniformIdFromName(prog, name), 1, &value[0]);
}

/*

matrices

*/

void GLUFShaderManager::GLUniformMatrix2f(GLuint loc, const glm::mat2& value) const
{
    glUniformMatrix2fv(loc, 1, 0, &value[0][0]);
}

void GLUFShaderManager::GLUniformMatrix3f(GLuint loc, const glm::mat3& value) const
{
    glUniformMatrix3fv(loc, 1, 0, &value[0][0]);
}

void GLUFShaderManager::GLUniformMatrix4f(GLuint loc, const glm::mat4& value) const
{
    glUniformMatrix4fv(loc, 1, 0, &value[0][0]);
}

void GLUFShaderManager::GLUniformMatrix2x3f(GLuint loc, const glm::mat2x3& value) const
{
    glUniformMatrix2x3fv(loc, 1, 0, &value[0][0]);
}

void GLUFShaderManager::GLUniformMatrix3x2f(GLuint loc, const glm::mat3x2& value) const
{
    glUniformMatrix3x2fv(loc, 1, 0, &value[0][0]);
}

void GLUFShaderManager::GLUniformMatrix2x4f(GLuint loc, const glm::mat2x4& value) const
{
    glUniformMatrix2x4fv(loc, 1, 0, &value[0][0]);
}

void GLUFShaderManager::GLUniformMatrix4x2f(GLuint loc, const glm::mat4x2& value) const
{
    glUniformMatrix4x2fv(loc, 1, 0, &value[0][0]);
}

void GLUFShaderManager::GLUniformMatrix3x4f(GLuint loc, const glm::mat3x4& value) const
{
    glUniformMatrix3x4fv(loc, 1, 0, &value[0][0]);
}

void GLUFShaderManager::GLUniformMatrix4x3f(GLuint loc, const glm::mat4x3& value) const
{
    //warning here can be IGNORED
    glUniformMatrix4x3fv(loc, 1, 0, &value[0][0]);
}

void GLUFShaderManager::GLUniformMatrix2f(const GLUFProgramPtr& prog, const std::string& name, const glm::mat2& value) const
{
    glUniformMatrix2fv(GetUniformIdFromName(prog, name), 1, 0, &value[0][0]);
}

void GLUFShaderManager::GLUniformMatrix3f(const GLUFProgramPtr& prog, const std::string& name, const glm::mat3& value) const
{
    glUniformMatrix3fv(GetUniformIdFromName(prog, name), 1, 0, &value[0][0]);
}

void GLUFShaderManager::GLUniformMatrix4f(const GLUFProgramPtr& prog, const std::string& name, const glm::mat4& value) const
{
    glUniformMatrix4fv(GetUniformIdFromName(prog, name), 1, 0, &value[0][0]);
}

void GLUFShaderManager::GLUniformMatrix2x3f(const GLUFProgramPtr& prog, const std::string& name, const glm::mat2x3& value) const
{
    glUniformMatrix2x3fv(GetUniformIdFromName(prog, name), 1, 0, &value[0][0]);
}

void GLUFShaderManager::GLUniformMatrix3x2f(const GLUFProgramPtr& prog, const std::string& name, const glm::mat3x2& value) const
{
    glUniformMatrix3x2fv(GetUniformIdFromName(prog, name), 1, 0, &value[0][0]);
}

void GLUFShaderManager::GLUniformMatrix2x4f(const GLUFProgramPtr& prog, const std::string& name, const glm::mat2x4& value) const
{
    glUniformMatrix2x4fv(GetUniformIdFromName(prog, name), 1, 0, &value[0][0]);
}

void GLUFShaderManager::GLUniformMatrix4x2f(const GLUFProgramPtr& prog, const std::string& name, const glm::mat4x2& value) const
{
    glUniformMatrix4x2fv(GetUniformIdFromName(prog, name), 1, 0, &value[0][0]);
}

void GLUFShaderManager::GLUniformMatrix3x4f(const GLUFProgramPtr& prog, const std::string& name, const glm::mat3x4& value) const
{
    glUniformMatrix3x4fv(GetUniformIdFromName(prog, name), 1, 0, &value[0][0]);
}

void GLUFShaderManager::GLUniformMatrix4x3f(const GLUFProgramPtr& prog, const std::string& name, const glm::mat4x3& value) const
{
    glUniformMatrix4x3fv(GetUniformIdFromName(prog, name), 1, 0, &value[0][0]);
}





/*

GLProgramUniform*


*/

//macro for easier readibility; the purpose of this line is to throw a 'NoActiveProgramUniformException' in debug mode 
#ifdef GLUF_DEBUG

#define HAS_ACTIVE_PROGRAM(ppo) ppo->GetActiveProgram();

#else

#define HAS_ACTIVE_PROGRAM(ppo)

#endif

/*

float's

*/

void GLUFShaderManager::GLProgramUniform1f(const GLUFSepProgramPtr& ppo, GLuint loc, const GLfloat& value) const
{
    HAS_ACTIVE_PROGRAM(ppo);

    glProgramUniform1f(ppo->GetActiveProgram()->GetId(), loc, value);
}

void GLUFShaderManager::GLProgramUniform2f(const GLUFSepProgramPtr& ppo, GLuint loc, const glm::vec2& value) const
{
    HAS_ACTIVE_PROGRAM(ppo);

    glProgramUniform2fv(ppo->GetActiveProgram()->GetId(), loc, 1, &value[0]);
}

void GLUFShaderManager::GLProgramUniform3f(const GLUFSepProgramPtr& ppo, GLuint loc, const glm::vec3& value) const
{
    HAS_ACTIVE_PROGRAM(ppo);

    glProgramUniform3fv(ppo->GetActiveProgram()->GetId(), loc, 1, &value[0]);
}

void GLUFShaderManager::GLProgramUniform4f(const GLUFSepProgramPtr& ppo, GLuint loc, const glm::vec4& value) const
{
    HAS_ACTIVE_PROGRAM(ppo);

    glProgramUniform4fv(ppo->GetActiveProgram()->GetId(), loc, 1, &value[0]);
}

void GLUFShaderManager::GLProgramUniform1f(const GLUFSepProgramPtr& ppo, const std::string& name, const GLfloat& value) const
{
    glProgramUniform1f(ppo->GetActiveProgram()->GetId(), GetUniformIdFromName(ppo, name), value);
}

void GLUFShaderManager::GLProgramUniform2f(const GLUFSepProgramPtr& ppo, const std::string& name, const glm::vec2& value) const
{
    glProgramUniform2fv(ppo->GetActiveProgram()->GetId(), GetUniformIdFromName(ppo, name), 1, &value[0]);
}

void GLUFShaderManager::GLProgramUniform3f(const GLUFSepProgramPtr& ppo, const std::string& name, const glm::vec3& value) const
{
    glProgramUniform3fv(ppo->GetActiveProgram()->GetId(), GetUniformIdFromName(ppo, name), 1, &value[0]);
}

void GLUFShaderManager::GLProgramUniform4f(const GLUFSepProgramPtr& ppo, const std::string& name, const glm::vec4& value) const
{
    glProgramUniform4fv(ppo->GetActiveProgram()->GetId(), GetUniformIdFromName(ppo, name), 1, &value[0]);
}


/*

int's

*/

void GLUFShaderManager::GLProgramUniform1i(const GLUFSepProgramPtr& ppo, GLuint loc, const GLint& value) const
{
    HAS_ACTIVE_PROGRAM(ppo);

    glProgramUniform1i(ppo->GetActiveProgram()->GetId(), loc, value);
}

void GLUFShaderManager::GLProgramUniform2i(const GLUFSepProgramPtr& ppo, GLuint loc, const glm::i32vec2& value) const
{
    HAS_ACTIVE_PROGRAM(ppo);

    glProgramUniform2iv(ppo->GetActiveProgram()->GetId(), loc, 1, &value[0]);
}

void GLUFShaderManager::GLProgramUniform3i(const GLUFSepProgramPtr& ppo, GLuint loc, const glm::i32vec3& value) const
{
    HAS_ACTIVE_PROGRAM(ppo);

    glProgramUniform3iv(ppo->GetActiveProgram()->GetId(), loc, 1, &value[0]);
}

void GLUFShaderManager::GLProgramUniform4i(const GLUFSepProgramPtr& ppo, GLuint loc, const glm::i32vec4& value) const
{
    HAS_ACTIVE_PROGRAM(ppo);

    glProgramUniform4iv(ppo->GetActiveProgram()->GetId(), loc, 1, &value[0]);
}

void GLUFShaderManager::GLProgramUniform1i(const GLUFSepProgramPtr& ppo, const std::string& name, const GLint& value) const
{
    glProgramUniform1i(ppo->GetActiveProgram()->GetId(), GetUniformIdFromName(ppo, name), value);
}

void GLUFShaderManager::GLProgramUniform2i(const GLUFSepProgramPtr& ppo, const std::string& name, const glm::i32vec2& value) const
{
    glProgramUniform2iv(ppo->GetActiveProgram()->GetId(), GetUniformIdFromName(ppo, name), 1, &value[0]);
}

void GLUFShaderManager::GLProgramUniform3i(const GLUFSepProgramPtr& ppo, const std::string& name, const glm::i32vec3& value) const
{
    glProgramUniform3iv(ppo->GetActiveProgram()->GetId(), GetUniformIdFromName(ppo, name), 1, &value[0]);
}

void GLUFShaderManager::GLProgramUniform4i(const GLUFSepProgramPtr& ppo, const std::string& name, const glm::i32vec4& value) const
{
    glProgramUniform4iv(ppo->GetActiveProgram()->GetId(), GetUniformIdFromName(ppo, name), 1, &value[0]);
}


/*

uint's

*/

void GLUFShaderManager::GLProgramUniform1ui(const GLUFSepProgramPtr& ppo, GLuint loc, const GLuint& value) const
{
    HAS_ACTIVE_PROGRAM(ppo);

    glProgramUniform1ui(ppo->GetActiveProgram()->GetId(), loc, value);
}

void GLUFShaderManager::GLProgramUniform2ui(const GLUFSepProgramPtr& ppo, GLuint loc, const glm::u32vec2& value) const
{
    HAS_ACTIVE_PROGRAM(ppo);

    glProgramUniform2uiv(ppo->GetActiveProgram()->GetId(), loc, 1, &value[0]);
}

void GLUFShaderManager::GLProgramUniform3ui(const GLUFSepProgramPtr& ppo, GLuint loc, const glm::u32vec3& value) const
{
    HAS_ACTIVE_PROGRAM(ppo);

    glProgramUniform3uiv(ppo->GetActiveProgram()->GetId(), loc, 1, &value[0]);
}

void GLUFShaderManager::GLProgramUniform4ui(const GLUFSepProgramPtr& ppo, GLuint loc, const glm::u32vec4& value) const
{
    HAS_ACTIVE_PROGRAM(ppo);

    glProgramUniform4uiv(ppo->GetActiveProgram()->GetId(), loc, 1, &value[0]);
}

void GLUFShaderManager::GLProgramUniform1ui(const GLUFSepProgramPtr& ppo, const std::string& name, const GLuint& value) const
{
    glProgramUniform1ui(ppo->GetActiveProgram()->GetId(), GetUniformIdFromName(ppo, name), value);
}

void GLUFShaderManager::GLProgramUniform2ui(const GLUFSepProgramPtr& ppo, const std::string& name, const glm::u32vec2& value) const
{
    glProgramUniform2uiv(ppo->GetActiveProgram()->GetId(), GetUniformIdFromName(ppo, name), 1, &value[0]);
}

void GLUFShaderManager::GLProgramUniform3ui(const GLUFSepProgramPtr& ppo, const std::string& name, const glm::u32vec3& value) const
{
    glProgramUniform3uiv(ppo->GetActiveProgram()->GetId(), GetUniformIdFromName(ppo, name), 1, &value[0]);
}

void GLUFShaderManager::GLProgramUniform4ui(const GLUFSepProgramPtr& ppo, const std::string& name, const glm::u32vec4& value) const
{
    glProgramUniform4uiv(ppo->GetActiveProgram()->GetId(), GetUniformIdFromName(ppo, name), 1, &value[0]);
}

/*

matricies

*/

void GLUFShaderManager::GLProgramUniformMatrix2f(const GLUFSepProgramPtr& ppo, GLuint loc, const glm::mat2& value) const
{
    HAS_ACTIVE_PROGRAM(ppo);

    glProgramUniformMatrix2fv(ppo->GetActiveProgram()->GetId(), loc, 1, 0, &value[0][0]);
}

void GLUFShaderManager::GLProgramUniformMatrix3f(const GLUFSepProgramPtr& ppo, GLuint loc, const glm::mat3& value) const
{
    HAS_ACTIVE_PROGRAM(ppo);

    glProgramUniformMatrix3fv(ppo->GetActiveProgram()->GetId(), loc, 1, 0, &value[0][0]);
}

void GLUFShaderManager::GLProgramUniformMatrix4f(const GLUFSepProgramPtr& ppo, GLuint loc, const glm::mat4& value) const
{
    HAS_ACTIVE_PROGRAM(ppo);

    glProgramUniformMatrix4fv(ppo->GetActiveProgram()->GetId(), loc, 1, 0, &value[0][0]);
}

void GLUFShaderManager::GLProgramUniformMatrix2x3f(const GLUFSepProgramPtr& ppo, GLuint loc, const glm::mat2x3& value) const
{
    HAS_ACTIVE_PROGRAM(ppo);

    glProgramUniformMatrix2x3fv(ppo->GetActiveProgram()->GetId(), loc, 1, 0, &value[0][0]);
}

void GLUFShaderManager::GLProgramUniformMatrix3x2f(const GLUFSepProgramPtr& ppo, GLuint loc, const glm::mat3x2& value) const
{
    HAS_ACTIVE_PROGRAM(ppo);

    glProgramUniformMatrix3x2fv(ppo->GetActiveProgram()->GetId(), loc, 1, 0, &value[0][0]);
}

void GLUFShaderManager::GLProgramUniformMatrix2x4f(const GLUFSepProgramPtr& ppo, GLuint loc, const glm::mat2x4& value) const
{
    HAS_ACTIVE_PROGRAM(ppo);

    glProgramUniformMatrix2x4fv(ppo->GetActiveProgram()->GetId(), loc, 1, 0, &value[0][0]);
}

void GLUFShaderManager::GLProgramUniformMatrix4x2f(const GLUFSepProgramPtr& ppo, GLuint loc, const glm::mat4x2& value) const
{
    HAS_ACTIVE_PROGRAM(ppo);

    glProgramUniformMatrix4x2fv(ppo->GetActiveProgram()->GetId(), loc, 1, 0, &value[0][0]);
}

void GLUFShaderManager::GLProgramUniformMatrix3x4f(const GLUFSepProgramPtr& ppo, GLuint loc, const glm::mat3x4& value) const
{
    HAS_ACTIVE_PROGRAM(ppo);

    glProgramUniformMatrix3x4fv(ppo->GetActiveProgram()->GetId(), loc, 1, 0, &value[0][0]);
}

void GLUFShaderManager::GLProgramUniformMatrix4x3f(const GLUFSepProgramPtr& ppo, GLuint loc, const glm::mat4x3& value) const
{
    HAS_ACTIVE_PROGRAM(ppo);

    glProgramUniformMatrix4x3fv(ppo->GetActiveProgram()->GetId(), loc, 1, 0, &value[0][0]);
}

void GLUFShaderManager::GLProgramUniformMatrix2f(const GLUFSepProgramPtr& ppo, const std::string& name, const glm::mat2& value) const
{
    glProgramUniformMatrix2fv(ppo->GetActiveProgram()->GetId(), GetUniformIdFromName(ppo, name), 1, 0, &value[0][0]);
}

void GLUFShaderManager::GLProgramUniformMatrix3f(const GLUFSepProgramPtr& ppo, const std::string& name, const glm::mat3& value) const
{
    glProgramUniformMatrix3fv(ppo->GetActiveProgram()->GetId(), GetUniformIdFromName(ppo, name), 1, 0, &value[0][0]);
}

void GLUFShaderManager::GLProgramUniformMatrix4f(const GLUFSepProgramPtr& ppo, const std::string& name, const glm::mat4& value) const
{
    glProgramUniformMatrix4fv(ppo->GetActiveProgram()->GetId(), GetUniformIdFromName(ppo, name), 1, 0, &value[0][0]);
}

void GLUFShaderManager::GLProgramUniformMatrix2x3f(const GLUFSepProgramPtr& ppo, const std::string& name, const glm::mat2x3& value) const
{
    glProgramUniformMatrix2x3fv(ppo->GetActiveProgram()->GetId(), GetUniformIdFromName(ppo, name), 1, 0, &value[0][0]);
}

void GLUFShaderManager::GLProgramUniformMatrix3x2f(const GLUFSepProgramPtr& ppo, const std::string& name, const glm::mat3x2& value) const
{
    glProgramUniformMatrix3x2fv(ppo->GetActiveProgram()->GetId(), GetUniformIdFromName(ppo, name), 1, 0, &value[0][0]);
}

void GLUFShaderManager::GLProgramUniformMatrix2x4f(const GLUFSepProgramPtr& ppo, const std::string& name, const glm::mat2x4& value) const
{
    glProgramUniformMatrix2x4fv(ppo->GetActiveProgram()->GetId(), GetUniformIdFromName(ppo, name), 1, 0, &value[0][0]);
}

void GLUFShaderManager::GLProgramUniformMatrix4x2f(const GLUFSepProgramPtr& ppo, const std::string& name, const glm::mat4x2& value) const
{
    glProgramUniformMatrix4x2fv(ppo->GetActiveProgram()->GetId(), GetUniformIdFromName(ppo, name), 1, 0, &value[0][0]);
}

void GLUFShaderManager::GLProgramUniformMatrix3x4f(const GLUFSepProgramPtr& ppo, const std::string& name, const glm::mat3x4& value) const
{
    glProgramUniformMatrix3x4fv(ppo->GetActiveProgram()->GetId(), GetUniformIdFromName(ppo, name), 1, 0, &value[0][0]);
}

void GLUFShaderManager::GLProgramUniformMatrix4x3f(const GLUFSepProgramPtr& ppo, const std::string& name, const glm::mat4x3& value) const
{
    glProgramUniformMatrix4x3fv(ppo->GetActiveProgram()->GetId(), GetUniformIdFromName(ppo, name), 1, 0, &value[0][0]);
}

/*
======================================================================================================================================================================================================

Buffer and Texture (alpha)


*/


/*

Texture Utilities:

*/



#define FOURCC_DXT1 0x31545844 // Equivalent to "DXT1" in ASCII
#define FOURCC_DXT3 0x33545844 // Equivalent to "DXT3" in ASCII
#define FOURCC_DXT5 0x35545844 // Equivalent to "DXT5" in ASCII

/*
LoadTextureDDS

    Parameters:
        'rawData': raw data loaded from file to put into OpenGL

    Returns:
        OpenGL Id of texture created

    Throws:
        'std::invalid_argument': if data is too small, is not the correct file format, 

*/
GLuint LoadTextureDDS(const std::vector<char>& rawData)
{
    //TODO support more compatibility, ie RGB, BGR, don't make it dependent on ABGR


    //verify size of header
    if (rawData.size() < 128)
    {
        GLUF_CRITICAL_EXCEPTION(std::invalid_argument("(LoadTextureDDS): Raw Data Too Small For Header!"));
    }

    //verify the type of file
    std::string filecode(rawData.begin(), rawData.begin() + 4);
    if (filecode != "DDS ")
    {
        GLUF_CRITICAL_EXCEPTION(std::invalid_argument("(LoadTextureDDS): Incorrect File Format!"));
    }

    //load the header

    //this is all the data I need, but I just need to load it properly to opengl (NOTE: this is all offset by 4 bytes because of the filecode)
    unsigned int height = *(unsigned int*)&(rawData[12]);
    unsigned int width = *(unsigned int*)&(rawData[16]);
    unsigned int linearSize = *(unsigned int*)&(rawData[20]);
    unsigned int mipMapCount = *(unsigned int*)&(rawData[28]);
    unsigned int flags = *(unsigned int*)&(rawData[80]);
    unsigned int fourCC = *(unsigned int*)&(rawData[84]);
    unsigned int RGBBitCount = *(unsigned int*)&(rawData[88]);
    unsigned int RBitMask = *(unsigned int*)&(rawData[92]);
    unsigned int GBitMask = *(unsigned int*)&(rawData[96]);
    unsigned int BBitMask = *(unsigned int*)&(rawData[100]);
    unsigned int ABitMask = *(unsigned int*)&(rawData[104]);

    //how big is it going to be including all mipmaps?
    unsigned int bufsize = mipMapCount > 1 ? linearSize * 2 : linearSize;

    //verify size of data again once header is loaded
                                                            //this does not work when the file is compressed
    /*if (rawData.size() < 128 + bufsize)
    {
        throw std::invalid_argument("(LoadTextureDDS): Raw Data Too Small!");
    }*/


    unsigned int components = (fourCC == FOURCC_DXT1) ? 3 : 4;
    unsigned int compressedFormat;
    switch (fourCC)
    {
    case FOURCC_DXT1:
        compressedFormat = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
        break;
    case FOURCC_DXT3:
        compressedFormat = GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
        break;
    case FOURCC_DXT5:
        compressedFormat = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
        break;
    default:
        compressedFormat = 0;//uncompressed
    }

    // Create one OpenGL texture
    GLuint textureID;
    glGenTextures(1, &textureID);

    //make sure OpenGL successfully created the texture before loading it
    if (textureID == 0)
        GLUF_CRITICAL_EXCEPTION(TextureCreationException());

    // "Bind" the newly created texture : all future texture functions will modify this texture
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, mipMapCount - 1);//REMEMBER it is max mip, NOT mip count
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_R, GL_RED);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_G, GL_GREEN);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_B, GL_BLUE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_A, GL_ALPHA);

    if (compressedFormat != 0)
    {
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

        unsigned int blockSize = (compressedFormat == GL_COMPRESSED_RGBA_S3TC_DXT1_EXT) ? 8 : 16;
        unsigned int offset = 128;// initial offset to compensate for header and file code

        /* load the mipmaps */
        for (unsigned int level = 0; level < mipMapCount/* && (width || height)*/; ++level)
        {
            unsigned int mipSize = ((width + 3) / 4)*((height + 3) / 4)*blockSize;
            glCompressedTexImage2D(GL_TEXTURE_2D, 
                level, compressedFormat, 
                width, height,
                0, mipSize, 
                rawData.data() + offset);

            offset += mipSize;
            width /= 2;
            height /= 2;

            // Deal with Non-Power-Of-Two textures
            if (width < 1) width = 1;
            if (height < 1) height = 1;

        }

    }
    else
    {
        unsigned int offset = 128;// initial offset to compensate for header and file code

        for (unsigned int level = 0; level < mipMapCount/* && (width || height)*/; ++level)
        {
            glTexImage2D(GL_TEXTURE_2D, 
                level, GL_RGBA, 
                width, height, 
                0, GL_RGBA, GL_UNSIGNED_BYTE, 
                rawData.data() + offset);

            unsigned int mipSize = (width * height * 4);
            offset += mipSize;
            width /= 2;
            height /= 2;

            // Deal with Non-Power-Of-Two textures
            if (width < 1) width = 1;
            if (height < 1) height = 1;
        }
    }

    glBindTexture(GL_TEXTURE_2D, 0);

    return textureID;
}

GLuint LoadTextureCubemapDDS(const std::vector<char>& rawData)
{    
    //TODO support more compatibility, ie RGB, BGR, don't make it dependent on ABGR


    //verify size of header
    if (rawData.size() < 128)
    {
        GLUF_CRITICAL_EXCEPTION(std::invalid_argument("(LoadTextureDDS): Raw Data Too Small For Header!"));
    }

    //verify the type of file 
    std::string filecode(rawData.begin(), rawData.begin() + 4);
    if (filecode != "DDS ")
    {
        GLUF_CRITICAL_EXCEPTION(std::invalid_argument("(LoadTextureDDS): Incorrect File Format!"));
    }
    
    //load the header

    //this is all the data I need, but I just need to load it properly to opengl (NOTE: this is all offset by 4 bytes because of the filecode)
    unsigned int height = *(unsigned int*)&(rawData[12]);
    unsigned int width = *(unsigned int*)&(rawData[16]);
    unsigned int linearSize = *(unsigned int*)&(rawData[20]);
    unsigned int mipMapCount = *(unsigned int*)&(rawData[28]);
    unsigned int flags = *(unsigned int*)&(rawData[80]);
    unsigned int fourCC = *(unsigned int*)&(rawData[84]);
    unsigned int RGBBitCount = *(unsigned int*)&(rawData[88]);
    unsigned int RBitMask = *(unsigned int*)&(rawData[92]);
    unsigned int GBitMask = *(unsigned int*)&(rawData[96]);
    unsigned int BBitMask = *(unsigned int*)&(rawData[100]);
    unsigned int ABitMask = *(unsigned int*)&(rawData[104]);

    //how big is it going to be including all mipmaps? 
    unsigned int bufsize = mipMapCount > 1 ? linearSize * 2 : linearSize;

    //verify size of data again once header is loaded
    if (rawData.size() < 128 + bufsize)
    {
        GLUF_CRITICAL_EXCEPTION(std::invalid_argument("(LoadTextureDDS): Raw Data Too Small!"));
    }
    

    unsigned int components = (fourCC == FOURCC_DXT1) ? 3 : 4;
    unsigned int compressedFormat;
    switch (fourCC)
    {
    case FOURCC_DXT1:
        compressedFormat = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
        break;
    case FOURCC_DXT3:
        compressedFormat = GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
        break;
    case FOURCC_DXT5:
        compressedFormat = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
        break;
    default:
        compressedFormat = 0;//uncompressed
    }

    // Create one OpenGL texture
    GLuint textureID;
    glGenTextures(1, &textureID);

    //make sure OpenGL successfully created the texture before loading it
    if (textureID == 0)
        GLUF_CRITICAL_EXCEPTION(TextureCreationException());


    // "Bind" the newly created texture : all future texture functions will modify this texture
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_BASE_LEVEL, 0);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAX_LEVEL, 0);//REMEMBER it is max mip, NOT mip count
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_SWIZZLE_R, GL_RED);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_SWIZZLE_G, GL_GREEN);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_SWIZZLE_B, GL_BLUE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_SWIZZLE_A, GL_ALPHA);

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    //this method is not the prettyest, but it is the easiest to load
    if (compressedFormat != 0)
    {
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

        unsigned int blockSize = (compressedFormat == GL_COMPRESSED_RGBA_S3TC_DXT1_EXT) ? 8 : 16;
        unsigned int offset = 128;// initial offset to compensate for header and file code

        unsigned int pertexSize = width;

        unsigned int mipSize = ((pertexSize + 3) / 4)*((pertexSize + 3) / 4)*blockSize;
        glCompressedTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 
            0, compressedFormat, 
            pertexSize, pertexSize,
            0, mipSize, 
            rawData.data() + offset);
        offset += mipSize;
        glCompressedTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, 
            0, compressedFormat, 
            pertexSize, pertexSize,
            0, mipSize, 
            rawData.data() + offset);
        offset += mipSize;
        glCompressedTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 
            0, compressedFormat, 
            pertexSize, pertexSize,
            0, mipSize, 
            rawData.data() + offset);
        offset += mipSize;
        glCompressedTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, 
            0, compressedFormat, 
            pertexSize, pertexSize,
            0, mipSize, 
            rawData.data() + offset);
        offset += mipSize;
        glCompressedTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, 
            0, compressedFormat, 
            pertexSize, pertexSize,
            0, mipSize, 
            rawData.data() + offset);
        offset += mipSize;
        glCompressedTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 
            0, compressedFormat, 
            pertexSize, pertexSize,
            0, mipSize, 
            rawData.data() + offset);

    }
    else
    {
        unsigned int offset = 128;// initial offset to compensate for header and file code
        unsigned int pertexSize = width;
        unsigned int mipSize = (pertexSize * pertexSize * 4);

        glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 
            0, GL_RGBA, 
            pertexSize, pertexSize, 
            0, GL_RGBA, GL_UNSIGNED_BYTE, 
            (const GLvoid*)&(rawData.begin() + offset));
        offset += mipSize;
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, 
            0, GL_RGBA, 
            pertexSize, pertexSize, 
            0, GL_RGBA, GL_UNSIGNED_BYTE, 
            (const GLvoid*)&(rawData.begin() + offset));
        offset += mipSize;
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 
            0, GL_RGBA, 
            pertexSize, pertexSize, 
            0, GL_RGBA, GL_UNSIGNED_BYTE, 
            (const GLvoid*)&(rawData.begin() + offset));
        offset += mipSize;
        glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, 
            0, GL_RGBA, 
            pertexSize, pertexSize, 
            0, GL_RGBA, GL_UNSIGNED_BYTE, 
            (const GLvoid*)&(rawData.begin() + offset));
        offset += mipSize;
        glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, 
            0, GL_RGBA, 
            pertexSize, pertexSize, 
            0, GL_RGBA, GL_UNSIGNED_BYTE, 
            (const GLvoid*)&(rawData.begin() + offset));
        offset += mipSize;
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 
            0, GL_RGBA, 
            pertexSize, pertexSize, 
            0, GL_RGBA, GL_UNSIGNED_BYTE, 
            (const GLvoid*)&(rawData.begin() + offset));
    }


    return textureID;
}



GLuint LoadTextureFromFile(const std::wstring& filePath, GLUFTextureFileFormat format)
{
    std::vector<char> memory;

    try
    {
        GLUFLoadFileIntoMemory(filePath, memory);
    }
    catch (const std::ios_base::failure& e)
    {
        GLUF_ERROR_LONG("(LoadTextureFromFile): " << e.what());
    }

    GLuint texId;
    try
    {
        texId = LoadTextureFromMemory(memory, format);
    }
    catch (const TextureCreationException& e)
    {
        GLUF_ERROR_LONG("(LoadTextureFromFile): " << e.what());
        RETHROW;
    }

    return texId;
}


GLuint LoadTextureFromMemory(const std::vector<char>& data, GLUFTextureFileFormat format)
{

    try
    {
        switch (format)
        {
        case TFF_DDS:
            return LoadTextureDDS(data);
        case TTF_DDS_CUBEMAP:
            return LoadTextureCubemapDDS(data);
        }
    }
    catch (const TextureCreationException& e)
    {
        GLUF_ERROR_LONG("(LoadTextureFromMemory): " << e.what());
        RETHROW;
    }
    return 0;
}


/*

Buffer Utilities

*/

const GLUFVertexAttribInfo& GLUFVertexArrayBase::GetAttribInfoFromLoc(GLUFAttribLoc loc) const
{
    auto val = mAttribInfos.find(loc);
    if (val == mAttribInfos.end())
        GLUF_NON_CRITICAL_EXCEPTION(std::invalid_argument("\"loc\" not found in attribute list"));

    return val->second;
}

GLUFVertexArrayBase::GLUFVertexArrayBase(GLenum PrimType, GLenum buffUsage, bool index) : mUsageType(buffUsage), mPrimitiveType(PrimType)
{
    SWITCH_GL_VERSION
    GL_VERSION_GREATER_EQUAL(30)
    {
        glGenVertexArrayBindVertexArray(&mVertexArrayId);

        if (mVertexArrayId == 0)
            GLUF_CRITICAL_EXCEPTION(MakeVOAException());

    }

	if (index)
	{
		glGenBuffers(1, &mIndexBuffer);
        if (mIndexBuffer == 0)
        {
            GLUF_ERROR("Failed to create index buffer!");
            GLUF_CRITICAL_EXCEPTION(MakeBufferException());
        }

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIndexBuffer);

		glGenBuffers(1, &mRangedIndexBuffer);
        if (mRangedIndexBuffer == 0)
        {
            GLUF_ERROR("Failed to create ranged index buffer!");
            GLUF_CRITICAL_EXCEPTION(MakeBufferException());
        }
	}
}

GLUFVertexArrayBase::~GLUFVertexArrayBase()
{
	BindVertexArray();

	glDeleteBuffers(1, &mIndexBuffer);
    glDeleteBuffers(1, &mRangedIndexBuffer);

    SWITCH_GL_VERSION
    GL_VERSION_GREATER_EQUAL(30)
	    glDeleteVertexArrays(1, &mVertexArrayId);

    UnBindVertexArray();
}

GLUFVertexArrayBase::GLUFVertexArrayBase(GLUFVertexArrayBase&& other)
{
    //set this class
    mVertexArrayId      = other.mVertexArrayId;
    mVertexCount        = other.mVertexCount;
    mUsageType          = other.mUsageType;
    mPrimitiveType      = other.mPrimitiveType;
    mAttribInfos        = std::move(other.mAttribInfos);
    mIndexBuffer        = other.mIndexBuffer;
    mRangedIndexBuffer  = other.mRangedIndexBuffer;
    mIndexCount         = other.mIndexCount;
    mTempVAOId          = other.mTempVAOId;//likely will be 0 anyways


    //reset other class
    other.mVertexArrayId        = 0;
    other.mVertexCount          = 0;
    other.mUsageType            = GL_STATIC_DRAW;
    other.mPrimitiveType        = GL_TRIANGLES;
    //other.mAttribInfos.clear();
    other.mIndexBuffer          = 0;
    other.mRangedIndexBuffer    = 0;
    other.mIndexCount           = 0;
    other.mTempVAOId            = 0;//likely will be 0 anyways
}

GLUFVertexArrayBase& GLUFVertexArrayBase::operator=(GLUFVertexArrayBase&& other)
{
    //set this class
    mVertexArrayId = other.mVertexArrayId;
    mVertexCount = other.mVertexCount;
    mUsageType = other.mUsageType;
    mPrimitiveType = other.mPrimitiveType;
    mAttribInfos = std::move(other.mAttribInfos);
    mIndexBuffer = other.mIndexBuffer;
    mRangedIndexBuffer = other.mRangedIndexBuffer;
    mIndexCount = other.mIndexCount;
    mTempVAOId = other.mTempVAOId;//likely will be 0 anyways


    //reset other class
    other.mVertexArrayId = 0;
    other.mVertexCount = 0;
    other.mUsageType = GL_STATIC_DRAW;
    other.mPrimitiveType = GL_TRIANGLES;
    //other.mAttribInfos.clear();
    other.mIndexBuffer = 0;
    other.mRangedIndexBuffer = 0;
    other.mIndexCount = 0;
    other.mTempVAOId = 0;//likely will be 0 anyways

    return *this;
}

void GLUFVertexArrayBase::AddVertexAttrib(const GLUFVertexAttribInfo& info)
{
    //don't do null checks, because BindVertexArray already does them for us
    BindVertexArray();

    //make sure the attribute contains valid data
    if (info.mBytesPerElement == 0 || info.mElementsPerValue == 0)
        GLUF_CRITICAL_EXCEPTION(std::invalid_argument("Invalid Data in Vertex Attribute Info!"));

    mAttribInfos.insert(std::pair<GLUFAttribLoc, GLUFVertexAttribInfo>(info.mVertexAttribLocation, info));

    //this is a bit inefficient to refresh every time an attribute is added, but this should not be significant
    RefreshDataBufferAttribute();

    //enable the new attribute
    glEnableVertexAttribArray(info.mVertexAttribLocation);

    UnBindVertexArray();
}

void GLUFVertexArrayBase::RemoveVertexAttrib(GLUFAttribLoc loc)
{
    auto val = mAttribInfos.find(loc);
    if (val == mAttribInfos.end())
        GLUF_NON_CRITICAL_EXCEPTION(std::invalid_argument("\"loc\" not found in attribute list"));

    mAttribInfos.erase(val);

    RefreshDataBufferAttribute();
}

void GLUFVertexArrayBase::BindVertexArray()
{
    NOEXCEPT_REGION_START

    SWITCH_GL_VERSION
    GL_VERSION_GREATER_EQUAL(30)
    {
        //store the old one before binding this one
        GLint tmpVAOId = 0;
        glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &tmpVAOId);
        mTempVAOId = static_cast<GLuint>(tmpVAOId);

        glBindVertexArray(mVertexArrayId);
    }

    NOEXCEPT_REGION_END
}

void GLUFVertexArrayBase::UnBindVertexArray()
{    
    NOEXCEPT_REGION_START

    SWITCH_GL_VERSION
    GL_VERSION_GREATER_EQUAL(30)
    {
        glBindVertexArray(mTempVAOId);
        mTempVAOId = 0;
    }

    NOEXCEPT_REGION_END
}

void GLUFVertexArrayBase::Draw()
{
    NOEXCEPT_REGION_START

    BindVertexArray();

    SWITCH_GL_VERSION
    GL_VERSION_LESS(30)
    {
        EnableVertexAttributes();//must disable and re-enable every time with openGL less than 3.0
    }

    if (mIndexBuffer != 0)
    {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIndexBuffer);
        glDrawElements(mPrimitiveType, mIndexCount, GL_UNSIGNED_INT, nullptr);
    }
    else
    {
        glDrawArrays(mPrimitiveType, 0, mVertexCount);
    }

    SWITCH_GL_VERSION
    GL_VERSION_LESS(30)
    {
        DisableVertexAttributes();
    }

    UnBindVertexArray();

    NOEXCEPT_REGION_END
}

void GLUFVertexArrayBase::DrawRange(GLuint start, GLuint count)
{
    NOEXCEPT_REGION_START

    BindVertexArray();

    SWITCH_GL_VERSION
    GL_VERSION_LESS(30)
    {
        EnableVertexAttributes();//must disable and re-enable every time with openGL less than 3.0
    }

    if (mIndexBuffer != 0)
    {
        //copy the range over an draw
        glBindBuffer(GL_COPY_READ_BUFFER, mIndexBuffer);
        glBindBuffer(GL_COPY_WRITE_BUFFER, mRangedIndexBuffer);

        glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, sizeof(GLuint) * start, 0, sizeof(GLuint) * count);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mRangedIndexBuffer);
        glDrawElements(mPrimitiveType, mIndexCount, GL_UNSIGNED_INT, nullptr);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIndexBuffer);//reset to the index buffer, because it is more likely to be used than the range index buffer
    }
    else
    {
        glDrawArrays(mPrimitiveType, (start < 0 || start > mVertexCount) ? 0 : start, (count > mVertexCount) ? mVertexCount : count);
    }

    SWITCH_GL_VERSION
    GL_VERSION_LESS(30)
    {
        DisableVertexAttributes();
    }

    UnBindVertexArray();

    NOEXCEPT_REGION_END
}

void GLUFVertexArrayBase::DrawInstanced(GLuint instances)
{
    NOEXCEPT_REGION_START

    BindVertexArray();

    SWITCH_GL_VERSION
    GL_VERSION_LESS(30)
    {
        EnableVertexAttributes();//must disable and re-enable every time with openGL less than 3.0
    }

    if (mIndexBuffer != 0)
    {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIndexBuffer);
        glDrawElementsInstanced(mPrimitiveType, mIndexCount, GL_UNSIGNED_INT, nullptr, instances);
    }
    else
    {
        glDrawArraysInstanced(mPrimitiveType, 0, mVertexCount, instances);
    }

    SWITCH_GL_VERSION
    GL_VERSION_LESS(30)
    {
        DisableVertexAttributes();
    }

    UnBindVertexArray();

    NOEXCEPT_REGION_END
}

//helper function
void GLUFVertexArrayBase::BufferIndicesBase(GLuint indexCount, const GLvoid* data)
{
    NOEXCEPT_REGION_START

    BindVertexArray();
    mIndexCount = indexCount;
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIndexBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * mIndexCount, data, mUsageType);

    UnBindVertexArray();
    NOEXCEPT_REGION_END
}

void GLUFVertexArrayBase::BufferIndices(const std::vector<GLuint>& indices)
{
    BufferIndicesBase(indices.size(), &indices[0]);
}

void GLUFVertexArrayBase::BufferIndices(const std::vector<glm::u32vec2>& indices)
{
    BufferIndicesBase(indices.size() * 2, &indices[0]);
}

void GLUFVertexArrayBase::BufferIndices(const std::vector<glm::u32vec3>& indices)
{
    BufferIndicesBase(indices.size() * 3, &indices[0]);
}

void GLUFVertexArrayBase::BufferIndices(const std::vector<glm::u32vec4>& indices)
{
    BufferIndicesBase(indices.size() * 4, &indices[0]);
}


/*
RoundNearestMultiple

    Parameters:
        'num': Number to round
        'multiple': which multiple to round to

    Throws:
        'std::invalid_argument' if multiple == 0
       
    Note:
        This always rounds up to the next multiple
        Usage is to make sure memory is within certain boundaries (i.e. 4 byte boundaries)

*/
int RoundNearestMultiple(unsigned int num, unsigned int multiple)
{
    if (multiple == 0)
    {
        GLUF_NON_CRITICAL_EXCEPTION(std::invalid_argument("Multiple Cannot Be 0"));
        return 0;//if the multiple is 0, still needs to return something in release mode
    }

	unsigned int nearestMultiple = 0;

    //loop up to the 
	for (unsigned int i = 0; i < num; i += multiple)
	{
		nearestMultiple = i;
	}

	nearestMultiple += multiple;
	return nearestMultiple;
}

void GLUFVertexArrayAoS::RefreshDataBufferAttribute()
{
    NOEXCEPT_REGION_START

    SWITCH_GL_VERSION
    GL_VERSION_GREATER_EQUAL(30)//this is done at draw time in opengl less than 3.0
    {

        BindVertexArray();

        glBindBuffer(GL_ARRAY_BUFFER, mDataBuffer);

        GLuint stride = GetVertexSize();
        for (auto it : mAttribInfos)
        {
            //the last parameter might be wrong
            glVertexAttribPointer(it.second.mVertexAttribLocation, it.second.mElementsPerValue, it.second.mType, GL_FALSE, stride, reinterpret_cast<GLvoid*>(static_cast<uintptr_t>(it.second.mOffset)));
        }

        UnBindVertexArray();
    }

    NOEXCEPT_REGION_END
}

GLUFVertexArrayAoS::GLUFVertexArrayAoS(GLenum PrimType, GLenum buffUsage, bool indexed) : GLUFVertexArrayBase(PrimType, buffUsage, indexed)
{
	//the VAO is already bound

	glGenBuffers(1, &mDataBuffer);

    if (mDataBuffer == 0)
        GLUF_CRITICAL_EXCEPTION(MakeBufferException());
}

GLUFVertexArrayAoS::~GLUFVertexArrayAoS()
{
	BindVertexArray();

	glDeleteBuffers(1, &mDataBuffer);

    UnBindVertexArray();
}

GLUFVertexArrayAoS::GLUFVertexArrayAoS(GLUFVertexArrayAoS&& other) : GLUFVertexArrayBase(std::move(other))
{
	mDataBuffer = other.mDataBuffer;

    other.mDataBuffer = 0;
}

GLUFVertexArrayAoS& GLUFVertexArrayAoS::operator=(GLUFVertexArrayAoS&& other)
{
    //since there is no possibility for user error, not catching dynamic cast here is A-OK

    GLUFVertexArrayBase* thisParentPtr = dynamic_cast<GLUFVertexArrayBase*>(this);
    *thisParentPtr = std::move(other);

    mDataBuffer = other.mDataBuffer;
    other.mDataBuffer = 0;

    return *this;
}

GLuint GLUFVertexArrayAoS::GetVertexSize() const noexcept
{
	GLuint stride = 0;
    NOEXCEPT_REGION_START

    //WOW: this before was allocating memory to find the size of the memory, then didn't even delete it
	for (auto it : mAttribInfos)
	{
		//round to the 4 bytes bounderies
		stride += it.second.mElementsPerValue * RoundNearestMultiple(it.second.mBytesPerElement, 4);
	}

    NOEXCEPT_REGION_END

	return stride;
}

void GLUFVertexArrayAoS::AddVertexAttrib(const GLUFVertexAttribInfo& info, GLuint offset)
{
    //don't do null checks, because BindVertexArray already does them for us
    BindVertexArray();

    //make sure the attribute contains valid data
    if (info.mBytesPerElement == 0 || info.mElementsPerValue == 0)
        GLUF_CRITICAL_EXCEPTION(std::invalid_argument("Invalid Data in Vertex Attribute Info!"));

    //integrate the offset into the data
    GLUFVertexAttribInfo tmpCopy = info;
    tmpCopy.mOffset = offset;

    mAttribInfos.insert(std::pair<GLUFAttribLoc, GLUFVertexAttribInfo>(tmpCopy.mVertexAttribLocation, tmpCopy));

    //this is a bit inefficient to refresh every time an attribute is added, but this should not be significant
    RefreshDataBufferAttribute();

    //enable the new attribute
    glEnableVertexAttribArray(info.mVertexAttribLocation);//not harmful in opengl less than 3.0

    UnBindVertexArray();
}

void GLUFVertexArrayAoS::EnableVertexAttributes() const
{
    NOEXCEPT_REGION_START

    glBindBuffer(GL_ARRAY_BUFFER, mDataBuffer);

    GLuint stride = GetVertexSize();
    for (auto it : mAttribInfos)
    {
        glEnableVertexAttribArray(it.second.mVertexAttribLocation);
        glVertexAttribPointer(it.second.mVertexAttribLocation, it.second.mElementsPerValue, it.second.mType, GL_FALSE, stride, reinterpret_cast<GLvoid*>(static_cast<uintptr_t>(it.second.mOffset)));
    }


    NOEXCEPT_REGION_END
}

void GLUFVertexArrayAoS::DisableVertexAttributes() const
{
    NOEXCEPT_REGION_START

    glBindBuffer(GL_ARRAY_BUFFER, mDataBuffer);

    for(auto it : mAttribInfos)
    {
        glDisableVertexAttribArray(it.second.mVertexAttribLocation);
    }

    NOEXCEPT_REGION_END
}



void GLUFVertexArraySoA::RefreshDataBufferAttribute()
{
    //this is done at draw time less than opengl 3.0
    SWITCH_GL_VERSION
    GL_VERSION_GREATER_EQUAL(30)
    {
        BindVertexArray();
        for (auto it : mAttribInfos)
        {
            glBindBuffer(GL_ARRAY_BUFFER, mDataBuffers[it.second.mVertexAttribLocation]);
            glVertexAttribPointer(it.second.mVertexAttribLocation, it.second.mElementsPerValue, it.second.mType, GL_FALSE, 0, nullptr);
        }
        UnBindVertexArray();
    }
}

GLuint GLUFVertexArraySoA::GetBufferIdFromAttribLoc(GLUFAttribLoc loc) const
{
    auto ret = mDataBuffers.find(loc);
    if (ret == mDataBuffers.end())
        GLUF_CRITICAL_EXCEPTION(InvalidAttrubuteLocationException());

    return ret->second;
}

GLUFVertexArraySoA::GLUFVertexArraySoA(GLenum PrimType, GLenum buffUsage, bool indexed) : GLUFVertexArrayBase(PrimType, buffUsage, indexed)
{
}

GLUFVertexArraySoA::~GLUFVertexArraySoA()
{
	BindVertexArray();
	for (auto it : mDataBuffers)
        glDeleteBuffers(1, &it.second);
    UnBindVertexArray();
}

GLUFVertexArraySoA::GLUFVertexArraySoA(GLUFVertexArraySoA&& other) : GLUFVertexArrayBase(std::move(other))
{
    mDataBuffers = std::move(other.mDataBuffers);
}

GLUFVertexArraySoA& GLUFVertexArraySoA::operator=(GLUFVertexArraySoA&& other)
{
    //since there is no possibility for user error, not catching dynamic cast here is A-OK

    GLUFVertexArrayBase* thisParentPtr = dynamic_cast<GLUFVertexArrayBase*>(this);
    *thisParentPtr = std::move(other);

    mDataBuffers = std::move(other.mDataBuffers);

    return *this;
}

void GLUFVertexArraySoA::GetBarebonesMesh(GLUFMeshBarebones& inData)
{
	BindVertexArray();

	std::map<GLUFAttribLoc, GLuint>::iterator it = mDataBuffers.find(GLUF_VERTEX_ATTRIB_POSITION);
	if (mIndexBuffer == 0 || it == mDataBuffers.end())
	{
        GLUF_CRITICAL_EXCEPTION(InvalidAttrubuteLocationException());
	}
	
	glBindBuffer(GL_ARRAY_BUFFER, it->second);
	glm::vec3* pVerts = (glm::vec3*)glMapBuffer(GL_ARRAY_BUFFER, GL_READ_ONLY);

    inData.mVertices = GLUFArrToVec(pVerts, mVertexCount);
	glUnmapBuffer(GL_ARRAY_BUFFER);


	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIndexBuffer);
	GLuint* pIndices = (GLuint*)glMapBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_READ_ONLY);

    inData.mIndices = GLUFArrToVec(pIndices, mIndexCount);
    glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);

    UnBindVertexArray();
}

void GLUFVertexArraySoA::AddVertexAttrib(const GLUFVertexAttribInfo& info)
{
    NOEXCEPT_REGION_START

	BindVertexArray();

    mAttribInfos.insert(std::pair<GLUFAttribLoc, GLUFVertexAttribInfo>(info.mVertexAttribLocation, info));

	GLuint newBuff = 0;
	glGenBuffers(1, &newBuff);
	mDataBuffers.insert(std::pair<GLUFAttribLoc, GLuint>(info.mVertexAttribLocation, newBuff));
	
	RefreshDataBufferAttribute();

    UnBindVertexArray();

    NOEXCEPT_REGION_END
}

void GLUFVertexArraySoA::RemoveVertexAttrib(GLUFAttribLoc loc)
{
    NOEXCEPT_REGION_START

	BindVertexArray();

	auto it = mDataBuffers.find(loc);

	glDeleteBuffers(1, &(it->second));
	mDataBuffers.erase(it);

    GLUFVertexArrayBase::RemoveVertexAttrib(loc);

    UnBindVertexArray();

    NOEXCEPT_REGION_END
}

void GLUFVertexArraySoA::EnableVertexAttributes() const
{
    NOEXCEPT_REGION_START

    auto it = mDataBuffers.begin();
    for (auto itAttrib : mAttribInfos)
    {
        glBindBuffer(GL_ARRAY_BUFFER, it->second);
        glEnableVertexAttribArray(itAttrib.second.mVertexAttribLocation);
        glVertexAttribPointer(itAttrib.second.mVertexAttribLocation, itAttrib.second.mElementsPerValue, itAttrib.second.mType, GL_FALSE, 0, nullptr);
        ++it;
    }

    NOEXCEPT_REGION_END
}

void GLUFVertexArraySoA::DisableVertexAttributes() const
{
    NOEXCEPT_REGION_START

    for (auto it : mAttribInfos)
    {
        glDisableVertexAttribArray(it.second.mVertexAttribLocation);
    }

    NOEXCEPT_REGION_END
}



/*
=======================================================================================================================================================================================================
Assimp Utility Functions

*/

std::shared_ptr<GLUFVertexArray> LoadVertexArrayFromScene(const aiScene* scene, GLuint meshNum)
{
    if (meshNum > scene->mNumMeshes)
        GLUF_CRITICAL_EXCEPTION(std::invalid_argument("\"meshNum\" is higher than number of meshes in \"scene\""));

	//const aiMesh* mesh = scene->mMeshes[meshNum];

	std::shared_ptr<GLUFVertexArray> arr = LoadVertexArrayFromScene(scene, g_stdAttrib, meshNum);

	/*if (mesh->HasPositions())
		vertexData->AddVertexAttrib(g_attribPOS);
	if (mesh->HasNormals())
		vertexData->AddVertexAttrib(g_attribNORM);
	if (mesh->HasTextureCoords(0))
		vertexData->AddVertexAttrib(g_attribUV0);
	if (mesh->HasTextureCoords(1))
		vertexData->AddVertexAttrib(g_attribUV1);
	if (mesh->HasTextureCoords(2))
		vertexData->AddVertexAttrib(g_attribUV2);
	if (mesh->HasTextureCoords(3))
		vertexData->AddVertexAttrib(g_attribUV3);
	if (mesh->HasTextureCoords(4))
		vertexData->AddVertexAttrib(g_attribUV4);
	if (mesh->HasTextureCoords(5))
		vertexData->AddVertexAttrib(g_attribUV5);
	if (mesh->HasTextureCoords(6))
		vertexData->AddVertexAttrib(g_attribUV6);
	if (mesh->HasTextureCoords(7))
		vertexData->AddVertexAttrib(g_attribUV7);

	if (mesh->HasVertexColors(0))
		vertexData->AddVertexAttrib(g_attribCOLOR0);
	if (mesh->HasVertexColors(1))
		vertexData->AddVertexAttrib(g_attribCOLOR1);
	if (mesh->HasVertexColors(2))
		vertexData->AddVertexAttrib(g_attribCOLOR2);
	if (mesh->HasVertexColors(3))
		vertexData->AddVertexAttrib(g_attribCOLOR3);
	if (mesh->HasVertexColors(4))
		vertexData->AddVertexAttrib(g_attribCOLOR4);
	if (mesh->HasVertexColors(5))
		vertexData->AddVertexAttrib(g_attribCOLOR5);
	if (mesh->HasVertexColors(6))
		vertexData->AddVertexAttrib(g_attribCOLOR6);
	if (mesh->HasVertexColors(7))
		vertexData->AddVertexAttrib(g_attribCOLOR7);
	if (mesh->HasTangentsAndBitangents())
	{
		vertexData->AddVertexAttrib(g_attribTAN);
		vertexData->AddVertexAttrib(g_attribBITAN);
	}


	if (mesh->HasPositions())
    vertexData->BufferData(GLUF_VERTEX_ATTRIB_POSITION, mesh->mNumVertices, mesh->mVertices);
    if (mesh->HasNormals())
    vertexData->BufferData(GLUF_VERTEX_ATTRIB_NORMAL, mesh->mNumVertices, mesh->mNormals);
    if (mesh->HasTextureCoords(0))
    vertexData->BufferData(GLUF_VERTEX_ATTRIB_UV0, mesh->mNumVertices, AssimpToGlm3_2(mesh->mTextureCoords[0], mesh->mNumVertices));
    if (mesh->HasTextureCoords(1))
    vertexData->BufferData(GLUF_VERTEX_ATTRIB_UV1, mesh->mNumVertices, AssimpToGlm3_2(mesh->mTextureCoords[1], mesh->mNumVertices));
    if (mesh->HasTextureCoords(2))
    vertexData->BufferData(GLUF_VERTEX_ATTRIB_UV2, mesh->mNumVertices, AssimpToGlm3_2(mesh->mTextureCoords[2], mesh->mNumVertices));
    if (mesh->HasTextureCoords(3))
    vertexData->BufferData(GLUF_VERTEX_ATTRIB_UV3, mesh->mNumVertices, AssimpToGlm3_2(mesh->mTextureCoords[3], mesh->mNumVertices));
    if (mesh->HasTextureCoords(4))
    vertexData->BufferData(GLUF_VERTEX_ATTRIB_UV4, mesh->mNumVertices, AssimpToGlm3_2(mesh->mTextureCoords[4], mesh->mNumVertices));
    if (mesh->HasTextureCoords(5))
    vertexData->BufferData(GLUF_VERTEX_ATTRIB_UV5, mesh->mNumVertices, AssimpToGlm3_2(mesh->mTextureCoords[5], mesh->mNumVertices));
    if (mesh->HasTextureCoords(6))
    vertexData->BufferData(GLUF_VERTEX_ATTRIB_UV6, mesh->mNumVertices, AssimpToGlm3_2(mesh->mTextureCoords[6], mesh->mNumVertices));
    if (mesh->HasTextureCoords(7))
    vertexData->BufferData(GLUF_VERTEX_ATTRIB_UV7, mesh->mNumVertices, AssimpToGlm3_2(mesh->mTextureCoords[7], mesh->mNumVertices));

    if (mesh->HasVertexColors(0))
    vertexData->BufferData(GLUF_VERTEX_ATTRIB_COLOR0, mesh->mNumVertices, mesh->mColors[0]);
    if (mesh->HasVertexColors(1))
    vertexData->BufferData(GLUF_VERTEX_ATTRIB_COLOR1, mesh->mNumVertices, mesh->mColors[1]);
    if (mesh->HasVertexColors(2))
    vertexData->BufferData(GLUF_VERTEX_ATTRIB_COLOR2, mesh->mNumVertices, mesh->mColors[2]);
    if (mesh->HasVertexColors(3))
    vertexData->BufferData(GLUF_VERTEX_ATTRIB_COLOR3, mesh->mNumVertices, mesh->mColors[3]);
    if (mesh->HasVertexColors(4))
    vertexData->BufferData(GLUF_VERTEX_ATTRIB_COLOR4, mesh->mNumVertices, mesh->mColors[4]);
    if (mesh->HasVertexColors(5))
    vertexData->BufferData(GLUF_VERTEX_ATTRIB_COLOR5, mesh->mNumVertices, mesh->mColors[5]);
    if (mesh->HasVertexColors(6))
    vertexData->BufferData(GLUF_VERTEX_ATTRIB_COLOR6, mesh->mNumVertices, mesh->mColors[6]);
    if (mesh->HasVertexColors(7))
    vertexData->BufferData(GLUF_VERTEX_ATTRIB_COLOR7, mesh->mNumVertices, mesh->mColors[7]);
    if (mesh->HasTangentsAndBitangents())
    {
    vertexData->BufferData(GLUF_VERTEX_ATTRIB_BITAN, mesh->mNumVertices, mesh->mBitangents);
    vertexData->BufferData(GLUF_VERTEX_ATTRIB_TAN, mesh->mNumVertices, mesh->mTangents);
    }

    std::vector<GLuint> indices;
    for (unsigned int i = 0; i < mesh->mNumFaces; ++i)
    {
    aiFace curr = mesh->mFaces[i];
    indices.push_back(curr.mIndices[0]);
    indices.push_back(curr.mIndices[1]);
    indices.push_back(curr.mIndices[2]);
    }
    vertexData->BufferIndices(&indices[0], indices.size());*/

return arr;
}

/*

GLUFAssimpVertexStruct


*/

struct GLUFAssimpVertexStruct : public GLUFVertexStruct
{
    std::vector<aiVector2D> v2;
    std::vector<aiVector3D> v3;
    std::vector<aiColor4D> v4;

    GLUFAssimpVertexStruct(size_t vec2Cnt, size_t vec3Cnt, size_t vec4Cnt)
    {
        v2.resize(vec2Cnt);
        v3.resize(vec3Cnt);
        v4.resize(vec4Cnt);
    }

    GLUFAssimpVertexStruct(){};

    virtual void* operator&() const override
    {
        char* ret = new char[size()];

        size_t v2Size = 2 * v2.size() * 4;
        size_t v3Size = 3 * v3.size() * 4;
        size_t v4Size = 4 * v4.size() * 4;

        memcpy(ret, v2.data(), v2Size);
        memcpy(ret + v2Size, v3.data(), v3Size);
        memcpy(ret + v2Size + v3Size, v4.data(), v4Size);

        return ret;
    }

    virtual size_t size() const override
    {
        return 4 * (2 * v2.size() + 3 * v3.size() + 4 * v4.size());
    }

    virtual size_t n_elem_size(size_t element)
    {
        size_t i = 0;
        if (element < v2.size())
            return sizeof(aiVector2D);

        i += v2.size();

        if (element < i + v3.size())
            return sizeof(aiVector3D);

        i += v3.size();

        if (element < i + v4.size())
            return sizeof(aiColor4D);

        return 0;//if it is too big, just return 0; not worth an exception
    }

    virtual void buffer_element(void* data, size_t element) override
    {
        size_t i = 0;
        if (element < v2.size())
        {
            v2[element] = static_cast<aiVector2D*>(data)[0];
            return;
        }

        i += v2.size();

        if (element < i + v3.size())
        {
            v3[element - i] = static_cast<aiVector3D*>(data)[0];
            return;
        }

        i += v3.size();

        if (element < i + v4.size())
        {
            v4[element - i] = static_cast<aiColor4D*>(data)[0];
            return;
        }
    }

    static GLUFGLVector<GLUFAssimpVertexStruct> MakeMany(size_t howMany, size_t vec2Cnt, size_t vec3Cnt, size_t vec4Cnt)
    {
        GLUFGLVector<GLUFAssimpVertexStruct> ret;
        ret.reserve(howMany);

        //the individual struct object that will be cloned to fill the return vector
        GLUFAssimpVertexStruct clone{ vec2Cnt, vec3Cnt, vec4Cnt };
        
        //use 'fill' version of vector::insert for increased speed
        ret.insert(ret.begin(), howMany, clone);

        return ret;
    }
};


std::shared_ptr<GLUFVertexArray> LoadVertexArrayFromScene(const aiScene* scene, const GLUFVertexAttribMap& inputs, GLuint meshNum)
{
    if (meshNum > scene->mNumMeshes)
        GLUF_CRITICAL_EXCEPTION(std::invalid_argument("\"meshNum\" is higher than the number of meshes in \"scene\""));

	const aiMesh* mesh = scene->mMeshes[meshNum];

	auto vertexData = std::make_shared<GLUFVertexArray>(GL_TRIANGLES, GL_STATIC_DRAW, mesh->HasFaces());

    //which vertex attributes go where in assimp loading vertex struct
    std::map<GLUFAttribLoc, int> vertexAttribLoc;

    //the number of each vec type
    unsigned char numVec2 = 0;
    unsigned char numVec3 = 0;
    unsigned char numVec4 = 0;

    auto itUV0 = inputs.find(GLUF_VERTEX_ATTRIB_UV0);
    auto it = itUV0;
    if (mesh->HasTextureCoords(0) && it != inputs.end())
    {
        vertexData->AddVertexAttrib(it->second, numVec2 * 8);
        vertexAttribLoc[GLUF_VERTEX_ATTRIB_UV0] = numVec2;
        ++numVec2;
    }

    auto itUV1 = inputs.find(GLUF_VERTEX_ATTRIB_UV1);
    it = itUV1;
    if (mesh->HasTextureCoords(1) && it != inputs.end())
    {
        vertexData->AddVertexAttrib(it->second, numVec2 * 8);
        vertexAttribLoc[GLUF_VERTEX_ATTRIB_UV1] = numVec2;
        ++numVec2;
    }

    auto itUV2 = inputs.find(GLUF_VERTEX_ATTRIB_UV2);
    it = itUV2;
    if (mesh->HasTextureCoords(2) && it != inputs.end())
    {
        vertexData->AddVertexAttrib(it->second, numVec2 * 8);
        vertexAttribLoc[GLUF_VERTEX_ATTRIB_UV2] = numVec2;
        ++numVec2;
    }

    auto itUV3 = inputs.find(GLUF_VERTEX_ATTRIB_UV3);
    it = itUV3;
    if (mesh->HasTextureCoords(3) && it != inputs.end())
    {
        vertexData->AddVertexAttrib(it->second, numVec2 * 8);
        vertexAttribLoc[GLUF_VERTEX_ATTRIB_UV3] = numVec2;
        ++numVec2;
    }

    auto itUV4 = inputs.find(GLUF_VERTEX_ATTRIB_UV4);
    it = itUV4;
    if (mesh->HasTextureCoords(4) && it != inputs.end())
    {
        vertexData->AddVertexAttrib(it->second, numVec2 * 8);
        vertexAttribLoc[GLUF_VERTEX_ATTRIB_UV4] = numVec2;
        ++numVec2;
    }

    auto itUV5 = inputs.find(GLUF_VERTEX_ATTRIB_UV5);
    it = itUV5;
    if (mesh->HasTextureCoords(5) && it != inputs.end())
    {
        vertexData->AddVertexAttrib(it->second, numVec2 * 8);
        vertexAttribLoc[GLUF_VERTEX_ATTRIB_UV5] = numVec2;
        ++numVec2;
    }

    auto itUV6 = inputs.find(GLUF_VERTEX_ATTRIB_UV6);
    it = itUV6;
    if (mesh->HasTextureCoords(6) && it != inputs.end())
    {
        vertexData->AddVertexAttrib(it->second, numVec2 * 8);
        vertexAttribLoc[GLUF_VERTEX_ATTRIB_UV6] = numVec2;
        ++numVec2;
    }

    auto itUV7 = inputs.find(GLUF_VERTEX_ATTRIB_UV7);
    it = itUV7;
    if (mesh->HasTextureCoords(7) && it != inputs.end())
    {
        vertexData->AddVertexAttrib(it->second, numVec2 * 8);
        vertexAttribLoc[GLUF_VERTEX_ATTRIB_UV7] = numVec2;
        ++numVec2;
    }


    unsigned int runningOffsetTotal = numVec2 * 8;


    //vec3's go between vec2's and vec4's
    auto itPos = inputs.find(GLUF_VERTEX_ATTRIB_POSITION);
    it = itPos;
    if (mesh->HasPositions() && it != inputs.end())
    {
        vertexData->AddVertexAttrib(it->second, numVec3 * 12 + runningOffsetTotal);
        vertexAttribLoc[GLUF_VERTEX_ATTRIB_POSITION] = numVec3 + numVec2;
        ++numVec3;
    }

    auto itNorm = inputs.find(GLUF_VERTEX_ATTRIB_NORMAL);
    it = itNorm;
    if (mesh->HasNormals() && it != inputs.end())
    {
        vertexData->AddVertexAttrib(it->second, numVec3 * 12 + runningOffsetTotal);
        vertexAttribLoc[GLUF_VERTEX_ATTRIB_NORMAL] = numVec3 + numVec2;
        ++numVec3;
    }


    auto itTan = inputs.find(GLUF_VERTEX_ATTRIB_TAN);
    auto itBitan = inputs.find(GLUF_VERTEX_ATTRIB_BITAN);
    if (mesh->HasTangentsAndBitangents() && itTan != inputs.end() && itBitan != inputs.end())
    {
        vertexData->AddVertexAttrib(itTan->second, numVec3 * 12 + runningOffsetTotal);
        vertexData->AddVertexAttrib(itBitan->second, (numVec3 + 1) * 12 + runningOffsetTotal);
        vertexAttribLoc[GLUF_VERTEX_ATTRIB_TAN] = numVec3 + numVec2;
        vertexAttribLoc[GLUF_VERTEX_ATTRIB_BITAN] = numVec3 + 1 + numVec2;
        numVec3 += 2;
    }

    //to keep from doing unncessessary addition
    unsigned char runningTotal = numVec3 + numVec2;

    runningOffsetTotal += numVec3 * 12;

    auto itCol0 = inputs.find(GLUF_VERTEX_ATTRIB_COLOR0);
    it = itCol0;
    if (mesh->HasVertexColors(0) && it != inputs.end())
    {
        vertexData->AddVertexAttrib(it->second, numVec4 * 16 + runningOffsetTotal);
        vertexAttribLoc[GLUF_VERTEX_ATTRIB_COLOR0] = numVec4 + runningTotal;
        ++numVec4;
    }

    auto itCol1 = inputs.find(GLUF_VERTEX_ATTRIB_COLOR1);
    it = itCol1;
    if (mesh->HasVertexColors(1) && it != inputs.end())
    {
        vertexData->AddVertexAttrib(it->second, numVec4 * 16 + runningOffsetTotal);
        vertexAttribLoc[GLUF_VERTEX_ATTRIB_COLOR1] = numVec4 + runningTotal;
        ++numVec4;
    }

    auto itCol2 = inputs.find(GLUF_VERTEX_ATTRIB_COLOR2);
    it = itCol2;
    if (mesh->HasVertexColors(2) && it != inputs.end())
    {
        vertexData->AddVertexAttrib(it->second, numVec4 * 16 + runningOffsetTotal);
        vertexAttribLoc[GLUF_VERTEX_ATTRIB_COLOR2] = numVec4 + runningTotal;
        ++numVec4;
    }

    auto itCol3 = inputs.find(GLUF_VERTEX_ATTRIB_COLOR3);
    it = itCol3;
    if (mesh->HasVertexColors(3) && it != inputs.end())
    {
        vertexData->AddVertexAttrib(it->second, numVec4 * 16 + runningOffsetTotal);
        vertexAttribLoc[GLUF_VERTEX_ATTRIB_COLOR3] = numVec4 + runningTotal;
        ++numVec4;
    }

    auto itCol4 = inputs.find(GLUF_VERTEX_ATTRIB_COLOR4);
    it = itCol4;
    if (mesh->HasVertexColors(4) && it != inputs.end())
    {
        vertexData->AddVertexAttrib(it->second, numVec4 * 16 + runningOffsetTotal);
        vertexAttribLoc[GLUF_VERTEX_ATTRIB_COLOR4] = numVec4 + runningTotal;
        ++numVec4;
    }
	
    auto itCol5 = inputs.find(GLUF_VERTEX_ATTRIB_COLOR5);
    it = itCol5;
    if (mesh->HasVertexColors(5) && it != inputs.end())
    {
        vertexData->AddVertexAttrib(it->second, numVec4 * 16 + runningOffsetTotal);
        vertexAttribLoc[GLUF_VERTEX_ATTRIB_COLOR5] = numVec4 + runningTotal;
        ++numVec4;
    }

    auto itCol6 = inputs.find(GLUF_VERTEX_ATTRIB_COLOR6);
    it = itCol6;
    if (mesh->HasVertexColors(6) && it != inputs.end())
    {
        vertexData->AddVertexAttrib(it->second, numVec4 * 16 + runningOffsetTotal);
        vertexAttribLoc[GLUF_VERTEX_ATTRIB_COLOR6] = numVec4 + runningTotal;
        ++numVec4;
    }

    auto itCol7 = inputs.find(GLUF_VERTEX_ATTRIB_COLOR7);
    it = itCol7;
    if (mesh->HasVertexColors(7) && it != inputs.end())
    {
        vertexData->AddVertexAttrib(it->second, numVec4 * 16 + runningOffsetTotal);
        vertexAttribLoc[GLUF_VERTEX_ATTRIB_COLOR7] = numVec4 + runningTotal;
        ++numVec4;
    }



    //the custom vertex array
    GLUFGLVector<GLUFAssimpVertexStruct> vertexBuffer = GLUFAssimpVertexStruct::MakeMany(mesh->mNumVertices, numVec2, numVec3, numVec4);

    if (mesh->HasPositions() && itPos != inputs.end())
    {
        //positions will ALWAYS be the first vec3
        vertexBuffer.buffer_element(mesh->mVertices, vertexAttribLoc[GLUF_VERTEX_ATTRIB_POSITION]);
    }
    if (mesh->HasNormals() && itNorm != inputs.end())
    {
        vertexBuffer.buffer_element(mesh->mNormals, vertexAttribLoc[GLUF_VERTEX_ATTRIB_NORMAL]);
    }



    /*

    Instead of flipping the pixels when loading textures, UV's will be flipped instead

    Also, since we are using 2 dimmentional texture coordinates, make sure to convert to 2 element vectors

    */

    //note: data2D must be size in length
    const auto FlipAndConvertUVArray = [](aiVector3D* data, aiVector2D*& data2D, const unsigned int size)
    {
        for (unsigned int i = 0; i < size; ++i)
        {
            data2D[i].x = data[i].x;
            data2D[i].y = 1.0f - data[i].y;
        }
    };

    aiVector2D* data2D = nullptr;
    if (mesh->HasTextureCoords(0) && itUV0 != inputs.end())
    {
        //do not initialize data2D until here, because if there is not a first texture coord, then there wil be no others
        data2D = new aiVector2D[mesh->mNumVertices];

        FlipAndConvertUVArray(mesh->mTextureCoords[0], data2D, mesh->mNumVertices);
        vertexBuffer.buffer_element(data2D, vertexAttribLoc[GLUF_VERTEX_ATTRIB_UV0]);
    }
    if (mesh->HasTextureCoords(1) && itUV1 != inputs.end())
    {
        FlipAndConvertUVArray(mesh->mTextureCoords[1], data2D, mesh->mNumVertices);
        vertexBuffer.buffer_element(data2D, vertexAttribLoc[GLUF_VERTEX_ATTRIB_UV1]);
    }
    if (mesh->HasTextureCoords(2) && itUV2 != inputs.end())
    {
        FlipAndConvertUVArray(mesh->mTextureCoords[2], data2D, mesh->mNumVertices);
        vertexBuffer.buffer_element(data2D, vertexAttribLoc[GLUF_VERTEX_ATTRIB_UV2]);
    }
    if (mesh->HasTextureCoords(3) && itUV3 != inputs.end())
    {
        FlipAndConvertUVArray(mesh->mTextureCoords[3], data2D, mesh->mNumVertices);
        vertexBuffer.buffer_element(data2D, vertexAttribLoc[GLUF_VERTEX_ATTRIB_UV3]);
    }
    if (mesh->HasTextureCoords(4) && itUV4 != inputs.end())
    {
        FlipAndConvertUVArray(mesh->mTextureCoords[4], data2D, mesh->mNumVertices);
        vertexBuffer.buffer_element(data2D, vertexAttribLoc[GLUF_VERTEX_ATTRIB_UV4]);
    }
    if (mesh->HasTextureCoords(5) && itUV5 != inputs.end())
    {
        FlipAndConvertUVArray(mesh->mTextureCoords[5], data2D, mesh->mNumVertices);
        vertexBuffer.buffer_element(data2D, vertexAttribLoc[GLUF_VERTEX_ATTRIB_UV5]);
    }
    if (mesh->HasTextureCoords(6) && itUV6 != inputs.end())
    {
        FlipAndConvertUVArray(mesh->mTextureCoords[6], data2D, mesh->mNumVertices);
        vertexBuffer.buffer_element(data2D, vertexAttribLoc[GLUF_VERTEX_ATTRIB_UV6]);
    }
    if (mesh->HasTextureCoords(7) && itUV7 != inputs.end())
    {
        FlipAndConvertUVArray(mesh->mTextureCoords[7], data2D, mesh->mNumVertices);
        vertexBuffer.buffer_element(data2D, vertexAttribLoc[GLUF_VERTEX_ATTRIB_UV7]);
    }
    delete[] data2D;


    if (mesh->HasVertexColors(0) && itCol0 != inputs.end())
    {
        vertexBuffer.buffer_element(mesh->mColors[0], vertexAttribLoc[GLUF_VERTEX_ATTRIB_COLOR0]);
    }
    if (mesh->HasVertexColors(1) && itCol1 != inputs.end())
    {
        vertexBuffer.buffer_element(mesh->mColors[1], vertexAttribLoc[GLUF_VERTEX_ATTRIB_COLOR1]);
    }
    if (mesh->HasVertexColors(2) && itCol2 != inputs.end())
    {
        vertexBuffer.buffer_element(mesh->mColors[2], vertexAttribLoc[GLUF_VERTEX_ATTRIB_COLOR2]);
    }
    if (mesh->HasVertexColors(3) && itCol3 != inputs.end())
    {
        vertexBuffer.buffer_element(mesh->mColors[3], vertexAttribLoc[GLUF_VERTEX_ATTRIB_COLOR3]);
    }
    if (mesh->HasVertexColors(4) && itCol4 != inputs.end())
    {
        vertexBuffer.buffer_element(mesh->mColors[4], vertexAttribLoc[GLUF_VERTEX_ATTRIB_COLOR4]);
    }
    if (mesh->HasVertexColors(5) && itCol5 != inputs.end())
    {
        vertexBuffer.buffer_element(mesh->mColors[5], vertexAttribLoc[GLUF_VERTEX_ATTRIB_COLOR5]);
    }
    if (mesh->HasVertexColors(6) && itCol6 != inputs.end())
    {
        vertexBuffer.buffer_element(mesh->mColors[6], vertexAttribLoc[GLUF_VERTEX_ATTRIB_COLOR6]);
    }
    if (mesh->HasVertexColors(7) && itCol7 != inputs.end())
    {
        vertexBuffer.buffer_element(mesh->mColors[7], vertexAttribLoc[GLUF_VERTEX_ATTRIB_COLOR7]);
    }
	if (mesh->HasTangentsAndBitangents() && itBitan != inputs.end() && itTan != inputs.end())
    {
        vertexBuffer.buffer_element(mesh->mTangents, vertexAttribLoc[GLUF_VERTEX_ATTRIB_TAN]);
        vertexBuffer.buffer_element(mesh->mBitangents, vertexAttribLoc[GLUF_VERTEX_ATTRIB_BITAN]);
	}

    //don't forget to buffer the actual data :) (i actually forgot this part at first)
    vertexData->BufferData(vertexBuffer);


	std::vector<glm::u32vec3> indices;
	for (unsigned int i = 0; i < mesh->mNumFaces; ++i)
	{
		aiFace curr = mesh->mFaces[i];
        indices.push_back({ curr.mIndices[0], curr.mIndices[1], curr.mIndices[2] });
	}
	vertexData->BufferIndices(indices);

	return vertexData;
}


std::vector<std::shared_ptr<GLUFVertexArray>> LoadVertexArraysFromScene(const aiScene* scene, GLuint meshOffset, GLuint numMeshes)
{
	std::vector<std::shared_ptr<GLUFVertexArray>> arrays;

	if (numMeshes > scene->mNumMeshes)
	{
		arrays.push_back(nullptr);
		return arrays;
	}

	for(unsigned int cnt = 0; cnt < numMeshes; ++cnt)
	{
		arrays.push_back(LoadVertexArrayFromScene(scene, cnt));//new GLUFVertexArray(GL_TRIANGLES, GL_STATIC_DRAW, mesh->HasFaces());
	}
	return arrays;
}

/*
GLUFVertexArray* LoadVertexArrayFromFile(std::string path)
{
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(path,
		aiProcess_CalcTangentSpace |
		aiProcess_Triangulate |
		aiProcess_JoinIdenticalVertices |
		aiProcess_SortByPType);

	const aiMesh* mesh = scene->mMeshes[0];

	GLUFVertexArray vertexData(GL_TRIANGLES, GL_STATIC_DRAW, mesh->HasFaces());

	if (mesh->HasPositions())
		vertexData.AddVertexAttrib(g_attribPOS);
	if (mesh->HasNormals())
		vertexData.AddVertexAttrib(g_attribNORM);
	if (mesh->HasTextureCoords(0))
		vertexData.AddVertexAttrib(g_attribUV);
	if (mesh->HasTangentsAndBitangents())
	{
		vertexData.AddVertexAttrib(g_attribTAN);
		vertexData.AddVertexAttrib(g_attribBITAN);
	}
	if (mesh->HasVertexColors(0))
		vertexData.AddVertexAttrib(g_attribCOLOR);


	if (mesh->HasPositions())
		vertexData.BufferData(VERTEX_ATTRIB_POSITION, mesh->mNumVertices, mesh->mVertices);
	if (mesh->HasNormals())
		vertexData.BufferData(VERTEX_ATTRIB_NORMAL, mesh->mNumVertices, mesh->mNormals);
	if (mesh->HasTextureCoords(0))
		vertexData.BufferData(VERTEX_ATTRIB_UV, mesh->mNumVertices, AssimpToGlm3_2(mesh->mTextureCoords[0], mesh->mNumVertices));
	if (mesh->HasTangentsAndBitangents())
	{
		vertexData.BufferData(VERTEX_ATTRIB_BITAN, mesh->mNumVertices, mesh->mBitangents);
		vertexData.BufferData(VERTEX_ATTRIB_TAN, mesh->mNumVertices, mesh->mTangents);
	}
	if (mesh->HasVertexColors(0))
		vertexData.BufferData(VERTEX_ATTRIB_COLOR, mesh->mNumVertices, mesh->mColors[0]);

	std::vector<GLuint> indices;
	for (unsigned int i = 0; i < mesh->mNumFaces; ++i)
	{
		aiFace curr = mesh->mFaces[i];
		indices.push_back(curr.mIndices[0]);
		indices.push_back(curr.mIndices[1]);
		indices.push_back(curr.mIndices[2]);
	}
	vertexData.BufferIndices(&indices[0], indices.size());

	return vertexData;
	return LoadVertexArray(scene);
}

GLUFVertexArray *LoadVertexArrayFromFile(unsigned int size, void* data)
{
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFileFromMemory(data, size,
		aiProcess_CalcTangentSpace |
		aiProcess_Triangulate |
		aiProcess_JoinIdenticalVertices |
		aiProcess_SortByPType);

	return LoadVertexArray(scene);
}*/

}
// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the OBJGLUF_EXPORTS
// symbol defined on the command line. This symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// OBJGLUF_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.
#pragma once


#pragma warning (disable : 4251)

#ifdef _WIN32
#ifdef OBJGLUF_EXPORTS
#define OBJGLUF_API __declspec(dllexport)
#else
#define OBJGLUF_API __declspec(dllimport)
#endif
#else
#define OBJGLUF_API
#endif

//TODO LIST:
//
//support normal mapping
//support tangent space
//support guided lighting
//support guided materials


#include <GL/glew.h>
#include <GLFW/glfw3.h>
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>

#ifdef USING_ASSIMP
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#endif

#include <vector>
#include <memory>
#include <map>
#include <list>
#include <string>
#include <sstream>
#include <locale>
#include <codecvt>
#include <stdlib.h>
#include <stack>
#include <mutex>
#include <exception>

#ifndef OBJGLUF_EXPORTS
#ifndef SUPPRESS_RADIAN_ERROR
#error "GLM is using radians as input, to suppress this error, #define SUPPRESS_RADIAN_ERROR"
#endif
#endif

#ifndef OBJGLUF_EXPORTS
#ifndef SUPPRESS_UTF8_ERROR
#error "ATTENTION, all strings MUST be in utf8 encoding"
#endif
#endif

//uncomment this to build in release mode (i.e. no non-critical exceptions)
#define GLUF_DEBUG
#ifndef GLUF_DEBUG
    #if _MSC_VER
        #pragma message ("Warning, Debug Mode Disabled, No Non-Critical Exceptions will not be thrown!")
    #elif
        #warning ("Warning, Debug Mode Disabled, No Non-Critical Exceptions will not be thrown!")
    #endif
#endif



/*
======================================================================================================================================================================================================
Added std Funcitonality

*/

namespace std
{
	//This is a very useful function that is not in the standard libraries
	template<class _Ty> inline
	const _Ty& (clamp)(const _Ty& _Value, const _Ty& _Min, const _Ty& _Max)
	{
		return std::min(std::max(_Value, _Min), _Max);
	}
}

namespace GLUF
{

/*
======================================================================================================================================================================================================
Added OpenGL Functionality

*/

//This first generates a single buffer, then immediatly binds it
#define glGenBufferBindBuffer(target, buffer) glGenBuffers(1, buffer); glBindBuffer(target, *buffer)

//This first generates a single vertex array, then immediatly binds it
#define glGenVertexArrayBindVertexArray(vertexArray) glGenVertexArrays(1, vertexArray); glBindVertexArray(*vertexArray)


/*

Helpful OpenGL Constants

*/

OBJGLUF_API GLuint GetGLVersionMajor();
OBJGLUF_API GLuint GetGLVersionMinor();
OBJGLUF_API GLuint GetGLVersion2Digit();// if OpenGL Version 4.3, this would return 43


/*
======================================================================================================================================================================================================
Debugging Macros and Setup Functions

*/

using GLUFErrorMethod = void(*)(const std::string& message, const char* funcName, const char* sourceFile, unsigned int lineNum);

#define GLUF_ERROR(message) GLUFGetErrorMethod()(message, __FUNCTION__, __FILE__, __LINE__);
#define GLUF_ERROR_LONG(chain) {std::stringstream ss; ss << chain;  GLUFGetErrorMethod()(ss.str(), __FUNCTION__, __FILE__, __LINE__);}
#define GLUF_ASSERT(expr)	{ if (!(expr)) { std::stringstream ss; ss << "ASSERTION FAILURE: \"" << #expr << "\""; GLUF_ERROR(ss.str().c_str()) } }

#ifdef GLUF_DEBUG

#define GLUF_NULLPTR_CHECK(ptr) {if (ptr == nullptr){throw std::invalid_argument("Null Pointer");}}

#define GLUF_CRITICAL_EXCEPTION(exception) throw exception;
#define GLUF_NON_CRITICAL_EXCEPTION(exception) throw exception;

//used for defining private constructors which can be accessed by 'std::make_shared'; NOTE: all parameters must be const references
#define GLUF_FORCE_SMART_POINTERS(CLASSNAME, ...) CLASSNAME(__VA_ARGS__); friend std::shared_ptr<CLASSNAME> std::make_shared<CLASSNAME>(__VA_ARGS__); CLASSNAME() = delete;

#else

#define GLUF_NULLPTR_CHECK(ptr)

#define GLUF_CRITICAL_EXCEPTION(exception) throw exception;
#define GLUF_NON_CRITICAL_EXCEPTION(exception) ;

#endif

OBJGLUF_API void GLUFRegisterErrorMethod(GLUFErrorMethod method);
OBJGLUF_API GLUFErrorMethod GLUFGetErrorMethod();


/*
======================================================================================================================================================================================================
Utility Macros

*/

#define GLUF_SAFE_DELETE(ptr) {if(ptr){delete(ptr);} (ptr) = nullptr;}
#define GLUF_NULL(type) (std::shared_ptr<type>(nullptr))//may be deprecated
#define GLUF_UNREFERENCED_PARAMETER(value) (value)
#define NOEXCEPT_REGION_START try{
#define NOEXCEPT_REGION_END }catch(...){}
#define noexcept throw()

/*
======================================================================================================================================================================================================
Multithreading Macros

*/

//for use with mutexes
#define GLUF_TSAFE_BEGIN(Mutex) { std::lock_guard<std::mutex> __lock__(Mutex);
#define GLUF_TSAFE_END __lock__.unlock();}
#define GLUF_TSAFE_SCOPE(Mutex) std::lock_guard<std::mutex> __lock__{Mutex};

/*

Local Lock

    A 'unique_lock' helper class for locking a scope

*/
class LocalLock
{
    std::unique_lock<std::mutex>& mLock;
public:

    LocalLock(std::unique_lock<std::mutex>& lock) :
        mLock(lock)
    {
        mLock.lock();
    }

    ~LocalLock()
    {
        mLock.unlock();
    }
};

//for use with locks
#define GLUF_TSAFE_LOCK(Lock) Lock.lock();
#define GLUF_TSAFE_UNLOCK(Lock) Lock.unlock();
#define GLUF_TSAFE_LOCK_REGION(Lock) LocalLock __lock__(Lock);


/*
======================================================================================================================================================================================================
Statistics

*/

class GLUFStatsData
{
public:
    double mPreviousFrame = 0.0;
    double mUpdateInterval = 1.0;//time in seconds
    unsigned long long mFrameCount = 0LL;//frame count since previous update
    float mCurrFPS = 0.0f;
    std::wstring mFormattedStatsData = L"";
};

OBJGLUF_API void GLUFStats_func();
OBJGLUF_API const std::wstring& GLUFGetFrameStatsString();
OBJGLUF_API const GLUFStatsData& GLUFGetFrameStats();
OBJGLUF_API const std::wstring& GLUFGetDeviceStatus();

#define GLUFStats GLUF::GLUFStats_func


/*
======================================================================================================================================================================================================
Macros for constant approach for varying operating systems

*/

#ifndef _T
#define _T __T
#endif

#ifndef __T
#define __T(str) L ## str
#endif

#ifndef __FUNCTION__
#define __FUNCTION__ __func__
#endif


/*
======================================================================================================================================================================================================
Timing Macros (Uses GLFW Built-In Timer)

*/

#define GLUFGetTime() glfwGetTime()
#define GLUFGetTimef() ((float)glfwGetTime())
#define GLUFGetTimeMs() ((unsigned int)(glfwGetTime() * 1000.0))


/*
======================================================================================================================================================================================================
Type Aliases

*/

using Color   = glm::u8vec4;//only accepts numbers from 0 to 255
using Color3f = glm::vec3;
using Color4f = glm::vec4;
using Vec4Array = std::vector<glm::vec4>;
using Vec3Array =  std::vector<glm::vec3>;
using Vec2Array = std::vector<glm::vec2>;
using IndexArray = std::vector<GLuint>;
using GLUFAttribLoc = GLuint;


/*
======================================================================================================================================================================================================
Mathematical and Conversion Macros

*/

#define GLUF_PI    3.141592653589793
#define GLUF_PI_F  3.1415927f
#define GLUF_PI_LD 3.141592653589793238L

#define GLUF_E     2.718281828459045
#define GLUF_E_F   2.7182818f
#define GLUF_E_LD  2.718281828459045235L

#define DEG_TO_RAD(value) ((value) *(GLUF_PI / 180))
#define DEG_TO_RAD_F(value) ((value) *(GLUF_PI_F / 180))
#define DEG_TO_RAD_LD(value) ((value) *(GLUF_PI_LD / 180))

#define RAD_TO_DEG(value) ((value) *(180 / GLUF_PI))
#define RAD_TO_DEG_F(value) ((value) *(180 / GLUF_PI_F))
#define RAD_TO_DEG_LD(value) ((value) *(180 / GLUF_PI_LD))

#define GLUF_60HZ 0.0166666666666


/*
======================================================================================================================================================================================================
GLUF API Core Controller Methods

*/

//call this first
OBJGLUF_API bool GLUFInit();

//call this after calling glfwMakeContextCurrent on the window
OBJGLUF_API bool GLUFInitOpenGLExtensions();

//call this at the very last moment before application termination
OBJGLUF_API void GLUFTerminate();

OBJGLUF_API const std::vector<std::string>& GLUFGetGLExtensions();

/*
======================================================================================================================================================================================================
Context Controller Methods

*/

#define EnableVSync() glfwSwapInterval(0)
#define DisableVSync() glfwSwapInterval(1)
#define SetVSyncState(value) glfwSwapInterval(value ? 1 : 0)


//NOTE: All methods below this must be called BEFORE window creation
#define SetMSAASamples(numSamples) glfwWindowHint(GLFW_SAMPLES, 4)//make sure to call EnableMSAA()

//NOTE: All methods below this must be called AFTER window creation
#define EnableMSAA() glEnable(GL_MULTISAMPLE)//make sure to call SetMSAASamples() before calling this


/*
======================================================================================================================================================================================================
IO and Stream Utilities

*/


/*
GLUFLoadFileIntoMemory

    Parameters:
        'path': path of file to load
        'binMemory': vector to fill with loaded memory

    Throws: 
        'std::ios_base::failure' in event of file errors

    Note:
        If 'binMemory' is not empty, the data will be overwritten
*/
OBJGLUF_API void GLUFLoadFileIntoMemory(const std::wstring& path, std::vector<char>& binMemory);
OBJGLUF_API void GLUFLoadFileIntoMemory(const std::string& path, std::vector<char>& binMemory);

/*
GLUFLoadBinaryArrayIntoString

    Parameters:
        'rawMemory': memory to be read from
        'size': size of rawMemory
        'outString': where the string will be output

    Throws:
        'std::invalid_argument': if 'rawMemory' == nullptr
        'std::ios_base::failure': if memory streaming was unsuccessful

*/
OBJGLUF_API void GLUFLoadBinaryArrayIntoString(char* rawMemory, std::size_t size, std::string& outString);
OBJGLUF_API void GLUFLoadBinaryArrayIntoString(const std::vector<char>& rawMemory, std::string& outString);


/*
======================================================================================================================================================================================================
OpenGL Basic Data Structures and Operators

Note: these only play a significant role in GLUFGui, but are presented here as basic types and utilities which can be used independently of the GLUFGui

*/


//Rect is supposed to be used where the origin is bottom left
struct OBJGLUF_API GLUFRect
{
    long left, top, right, bottom;
};

struct OBJGLUF_API GLUFRectf
{
    float left, top, right, bottom;
};

struct OBJGLUF_API GLUFPoint
{
    union{ long x, width; };
    union{ long y, height; };

    GLUFPoint(long val1, long val2) : x(val1), y(val2){}
    GLUFPoint() : x(0L), y(0L){}
};

inline GLUFPoint operator /(const GLUFPoint& pt0, const GLUFPoint& pt1)
{
    return{ pt0.x / pt1.x, pt0.y / pt1.y };
}

inline GLUFPoint operator /(const GLUFPoint& pt0, const long& f)
{
    return{ pt0.x / f, pt0.y / f };
}

inline GLUFPoint operator -(const GLUFPoint& pt0, const GLUFPoint& pt1)
{
    return{ pt0.x - pt1.x, pt0.y - pt1.y };
}

inline bool operator ==(const GLUFRect& rc0, const GLUFRect& rc1)
{
    return
        (
        rc0.left == rc1.left && rc0.right == rc1.right &&
        rc0.top == rc1.top  && rc0.bottom == rc1.bottom
        );
}

inline bool operator !=(const GLUFRect& rc0, const GLUFRect& rc1)
{
    return !(rc0 == rc1);
}

OBJGLUF_API bool		GLUFPtInRect(const GLUFRect& rect, const GLUFPoint& pt);
OBJGLUF_API void		GLUFSetRectEmpty(GLUFRect& rect);
OBJGLUF_API void		GLUFSetRect(GLUFRect& rect, long left, long top, long right, long bottom);
OBJGLUF_API void		GLUFSetRect(GLUFRectf& rect, float left, float top, float right, float bottom);
OBJGLUF_API void		GLUFOffsetRect(GLUFRect& rect, long x, long y);
OBJGLUF_API void        GLUFRepositionRect(GLUFRect& rect, long newX, long newY);
OBJGLUF_API long		GLUFRectHeight(const GLUFRect& rect);
OBJGLUF_API long		GLUFRectWidth(const GLUFRect& rect);
OBJGLUF_API void		GLUFInflateRect(GLUFRect& rect, long dx, long dy);//center stays in same spot
OBJGLUF_API void        GLUFResizeRect(GLUFRect& rect, long newWidth, long newHeight);//bottom left stays in same spot
OBJGLUF_API bool		GLUFIntersectRect(const GLUFRect& rect0, const GLUFRect& rect1, GLUFRect& rectIntersect);
OBJGLUF_API GLUFPoint	GLUFMultPoints(const GLUFPoint& pt0, const GLUFPoint& pt1);

/*
======================================================================================================================================================================================================
Misc. GLUF Classes


*/



/*
GLUFMatrixStack

    Interface:
        -Use just as using std::stack, except slightly different naming convention

*/
class OBJGLUF_API GLUFMatrixStack
{
	std::stack<glm::mat4> mStack;
	static glm::mat4 mIdentity;
public:
	void Push(const glm::mat4& matrix);
	void Pop(void);
	const glm::mat4& Top() const;//this returns the matrix that is a concatination of all subsequent matrices
	size_t Size() const;
	bool Empty() const;
};


/*
MemStreamBuf
    
    A small class that takes raw data and creates a stream out of it

    Throws:
        -Does not catch any of 'std::streambuf's exceptions
*/
class OBJGLUF_API MemStreamBuf : public std::streambuf
{
public:
	MemStreamBuf(char* data, std::ptrdiff_t length)
	{
		setg(data, data, data + length);
	}
};





/*
======================================================================================================================================================================================================
String Utilities

*/

/*
GLUFSplitStr

    Parameters:
        's': string to be split
        'delim': character to look for as a split point
        'elems': vector of strings as result of split
        'keepDelim': does 'delim' get kept in string when splitting

    Returns:
        'elems'
*/

inline std::vector<std::wstring> &GLUFSplitStr(const std::wstring &s, wchar_t delim, std::vector<std::wstring> &elems, bool keepDelim = false)
{
	std::wstringstream ss(s);
	std::wstring item;
	while (std::getline(ss, item, delim))
	{
		if (keepDelim)//OOPS forgot this earlier
			item += delim;
		elems.push_back(item);
	}
	return elems;
}

inline std::vector<std::wstring> GLUFSplitStr(const std::wstring &s, wchar_t delim, bool keepDelim = false)
{
	std::vector<std::wstring> elems;
	GLUFSplitStr(s, delim, elems, keepDelim);
	return elems;
}

inline std::vector<std::string> &GLUFSplitStr(const std::string &s, char delim, std::vector<std::string> &elems, bool keepDelim = false)
{
	std::stringstream ss(s);
	std::string item;
	while (std::getline(ss, item, delim))
	{
		if (keepDelim)//OOPS forgot this earlier
			item += delim;
		elems.push_back(item);
	}
	return elems;
}

inline std::vector<std::string> GLUFSplitStr(const std::string &s, char delim, bool keepDelim = false)
{
	std::vector<std::string> elems;
	GLUFSplitStr(s, delim, elems, keepDelim);
	return elems;
}




/*
======================================================================================================================================================================================================
Datatype Conversion Functions

*/


/*
GLUFArrToVec

    Parameters:
        'arr': array to put into vector
        'len': length in elements of 'arr'

    Returns:
        -vector representing 'arr'

    Throws:
        'std::invalid_argument' if 'arr' == nullptr

    Note:
        This is only for legacy purposes; it is not efficient to do this regularly,
            so avoid using C-Style arrays to begin with!
*/
template<typename T>
inline std::vector<T> GLUFArrToVec(T* arr, unsigned long len);


/*
GLUFAdoptArray

    Parameters:
        'arr': array to have vector adopt
        'len': length in elements of 'arr'

    Returns:
        -vector containing 'arr'

    Throws:
        'std::invalid_argument' if 'arr' == nullptr

    Note:
        This actually tells the vector to use the C-Style array, and the C-Style array parameter will be set to 'nullptr'

*/

template<typename T>
inline std::vector<T> GLUFAdoptArray(T*& arr, unsigned long len) noexcept;

/*
GLUFGetVec2FromRect
    
    Parameters:
        'rect': the rect to retreive from
        'x': true: right; false: left
        'y': true: top; false: bottom

*/
OBJGLUF_API glm::vec2 GLUFGetVec2FromRect(const GLUFRect& rect, bool x, bool y);
OBJGLUF_API glm::vec2 GLUFGetVec2FromRect(const GLUFRectf& rect, bool x, bool y);

//used for getting vertices from rects 0,0 is bottom left
OBJGLUF_API GLUFPoint GLUFGetPointFromRect(const GLUFRect& rect, bool x, bool y);


OBJGLUF_API Color4f GLUFColorToFloat(const Color& color);//takes 0-255 to 0.0f - 1.0f
OBJGLUF_API Color3f GLUFColorToFloat3(const Color& color);//takes 0-255 to 0.0f - 1.0f




/*
======================================================================================================================================================================================================
Shader API (Alpha)

Note:
    -Classes and Structs for dealing directly with OpenGL may use some C types

*/


enum GLUFLocationType
{
	GLT_ATTRIB = 0,
	GLT_UNIFORM,
};

struct OBJGLUF_API GLUFShaderInfoStruct
{
	bool mSuccess = false;

    std::string mLog;

	operator bool() const
	{
		return mSuccess;
	}
};

enum GLUFShaderType
{
	SH_VERTEX_SHADER = GL_VERTEX_SHADER,
	SH_TESS_CONTROL_SHADER = GL_TESS_CONTROL_SHADER,
	SH_TESS_EVALUATION_SHADER = GL_TESS_EVALUATION_SHADER,
	SH_GEOMETRY_SHADER = GL_GEOMETRY_SHADER,
	SH_FRAGMENT_SHADER = GL_FRAGMENT_SHADER
};

enum GLUFProgramStage
{
    PPO_INVALID_SHADER_BIT = 0,//this is 0, since passing a null bitfield signifies not to use this program for any stage
    PPO_VERTEX_SHADER_BIT = GL_VERTEX_SHADER_BIT,
    PPO_TESS_CONTROL_SHADER_BIT = GL_TESS_CONTROL_SHADER_BIT,
    PPO_TESS_EVALUATION_SHADER_BIT = GL_TESS_EVALUATION_SHADER_BIT,
    PPO_GEOMETRY_SHADER_BIT = GL_GEOMETRY_SHADER_BIT,
    PPO_FRAGMENT_SHADER_BIT = GL_FRAGMENT_SHADER_BIT
};

/*
GLUFShaderTypeToProgramStage

    Parameters:
        'type': which shader type it is

    Returns:
        corresponding 'GLUFProgramStage' to 'type'
*/
GLUFProgramStage GLUFShaderTypeToProgramStage(GLUFShaderType type);

//use type alias because the compile and link output are both the same
using GLUFCompileOutputStruct = GLUFShaderInfoStruct;
using GLUFLinkOutputStruct    = GLUFShaderInfoStruct;


/*

Opaque Shader Classes

*/

class OBJGLUF_API GLUFShader;
class OBJGLUF_API GLUFComputeShader;
class OBJGLUF_API GLUFProgram;
class OBJGLUF_API GLUFSeparateProgram;


/*

Shader Aliases

*/

using GLUFShaderPtr         = std::shared_ptr<GLUFShader>;
using GLUFShaderPtrWeak     = std::weak_ptr<GLUFShader>;
using GLUFProgramPtr        = std::shared_ptr<GLUFProgram>;
using GLUFProgramPtrWeak    = std::weak_ptr<GLUFProgram>;
using GLUFSepProgramPtr     = std::shared_ptr<GLUFSeparateProgram>;
using GLUFSepProgramPtrWeak = std::weak_ptr<GLUFSeparateProgram>;

using GLUFShaderSourceList      = std::map<GLUFShaderType, std::string>;
using GLUFShaderPathList        = std::map<GLUFShaderType, std::wstring>;//use wstring, because paths may have unicode characters, but the source shall not
using GLUFShaderIdList          = std::vector<GLuint>;
using GLUFProgramIdList         = std::vector<GLuint>;
using GLUFShaderNameList        = std::vector<std::wstring>;
using GLUFProgramNameList       = std::vector<std::wstring>;
using GLUFShaderPtrList         = std::vector<GLUFShaderPtr>;
using GLUFProgramPtrList        = std::vector<GLUFProgramPtr>;
using GLUFProgramPtrMap         = std::map<GLUFShaderType, GLUFProgramPtr>;
using GLUFShaderPtrListWeak     = std::vector<GLUFShaderPtrWeak>;
using GLUFProgramPtrListWeak    = std::vector<GLUFProgramPtrWeak>;

using GLUFVariableLocMap    = std::map<std::string, GLuint>;
using GLUFVariableLocPair = std::pair < std::string, GLuint > ;

/*

Shader Exceptions

*/

//macro for forcing an exception to show up on the log upon contruction
#define EXCEPTION_CONSTRUCTOR_BODY \
    GLUF_ERROR_LONG("GLUF Exception Thrown: \"" << what() << "\"");
#define EXCEPTION_CONSTRUCTOR(class_name) \
class_name() \
{ \
    GLUF_ERROR_LONG("GLUF Exception Thrown: \"" << what() << "\""); \
}

/*
GLUFException

    Serves as base class for all other GLUF exceptions.  
    Override MyUniqueMessage in children
*/
class GLUFException : public std::exception
{ 
public:
    
    const char* what() const = 0;
};

class UseProgramException : public GLUFException
{
public:
    virtual const char* what() const override
    {
        return "Failed to Use Program!";
    }

    EXCEPTION_CONSTRUCTOR(UseProgramException)
};

class MakeShaderException : public GLUFException
{
public:
    virtual const char* what() const override
    {
        return "Shading Creation Failed!";
    }

    EXCEPTION_CONSTRUCTOR(MakeShaderException)
};

class MakeProgramException : public GLUFException
{
public:
    virtual const char* what() const override
    {
        return "Program Creation Failed!";
    }

    EXCEPTION_CONSTRUCTOR(MakeProgramException)
};

class MakePPOException : public GLUFException
{
public:
    virtual const char* what() const override
    {
        return "PPO Creation Failed!";
    }

    EXCEPTION_CONSTRUCTOR(MakePPOException)
};

class NoActiveProgramUniformException : public GLUFException
{
public:
    virtual const char* what() const override
    {
        return "Attempt to Buffer Uniform to PPO with no Active Program!";
    }

    EXCEPTION_CONSTRUCTOR(NoActiveProgramUniformException)
};

/*
GLUFShaderManager

    Multithreading:
        -Thread-Safe Functions:
            'CreateShaderFrom*'
            'CreateProgram'
            'GetShader*Location(s)'
            'Get*Log'
            'FlushLogs'
            'GL*'
            'GetUniformIdFromName'


    Data Members:
        'mCompileLogs': a map of logs for each compiled shader
        'mLinkLogs': a map of logs for each linked program
        'mCompLogMutex': mutual exclution protection for compile logs
        'mLinkLogMutex': mutual exclusion protection for link logs

    Note:
        Much of this is pass-by-reference in order to still have valid data members even
            if exceptions are thrown

*/

class OBJGLUF_API GLUFShaderManager
{

	//a list of logs
	std::map<GLUFShaderPtr, GLUFShaderInfoStruct>  mCompileLogs;
	std::map<GLUFProgramPtr, GLUFShaderInfoStruct> mLinklogs;

    mutable std::mutex mCompLogMutex;
    mutable std::mutex mLinkLogMutex;

    /*
    AddCompileLog

        Parameters:
            'shader': shader to add to pair
            'log': log to accompany it

        Note:
            This is meant to be use in order to handle thread-safe log access
    */
    void AddCompileLog(const GLUFShaderPtr& shader, const GLUFShaderInfoStruct& log);


    /*
    AddLinkLog

    Parameters:
        'program': program to add to pair
        'log': log to accompany it

    Note:
        This is meant to be use in order to handle thread-safe log access
    */
    void AddLinkLog(const GLUFProgramPtr& program, const GLUFShaderInfoStruct& log);

    /*
    GetUniformIdFromName

        Parameters:
            'prog': the program to access
            'ppo': the ppo to access
            'name': the name of the uniform

        Returns:
            the id of the uniform
    
    */
    GLuint GetUniformIdFromName(const GLUFSepProgramPtr& ppo, const std::string& name) const;
    GLuint GetUniformIdFromName(const GLUFProgramPtr& prog, const std::string& name) const;

public:

    /*
    CreateShader*
         
        Parameters:
            'filePath': path to file containing shader
            'text': text to compile
            'memory': raw memory of text file loaded
            'type': self-explanitory
            'outShader': your brand spankin' new shader

        Throws:
            -CreateShaderFromFile: 'std::ios_base::failure': if file had issues reading
            'MakeShaderException' if compilation, loading, or anything else failed
    
    */

	void CreateShaderFromFile(GLUFShaderPtr& outShader, const std::wstring& filePath, GLUFShaderType type);
    void CreateShaderFromText(GLUFShaderPtr& outShader, const std::string& text, GLUFShaderType type);
    void CreateShaderFromMemory(GLUFShaderPtr& outShader, const std::vector<char>& memory, GLUFShaderType type);


    /*
    CreateProgram
        
        Parameters:
            'shaders': simply a list of precompiled shaders to link
            'shaderSources': a list of shader files in strings to be compiled
            'shaderPaths': a list of shader file paths to be loaded into strings and compiled
            'separate': WIP Don't Use
            'outProgram': your brank spankin' new program

        Throws:
            'std::ios_base::failure': if file loading has issues
            'MakeProgramException' if linking failed
            'MakeShaderException' if shaders need to be compiled and fail
    
    */

	void CreateProgram(GLUFProgramPtr& outProgram, GLUFShaderPtrList shaders, bool separate = false);
    void CreateProgram(GLUFProgramPtr& outProgram, GLUFShaderSourceList shaderSources, bool separate = false);
    void CreateProgram(GLUFProgramPtr& outProgram, GLUFShaderPathList shaderPaths, bool separate = false);


    /*
    GetShader*Location
    
        Parameters:
            'prog': the program to access
            'locType': which location type? (attribute or uniform)
            'varName': name of variable to get id of

        Returns:
            'GLuint': location id of variable
            'GLUFVariableLocMap': map of attribute/variable names to their respective id's

        Throws:
            'std::invalid_argument': if 'prog' == nullptr
            'std::invalid_argument': if 'varName' does not exist
    
    */

	const GLuint GetShaderVariableLocation(const GLUFProgramPtr& program, GLUFLocationType locType, const std::string& varName) const;
    const GLUFVariableLocMap& GetShaderAttribLocations(const GLUFProgramPtr& program) const;
    const GLUFVariableLocMap& GetShaderUniformLocations(const GLUFProgramPtr& program) const;

    const GLUFVariableLocMap GetShaderAttribLocations(const GLUFSepProgramPtr& program) const;
    const GLUFVariableLocMap GetShaderUniformLocations(const GLUFSepProgramPtr& program) const;


    /*
    Delete*

        Parameters:
            'shader': shader to be deleted
            'program': program to be deleted

        Note:
            -This does not delete the smart pointers, however it does bring the shader to a state
                in which it can be reused

        Throws:
            no-throw guarantee
    
    */

	void DeleteShader(GLUFShaderPtr& shader) noexcept;
	void DeleteProgram(GLUFProgramPtr& program) noexcept;


    //self-explanitory
    void FlushLogs();

    /*
    Get
        
        Parameters:
            'shader': the shader to access
            'program': the program to access

        Returns:
            'GLuint': id of shader/program
            'GLUFShaderType': what type of shader it is
            'GLUFCompileOutputStruct': get log file for compiling
            'GLUFLinkOutputStruct': get log file for linking

        Throws
            'std::invalid_argument': if shader/program == nullptr
    */
    const GLuint	        GetShaderId(const GLUFShaderPtr& shader) const;
    const GLUFShaderType    GetShaderType(const GLUFShaderPtr& shader) const;
    const GLuint	        GetProgramId(const GLUFProgramPtr& program) const;

    const GLUFCompileOutputStruct   GetShaderLog(const GLUFShaderPtr& shaderPtr) const;
    const GLUFLinkOutputStruct      GetProgramLog(const GLUFProgramPtr& programPtr) const;


    /*
    UseProgram
        
        Parameters:
            'program': program to bind for drawing or other purposes

        Throws:
            'std::invalid_argument': if program == nullptr
            'UseProgramException': if program id == 0 (For binding the null program, use UseProgramNull() instead)
    
    */

    void UseProgram(const GLUFProgramPtr& program) const;
    void UseProgram(const GLUFSepProgramPtr& ppo) const;

    //bind program id and ppo id 0
    void UseProgramNull() const noexcept;


    /*
    AttachPrograms

        Parameters:
            'ppo': the programmible pipeline object
            'programs': the list of programs to add
            'stages': which stages to add program to
            'program': single program to add

        Throws:
            'std::invalid_argument': if ppo == nullptr
            'std::invalid_argument': if program(s) == nullptr
    */

    void AttachPrograms(GLUFSepProgramPtr& ppo, const GLUFProgramPtrList& programs) const;
    void AttachProgram(GLUFSepProgramPtr& ppo, const GLUFProgramPtr& program) const;


    /*
    ClearPrograms

        Parameters:
            'ppo': the programmible pipeline objects
            'stages': which stages should be cleared (reset to default)

        Throws:
            'std::invalid_argument': if ppo == nullptr
    */

    void ClearPrograms(GLUFSepProgramPtr& ppo, GLbitfield stages = GL_ALL_SHADER_BITS) const;


    /*
    CreateSeparateProgram

        Parameters:
            'ppo': a null pointer reference to the ppo that will be created
            'programs': the list of programs to initialize the ppo with

        Throws:
            'std::invalid_argument': if any of the programs are nullptr or have an id of 0
            'MakePPOException': if ppo creation failed
    
    */
    void CreateSeparateProgram(GLUFSepProgramPtr& ppo, const GLUFProgramPtrList& programs) const;

    /*
    GLUniform*

    Note:  
        the 'program' parameter is used as a look-up when the 'name' parameter is used,
            however you still must call 'UseProgram' before calling this

    OpenGL Version Restrictions:
        -GLUniform*ui* requires openGL version 3.0 or lator

    Parameters:
        'program': the program which the uniform will be set to
        'value': the data to set as the uniform
        'loc': the location of the uniform
        'name': the name of the uniform

    Throws:
        'std::invalid_argument': 'name' does not exist

    */

    void GLUniform1f(GLuint loc, const GLfloat& value) const noexcept;
    void GLUniform2f(GLuint loc, const glm::vec2& value) const noexcept;
    void GLUniform3f(GLuint loc, const glm::vec3& value) const noexcept;
    void GLUniform4f(GLuint loc, const glm::vec4& value) const noexcept;
    void GLUniform1i(GLuint loc, const GLint& value) const noexcept;
    void GLUniform2i(GLuint loc, const glm::i32vec2& value) const noexcept;
    void GLUniform3i(GLuint loc, const glm::i32vec3& value) const noexcept;
    void GLUniform4i(GLuint loc, const glm::i32vec4& value) const noexcept;
    void GLUniform1ui(GLuint loc, const GLuint& value) const noexcept;
    void GLUniform2ui(GLuint loc, const glm::u32vec2& value) const noexcept;
    void GLUniform3ui(GLuint loc, const glm::u32vec3& value) const noexcept;
    void GLUniform4ui(GLuint loc, const glm::u32vec4& value) const noexcept;

    void GLUniformMatrix2f(GLuint loc, const glm::mat2& value) const noexcept;
    void GLUniformMatrix3f(GLuint loc, const glm::mat3& value) const noexcept;
    void GLUniformMatrix4f(GLuint loc, const glm::mat4& value) const noexcept;
    void GLUniformMatrix2x3f(GLuint loc, const glm::mat2x3& value) const noexcept;
    void GLUniformMatrix3x2f(GLuint loc, const glm::mat3x2& value) const noexcept;
    void GLUniformMatrix2x4f(GLuint loc, const glm::mat2x4& value) const noexcept;
    void GLUniformMatrix4x2f(GLuint loc, const glm::mat4x2& value) const noexcept;
    void GLUniformMatrix3x4f(GLuint loc, const glm::mat3x4& value) const noexcept;
    void GLUniformMatrix4x3f(GLuint loc, const glm::mat4x3& value) const noexcept;



    void GLUniform1f(const GLUFProgramPtr& prog, const std::string& name, const GLfloat& value) const;
    void GLUniform2f(const GLUFProgramPtr& prog, const std::string& name, const glm::vec2& value) const;
    void GLUniform3f(const GLUFProgramPtr& prog, const std::string& name, const glm::vec3& value) const;
    void GLUniform4f(const GLUFProgramPtr& prog, const std::string& name, const glm::vec4& value) const;
    void GLUniform1i(const GLUFProgramPtr& prog, const std::string& name, const GLint& value) const;
    void GLUniform2i(const GLUFProgramPtr& prog, const std::string& name, const glm::i32vec2& value) const;
    void GLUniform3i(const GLUFProgramPtr& prog, const std::string& name, const glm::i32vec3& value) const;
    void GLUniform4i(const GLUFProgramPtr& prog, const std::string& name, const glm::i32vec4& value) const;
    void GLUniform1ui(const GLUFProgramPtr& prog, const std::string& name, const GLuint& value) const;
    void GLUniform2ui(const GLUFProgramPtr& prog, const std::string& name, const glm::u32vec2& value) const;
    void GLUniform3ui(const GLUFProgramPtr& prog, const std::string& name, const glm::u32vec3& value) const;
    void GLUniform4ui(const GLUFProgramPtr& prog, const std::string& name, const glm::u32vec4& value) const;

    void GLUniformMatrix2f(const GLUFProgramPtr& prog, const std::string& name, const glm::mat2& value) const;
    void GLUniformMatrix3f(const GLUFProgramPtr& prog, const std::string& name, const glm::mat3& value) const;
    void GLUniformMatrix4f(const GLUFProgramPtr& prog, const std::string& name, const glm::mat4& value) const;
    void GLUniformMatrix2x3f(const GLUFProgramPtr& prog, const std::string& name, const glm::mat2x3& value) const;
    void GLUniformMatrix3x2f(const GLUFProgramPtr& prog, const std::string& name, const glm::mat3x2& value) const;
    void GLUniformMatrix2x4f(const GLUFProgramPtr& prog, const std::string& name, const glm::mat2x4& value) const;
    void GLUniformMatrix4x2f(const GLUFProgramPtr& prog, const std::string& name, const glm::mat4x2& value) const;
    void GLUniformMatrix3x4f(const GLUFProgramPtr& prog, const std::string& name, const glm::mat3x4& value) const;
    void GLUniformMatrix4x3f(const GLUFProgramPtr& prog, const std::string& name, const glm::mat4x3& value) const;

    /*
    GLProgramUniform*
    
        Note:
            this is only useful when using PPO's; if using regular programs, just use 'GLUniform*'
            the stage of the program which is being modified is set by 'GLActiveShaderProgram'

        Parameters:
            'ppo': ppo set the uniform
            'value': the data to set as the uniform
            'loc': the location of the uniform
            'name': the name of the uniform

        Throws:
            'std::invalid_argument': if 'ppo' == nullptr
            'std::invalid_argument': if 'loc' or 'name' does not exist in active program
            'NoActiveProgramUniformException': if 'GLActiveShaderProgram' was not called first
    
    */

    void GLProgramUniform1f(const GLUFSepProgramPtr& ppo, GLuint loc, const GLfloat& value) const;
    void GLProgramUniform2f(const GLUFSepProgramPtr& ppo, GLuint loc, const glm::vec2& value) const;
    void GLProgramUniform3f(const GLUFSepProgramPtr& ppo, GLuint loc, const glm::vec3& value) const;
    void GLProgramUniform4f(const GLUFSepProgramPtr& ppo, GLuint loc, const glm::vec4& value) const;
    void GLProgramUniform1i(const GLUFSepProgramPtr& ppo, GLuint loc, const GLint& value) const;
    void GLProgramUniform2i(const GLUFSepProgramPtr& ppo, GLuint loc, const glm::i32vec2& value) const;
    void GLProgramUniform3i(const GLUFSepProgramPtr& ppo, GLuint loc, const glm::i32vec3& value) const;
    void GLProgramUniform4i(const GLUFSepProgramPtr& ppo, GLuint loc, const glm::i32vec4& value) const;
    void GLProgramUniform1ui(const GLUFSepProgramPtr& ppo, GLuint loc, const GLuint& value) const;
    void GLProgramUniform2ui(const GLUFSepProgramPtr& ppo, GLuint loc, const glm::u32vec2& value) const;
    void GLProgramUniform3ui(const GLUFSepProgramPtr& ppo, GLuint loc, const glm::u32vec3& value) const;
    void GLProgramUniform4ui(const GLUFSepProgramPtr& ppo, GLuint loc, const glm::u32vec4& value) const;

    void GLProgramUniformMatrix2f(const GLUFSepProgramPtr& ppo, GLuint loc, const glm::mat2& value) const;
    void GLProgramUniformMatrix3f(const GLUFSepProgramPtr& ppo, GLuint loc, const glm::mat3& value) const;
    void GLProgramUniformMatrix4f(const GLUFSepProgramPtr& ppo, GLuint loc, const glm::mat4& value) const;
    void GLProgramUniformMatrix2x3f(const GLUFSepProgramPtr& ppo, GLuint loc, const glm::mat2x3& value) const;
    void GLProgramUniformMatrix3x2f(const GLUFSepProgramPtr& ppo, GLuint loc, const glm::mat3x2& value) const;
    void GLProgramUniformMatrix2x4f(const GLUFSepProgramPtr& ppo, GLuint loc, const glm::mat2x4& value) const;
    void GLProgramUniformMatrix4x2f(const GLUFSepProgramPtr& ppo, GLuint loc, const glm::mat4x2& value) const;
    void GLProgramUniformMatrix3x4f(const GLUFSepProgramPtr& ppo, GLuint loc, const glm::mat3x4& value) const;
    void GLProgramUniformMatrix4x3f(const GLUFSepProgramPtr& ppo, GLuint loc, const glm::mat4x3& value) const;



    void GLProgramUniform1f(const GLUFSepProgramPtr& ppo, const std::string& name, const GLfloat& value) const;
    void GLProgramUniform2f(const GLUFSepProgramPtr& ppo, const std::string& name, const glm::vec2& value) const;
    void GLProgramUniform3f(const GLUFSepProgramPtr& ppo, const std::string& name, const glm::vec3& value) const;
    void GLProgramUniform4f(const GLUFSepProgramPtr& ppo, const std::string& name, const glm::vec4& value) const;
    void GLProgramUniform1i(const GLUFSepProgramPtr& ppo, const std::string& name, const GLint& value) const;
    void GLProgramUniform2i(const GLUFSepProgramPtr& ppo, const std::string& name, const glm::i32vec2& value) const;
    void GLProgramUniform3i(const GLUFSepProgramPtr& ppo, const std::string& name, const glm::i32vec3& value) const;
    void GLProgramUniform4i(const GLUFSepProgramPtr& ppo, const std::string& name, const glm::i32vec4& value) const;
    void GLProgramUniform1ui(const GLUFSepProgramPtr& ppo, const std::string& name, const GLuint& value) const;
    void GLProgramUniform2ui(const GLUFSepProgramPtr& ppo, const std::string& name, const glm::u32vec2& value) const;
    void GLProgramUniform3ui(const GLUFSepProgramPtr& ppo, const std::string& name, const glm::u32vec3& value) const;
    void GLProgramUniform4ui(const GLUFSepProgramPtr& ppo, const std::string& name, const glm::u32vec4& value) const;

    void GLProgramUniformMatrix2f(const GLUFSepProgramPtr& ppo, const std::string& name, const glm::mat2& value) const;
    void GLProgramUniformMatrix3f(const GLUFSepProgramPtr& ppo, const std::string& name, const glm::mat3& value) const;
    void GLProgramUniformMatrix4f(const GLUFSepProgramPtr& ppo, const std::string& name, const glm::mat4& value) const;
    void GLProgramUniformMatrix2x3f(const GLUFSepProgramPtr& ppo, const std::string& name, const glm::mat2x3& value) const;
    void GLProgramUniformMatrix3x2f(const GLUFSepProgramPtr& ppo, const std::string& name, const glm::mat3x2& value) const;
    void GLProgramUniformMatrix2x4f(const GLUFSepProgramPtr& ppo, const std::string& name, const glm::mat2x4& value) const;
    void GLProgramUniformMatrix4x2f(const GLUFSepProgramPtr& ppo, const std::string& name, const glm::mat4x2& value) const;
    void GLProgramUniformMatrix3x4f(const GLUFSepProgramPtr& ppo, const std::string& name, const glm::mat3x4& value) const;
    void GLProgramUniformMatrix4x3f(const GLUFSepProgramPtr& ppo, const std::string& name, const glm::mat4x3& value) const;


    /*
    GLActiveShaderProgram

        Parameters:
            'ppo': the ppo object to look for the shader program in
            'stage': the program stage to bind

        Note:
            this is called before 'GLProgramUniform*'

            this tells OpenGL which program to look in to find the following uniforms

            'stage' may be part of a program with multiple stages, however this will not affect performance
            
    */

    void GLActiveShaderProgram(GLUFSepProgramPtr& ppo, GLUFShaderType stage) const;

};


/*
======================================================================================================================================================================================================

Global Shader Manager Instance and Usage Example

*/

extern GLUFShaderManager OBJGLUF_API g_ShaderManager;

#define GLUFSHADERMANAGER GLUF::g_ShaderManager

/*
Usage examples


//create the shader
GLUFShaderPtr shad = GLUFSHADERMANAGER.CreateShader("shader.glsl", ST_VERTEX_SHADER);

*/


/*
======================================================================================================================================================================================================

Buffer and Texture (alpha)

    Note:
        This does not use the manager system the shader does for flexibility and testing, however that might change in future releases

*/

/*

Texture Utilities:

    Note:
        Not much is here, just the barebones, however more is planned for the future

*/

class TextureCreationException : public GLUFException
{
public:
    virtual const char* what() const override
    {
        return "Failed to Create Texture!";
    }

    EXCEPTION_CONSTRUCTOR(TextureCreationException)
};

enum GLUFTextureFileFormat
{
	TFF_DDS = 0,//we will ONLY support dds's, because they are flexible enough, AND have mipmaps
	TTF_DDS_CUBEMAP = 1
};


/*
LoadTextureFrom*

    Parameters:
        'filePath': path to file to open
        'format': texture file format to be loaded
        'data': raw data to load texture from

    Returns:
        OpenGL texture ID of the created texture

    Throws:
        'std::ios_base::failure': if filePath could not be found, or stream failed
        'TextureCreationException': if texture creation failed

    Note:
        All textures currently must be in ABGR format
        When using loaded textures, flip the UV coords of your meshes (the included mesh loaders do that for you) due to the DDS files loading upside-down
        If formats other than ABGR are supported, ABGR will likely be faster at loading

*/
GLuint OBJGLUF_API LoadTextureFromFile(const std::wstring& filePath, GLUFTextureFileFormat format);
GLuint OBJGLUF_API LoadTextureFromMemory(const std::vector<char>& data, GLUFTextureFileFormat format);//this is broken, WHY




/*

Buffer Utilities

*/


struct GLUFMeshBarebones
{
	Vec3Array mVertices;
	IndexArray mIndices;
};



/*
GLUFVertexAttribInfo

    Member Data:
        'mBytesPerElement': the number of bytes per element of the vertex
        'mElementsPerValue': the number of elements per vector value
        'mVertexAttribLocation': the location of the vertex attribute
        'mType': primitive type of the data
*/

struct OBJGLUF_API GLUFVertexAttribInfo
{
	unsigned short mBytesPerElement;//int would be 4
	unsigned short mElementsPerValue;//vec4 would be 4
	GLUFAttribLoc  mVertexAttribLocation;
	GLenum         mType;//float would be GL_FLOAT
    GLuint         mOffset;//this is always 0; it is used internally in 'GLUFVertexArrayAoS'
};

/*

Vertex Array Exceptions


*/

class MakeVOAException : public GLUFException
{
public:
    virtual const char* what() const override
    {
        return "VAO Creation Failed!";
    }

    EXCEPTION_CONSTRUCTOR(MakeVOAException)
};

class InvalidSoABufferLenException : public GLUFException
{
public:
    virtual const char* what() const override
    {
        return "Buffer Passed Has Length Inconsistent With the Vertex Attributes!";
    }

    EXCEPTION_CONSTRUCTOR(InvalidSoABufferLenException)
};

class MakeBufferException : public GLUFException
{
public:
    virtual const char* what() const override
    {
        return "Buffer Creation Failed!";
    }

    EXCEPTION_CONSTRUCTOR(MakeBufferException)
};

class InvalidAttrubuteLocationException : public GLUFException
{
public:
    virtual const char* what() const override
    {
        return "Attribute Location Not Found in This Buffer!";
    }

    EXCEPTION_CONSTRUCTOR(InvalidAttrubuteLocationException)
};

/*
GLUFVertexArrayBase

    Member Data:
        'mVertexArrayId': The OpenGL assigned id for this vertex array
        'mVertexCount': the number of vertices in the array
        'mUsageType': the OpenGL Usage type (i.e. GL_READ_WRITE)
        'mPrimitiveType': the OpenGL primitive type (i.e. GL_TRIANGLES)
        'mAttribInfos': a map of attribute locations
        'mIndexBuffer': the location of the single index array
        'mRangedIndexBuffer': the location of a dynamic buffer holding a range of indices
        'mIndexCount': the number of indices (number of faces * number of vertices per primitive)
        'mTempVAOId': the temperary id of the VAO; saved before binding this VAO

*/

class OBJGLUF_API GLUFVertexArrayBase
{
protected:
	GLuint mVertexArrayId = 0;
	GLuint mVertexCount = 0;
	

	GLenum mUsageType;
	GLenum mPrimitiveType;
	std::map<GLUFAttribLoc, GLUFVertexAttribInfo> mAttribInfos;

	GLuint mIndexBuffer = 0;
	GLuint mRangedIndexBuffer = 0;
	GLuint mIndexCount  = 0;

    GLuint mTempVAOId = 0;

    /*
    
    Internal Methods:
        
        RefreshDataBufferAttribute:
            -reassign the OpenGL buffer attributes to VAO
        
        GetAttribInfoFromLoc:
            -simple map lookup for location
            -throws 'std::invalid_argument' if loc does not exist

        BufferIndicesBase:
            -similer code for each of the 'BufferIndices' functions
    
    */
	virtual void RefreshDataBufferAttribute() = 0 noexcept;
	const GLUFVertexAttribInfo& GetAttribInfoFromLoc(GLUFAttribLoc loc) const;
    void BufferIndicesBase(GLuint indexCount, const GLvoid* data) noexcept;


    //disallow copy constructor and assignment operator
	GLUFVertexArrayBase(const GLUFVertexArrayBase& other) = delete;
    GLUFVertexArrayBase& operator=(const GLUFVertexArrayBase& other) = delete;
public:
    /*
    
    Constructor:
        
        Parameters:
            'index': does this VAO use indices, or just vertices
            'primType': which primative type does it use
            'buffUsage': which buffer useage type

        Throws:
            'MakeVAOException': if construction fails
            'MakeBufferException': if index buffer fails to be created (This will not be thrown if 'index' = false)
    */
    
    GLUFVertexArrayBase(GLenum primType = GL_TRIANGLES, GLenum buffUsage = GL_STATIC_DRAW, bool index = true);
	
    
    ~GLUFVertexArrayBase();

    /*
    
    Move Copy Constructor and Move Assignment Operator
        
        Throws:
            May Throw something in Map Copy Constructor
    */

    GLUFVertexArrayBase(GLUFVertexArrayBase&& other);
    GLUFVertexArrayBase& operator=(GLUFVertexArrayBase&& other);

    /*
    Add/RemoveVertexAttrib

        Parameters:
            'info': data structure containing the attribute information
            'loc': OpenGL location of the attribute to remove

        Throws:
            'std::invalid_argument': if loc does not exist or if 'mBytesPerElement == 0' of if 'mElementsPerValue == 0'
    
    */
	//this would be used to add color, or normals, or texcoords, or even positions.  NOTE: this also deletes ALL DATA in this buffer
	virtual void AddVertexAttrib(const GLUFVertexAttribInfo& info);
	virtual void RemoveVertexAttrib(GLUFAttribLoc loc);

    /*
    BindVertexArray

        Binds this vertex array for modifications.

        Throws:
            no-throw guarantee

        Preconditions:
            Unkown VAO is bound

        Postconditions
            This VAO is bound
    */
    void BindVertexArray() noexcept;

    /*
    UnBindVertexArray

        Binds old vertex array back; 

        Note:
            Undefined results if this is called before 'BindVertexArray'

        Throws:
            no-throw guarantee

        Preconditions:
            This VAO is bound

        Postconditions
            Old Unknown VAO is bound
    */
    void UnBindVertexArray() noexcept;


    /*
    Draw*

        -These functions wrap the whole drawing process into one function call;
        -BindVertexArray is automatically called

        Parameters:
            'start': starting index of ranged draw
            'count': number of indices relative to 'start' to draw
            'instances': number of times to draw the object in instanced draw

        Throws:
            no-throw guarantee

        Draw:
            Draws the vertex array with the currently bound shader
        
        DrawRange:
            Draws the range of indices from 'start' to 'start' + 'count'

        DrawInstanced:
            Draws 'instances' number of the object.  NOTE: this will draw the object EXACTLY the same, so 
                this is not very useful without a special instanced shader program
    */
    void Draw() noexcept;
    void DrawRange(GLuint start, GLuint count) noexcept;
    void DrawInstanced(GLuint instances) noexcept;


    /*
    BufferIndices
        
        Parameters:
            'indices': array of indices to be buffered

        Throws:
            no-throw guarantee

        Note:
            this could be a template, but is intentionally not to ensure the user
                is entering reasonible data to give predictible results
    */

	void BufferIndices(const std::vector<GLuint>& indices) noexcept;
    void BufferIndices(const std::vector<glm::u32vec2>& indices) noexcept;
    void BufferIndices(const std::vector<glm::u32vec3>& indices) noexcept;
    void BufferIndices(const std::vector<glm::u32vec4>& indices) noexcept;
	//void BufferFaces(GLuint* indices, GLuint FaceCount);

    /*
    Enable/DisableVertexAttributes

        -Iterates through all vertex attributes and enables them.
        -This is mostly for internal use, but is exposed as an interface to allow
            for flexibility

        Throws:
            no-throw guarantee

        Preconditions:
            VAO is currently bound

        Note:
            Calling these functions before calling 'BindVertexArray' is not harmful within
                this API, however it will be harmful using raw OpenGL VAO objects.
            Calling these functions before calling 'BindVertexArray' will not be useful
    
    */

    virtual void EnableVertexAttributes() const noexcept = 0;
    virtual void DisableVertexAttributes() const noexcept = 0;
};



/*
GLUFVertexStruct

    Base struct for data used in 'GLUFVertexArrayAoS'

*/

struct GLUFVertexStruct
{
    virtual void* operator&() const = 0;
    virtual size_t size() const = 0;
    virtual size_t n_elem_size(size_t element) = 0;
    virtual void buffer_element(void* data, size_t element) = 0;
};

/*

GLUFGLVector
    
    -a small excention to the std::vector class;
    -use this just like you would std::vector, except T MUST be derived from 'GLUFVertexStruct'
    -T::size() must be the same for every element of the vector, however it will only throw an exception when calling gl_data()

    Data Members:
        'mGLData': the cached data from 'gl_data()', gets destroyed when vector does
*/

template<typename T>
class GLUFGLVector : public std::vector<T>
{
    mutable char* mGLData = nullptr;
public:

    /*
    Default Constructor
    */
    GLUFGLVector(){}

    /*

    Move Copy Constructor and Move Assignment Operator

        Throws:
            May throw something in 'std::vector's move constructor or assignment operator
    */

    GLUFGLVector(GLUFGLVector&& other) : std::vector<T>(std::move(other));
    GLUFGLVector& operator=(GLUFGLVector&& other);

    /*
    Copy Constructor and Assignment Operator
        
        Throws:
            May throw something in 'std::vector's copy constructor or assignment operator
    */
    GLUFGLVector(const GLUFGLVector& other) : std::vector<T>(other), mGLData(0) {}
    GLUFGLVector& operator=(const GLUFGLVector& other);

    /*
    gl_data

        Returns:
            contiguous, raw data of each element in the vector

        Throws:
            'std::length_error': if any two elements in the vector are not the same length

    */
    void* gl_data() const;

    /*
    gl_delete_data

        deletes 'mGLData'

        Returns:
            always nullptr; usage: "rawData = vec.gl_delete_data();"

        Throws:
            no-throw guarantee
    */
    void* gl_delete_data() const;

    /*
    buffer_element

        Parameters:
            'data': raw data to buffer
            'element': which element in the Vertex is it modifying 

        Throws:
            undefined
    
    */
    void buffer_element(void* data, size_t element);
};

/*

GLUFVertexArrayAoS:
    
    An Array of Structures approach to OpenGL buffers

    Note:
        All data structures must be on 4 byte bounderies

    Data Members:
        'mDataBuffer': the single buffer that holds the array

*/

class OBJGLUF_API GLUFVertexArrayAoS : public GLUFVertexArrayBase
{


    /*
    Add/RemoveVertexAttrib
        
        "AddVertexAttrib" is deleted on GLUFVertexArrayAoS, because this needs an offset parameter for adding attributes

        "RemoveVertexAttrib" is disabled, because removing an attribute will mess up the offset and strides of the buffer

    */
    //this would be used to add color, or normals, or texcoords, or even positions.  NOTE: this also deletes ALL DATA in this buffer
    virtual void AddVertexAttrib(const GLUFVertexAttribInfo& info) sealed {}
    virtual void RemoveVertexAttrib(GLUFAttribLoc loc) sealed {}

protected:
	GLuint mDataBuffer = 0;




    //see 'GLUFVertexArrayBase' Docs
	virtual void RefreshDataBufferAttribute();

public:

    /*
    
    Constructor:
        
        Parameters:
            'index': does this VAO use indices, or just vertices
            'primType': which primative type does it use
            'buffUsage': which buffer useage type

        Throws:
            'MakeVAOException': if construction fails         
            'MakeBufferException': if buffer creation specifically fails
    */
	GLUFVertexArrayAoS(GLenum primType = GL_TRIANGLES, GLenum buffUsage = GL_STATIC_DRAW, bool indexed = true);

    ~GLUFVertexArrayAoS();


    /*

    Move Copy Constructor and Move Assignment Operator

        Throws:
            May Throw something in Map Copy Constructor

    */

    GLUFVertexArrayAoS(GLUFVertexArrayAoS&& other);
    GLUFVertexArrayAoS& operator=(GLUFVertexArrayAoS&& other);


    /*
    GetVertexSize

        Returns:
            Size of each vertex, including possible 4 byte padding for each element in the array
    
        Throws:
            no-throw guarantee
    */
	GLuint GetVertexSize() const noexcept;



    /*
    Add/RemoveVertexAttrib

        Parameters:
            'info': data structure containing the attribute information
            'offset': offset within data buffer of each element within the vertex

        Throws:
            'std::invalid_argument': if 'mBytesPerElement == 0' or if 'mElementsPerValue == 0'

    */
    //this would be used to add color, or normals, or texcoords, or even positions.  NOTE: this also deletes ALL DATA in this buffer
    virtual void AddVertexAttrib(const GLUFVertexAttribInfo& info, GLuint offset);


    /*
    BufferData

        -To add whole vertex arrays to the VAO. Truncates old data

        Parameters:
            'data': the array of structures 
            'T': data structure derived from 'GLUFVertexStruct'

        Throws:
            'std::invalid_argument' if derived 'GLUFVertexStruct' is the wrong size or is not derived from 'GLUFVertexStruct', but this does NOT enforce the layout
    
    */
    template<typename T>
    void BufferData(const GLUFGLVector<T>& data);


    /*
    BufferSubData

        -To modify vertices

        Parameters:
            'vertexLocations': overwrite these vertices (if size == 1, then that will be the first vertex, then will overwrite sequentially)
            'data': List of Data which derives from GLUFAoSStruct
            'isSorted': set this to 'true' if the vertex locations are in order (not necessessarily contiguous though)
            
        Template Parameters:
            'T': vertex type, derived from GLUFVertexStruct, which CANNOT contain any pointers, it must only contain primative types, or structures of primative types

        Throws:
            'std::invalid_argument' if derived 'GLUFVertexStruct' is the wrong size, but this does NOT enforce the layout
            'std::invalid_argument' if size of data array is not the size of the vertex locations, and the size of the vertex locations is not equal to 1
            'std::invalid_argument' if 'T' is not the correct base
            'std::invalid_argument' if 'vertexLocations' contains any duplicates

        Note:
            As efficient as possible, but still not fast.  If this is needed frequently, consider using 'GLUFVertexArraySoA'

            ****TODO: Possibly one that accepts an array of vertex locations along with chunk sizes****
    */
    template<typename T>
    void BufferSubData(GLUFGLVector<T> data, std::vector<GLuint> vertexLocations, bool isSorted = false);

    /*
    Enable/DisableVertexAttrib

        See 'GLUFVertexArrayBase' for doc's
    
    */
    virtual void EnableVertexAttributes() const noexcept override;
    virtual void DisableVertexAttributes() const noexcept override;

};


/*
GLUFVertexArraySoA

    Structure of Arrays approach to OpenGL Buffers

    Data Members:
        'mDataBuffers': one OpenGL buffer per vertex attribute

*/
class OBJGLUF_API GLUFVertexArraySoA : public GLUFVertexArrayBase
{
protected:
	//       Attrib Location, buffer location
	std::map<GLUFAttribLoc, GLuint> mDataBuffers;

    //see parent docs
	virtual void RefreshDataBufferAttribute();

    /*
    GetBufferIdFromAttribLoc

        Parameters:
            'loc': location of the attribute in the shader

        Returns:
            buffer Id which contains data to put in this attrubute

        Throws:
            'InvalidAttrubuteLocationException': if no attrubte has location 'loc'
    */
	GLuint GetBufferIdFromAttribLoc(GLUFAttribLoc loc) const;

public:
    /*

    Constructor:

        Parameters:
            'index': does this VAO use indices, or just vertices
            'primType': which primative type does it use
            'buffUsage': which buffer useage type

        Throws:
            'MakeVAOException': if construction fails

    */
	GLUFVertexArraySoA(GLenum primType = GL_TRIANGLES, GLenum buffUsage = GL_STATIC_DRAW, bool indexed = true);

    ~GLUFVertexArraySoA();

    /*

    Move Copy Constructor and Move Assignment Operator

        Throws:
            May Throw something in Map Copy Constructor

    */

    GLUFVertexArraySoA(GLUFVertexArraySoA&& other);
    GLUFVertexArraySoA& operator=(GLUFVertexArraySoA&& other);


    /*
    GetBarebonesMesh

        Parameters:
            'inData': structure to be written to with OpenGL Buffer Data; all data already inside will be deleted

        Throws:
            'InvalidAttrubuteLocationException': if this SoA does not have a position or index buffer
    
    */

	void GetBarebonesMesh(GLUFMeshBarebones& inData);

    /*
    BufferData
        
        Parameters:
            'loc': the attribute location for the buffer
            'data': the data to buffer

        Throws:
            'InvalidSoABufferLenException': if data.size() != mVertexCount
            'InvalidAttrubuteLocationException': if loc does not exist in this buffer
    */
    template<typename T>
    void BufferData(GLUFAttribLoc loc, const std::vector<T>& data);

    /*
    BufferSubData

        Parameters:
            'loc': the attribute location for the buffer
            'vertexOffsetCount': number of vertices to offset by when writing
            'data': the data to buffer

        Throws:
            'InvalidAttrubuteLocationException': if loc does not exist in this buffer
    */
    template<typename T>
    void BufferSubData(GLUFAttribLoc loc, GLuint vertexOffsetCount, const std::vector<T>& data);

	//this would be used to add color, or normals, or texcoords, or even positions.  NOTE: this also deletes ALL DATA in this buffer

    /*
    Add/RemoveVertexAttrib

        Parameters:
            'info': information for the vertex attribute to add
            'loc': location of the vertex attribute to remove

        Throws:
            no-throw guarantee

        Note:
            if 'info.mVertexAttribLocation' already exists, the new
                attribute simply is not added
            if 'loc' does not exist, nothing is deleted
    */
	virtual void AddVertexAttrib(const GLUFVertexAttribInfo& info) noexcept;
	virtual void RemoveVertexAttrib(GLUFAttribLoc loc) noexcept;


    /*
    Enable/DisableVertexAttrib

    See 'GLUFVertexArrayBase' for doc's

    */
    virtual void EnableVertexAttributes() const noexcept override;
    virtual void DisableVertexAttributes() const noexcept override;
};


/*

Vertex Array Object Aliases

*/

using GLUFVertexArraySoAPtr = std::shared_ptr<GLUFVertexArraySoA>;
using GLUFVertexArrayAoSPtr = std::shared_ptr<GLUFVertexArrayAoS>;
using GLUFVertexArray       = GLUFVertexArrayAoS;
using GLUFVertexArrayPtr    = std::shared_ptr<GLUFVertexArray>;


/*
=======================================================================================================================================================================================================
Utility Functions if Assimp is being used

    Note:
        this part of the library does use nakid pointers to match up with Assimp usage examples

*/

#ifdef USING_ASSIMP

/*

Data Members for Assimp Loading

GLUFVertexAttribMap:
Data structure of all of the vertex attributes to load from the assimp scene

GLUFVertexAttribPair:
Pair of a vertex attribute, and the attribute info to go with it

*/
using GLUFVertexAttribMap = std::map<unsigned char, GLUFVertexAttribInfo>;
using GLUFVertexAttribPair = std::pair<unsigned char, GLUFVertexAttribInfo>;


/*

Assimp Loading Exceptions

*/


/*
LoadVertexArrayFromScene

    Parameters:
        'scene': assimp 'aiScene': to load from
        'meshNum': which mesh number to load from the scene
        'inputs': which vertex attributes to load

    Returns:
        shared pointer to the loaded vertex array

    Throws:
        'std::invalid_argument': if 'meshNum' is higher than the number of meshes in 'scene'

        TOOD: SEE ASSIMP DOCUMENTATION TO SEE WHAT IT THROWS PLUS WHAT THIS MIGHT THROW

*/
std::shared_ptr<GLUFVertexArray>				OBJGLUF_API LoadVertexArrayFromScene(const aiScene* scene, GLuint meshNum = 0);
std::shared_ptr<GLUFVertexArray>				OBJGLUF_API LoadVertexArrayFromScene(const aiScene* scene, const GLUFVertexAttribMap& inputs, GLuint meshNum = 0);



/*
LoadVertexArraysFromScene
    
    Parameters:
        'scene': assimp 'aiScene': to load from
        'meshOffset': how many meshes in to start
        'numMeshes': how many meshes to load, starting at 'meshOffset'
        'inputs': which vertex attributes to load from each mesh; if inputs.size() == 1, 
            then that set of attributes is used for each mesh

    Returns:
        array of shared pointers to the loaded vertex arrays

    Throws:
        TOOD: SEE ASSIMP DOCUMENTATION TO SEE WHAT IT THROWS PLUS WHAT THIS MIGHT THROW
*/
std::vector<std::shared_ptr<GLUFVertexArray>>	OBJGLUF_API LoadVertexArraysFromScene(const aiScene* scene, GLuint meshOffset = 0, GLuint numMeshes = 1);
std::vector<std::shared_ptr<GLUFVertexArray>>	OBJGLUF_API LoadVertexArraysFromScene(const aiScene* scene, const std::vector<const GLUFVertexAttribMap&>& inputs, GLuint meshOffset = 0, GLuint numMeshes = 1);


//the unsigned char represents the below #defines (GLUF_VERTEX_ATTRIB_*)
#endif


/*
=======================================================================================================================================================================================================
A Set of Guidelines for Attribute Locations (organized by number of vector elements)

*/
//2 elements

#define GLUF_VERTEX_ATTRIB_UV0			1
#define GLUF_VERTEX_ATTRIB_UV1			2
#define GLUF_VERTEX_ATTRIB_UV2			3
#define GLUF_VERTEX_ATTRIB_UV3			4
#define GLUF_VERTEX_ATTRIB_UV4			5
#define GLUF_VERTEX_ATTRIB_UV5			6
#define GLUF_VERTEX_ATTRIB_UV6			7
#define GLUF_VERTEX_ATTRIB_UV7			8
//3 elements

#define GLUF_VERTEX_ATTRIB_POSITION		9
#define GLUF_VERTEX_ATTRIB_NORMAL		10
#define GLUF_VERTEX_ATTRIB_TAN			11
#define GLUF_VERTEX_ATTRIB_BITAN		12
//4 elements

#define GLUF_VERTEX_ATTRIB_COLOR0		13
#define GLUF_VERTEX_ATTRIB_COLOR1		14
#define GLUF_VERTEX_ATTRIB_COLOR2		15
#define GLUF_VERTEX_ATTRIB_COLOR3		16
#define GLUF_VERTEX_ATTRIB_COLOR4		17
#define GLUF_VERTEX_ATTRIB_COLOR5		18
#define GLUF_VERTEX_ATTRIB_COLOR6		19
#define GLUF_VERTEX_ATTRIB_COLOR7		20


/*
=======================================================================================================================================================================================================
Premade Attribute Info's which comply with Assimp capibilities, but are not exclusive to them

*/
extern const GLUFVertexAttribInfo OBJGLUF_API g_attribPOS;
extern const GLUFVertexAttribInfo OBJGLUF_API g_attribNORM;
extern const GLUFVertexAttribInfo OBJGLUF_API g_attribUV0;
extern const GLUFVertexAttribInfo OBJGLUF_API g_attribUV1;
extern const GLUFVertexAttribInfo OBJGLUF_API g_attribUV2;
extern const GLUFVertexAttribInfo OBJGLUF_API g_attribUV3;
extern const GLUFVertexAttribInfo OBJGLUF_API g_attribUV4;
extern const GLUFVertexAttribInfo OBJGLUF_API g_attribUV5;
extern const GLUFVertexAttribInfo OBJGLUF_API g_attribUV6;
extern const GLUFVertexAttribInfo OBJGLUF_API g_attribUV7;
extern const GLUFVertexAttribInfo OBJGLUF_API g_attribCOLOR0;
extern const GLUFVertexAttribInfo OBJGLUF_API g_attribCOLOR1;
extern const GLUFVertexAttribInfo OBJGLUF_API g_attribCOLOR2;
extern const GLUFVertexAttribInfo OBJGLUF_API g_attribCOLOR3;
extern const GLUFVertexAttribInfo OBJGLUF_API g_attribCOLOR4;
extern const GLUFVertexAttribInfo OBJGLUF_API g_attribCOLOR5;
extern const GLUFVertexAttribInfo OBJGLUF_API g_attribCOLOR6;
extern const GLUFVertexAttribInfo OBJGLUF_API g_attribCOLOR7;
extern const GLUFVertexAttribInfo OBJGLUF_API g_attribTAN;
extern const GLUFVertexAttribInfo OBJGLUF_API g_attribBITAN;

extern std::map<unsigned char, GLUFVertexAttribInfo> OBJGLUF_API g_stdAttrib;

#define GLUFVertAttrib(location, bytes, count, type) {bytes, count, location, type}

}

/*
=======================================================================================================================================================================================================
Global methods for converting assimp and glm data types

*/

#ifdef USING_ASSIMP
inline glm::vec2 AssimpToGlm(aiVector2D v)
{
	return glm::vec2(v.x, v.y);
}
inline glm::vec2 AssimpToGlm3_2(aiVector3D v)
{
	return glm::vec2(v.x, v.y);
}
inline glm::vec2* AssimpToGlm(aiVector2D* v, GLuint count)
{
	glm::vec2* ret = new glm::vec2[count];
	for (GLuint i = 0; i < count; ++i)
		ret[i] = AssimpToGlm(v[i]);
	return ret;
}
inline glm::vec2* AssimpToGlm3_2(aiVector3D* v, GLuint count)
{
	glm::vec2* ret = new glm::vec2[count];
	for (GLuint i = 0; i < count; ++i)
		ret[i] = AssimpToGlm3_2(v[i]);
	return ret;
}



inline glm::vec3 AssimpToGlm(aiVector3D v)
{
	return glm::vec3(v.x, v.y, v.z);
}
inline glm::vec3* AssimpToGlm(aiVector3D* v, GLuint count)
{
	glm::vec3* ret = new glm::vec3[count];
	for (GLuint i = 0; i < count; ++i)
		ret[i] = AssimpToGlm(v[i]);
	return ret;
}
inline glm::vec3 AssimpToGlm(aiColor3D v)
{
	return glm::vec3(v.r, v.g, v.b);
}

inline glm::vec4 AssimpToGlm(aiColor4D v)
{
	return glm::vec4(v.r, v.g, v.b, v.a);
}

inline GLUF::Color AssimpToGlm4_3u8(aiColor4D col)
{
	return GLUF::Color(col.r * 255, col.g * 255, col.b * 255, col.a * 255);
}

#endif

//template implementations
#include "ObjGLUFTemplates.inl"
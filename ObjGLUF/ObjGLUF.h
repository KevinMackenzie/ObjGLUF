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

extern GLuint OBJGLUF_API g_GLVersionMajor;
extern GLuint OBJGLUF_API g_GLVersionMinor;


/*
======================================================================================================================================================================================================
Debugging Macros and Setup Functions

*/

using GLUFErrorMethod = void(*)(const std::string& message, const char* funcName, const char* sourceFile, unsigned int lineNum);

#define GLUF_ERROR(message) GLUFGetErrorMethod()(message, __FUNCTION__, __FILE__, __LINE__);
#define GLUF_ERROR_LONG(chain) {std::stringstream ss; ss << #chain;  GLUFGetErrorMethod()(ss.str(), __FUNCTION__, __FILE__, __LINE__);}
#define GLUF_ASSERT(expr)	{ if (!(expr)) { std::stringstream ss; ss << "ASSERTION FAILURE:" << #expr; GLUF_ERROR(ss.str().c_str()) } }

OBJGLUF_API void GLUFRegisterErrorMethod(GLUFErrorMethod method);
OBJGLUF_API GLUFErrorMethod GLUFGetErrorMethod();


/*
======================================================================================================================================================================================================
Utility Macros

*/

#define GLUF_SAFE_DELETE(ptr) {}
#define GLUF_SAFE_DELETE(ptr) {if(ptr){delete(ptr);} (ptr) = nullptr;}
#define GLUF_NULL(type) (std::shared_ptr<type>(nullptr))
#define GLUF_UNREFERENCED_PARAMETER(value) (value)


/*
======================================================================================================================================================================================================
Multithreading Macros

*/

//for use with mutexes
#define GLUF_TSAFE_BEGIN(Mutex) { std::unique_lock<std::mutex> __lock__(Mutex);
#define GLUF_TSAFE_END __lock__.unlock();}
#define GLUF_TSAFE_REGION(Mutex) std::unique_lock<mutex> __lock__(Mutex);

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
    //TODO:
};

OBJGLUF_API void GLUFStats_func();
OBJGLUF_API const std::wstring& GLUFGetFrameStatsString();
OBJGLUF_API const GLUFStatsData& GLUFGetFrameStats();//WIP
OBJGLUF_API const std::wstring& GLUFGetDeviceStats();

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
#define GLUFGetTimeMs() ((int)(glfwGetTime() * 1000.0))


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

#define DEG_TO_RAD(value) ((value) *(GLUF_PI / 180))
#define DEG_TO_RAD_F(value) ((value) *(GLUF_PI_F / 180))
#define DEG_TO_RAD_LD(value) ((value) *(GLUF_PI_LD / 180))

#define RAD_TO_DEG(value) ((value) *(180 / GLUFPI))
#define RAD_TO_DEG_F(value) ((value) *(180 / GLUFPI_F))
#define RAD_TO_DEG_LD(value) ((value) *(180 / GLUFPI_LD))


/*
======================================================================================================================================================================================================
GLUF API Core Controller Methods

*/

//call this first
OBJGLUF_API bool GLUFInit();

//call this after calling glfwMakeContextCurrent on the window
OBJGLUF_API bool GLUFInitOpenGLExtentions();

//call this at the very last moment before application termination
OBJGLUF_API void GLUFTerminate();


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

OBJGLUF_API bool		GLUFPtInRect(GLUFRect rect, GLUFPoint pt);
OBJGLUF_API void		GLUFSetRectEmpty(GLUFRect& rect);
OBJGLUF_API void		GLUFSetRect(GLUFRect& rect, long left, long top, long right, long bottom);
OBJGLUF_API void		GLUFSetRect(GLUFRectf& rect, float left, float top, float right, float bottom);
OBJGLUF_API void		GLUFOffsetRect(GLUFRect& rect, long x, long y);
OBJGLUF_API long		GLUFRectHeight(GLUFRect rect);
OBJGLUF_API long		GLUFRectWidth(GLUFRect rect);
OBJGLUF_API void		GLUFInflateRect(GLUFRect& rect, long dx, long dy);
OBJGLUF_API bool		GLUFIntersectRect(GLUFRect rect0, GLUFRect rect1, GLUFRect& rectIntersect);
OBJGLUF_API GLUFPoint	GLUFMultPoints(GLUFPoint pt0, GLUFPoint pt1);

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

OBJGLUF_API inline std::vector<std::wstring> &GLUFSplitStr(const std::wstring &s, wchar_t delim, std::vector<std::wstring> &elems, bool keepDelim = false)
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

OBJGLUF_API inline std::vector<std::wstring> GLUFSplitStr(const std::wstring &s, wchar_t delim, bool keepDelim = false)
{
	std::vector<std::wstring> elems;
	GLUFSplitStr(s, delim, elems, keepDelim);
	return elems;
}

OBJGLUF_API inline std::vector<std::string> &GLUFSplitStr(const std::string &s, char delim, std::vector<std::string> &elems, bool keepDelim = false)
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

OBJGLUF_API inline std::vector<std::string> GLUFSplitStr(const std::string &s, char delim, bool keepDelim = false)
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
        'std::invalid_argument' if 'arr' == nullptr OR if 'arr' size < len

    Note:
        This is only for legacy purposes; it is not efficient to do this regularly,
            so avoid using C-Style arrays to begin with!
*/
template<typename T>
OBJGLUF_API inline std::vector<T> GLUFArrToVec(T* arr, unsigned long len)
{
    if (!T)
        throw std::invalid_argument("GLUFArrToVec: \'arr\' == nullptr");

    if (sizeof(arr) < sizeof(T) * len)
        throw std::invalid_argument("GLUFArrToVec: \'arr\' is too small");

    return std::vector<T>(arr, arr + len);
}

//used for getting vertices from rects 0,0 is bottom left
OBJGLUF_API glm::vec2 GLUFGetVec2FromRect(GLUFRect rect, bool x, bool y);
OBJGLUF_API glm::vec2 GLUFGetVec2FromRect(GLUFRectf rect, bool x, bool y);

//used for getting vertices from rects 0,0 is bottom left
OBJGLUF_API GLUFPoint GLUFGetPointFromRect(GLUFRect rect, bool x, bool y);


OBJGLUF_API Color4f GLUFColorToFloat(Color color);//takes 0-255 to 0.0f - 1.0f
OBJGLUF_API Color3f GLUFColorToFloat3(Color color);//takes 0-255 to 0.0f - 1.0f




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
using GLUFProgramPtrStagesMap   = std::map<GLbitfield, GLUFProgramPtr>;
using GLUFProgramPtrStagesPair  = std::pair<GLbitfield, GLUFProgramPtr>;
using GLUFProgramPtrMap         = std::map<GLUFShaderType, GLUFProgramPtr>;
using GLUFShaderPtrListWeak     = std::vector<GLUFShaderPtrWeak>;
using GLUFProgramPtrListWeak    = std::vector<GLUFProgramPtrWeak>;

using GLUFVariableLocMap    = std::map<std::string, GLuint>;
using GLUFVariableLocPair   = std::pair<std::string, GLuint>;

/*

Shader Exceptions

*/



/*
GLUFException

    Serves as base class for all other GLUF exceptions.  
    Override myUniqueMessage in children
*/
class GLUFException : public std::exception
{
    const std::string& mPostfix = " See Log For More Info.";
    
    //automatically add message to log file when exception thrown
    GLUFException()
    {
        std::stringstream ss;
        ss << "GLUF Exception Thrown: \"" << what() << mPostfix << "\"";
        GLUF_ERROR(ss.str());
    }

    const char* what()
    {
        return (MyUniqueMessage() + mPostfix).c_str();
    }

    
    virtual const std::string& MyUniqueMessage(){};
};

class UseProgramException : public GLUFException
{
    virtual const std::string& MyUniqueMessage() override
    {
        return "Failed to Use Program!";
    }
};

class MakeShaderException : public GLUFException
{
    virtual const std::string& MyUniqueMessage() override
    {
        return "Shading Creation Failed!";
    }
};

class MakeProgramException : public GLUFException
{
    virtual const std::string& MyUniqueMessage() override
    {
        return "Program Creation Failed!";
    }
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


    Data Members:
        'mCompileLogs': a map of logs for each compiled shader
        'mLinkLogs': a map of logs for each linked program
        'mMutex': mutual exclution protection for logs
        'mLock': single lock for protecting data interally

*/

class OBJGLUF_API GLUFShaderManager
{

	//a list of logs
	std::map<GLUFShaderPtr, GLUFShaderInfoStruct>  mCompileLogs;
	std::map<GLUFProgramPtr, GLUFShaderInfoStruct> mLinklogs;

    std::mutex mMutex;
    std::unique_lock<std::mutex> mLock;

public:

    /*
    CreateShader*
         
        Parameters:
            'filePath': path to file containing shader
            'text': text to compile
            'memory': raw memory of text file loaded
            'type': self-explanitory

        Returns:
            -Your brand spankin' new Shader

        Throws:
            -CreateShaderFromFile: 'std::ios_base::failure': if file had issues reading
            'MakeShaderException' if compilation, loading, or anything else failed
    
    */

	GLUFShaderPtr CreateShaderFromFile(const std::wstring& filePath, GLUFShaderType type);
	GLUFShaderPtr CreateShaderFromText(const std::string& text, GLUFShaderType type);
	GLUFShaderPtr CreateShaderFromMemory(const std::vector<char>& memory, GLUFShaderType type);


    /*
    CreateProgram
        
        Parameters:
            'shaders': simply a list of precompiled shaders to link
            'shaderSources': a list of shader files in strings to be compiled
            'shaderPaths': a list of shader file paths to be loaded into strings and compiled
            'separate': WIP Don't Use

        Returns:
            -Your brank spankin' new Program

        Throws:
            'std::ios_base::failure': if file loading has issues
            'MakeProgramException' if linking failed
            'MakeShaderException' if shaders need to be compiled and fail
    
    */

	GLUFProgramPtr CreateProgram(GLUFShaderPtrList shaders, bool separate = false);
	GLUFProgramPtr CreateProgram(GLUFShaderSourceList shaderSources, bool separate = false);
	GLUFProgramPtr CreateProgram(GLUFShaderPathList shaderPaths, bool separate = false);


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
    
    */

	const GLuint GetShaderVariableLocation(const GLUFProgramPtr& prog, GLUFLocationType locType, const std::string& varName) const;
    const GLUFVariableLocMap& GetShaderAttribLocations(const GLUFProgramPtr& prog) const;
    const GLUFVariableLocMap& GetShaderUniformLocations(const GLUFProgramPtr& prog) const;

    const GLUFVariableLocMap &GetShaderAttribLocations(const GLUFSepProgramPtr& prog) const;
    const GLUFVariableLocMap &GetShaderUniformLocations(const GLUFSepProgramPtr& prog) const;


    /*
    Delete*

        Parameters:
            'shader': shader to be deleted
            'program': program to be deleted

        Note:
            -This does not delete the smart pointers, it only deletes the program from an Open-GL
                perspective.  Log files and variable/attribute location id's stay intact

        Throws:
            'std::invalid_argument': if shader/program == nullptr
    
    */

	void DeleteShader(GLUFShaderPtr shader);
	void DeleteProgram(GLUFProgramPtr program);


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
    void UseProgram(const GLUFSepProgramPtr& program) const;

    //bind program id and ppo id 0
	void UseProgramNull();

    /*
    ======================================
    ========        NOTE:
    =====================================
    
        Until further notice, PPO are not supported and your application will not link
    */



    /*
    AttachPrograms

        Parameters:
            'ppo': the programmible pipeline object
            'programs': the list of programs to add and respective stages to add
            'stages': which stages to add program to
            'program': single program to add

        Throws:
            'std::invalid_argument': if ppo == nullptr
    */

    void AttachPrograms(const GLUFSepProgramPtr& ppo, const GLUFProgramPtrStagesMap& programs);
    void AttachProgram(const GLUFSepProgramPtr& ppo, GLbitfield stages, const GLUFProgramPtr& program);


    /*
    ClearPrograms

        Parameters:
            'ppo': the programmible pipeline objects
            'stages': which stages should be cleared (reset to default)

        Throws:
            'std::invalid_argument': if ppo == nullptr
    */

    void ClearPrograms(const GLUFSepProgramPtr& ppo, GLbitfield stages = GL_ALL_SHADER_BITS);


    //program pipelines are broken
    //GLUFSepProgramPtr CreateSeparateProgram(const GLUFProgramPtrStagesMap& programs);


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
    virtual const std::string& MyUniqueMessage() override
    {
        return "Failed to Create Texture!";
    }
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
};

/*

Vertex Array Exceptions


*/

class NullVAOException : public GLUFException
{
    virtual const std::string& MyUniqueMessage() override
    {
        return "Attempt To Bind Null VAO!";
    }
};

class MakeVOAException : public GLUFException
{
    virtual const std::string& MyUniqueMessage() override
    {
        return "VAO Creation Failed!";
    }
};

class InvalidSoABufferLenException : public GLUFException
{
    virtual const std::string& MyUniqueMessage() override
    {
        return "Buffer Passed Has Length Inconsistent With the Vertex Attributes!";
    }
};

class MakeBufferException : public GLUFException
{
    virtual const std::string& MyUniqueMessage() override
    {
        return "Buffer Creation Failed!";
    }
};

class InvalidAttrubuteLocationException : public GLUFException
{
    virtual const std::string& MyUniqueMessage() override
    {
        return "Attribute Location Not Found in This Buffer!";
    }
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

    Note:
        Safe to assume that if constructor does not throw 'MakeVAOException', then no other
            member functions will throw a 'NullVAOException' unless move copy constructor or 
            move assignment operator gets this passed to it
        3rd party children modify this behavior
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
            -reverse vector lookup on mAttribInfos

    
    */
	virtual void RefreshDataBufferAttribute() = 0;
	const GLUFVertexAttribInfo& GetAttribInfoFromLoc(GLUFAttribLoc loc) const;


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
    */
    
    GLUFVertexArrayBase(GLenum primType = GL_TRIANGLES, GLenum buffUsage = GL_STATIC_DRAW, bool index = true);
	
    
    ~GLUFVertexArrayBase();

    /*
    
    Move Copy Constructor and Move Assignment Operator
        
        Throws:
            TODO: what does std::map throw?
    */

    GLUFVertexArrayBase(GLUFVertexArrayBase&& other);
    GLUFVertexArrayBase& operator=(GLUFVertexArrayBase&& other);

    /*
    Add/RemoveVertexAttrib

        Parameters:
            'info': data structure containing the attribute information
            'loc': OpenGL location of the attribute to remove

        Throws:
            'std::invalid_argument': if 'info' contains jibberish or if loc does not exist
            'NullVAOException': if the VAO is failed to be created
    
    */
	//this would be used to add color, or normals, or texcoords, or even positions.  NOTE: this also deletes ALL DATA in this buffer
	virtual void AddVertexAttrib(const GLUFVertexAttribInfo& info);
	virtual void RemoveVertexAttrib(GLUFAttribLoc loc);

    /*
    BindVertexArray

        Binds this vertex array for modifications.

        Throws:
            'NullVAOException': if the VAO failed to be created

        Preconditions:
            Unkown VAO is bound

        Postconditions
            This VAO is bound
    */
	void BindVertexArray();

    /*
    UnBindVertexArray

        Binds old vertex array back

        Throws:
            'NullVAOException': if the VAO failed to be created

        Preconditions:
            This VAO is bound

        Postconditions
            Old Unknown VAO is bound
    */
    void UnBindVertexArray();


    /*
    Draw*

        -These functions wrap the whole drawing process into one function call;
        -BindVertexArray is automatically called

        Parameters:
            'start': starting index of ranged draw
            'count': number of indices relative to 'start' to draw
            'instances': number of times to draw the object in instanced draw

        Throws:
            'NullVAOException': if the VAO failed to be created

        Draw:
            Draws the vertex array with the currently bound shader
        
        DrawRange:
            Draws the range of indices from 'start' to 'start' + 'count'

        DrawInstanced:
            Draws 'instances' number of the object.  NOTE: this will draw the object EXACTLY the same, so 
                this is not very useful without a special instanced shader program
    */
	void Draw();
	void DrawRange(GLuint start, GLuint count);
	void DrawInstanced(GLuint instances);


    /*
    BufferIndices
        
        Parameters:
            'indices': array of indices to be buffered

        Throws:
            'NullVAOException': if VAO creation failed
    */

	void BufferIndices(const std::vector<GLuint>& indices);
	void BufferIndices(const std::vector<glm::u32vec2>& indices);
    void BufferIndices(const std::vector<glm::u32vec3>& indices);
    void BufferIndices(const std::vector<glm::u32vec4>& indices);
	//void BufferFaces(GLuint* indices, unsigned int FaceCount);

    /*
    Enable/DisableVertexAttributes

        -Iterates through all vertex attributes and enables them.
        -This is mostly for internal use, but is exposed as an interface to allow
            for flexibility

        Throws:
            'NullVAOException': if VAO creation failed

        Preconditions:
            VAO is currently bound

        Note:
            Calling these functions before calling 'BindVertexArray' is not harmful within
                this API, however it will be harmful using raw OpenGL VAO objects.
            Calling these functions before calling 'BindVertexArray' will not be useful
    
    */

	virtual void EnableVertexAttributes() const;
	virtual void DisableVertexAttributes() const;
};



/*
GLUFAoSStruct

    Base struct for data used in 'GLUFVertexArrayAoS'

*/

struct GLUFAoSStruct{};

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
        TODO: whatever parent may throw

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
	unsigned int GetVertexSize() const;


    /*
    BufferData

        -To add whole vertex arrays to the VAO. Truncates old data

        Parameters:
            'data': the array of structures 

        Throws:
            'NullVAOException': if VAO creation failed
            'MakeBufferException': if buffer creation failed
    
    */
	void BufferData(const std::vector<GLUFAoSStruct&>& data);


    /*
    BufferSubData

        -To modify vertices, or parts of vertices

        Parameters:
            'ValueOffsetCount': start overwriting after this many elements
            'data': List of Data which derives from GLUFAoSStruct
    */
	void BufferSubData(const std::vector<GLUFAoSStruct&>& data, unsigned int ValueOffsetCount);

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
    GetBarebonesMesh

        Returns:
            Copy of barbones mesh data retrieved from OpenGL buffer

        Throws:
            'MakeVAOException': if VAO creation failed
    
    */

	GLUFMeshBarebones& GetBarebonesMesh();

    /*
    BufferData
        
        Parameters:
            'loc': the attribute location for the buffer
            'data': the data to buffer

        Throws:
            'NullVAOException': if VAO creation failed
            'InvalidSoABufferLenException': if data.size() != mVertexCount
            'InvalidAttrubuteLocationException': if loc does not exist in this buffer
    */
    template<typename T>
    void BufferData(GLUFAttribLoc loc, const std::vector<T>& data)
    {
        if (mVertexArrayId == 0)
            throw NullVAOException();
        if (mAttribInfos.find(loc) == mAttribInfos.end())
            throw InvalidAttrubuteLocationException();

        BindVertexArray();
        glBindBuffer(GL_ARRAY_BUFFER, GetBufferIdFromAttribLoc(loc));
        
        if (mVertexCount != 0)
        {
            if (data.size() != mVertexCount)
            {
                throw InvalidSoABufferLenException();
            }
        }
        else
        {
            mVertexCount = data.size();
        }

        GLUFVertexAttribInfo info = GetAttribInfoFromLoc(loc);
        GLuint bytesPerValue = info.mBytesPerElement * info.mElementsPerValue;
        glBufferData(GL_ARRAY_BUFFER, mVertexCount * bytesPerValue, data.data(), mUsageType);
        UnBindVertexArray();
    }

    /*
    BufferSubData

        Parameters:
            'loc': the attribute location for the buffer
            'vertexOffsetCount': number of vertices to offset by when writing
            'data': the data to buffer

        Throws:
            'NullVAOException': if VAO creation failed
            'InvalidAttrubuteLocationException': if loc does not exist in this buffer
    */
    template<typename T>
    void BufferSubData(GLUFAttribLoc loc, GLuint vertexOffsetCount, const std::vector<T>& data)
    {
        if (mVertexArrayId == 0)
            throw NullVAOException();
        if (mAttribInfos.find(loc) == mAttribInfos.end())
            throw InvalidAttrubuteLocationException();

        BindVertexArray();
        glBindBuffer(GL_ARRAY_BUFFER, GetBufferIdFromAttribLoc(loc));


        GLUFVertexAttribInfo info = GetAttribInfoFromLoc(loc);
        GLuint bytesPerValue = info.mBytesPerElement * info.mElementsPerValue;
        glBufferSubData(GL_ARRAY_BUFFER, vertexOffsetCount * bytesPerValue, mVertexCount * bytesPerValue, data.data());
        UnBindVertexArray();
    }

	//this would be used to add color, or normals, or texcoords, or even positions.  NOTE: this also deletes ALL DATA in this buffer

    /*
    Add/RemoveVertexAttrib

        Parameters:
            'info': information for the vertex attribute to add
            'loc': location of the vertex attribute to remove

        Throws:
            'NullVAOException': if VAO creation failed

        Note:
            if 'info.mVertexAttribLocation' already exists, the new
                attribute simply is not added
            if 'loc' does not exist, nothing is deleted
    */
	virtual void AddVertexAttrib(const GLUFVertexAttribInfo& info);
	virtual void RemoveVertexAttrib(GLUFAttribLoc loc);

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
LoadVertexArrayFromScene

    Parameters:
        'scene': assimp 'aiScene': to load from
        'meshNum': which mesh number to load from the scene
        'inputs': which vertex attributes to load

    Returns:
        shared pointer to the loaded vertex array

    Throws:
        TOOD: SEE ASSIMP DOCUMENTATION TO SEE WHAT IT THROWS PLUS WHAT THIS MIGHT THROW

*/
std::shared_ptr<GLUFVertexArray>				OBJGLUF_API LoadVertexArrayFromScene(const aiScene* scene, unsigned int meshNum = 0);
std::shared_ptr<GLUFVertexArray>				OBJGLUF_API LoadVertexArrayFromScene(const aiScene* scene, const GLUFVertexAttribMap& inputs, unsigned int meshNum = 0);



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
std::vector<std::shared_ptr<GLUFVertexArray>>	OBJGLUF_API LoadVertexArraysFromScene(const aiScene* scene, unsigned int meshOffset = 0, unsigned int numMeshes = 1);
std::vector<std::shared_ptr<GLUFVertexArray>>	OBJGLUF_API LoadVertexArraysFromScene(const aiScene* scene, const std::vector<const GLUFVertexAttribMap&>& inputs, unsigned int meshOffset = 0 unsigned int numMeshes = 1);


//the unsigned char represents the below #defines (GLUF_VERTEX_ATTRIB_*)
#endif


/*
=======================================================================================================================================================================================================
A Set of Guidelines for Attribute Locations

*/
#define GLUF_VERTEX_ATTRIB_POSITION		0
#define GLUF_VERTEX_ATTRIB_NORMAL		1
#define GLUF_VERTEX_ATTRIB_UV0			2
#define GLUF_VERTEX_ATTRIB_COLOR0		3
#define GLUF_VERTEX_ATTRIB_TAN			4
#define GLUF_VERTEX_ATTRIB_BITAN		5

#define GLUF_VERTEX_ATTRIB_UV1			10
#define GLUF_VERTEX_ATTRIB_UV2			11
#define GLUF_VERTEX_ATTRIB_UV3			12
#define GLUF_VERTEX_ATTRIB_UV4			13
#define GLUF_VERTEX_ATTRIB_UV5			14
#define GLUF_VERTEX_ATTRIB_UV6			15
#define GLUF_VERTEX_ATTRIB_UV7			16

#define GLUF_VERTEX_ATTRIB_COLOR1		17
#define GLUF_VERTEX_ATTRIB_COLOR2		18
#define GLUF_VERTEX_ATTRIB_COLOR3		19
#define GLUF_VERTEX_ATTRIB_COLOR4		20
#define GLUF_VERTEX_ATTRIB_COLOR5		21
#define GLUF_VERTEX_ATTRIB_COLOR6		22
#define GLUF_VERTEX_ATTRIB_COLOR7		23


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
inline glm::vec2* AssimpToGlm(aiVector2D* v, unsigned int count)
{
	glm::vec2* ret = new glm::vec2[count];
	for (unsigned int i = 0; i < count; ++i)
		ret[i] = AssimpToGlm(v[i]);
	return ret;
}
inline glm::vec2* AssimpToGlm3_2(aiVector3D* v, unsigned int count)
{
	glm::vec2* ret = new glm::vec2[count];
	for (unsigned int i = 0; i < count; ++i)
		ret[i] = AssimpToGlm3_2(v[i]);
	return ret;
}



inline glm::vec3 AssimpToGlm(aiVector3D v)
{
	return glm::vec3(v.x, v.y, v.z);
}
inline glm::vec3* AssimpToGlm(aiVector3D* v, unsigned int count)
{
	glm::vec3* ret = new glm::vec3[count];
	for (unsigned int i = 0; i < count; ++i)
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
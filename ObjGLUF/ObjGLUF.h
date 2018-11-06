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
#pragma once

#include "Exports.h"

//TODO LIST:
//
//support normal mapping
//support tangent space
//support guided lighting
//support guided materials


#include <glad/glad.h>
#include <GLFW/glfw3.h>
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>

#ifdef USING_ASSIMP
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#endif

#include <algorithm>
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

using ErrorMethod = void(*)(const std::string& message, const char* funcName, const char* sourceFile, unsigned int lineNum);

#define GLUF_ERROR(message) GetErrorMethod()(message, __FUNCTION__, __FILE__, __LINE__);
#define GLUF_ERROR_LONG(chain) {std::stringstream ss; ss << chain;  GetErrorMethod()(ss.str(), __FUNCTION__, __FILE__, __LINE__);}
#define GLUF_ASSERT(expr)    { if (!(expr)) { std::stringstream ss; ss << "ASSERTION FAILURE: \"" << #expr << "\""; _ERROR(ss.str().c_str()) } }


#ifdef GLUF_DEBUG

#define GLUF_NULLPTR_CHECK(ptr) {if (ptr == nullptr){throw std::invalid_argument("Null Pointer");}}

#define GLUF_CRITICAL_EXCEPTION(exception) throw exception;
#define GLUF_NON_CRITICAL_EXCEPTION(exception) throw exception;

#else

#define NULLPTR_CHECK(ptr)

#define CRITICAL_EXCEPTION(exception) throw exception;
#define NON_CRITICAL_EXCEPTION(exception) ;

#endif

OBJGLUF_API void RegisterErrorMethod(ErrorMethod method);
OBJGLUF_API ErrorMethod GetErrorMethod();


/*
======================================================================================================================================================================================================
Utility Macros

*/

#define GLUF_SAFE_DELETE(ptr) {if(ptr){delete(ptr);} (ptr) = nullptr;}
#define GLUF_NULL(type) (std::shared_ptr<type>(nullptr))//may be deprecated
#define GLUF_UNREFERENCED_PARAMETER(value) (value)

/*
======================================================================================================================================================================================================
Multithreading Macros

*/

//for use with mutexes
#define _TSAFE_BEGIN(Mutex) { std::lock_guard<std::mutex> __lock__(Mutex);
#define _TSAFE_END __lock__.unlock();}
#define _TSAFE_SCOPE(Mutex) std::lock_guard<std::mutex> __lock__{Mutex};

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
#define _TSAFE_LOCK(Lock) Lock.lock();
#define _TSAFE_UNLOCK(Lock) Lock.unlock();
#define _TSAFE_LOCK_REGION(Lock) LocalLock __lock__(Lock);


/*
======================================================================================================================================================================================================
Statistics

*/

class StatsData
{
public:
    double mPreviousFrame = 0.0;
    double mUpdateInterval = 1.0;//time in seconds
    unsigned long long mFrameCount = 0LL;//frame count since previous update
    float mCurrFPS = 0.0f;
    std::wstring mFormattedStatsData = L"";
};

OBJGLUF_API void Stats_func();
OBJGLUF_API const std::wstring& GetFrameStatsString();
OBJGLUF_API const StatsData& GetFrameStats();
OBJGLUF_API const std::wstring& GetDeviceStatus();

#define Stats ::Stats_func


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

#define GetTime() glfwGetTime()
#define GetTimef() ((float)glfwGetTime())
#define GetTimeMs() ((unsigned int)(glfwGetTime() * 1000.0))


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
using AttribLoc = GLuint;


/*
======================================================================================================================================================================================================
Mathematical and Conversion Macros

*/

#define _PI    3.141592653589793
#define _PI_F  3.1415927f
#define _PI_LD 3.141592653589793238L

#define _E     2.718281828459045
#define _E_F   2.7182818f
#define _E_LD  2.718281828459045235L

#define DEG_TO_RAD(value) ((value) *(_PI / 180))
#define DEG_TO_RAD_F(value) ((value) *(_PI_F / 180))
#define DEG_TO_RAD_LD(value) ((value) *(_PI_LD / 180))

#define RAD_TO_DEG(value) ((value) *(180 / _PI))
#define RAD_TO_DEG_F(value) ((value) *(180 / _PI_F))
#define RAD_TO_DEG_LD(value) ((value) *(180 / _PI_LD))

#define _60HZ 0.0166666666666


/*
======================================================================================================================================================================================================
 API Core Controller Methods

*/

//call this first
OBJGLUF_API bool Init();

//call this after calling glfwMakeContextCurrent on the window
OBJGLUF_API bool InitOpenGLExtensions();

//call this at the very last moment before application termination
OBJGLUF_API void Terminate();

OBJGLUF_API const std::vector<std::string>& GetGLExtensions();

/*
======================================================================================================================================================================================================
Context Controller Methods

*/

#define EnableVSync() glfwSwapInterval(0)
#define DisableVSync() glfwSwapInterval(1)
#define SetVSyncState(value) glfwSwapInterval(value ? 1 : 0)


//NOTE: All methods below this must be called BEFORE window creation
#define SetMSAASamples(numSamples) glfwWindowHint(GLFW_SAMPLES, numSamples)//make sure to call EnableMSAA()

//NOTE: All methods below this must be called AFTER window creation
#define EnableMSAA() glEnable(GL_MULTISAMPLE)//make sure to call SetMSAASamples() before calling this


/*
======================================================================================================================================================================================================
IO and Stream Utilities

*/


/*
LoadFileIntoMemory

    Parameters:
        'path': path of file to load
        'binMemory': vector to fill with loaded memory

    Throws: 
        'std::ios_base::failure' in event of file errors

    Note:
        If 'binMemory' is not empty, the data will be overwritten
*/
OBJGLUF_API void LoadFileIntoMemory(const std::string& path, std::vector<char>& binMemory);
OBJGLUF_API void LoadFileIntoMemory(const std::string& path, std::string& memory);

/*
LoadBinaryArrayIntoString

    Parameters:
        'rawMemory': memory to be read from
        'size': size of rawMemory
        'outString': where the string will be output

    Throws:
        'std::invalid_argument': if 'rawMemory' == nullptr
        'std::ios_base::failure': if memory streaming was unsuccessful

*/
OBJGLUF_API void LoadBinaryArrayIntoString(char* rawMemory, std::size_t size, std::string& outString);
OBJGLUF_API void LoadBinaryArrayIntoString(const std::vector<char>& rawMemory, std::string& outString);


/*
======================================================================================================================================================================================================
OpenGL Basic Data Structures and Operators

Note: these only play a significant role in Gui, but are presented here as basic types and utilities which can be used independently of the Gui

*/


//Rect is supposed to be used where the origin is bottom left
struct OBJGLUF_API Rect
{
    union{ long left, x; };
    long top, right;
    union{ long bottom, y; };
};

struct OBJGLUF_API Rectf
{
    float left, top, right, bottom;
};

struct OBJGLUF_API Point
{
    union{ long x, width; };
    union{ long y, height; };

    Point(long val1, long val2) : x(val1), y(val2){}
    Point() : x(0L), y(0L){}
};

inline Point operator /(const Point& pt0, const Point& pt1)
{
    return{ pt0.x / pt1.x, pt0.y / pt1.y };
}

inline Point operator /(const Point& pt0, const long& f)
{
    return{ pt0.x / f, pt0.y / f };
}

inline Point operator -(const Point& pt0, const Point& pt1)
{
    return{ pt0.x - pt1.x, pt0.y - pt1.y };
}

inline bool operator ==(const Rect& rc0, const Rect& rc1)
{
    return
        (
        rc0.left == rc1.left && rc0.right == rc1.right &&
        rc0.top == rc1.top  && rc0.bottom == rc1.bottom
        );
}

inline bool operator !=(const Rect& rc0, const Rect& rc1)
{
    return !(rc0 == rc1);
}

inline void bFlip(bool& b)
{
    b = !b;
}

OBJGLUF_API bool        PtInRect(const Rect& rect, const Point& pt);
OBJGLUF_API void        SetRectEmpty(Rect& rect);
OBJGLUF_API void        SetRect(Rect& rect, long left, long top, long right, long bottom);
OBJGLUF_API void        SetRect(Rectf& rect, float left, float top, float right, float bottom);
OBJGLUF_API void        OffsetRect(Rect& rect, long x, long y);
OBJGLUF_API void        RepositionRect(Rect& rect, long newX, long newY);
OBJGLUF_API long        RectHeight(const Rect& rect);
OBJGLUF_API long        RectWidth(const Rect& rect);
OBJGLUF_API void        InflateRect(Rect& rect, long dx, long dy);//center stays in same spot
OBJGLUF_API void        ResizeRect(Rect& rect, long newWidth, long newHeight);//bottom left stays in same spot
OBJGLUF_API bool        IntersectRect(const Rect& rect0, const Rect& rect1, Rect& rectIntersect);
OBJGLUF_API Point        MultPoints(const Point& pt0, const Point& pt1);

/*
======================================================================================================================================================================================================
Misc.  Classes


*/



/*
MatrixStack

    Interface:
        -Use just as using std::stack, except slightly different naming convention

*/
class OBJGLUF_API MatrixStack
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




//macro for forcing an exception to show up on the log upon contruction
#define EXCEPTION_CONSTRUCTOR_BODY \
    GLUF_ERROR_LONG(" Exception Thrown: \"" << what() << "\"");
#define EXCEPTION_CONSTRUCTOR(class_name) \
class_name() \
{ \
    GLUF_ERROR_LONG(" Exception Thrown: \"" << what() << "\""); \
}

/*
Exception

    Serves as base class for all other  exceptions.
    Override MyUniqueMessage in children
*/
    class Exception : public std::exception {
    public:

        const char *what() const noexcept = 0;
    };



/*
======================================================================================================================================================================================================
String Utilities

*/

/*
SplitStr

    Parameters:
        's': string to be split
        'delim': character to look for as a split point
        'elems': vector of strings as result of split
        'keepDelim': does 'delim' get kept in string when splitting

    Returns:
        'elems'
*/

inline std::vector<std::wstring> &SplitStr(const std::wstring &s, wchar_t delim, std::vector<std::wstring> &elems, bool keepDelim = false, bool removeBlank = false)
{
    std::wstringstream ss(s);
    std::wstring item;
    while (std::getline(ss, item, delim))
    {
        if (keepDelim)//OOPS forgot this earlier
            item += delim;
        if (removeBlank && item == L"")
            continue;
        if(item[0] != '\0')
            elems.push_back(item);
    }
    return elems;
}

inline std::vector<std::wstring> SplitStr(const std::wstring &s, wchar_t delim, bool keepDelim = false, bool removeBlank = false)
{
    std::vector<std::wstring> elems;
    SplitStr(s, delim, elems, keepDelim, removeBlank);
    return elems;
}

inline std::vector<std::string> &SplitStr(const std::string &s, char delim, std::vector<std::string> &elems, bool keepDelim = false, bool removeBlank = false)
{
    std::stringstream ss(s);
    std::string item;
    while (std::getline(ss, item, delim))
    {
        if (keepDelim)//OOPS forgot this earlier
            item += delim;
        if (removeBlank && item == "")
            continue;
        if (item[0] != '\0')
            elems.push_back(item);
    }
    return elems;
}

inline std::vector<std::string> SplitStr(const std::string &s, char delim, bool keepDelim = false, bool removeBlank = false)
{
    std::vector<std::string> elems;
    SplitStr(s, delim, elems, keepDelim, removeBlank);
    return elems;
}




/*
======================================================================================================================================================================================================
Datatype Conversion Functions

*/


/*
ArrToVec

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
inline std::vector<T> ArrToVec(T* arr, unsigned long len);


/*
AdoptArray

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
inline std::vector<T> AdoptArray(T*& arr, unsigned long len) noexcept;

/*
GetVec2FromRect
    
    Parameters:
        'rect': the rect to retreive from
        'x': true: right; false: left
        'y': true: top; false: bottom

*/
OBJGLUF_API glm::vec2 GetVec2FromRect(const Rect& rect, bool x, bool y);
OBJGLUF_API glm::vec2 GetVec2FromRect(const Rectf& rect, bool x, bool y);

//used for getting vertices from rects 0,0 is bottom left
OBJGLUF_API Point GetPointFromRect(const Rect& rect, bool x, bool y);


OBJGLUF_API Color4f ColorToFloat(const Color& color);//takes 0-255 to 0.0f - 1.0f
OBJGLUF_API Color3f ColorToFloat3(const Color& color);//takes 0-255 to 0.0f - 1.0f


/*
WStringToString

    Converts std::wstring to std::string

*/

OBJGLUF_API std::string WStringToString(const std::wstring& str) noexcept;


/*
StringToWString

    Converts std::string to std::wstring

*/

OBJGLUF_API std::wstring StringToWString(const std::string& str) noexcept;

}

//template implementations
#include "ObjGLUFTemplates.inl"

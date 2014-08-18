// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the OBJGLUF_EXPORTS
// symbol defined on the command line. This symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// OBJGLUF_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.
#pragma once

#pragma warning (disable : 4251)

#ifdef OBJGLUF_EXPORTS
#define OBJGLUF_API __declspec(dllexport)
#else
#define OBJGLUF_API __declspec(dllimport)
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

#include <vector>
#include <memory>
#include <map>
#include <list>
#include <string>
#include <sstream>
#include <locale>
#include <codecvt>
#include <stdlib.h>

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

#ifdef USING_ASSIMP
inline glm::vec2 AssimpToGlm(aiVector2D v)
{
	return glm::vec2(v.x, v.y);
}

inline glm::vec3 AssimpToGlm(aiVector3D v)
{
	return glm::vec3(v.x, v.y, v.z);
}
inline glm::vec3 AssimpToGlm(aiColor3D v)
{
	return glm::vec3(v.r, v.g, v.b);
}

inline glm::vec4 AssimpToGlm(aiColor4D v)
{
	return glm::vec4(v.r, v.g, v.b, v.a);
}

#endif


typedef void(*GLUFErrorMethod)(const char* message, const char* funcName, const char* sourceFile, unsigned int lineNum);

OBJGLUF_API void GLUFRegisterErrorMethod(GLUFErrorMethod method);
OBJGLUF_API GLUFErrorMethod GLUFGetErrorMethod();

#ifndef __FUNCTION__
#define __FUNCTION__ __func__//not always defined on non-windows systems
#endif

#define GLUF_ERROR(message) GLUFGetErrorMethod()(message, __FUNCTION__, __FILE__, __LINE__);
#define GLUF_ASSERT(expr)	{ if (!(expr)) { std::stringstream ss; ss << "ASSERTION FAILURE:" << #expr; GLUF_ERROR(ss.str().c_str()) } }

#define GLUF_SAFE_DELETE(ptr) {delete(ptr); (ptr) = nullptr;}
#define GLUF_NULL(type) (std::shared_ptr<type>(nullptr))
#define GLUF_UNREFERENCED_PARAMETER(value) (value)

typedef unsigned long GLUFResult;

//GLUFResult Values
#define GR_FAILURE			0
#define GR_SUCCESS			1
#define GR_OUTOFMEMORY		2
#define GR_INVALIDARG		3
#define GR_NOTIMPL			4

#define GLUF_FAILED(result) ((result == GR_FAILURE) ? true : false)
#define GLUF_SUCCEEDED(result) ((result == GR_SUCCESS) ? true : false)
#define GLUF_V_RETURN(result) {if(result != GR_SUCCESS) return result;}

#define GLUFTRACE_ERR(str, gr) GLUFTrace(__FILE__, __FUNCTION__, (unsigned long)__LINE__, gr, str);

OBJGLUF_API GLUFResult GLUFTrace(const char*, const char*, unsigned long, GLUFResult, const char*);

//not defined if not windows
#ifndef _T
#define _T(str) __T(str)
#endif
#ifndef __T
#define __T(str) L ## str
#endif

#define GLUFGetTime() glfwGetTime()

typedef glm::u8vec4 Color;//only accepts numbers from 0 to 255
typedef glm::vec3 Color3f;
typedef glm::vec4 Color4f;


//a little treat for initializing streambuf's with existing data
struct OBJGLUF_API MemStreamBuf : public std::streambuf
{
	MemStreamBuf(char* data, std::ptrdiff_t length)
	{
		setg(data, data, data + length);
	}
};


#define GLUF_UNIVERSAL_TRANSFORM_UNIFORM_BLOCK_LOCATION 0
//#define GLUF_UNIVERSAL_VERTEX_POSITION_LOCATION 0
//#define GLUF_UNIVERSAL_VERTEX_NORMAL_LOCATION 1
//#define GLFU_UNIVERSAL_VERTEX_UVCOORD_LOCATION 2

#define GLUF_PI (double)3.14159265358979
#define GLUF_PI_F (float)3.14159265358979

#define DEG_TO_RAD(value) (value *(GLUF_PI / 180))
#define DEG_TO_RAD_F(value) (value *(GLUF_PI_F / 180))


OBJGLUF_API bool GLUFInit();

//call this after calling glfwMakeContextCurrent on the window
OBJGLUF_API bool GLUFInitOpenGLExtentions();


//this loads an entire file into a binary array, path is input, rawSize and rawData are outputs
OBJGLUF_API char* GLUFLoadFileIntoMemory(const wchar_t* path, unsigned long* rawSize);

typedef std::vector<glm::vec4> Vec4Array;
typedef std::vector<glm::vec3> Vec3Array;
typedef std::vector<glm::vec2> Vec2Array;
typedef std::vector<GLushort>  IndexArray;

#include <stack>

class OBJGLUF_API GLUFMatrixStack
{
	std::stack<glm::mat4> mStack;
public:
	void Push(const glm::mat4& matrix);
	void Pop(void);
	const glm::mat4& Top();//this returns the matrix that is a concatination of all subsequent matrices
	size_t Size();
	void Empty();
};


//this rect is supposed to be used where the origin is bottom left, and upper left corner is (1,1)
struct OBJGLUF_API GLUFRect
{
	float left, top, right, bottom;
};

struct OBJGLUF_API GLUFPoint
{
	union{ float x, width;  };
	union{ float y, height; };

	GLUFPoint(float val1, float val2) : x(val1), y(val2){}
	GLUFPoint() : x(0), y(0){}
};

OBJGLUF_API bool		GLUFPtInRect(GLUFRect rect, GLUFPoint pt);
OBJGLUF_API void		GLUFSetRectEmpty(GLUFRect& rect);
OBJGLUF_API void		GLUFSetRect(GLUFRect& rect, float left, float top, float right, float bottom);
OBJGLUF_API void		GLUFOffsetRect(GLUFRect& rect, float x, float y);
OBJGLUF_API float		GLUFRectHeight(GLUFRect rect);
OBJGLUF_API float		GLUFRectWidth(GLUFRect rect);
OBJGLUF_API void		GLUFInflateRect(GLUFRect& rect, float dx, float dy);
OBJGLUF_API bool		GLUFIntersectRect(GLUFRect rect0, GLUFRect rect1, GLUFRect& rectIntersect);
OBJGLUF_API GLUFRect	GLUFScreenToClipspace(GLUFRect screenCoords);//this is used to tranlate screen coordinates (where origin is bottom left and 1,1 is upper left) to clip space (where origin is the middle, and 1,1 is still upper left)
OBJGLUF_API glm::vec3	GLUFScreenToClipspace(glm::vec3 vec);//same as above
OBJGLUF_API void		GLUFFlipPoint(GLUFPoint& pt);//this expects a normalized value
OBJGLUF_API void		GLUFNormPoint(GLUFPoint& pt, GLUFPoint max);//max is a point that contains the values to normalize by.  i.e. screen size
OBJGLUF_API void		GLUFNormRect(GLUFRect& rect, float xClamp, float yClamp);
OBJGLUF_API GLUFPoint	GLUFMultPoints(GLUFPoint pt0, GLUFPoint pt1);

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

template<typename T>
OBJGLUF_API inline size_t GLUFGetVectorSize(std::vector<T> vec)
{
	return vec.size();
}

//used for getting vertices from rects 0,0 is bottom left
OBJGLUF_API glm::vec2 GLUFGetVec2FromRect(GLUFRect rect, bool x, bool y);

//used for getting vertices from rects 0,0 is bottom left
OBJGLUF_API GLUFPoint GLUFGetPointFromRect(GLUFRect rect, bool x, bool y);


OBJGLUF_API Color4f GLUFColorToFloat(Color color);//takes 0-255 to 0.0f - 1.0f

enum GLUFShaderType
{
	SH_VERTEX_SHADER = GL_VERTEX_SHADER,
	SH_TESS_CONTROL_SHADER = GL_TESS_CONTROL_SHADER,
	SH_TESS_EVALUATION_SHADER = GL_TESS_EVALUATION_SHADER,
	SH_GEOMETRY_SHADER = GL_GEOMETRY_SHADER,
	SH_FRAGMENT_SHADER = GL_FRAGMENT_SHADER
};

enum GLUFShaderInputVertexDataType
{
	IN_POSITIONS = 0,
	IN_NORMALS = 1,
	IN_UVCOORDS = 2
};

enum GLUFShaderUniformTransformMatrixType
{
	IN_M = 4,
	IN_V = 3,
	IN_P = 2,
	IN_MV = 1,
	IN_MVP = 0
};

struct OBJGLUF_API GLUFShaderInfoStruct
{
	bool mSuccess = false;

	char* mLog;

	operator bool() const
	{
		return mSuccess;
	}
};

typedef GLUFShaderInfoStruct GLUFCompileOutputStruct;
typedef GLUFShaderInfoStruct GLUFLinkOutputStruct;


//make all of the classes opaque
class OBJGLUF_API GLUFShader;
class OBJGLUF_API GLUFComputeShader;
class OBJGLUF_API GLUFProgram;
class OBJGLUF_API GLUFSeperateProgram;

typedef std::shared_ptr<GLUFShader>  GLUFShaderPtr;
typedef std::weak_ptr<GLUFShader>    GLUFShaderPtrWeak;
typedef std::shared_ptr<GLUFProgram> GLUFProgramPtr;
typedef std::weak_ptr<GLUFProgram>   GLUFProgramPtrWeak;
typedef std::shared_ptr<GLUFSeperateProgram> GLUFSepProgramPtr;
typedef std::weak_ptr<GLUFSeperateProgram> GLUFSepProgramPtrWeak;

typedef std::map<GLUFShaderType, const char*> GLUFShaderSourceList;
typedef std::map<GLUFShaderType, std::wstring> GLUFShaderPathList;//do a little bit of fudging to get around this being the same as the above
typedef std::vector<GLuint> GLUFShaderIdList;
typedef std::vector<GLuint> GLUFProgramIdList;
typedef std::vector<std::wstring> GLUFShaderNameList;
typedef std::vector<std::wstring> GLUFProgramNameList;
typedef std::vector<GLUFShaderPtr> GLUFShaderPtrList;
typedef std::vector<GLUFProgramPtr> GLUFProgramPtrList;
typedef std::map<GLbitfield, GLUFProgramPtr> GLUFProgramPtrStagesMap;
typedef std::pair<GLbitfield, GLUFProgramPtr> GLUFProgramPtrStagesPair;
typedef std::map<GLUFShaderType, GLUFProgramPtr> GLUFProgramPtrMap;
typedef std::vector<GLUFShaderPtrWeak> GLUFShaderPtrListWeak;
typedef std::vector<GLUFProgramPtrWeak> GLUFProgramPtrListWeak;


class OBJGLUF_API GLUFShaderManager
{

	//a list of logs
	std::map<GLUFShaderPtr, GLUFShaderInfoStruct> mCompileLogs;
	std::map<GLUFProgramPtr, GLUFShaderInfoStruct> mLinklogs;

	//a little helper function for creating things
	//GLUFShaderPtr CreateShader(std::wstring shad, GLUFShaderType type, bool file, bool seperate = false);

	friend class GLUFBufferManager;

public:


	//for creating things

	GLUFShaderPtr CreateShaderFromFile(std::wstring filePath, GLUFShaderType type);
	GLUFShaderPtr CreateShaderFromMemory(const char* text, GLUFShaderType type);

	GLUFProgramPtr CreateProgram(GLUFShaderPtrList shaders, bool seperate = false);
	GLUFProgramPtr CreateProgram(GLUFShaderSourceList shaderSources, bool seperate = false);
	GLUFProgramPtr CreateProgram(GLUFShaderPathList shaderPaths, bool seperate = false);

	GLUFSepProgramPtr CreateSeperateProgram(GLUFProgramPtrStagesMap programs);

	//for removing things

	void DeleteShader(GLUFShaderPtr shader);
	void DeleteProgram(GLUFProgramPtr program);

	void FlushSavedShaders();
	void FlushSavedPrograms();
	void FlushLogs();


	//for accessing things
	const GLuint	  GetShaderId(GLUFShaderPtr shader) const;
	const GLUFShaderType  GetShaderType(GLUFShaderPtr shader) const;
	const GLuint	  GetProgramId(GLUFProgramPtr program) const;

	const GLUFCompileOutputStruct GetShaderLog(GLUFShaderPtr shaderPtr) const;

	const GLUFLinkOutputStruct GetProgramLog(GLUFProgramPtr programPtr) const;

	void AttachPrograms(GLUFSepProgramPtr ppo, GLUFProgramPtrStagesMap programs);
	void AttachProgram(GLUFSepProgramPtr ppo, GLbitfield stages, GLUFProgramPtr);

	//this clears all programs from the given stages
	void ClearPrograms(GLUFSepProgramPtr ppo, GLbitfield stages = GL_ALL_SHADER_BITS);

	//for using things

	void UseProgram(GLUFProgramPtr program);
	void UseProgram(GLUFSepProgramPtr program);
	void UseProgramNull();


};

#define ShaderManager static_pointer_cast<GLRenderer>(QuicksandEngine::g_pApp->m_Renderer)->mShaderManager


/*
Usage examples


//create the shader
GLUFShaderPtr shad = CreateShader("shader.glsl", ST_VERTEX_SHADER);



*/


//typedef GLuint GLBuffer;
//typedef GLuint GLVertexBuffer;
//typedef GLuint GLIndexBuffer;

enum GLUFTransformUniformType
{
	UT_MODEL = 0,
	UT_VIEW = 1,
	UT_PROJ = 2,
	UT_MODELVIEW = 3,
	UT_MVP = 4
};

//this matches with the layout location of them in the shader
enum GLUFVertexAttributeType
{
	VAO_POSITIONS = 0,
	VAO_NORMALS = 1,
	VAO_TEXCOORDS = 2,
	VAO_INDICIES = 3,
};
/*
struct GLUFMatrixTransformBlockPtrs
{
protected:
	friend class GLUFMatrixTransformBlock;
	glm::mat4 *pM;
	glm::mat4 *pV;
	glm::mat4 *pP;
	glm::mat4 *pMV;
	glm::mat4 *pMVP;
public:

	GLUFMatrixTransformBlockPtrs(glm::mat4 *m, glm::mat4 *v, glm::mat4 *p, glm::mat4 *mv, glm::mat4 *mvp) : pM(new glm::mat4(*m)), pV(new glm::mat4(*v)), pP(new glm::mat4(*p)), pMV(new glm::mat4(*mv)), pMVP(new glm::mat4(*mvp))
	{}
	~GLUFMatrixTransformBlockPtrs(){ delete pM, pV, pP, pMV, pMVP; }

	bool ModifyValue(GLUFTransformUniformType type, glm::mat4 data)
	{
		switch (type)
		{
		case UT_MODEL:
			*pM = data;
			*pMV = *pV * *pM;
			*pMVP = *pP * *pMV;
		case UT_VIEW:
			*pV = data;
			*pMV = *pV * *pM;
			*pMVP = *pP * *pMV;
		case UT_PROJ:
			*pP = data;
			*pMVP = *pP * *pMV;
		default:
			GLUF_ERROR("Cannot modify MV or MVP data directly");
			return false;
		}
		return true;
	}

	glm::mat4 operator [](unsigned short index)
	{
		switch (index)
		{
		case UT_MODEL:
			return *pM;
		case UT_VIEW:
			return *pV;
		case UT_PROJ:
			return *pP;
		case UT_MODELVIEW:
			return *pMV;
		case UT_MVP:
			return *pMVP;
		default:
			return glm::mat4();
		}
	}
};*/

struct OBJGLUF_API GLUFMatrixTransformBlockParam
{
protected:
	friend struct GLUFMatrixTransformBlock;
	friend class GLUFUniformBuffer;
	friend class GLUFBufferManager;
	glm::mat4 pM;
	glm::mat4 pV;
	glm::mat4 pP;
	glm::mat4 pMV;
	glm::mat4 pMVP;
public:

	GLUFMatrixTransformBlockParam(glm::mat4 m = glm::mat4(), glm::mat4 v = glm::mat4(), glm::mat4 p = glm::mat4()) : pM(m), pV(v), pP(p), pMV(v * m), pMVP(p * v * m){}
};

struct OBJGLUF_API GLUFMatrixTransformBlock
{
protected:
	//friend GLUFMatrixTransformBlockPtrs;
	friend class GLUFUniformBuffer;
	friend GLUFBufferManager;
	glm::mat4* pM;
	glm::mat4* pV;
	glm::mat4* pP;
	glm::mat4* pMV;
	glm::mat4* pMVP;

	GLUFMatrixTransformBlock() : 
		pM(new glm::mat4()), pV(new glm::mat4()), pP(new glm::mat4()), pMV(new glm::mat4()), pMVP(new glm::mat4())//possibly make this more efficient
	{}
public:


	GLUFMatrixTransformBlock& operator=(GLUFMatrixTransformBlock other)
	{
		pM = other.pM;
		pV = other.pV;
		pP = other.pP;

		pMV = other.pMV;
		pMVP = other.pMVP;
		return *this;
	}

	GLUFMatrixTransformBlock& operator=(GLUFMatrixTransformBlockParam other)
	{
		*pM = other.pM;
		*pV = other.pV;
		*pP = other.pP;

		*pMV = other.pMV;
		*pMVP = other.pMVP;
		return *this;
	}

	
	~GLUFMatrixTransformBlock()
	{
		pM = pV = pP = pMV = pMVP = nullptr;
	}

	void TrueDelete(){ delete pM, pV, pP, pMV, pMVP; pM = pV = pP = pMV = pMVP = nullptr; }

	bool ModifyValue(GLUFTransformUniformType type, glm::mat4 data)
	{
		switch (type)
		{
		case UT_MODEL:
			*pM = data;
			*pMV = *pV * *pM;
			*pMVP = *pP * *pMV;
			break;
		case UT_VIEW:
			*pV = data;
			*pMV = *pV * *pM;
			*pMVP = *pP * *pMV;
			break;
		case UT_PROJ:
			*pP = data;
			*pMVP = *pP * *pMV;
			break;
		default:
			GLUF_ERROR("Cannot modify MV or MVP data directly");
			return false;
		}
		return true;
	}

	glm::mat4 operator [](unsigned short index)
	{
		switch (index)
		{
		case UT_MODEL:
			return *pM;
		case UT_VIEW:
			return *pV;
		case UT_PROJ:
			return *pP;
		case UT_MODELVIEW:
			return *pMV;
		case UT_MVP:
			return *pMVP;
		default:
			return glm::mat4();
		}
	}
};

struct OBJGLUF_API GLUFVAOData
{
	Vec3Array* mPositions;
	Vec3Array* mNormals;
	Vec2Array* mUVCoords;
	IndexArray* mIndicies;

	GLUFVAOData(Vec3Array* positions, Vec3Array* normals, Vec2Array* uvCoords, IndexArray* indicies) : mPositions(positions), mNormals(normals), mUVCoords(uvCoords), mIndicies(indicies){}

	~GLUFVAOData(){ mPositions = nullptr; mNormals = nullptr; mUVCoords = nullptr; mIndicies = nullptr; }
}; 


struct OBJGLUF_API GLUFUniformBlockStandardLayoutData
{
	GLushort mBinding;
	const char* mName;//if this is NOT NULL then it is assumed that this string should be used

	GLubyte* m_pData;//this should be pre-aligned to the layout in use
};

//WORK IN PROGRESS
struct GLUFUniformBlockSharedLayoutData
{
	GLushort mBinding;
	const char* mName;//this is the name of the uniform block, if this is not null, then this will be used to retreive the binding point

	const char** mVariableNames;//This is an array of strings in the format $(UniformBlockName).$(AttributeName) in the order they should be assigned
	
	GLuint  *m_pLengths;//this is a list of lengths for each element in the m_pData array;
	GLubyte *m_pData;//This is the data that is in order
};

class OBJGLUF_API GLUFUniformBuffer;
class OBJGLUF_API GLUFVertexArrayObject;
class OBJGLUF_API GLUFTextureBuffer;


typedef std::weak_ptr<GLUFUniformBuffer> GLUFUniformBufferPtrWeak;
typedef std::weak_ptr<GLUFVertexArrayObject> GLUFVertexArrayPtrWeak;
typedef std::weak_ptr<GLUFTextureBuffer> GLUFTexturePtrWeak;

typedef std::shared_ptr<GLUFUniformBuffer> GLUFUniformBufferPtr;
typedef std::shared_ptr<GLUFVertexArrayObject> GLUFVertexArrayPtr;
typedef std::shared_ptr<GLUFTextureBuffer> GLUFTexturePtr;

class OBJGLUF_API GLUFMaterial
{
protected:
	Color mDiffuse, mAmbient, mSpecular, mEmissive;
	GLfloat mPower;

	GLUFTexturePtr m_pTexture;

public:
	void SetDiffuse(Color diffuse){ mDiffuse = diffuse; }
	Color GetDiffuse(){ return mDiffuse; }

	void SetAmbient(Color ambient){ mAmbient = ambient; }
	Color GetAmbient(){ return mAmbient; }

	void SetSpecular(Color specular, GLfloat power){ mSpecular = specular; mPower = power; }
	Color GetSpecular(){ return mSpecular; }
	GLfloat GetPower(){ return mPower; }

	void SetEmissive(Color emissive){ mEmissive = emissive; }
	Color GetEmissive(){ return mEmissive; }

	void SetTexture(GLUFTexturePtr texture){ m_pTexture = texture; }
	GLUFTexturePtr GetTexture(){ return m_pTexture; }

	void SetAlpha(glm::u8 alpha){ mDiffuse.a = alpha; }
	bool HasAlpha() const { return GetAlpha() != 1.0f; }
	float GetAlpha() const { return mDiffuse.a; }

};

typedef std::shared_ptr<GLUFMaterial> GLUFMaterialPtr;

enum GLUFTextureFileFormat
{
	TFF_DDS = 0//,
	//TFF_PNG = 1,
	//TFF_JPG = 2
};

//this is a similar system to the shaders system
class OBJGLUF_API GLUFBufferManager
{
	//std::map<std::wstring, GLUniformBufferPtrWeak> mUniformBuffers;
	//std::map<std::wstring, GLVertexArrayPtrWeak> mVertexArrayBuffers;
	//friend class DdsResourceLoader;

	//GLTexturePtr CreateTexture(GLuint glTexId);

	//friend class Scene;

public:


	void ResetBufferBindings();//this is used to "unuse" the bound textures and buffers after done drawing

	GLUFVertexArrayPtr CreateVertexArray();
	GLUFUniformBufferPtr CreateUniformArray();

	void DeleteVertexBuffer(GLUFVertexArrayPtr vertArray);
	void DeleteUniformBuffer(GLUFUniformBufferPtr buffer);

	void DrawVertexArray(GLUFVertexArrayPtr vertArray, GLenum topology = GL_TRIANGLES);
	void DrawVertexArrayInstanced(GLUFVertexArrayPtr vertArray, GLuint instanceCount, GLenum topology = GL_TRIANGLES);

	GLuint GetUniformTransformBufferId(GLUFUniformBufferPtr buffer);

	void BindUniformArray(GLUFUniformBufferPtr buffer);

	void ModifyVertexArray(GLUFVertexArrayPtr vertArray, GLUFVertexAttributeType type, Vec3Array data);
	void ModifyVertexArray(GLUFVertexArrayPtr vertArray, GLUFVertexAttributeType type, Vec2Array data);
	void MoidfyVertexIncidies(GLUFVertexArrayPtr vertArray, IndexArray data);
	void ModifyVertexArray(GLUFVertexArrayPtr vertArray, Vec3Array positions, Vec3Array normals, Vec2Array texCoords, IndexArray indicies);

	GLUFMatrixTransformBlock MapUniformTransform(GLUFUniformBufferPtr buffer);
	void UnMapUniformTransform(GLUFUniformBufferPtr buffer);

	GLUFVAOData MapVertexArray(GLUFVertexArrayPtr vertArray);
	void        UnMapVertexArray(GLUFVertexArrayPtr vertArray);

	//this is for modifying standard things such as transforms, or material.
	void ModifyUniformTransformMatrix(GLUFUniformBufferPtr buffer, GLUFTransformUniformType type, glm::mat4 data);//the transform uniform block that is standard to all shaders will always be location of 1
	void ModifyUniformTransformMatrix(GLUFUniformBufferPtr buffer, GLUFMatrixTransformBlockParam data);//this REQUIRES all matrices to exist
	void ModifyUniformMaterial(GLUFUniformBufferPtr buffer, GLUFMaterial mat){};//todo
	void ModifyLighting(GLUFUniformBufferPtr buffer){ };//todo

	//TODO: add support for custom uniform blocks
	//WORK IN PROGRESS
	void ModifyUniformBlock(GLUFUniformBufferPtr buffer, GLUFUniformBlockStandardLayoutData param){};


	//texture methods
	GLUFTexturePtr CreateTextureBuffer();

	GLuint GetTextureBufferId(GLUFTexturePtr texture);

	void UseTexture(GLUFTexturePtr texture, GLuint samplerLocation, GLenum bindingPoint);

	//NOTE: call CreateTextureBuffer() FIRST
	void LoadTextureFromFile(GLUFTexturePtr texture, std::wstring filePath, GLUFTextureFileFormat format);

	//NOTE: call CreateTextureBuffer() FIRST
	void LoadTextureFromMemory(GLUFTexturePtr texture, char* data, unsigned int length, GLUFTextureFileFormat format);

	bool CompareTextures(GLUFTexturePtr texture, GLUFTexturePtr texture1);

	//void BufferTexture(GLUFTexturePtr texture, GLsizei length, GLvoid* data){};

};

//global instances of the managers (becuase they are not static, and have to member variables)
extern GLUFBufferManager OBJGLUF_API g_BufferManager;
extern GLUFShaderManager OBJGLUF_API g_ShaderManager;

#define GLUFBUFFERMANAGER g_BufferManager
#define GLUFSHADERMANAGER g_ShaderManager
// ObjGLUFUF.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#define USING_ASSIMP
#include "ObjGLUF.h"
#include <fstream>
#include <sstream>
#include <GLFW/glfw3.h>

GLUFErrorMethod ErrorMethod;
//GLUFBufferManager g_BufferManager;
GLUFShaderManager g_ShaderManager;

//initialize the standard vertex attributes
//							Name				bytes,	count,	location,				type
const GLUFVertexAttribInfo	g_attribPOS		= { 4,		3,		VERTEX_ATTRIB_POSITION,	GL_FLOAT };
const GLUFVertexAttribInfo	g_attribNORM	= { 4,		3,		VERTEX_ATTRIB_NORMAL,	GL_FLOAT };
const GLUFVertexAttribInfo	g_attribUV0		= { 4,		2,		VERTEX_ATTRIB_UV0,		GL_FLOAT };
const GLUFVertexAttribInfo	g_attribUV1		= { 4,		2,		VERTEX_ATTRIB_UV1,		GL_FLOAT };
const GLUFVertexAttribInfo	g_attribUV2		= { 4,		2,      VERTEX_ATTRIB_UV2,		GL_FLOAT };
const GLUFVertexAttribInfo	g_attribUV3		= { 4,		2,		VERTEX_ATTRIB_UV3,		GL_FLOAT };
const GLUFVertexAttribInfo	g_attribUV4		= { 4,		2,		VERTEX_ATTRIB_UV4,		GL_FLOAT };
const GLUFVertexAttribInfo	g_attribUV5		= { 4,		2,		VERTEX_ATTRIB_UV5,		GL_FLOAT };
const GLUFVertexAttribInfo	g_attribUV6		= { 4,		2,		VERTEX_ATTRIB_UV6,		GL_FLOAT };
const GLUFVertexAttribInfo	g_attribUV7		= { 4,		2,		VERTEX_ATTRIB_UV7,		GL_FLOAT };
const GLUFVertexAttribInfo	g_attribCOLOR0	= { 4,		4,		VERTEX_ATTRIB_COLOR0,	GL_FLOAT };
const GLUFVertexAttribInfo	g_attribCOLOR1	= { 4,		4,		VERTEX_ATTRIB_COLOR1,	GL_FLOAT };
const GLUFVertexAttribInfo	g_attribCOLOR2	= { 4,		4,		VERTEX_ATTRIB_COLOR2,	GL_FLOAT };
const GLUFVertexAttribInfo	g_attribCOLOR3	= { 4,		4,		VERTEX_ATTRIB_COLOR3,	GL_FLOAT };
const GLUFVertexAttribInfo	g_attribCOLOR4	= { 4,		4,		VERTEX_ATTRIB_COLOR4,	GL_FLOAT };
const GLUFVertexAttribInfo	g_attribCOLOR5	= { 4,		4,		VERTEX_ATTRIB_COLOR5,	GL_FLOAT };
const GLUFVertexAttribInfo	g_attribCOLOR6	= { 4,		4,		VERTEX_ATTRIB_COLOR6,	GL_FLOAT };
const GLUFVertexAttribInfo	g_attribCOLOR7	= { 4,		4,		VERTEX_ATTRIB_COLOR7,	GL_FLOAT };
const GLUFVertexAttribInfo	g_attribTAN		= { 4,		3,		VERTEX_ATTRIB_TAN,		GL_FLOAT };
const GLUFVertexAttribInfo	g_attribBITAN	= { 4,		3,		VERTEX_ATTRIB_BITAN,	GL_FLOAT };


void GLUFRegisterErrorMethod(GLUFErrorMethod method)
{
	ErrorMethod = method;
}

void GLFWErrorMethod(int error, const char* description)
{
	std::stringstream ss;
	ss << "GLFW ERROR: Error Code:" << error << "; " << description << std::endl;
	GLUF_ERROR(ss.str().c_str());
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

bool GLUFInitOpenGLExtentions()
{
	GLenum err = glewInit();
	if (err != GLEW_OK)
	{
		GLUF_ERROR("Failed to initialize OpenGL Extensions using GLEW");
		return false;
	}

	return true;
}

char* GLUFLoadFileIntoMemory(const wchar_t* path, unsigned long* rawSize)
{
	GLUF_ASSERT(path);
	GLUF_ASSERT(rawSize);

	std::ifstream inFile(path, std::ios::binary);
	inFile.seekg(0, std::ios::end);
#pragma warning(disable : 4244)
	*rawSize = inFile.tellg();
#pragma warning(default : 4244)
	inFile.seekg(0, std::ios::beg);

	char* rawData;
	rawData = (char*)malloc(*rawSize);
	//if (sizeof(rawData) != *rawSize)
	//	return false;

	if (inFile.read(rawData, *rawSize))
	{
		return rawData;
	}
	else
	{
		GLUF_ERROR("Failed to load file into memory");
		free(rawData);
		rawData = nullptr;
		*rawSize = 0;
		return nullptr;
	}
	
}

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

const glm::mat4& GLUFMatrixStack::Top(void)
{
	return mStack.top();
}

size_t GLUFMatrixStack::Size(void)
{
	return mStack.size();
}

void GLUFMatrixStack::Empty(void)
{
	mStack.empty();
}

bool GLUFPtInRect(GLUFRect rect, GLUFPoint pt)
{
	//for the first comparison, it is impossible for both statements to be false, 
	//because if the y is greater than the top, it is automatically greater than the bottom, and vise versa
	return	(pt.y >= rect.bottom && pt.y <= rect.top) &&
		(pt.x <= rect.right && pt.x >= rect.left);
}

void GLUFSetRectEmpty(GLUFRect& rect)
{
	rect.top = rect.bottom = rect.left = rect.right = 0.0f;
}

void GLUFSetRect(GLUFRect& rect, float left, float top, float right, float bottom)
{
	rect.top = top;
	rect.bottom = bottom;
	rect.left = left;
	rect.right = right;
}

void GLUFOffsetRect(GLUFRect& rect, float x, float y)
{
	rect.top += y;
	rect.bottom += y;
	rect.left += x;
	rect.right += x;
}

float GLUFRectHeight(GLUFRect rect)
{
	return rect.top - rect.bottom;
}

float GLUFRectWidth(GLUFRect rect)
{
	return rect.right - rect.left;
}


void GLUFInflateRect(GLUFRect& rect, float dx, float dy)
{
	float dx2 = dx / 2.0f;
	float dy2 = dy / 2.0f;
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

GLUFRect GLUFScreenToClipspace(GLUFRect screenCoords)
{

	screenCoords.left = screenCoords.left * 2.0f;
	screenCoords.bottom = screenCoords.bottom * 2.0f;
	screenCoords.top = 1.0f - ((1.0f - screenCoords.top) * 2.0f);
	screenCoords.right = 1.0f - ((1.0f - screenCoords.right) * 2.0f);

	screenCoords.left -= 1.0f;
	screenCoords.bottom -= 1.0f;

	return screenCoords;
}

glm::vec3 GLUFScreenToClipspace(glm::vec3 vec)
{
	return glm::vec3(1.0f - ((1.0f - vec.x) * 2.0f), 1.0f - ((1.0f - vec.y) * 2.0f), 1.0f - ((1.0f - vec.z) * 2.0f));
}

void GLUFFlipPoint(GLUFPoint& pt)
{
	pt.y = 1.0f - pt.y;
}

void GLUFNormPoint(GLUFPoint& pt, GLUFPoint max)
{
	pt.x = pt.x / max.x;
	pt.y = pt.y / max.y;
}

void GLUFNormRect(GLUFRect& rect, float xClamp, float yClamp)
{
	rect.left /= xClamp;
	rect.right /= xClamp;
	rect.top /= yClamp;
	rect.bottom /= yClamp;
}

Color4f GLUFColorToFloat(Color color)
{
	Color4f col;
	col.x = (float)color.x / 255.0f;
	col.y = (float)color.y / 255.0f;
	col.z = (float)color.z / 255.0f;
	col.w = (float)color.w / 255.0f;
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

GLuint LoadTextureFromFile(std::wstring filePath, GLUFTextureFileFormat format)
{
	GLuint tex = 0;
	gli::texture2D Texture(gli::load_dds(filePath.c_str()));
	assert(!Texture.empty());
	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, GLint(Texture.levels() - 1));
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_R, GL_RED);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_G, GL_GREEN);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_B, GL_BLUE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_A, GL_ALPHA);
	glTexStorage2D(GL_TEXTURE_2D,
		GLint(Texture.levels()),
		GLenum(gli::internal_format(Texture.format())),
		GLsizei(Texture.dimensions().x),
		GLsizei(Texture.dimensions().y));
	if (gli::is_compressed(Texture.format()))
	{
		for (gli::texture2D::size_type Level = 0; Level < Texture.levels(); ++Level)
		{
			glCompressedTexSubImage2D(GL_TEXTURE_2D,
				GLint(Level),
				0, 0,
				GLsizei(Texture[Level].dimensions().x),
				GLsizei(Texture[Level].dimensions().y),
				GLenum(gli::internal_format(Texture.format())),
				GLsizei(Texture[Level].size()),
				Texture[Level].data());
		}
	}
	else
	{
		for (gli::texture2D::size_type Level = 0; Level < Texture.levels(); ++Level)
		{
			glTexSubImage2D(GL_TEXTURE_2D,
				GLint(Level),
				0, 0,
				GLsizei(Texture[Level].dimensions().x),
				GLsizei(Texture[Level].dimensions().y),
				GLenum(gli::external_format(Texture.format())),
				GLenum(gli::type_format(Texture.format())),
				Texture[Level].data());
		}
	}

	glBindTexture(GL_TEXTURE_2D, 0);

	return tex;
}


GLuint LoadTextureFromMemory(char* data, unsigned int length, GLUFTextureFileFormat format)
{
	GLuint tex = 0;
	gli::texture2D Texture(gli::load_dds_memory(data, length));
	assert(!Texture.empty());
	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, GLint(Texture.levels() - 1));
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_R, GL_RED);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_G, GL_GREEN);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_B, GL_BLUE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_A, GL_ALPHA);
	glTexStorage2D(GL_TEXTURE_2D,
		GLint(Texture.levels()),
		GLenum(gli::internal_format(Texture.format())),
		GLsizei(Texture.dimensions().x),
		GLsizei(Texture.dimensions().y));
	if (gli::is_compressed(Texture.format()))
	{
		for (gli::texture2D::size_type Level = 0; Level < Texture.levels(); ++Level)
		{
			glCompressedTexSubImage2D(GL_TEXTURE_2D,
				GLint(Level),
				0, 0,
				GLsizei(Texture[Level].dimensions().x),
				GLsizei(Texture[Level].dimensions().y),
				GLenum(gli::internal_format(Texture.format())),
				GLsizei(Texture[Level].size()),
				Texture[Level].data());
		}
	}
	else
	{
		for (gli::texture2D::size_type Level = 0; Level < Texture.levels(); ++Level)
		{
			glTexSubImage2D(GL_TEXTURE_2D,
				GLint(Level),
				0, 0,
				GLsizei(Texture[Level].dimensions().x),
				GLsizei(Texture[Level].dimensions().y),
				GLenum(gli::external_format(Texture.format())),
				GLenum(gli::type_format(Texture.format())),
				Texture[Level].data());
		}
	}

	glBindTexture(GL_TEXTURE_2D, 0);

	return tex;
}

////////////////////////////
//
//Shader Stuff
//////////////////////////


class GLUFShader
{
	friend GLUFShaderManager;
	friend GLUFProgram;

	GLuint mShaderId;

	std::string mTmpShaderText;

	GLUFShaderType mShaderType;

public:

	GLUFShader();
	~GLUFShader();

	//common shader is if the shader will not be deleted after building into a program
	//this is used for things like lighting functions
	void Init(GLUFShaderType shaderType);

	void Load(const char* shaderText, bool append = false);
	void LoadFromMemory(char* shaderData, size_t length, bool append = false);
	bool LoadFromFile(const wchar_t* filePath, bool append = false);

	void FlushText(void){ mTmpShaderText.clear(); }

	void Compile(GLUFShaderInfoStruct& retStruct);


};

typedef std::pair<GLUFShaderType, GLUFShaderPtr > GLUFShaderP;
//this is a special instance
class GLUFComputeShader
{
	friend GLUFShaderManager;
	//TODO: low priority
};

class GLUFProgram
{
	friend GLUFShaderManager;

	GLuint mUniformBuffId;
	GLuint mProgramId;
	std::map<GLUFShaderType, GLUFShaderPtr > mShaderBuff;

public:

	GLUFProgram();
	~GLUFProgram();

	void Init();

	void AttachShader(GLUFShaderPtr shader);
	void FlushShaders(void);

	void Build(GLUFShaderInfoStruct& retStruct, bool seperate);

	GLuint GetId(){ return mProgramId; }
};

class GLUFSeperateProgram
{
	friend GLUFShaderManager;
	GLuint mPPOId;

	GLUFProgramPtrStagesMap m_Programs;//so the programs don't go deleting themselves until the PPO is destroyed

public:
	~GLUFSeperateProgram(){ glDeleteProgramPipelines(1, &mPPOId); }

	void Init(){ glGenProgramPipelines(1, &mPPOId); }
	
	void AttachProgram(GLUFProgramPtr program, GLbitfield stages){ m_Programs.insert(GLUFProgramPtrStagesPair(stages, program)); glUseProgramStages(mPPOId, stages, program->GetId()); }

};



////////////////////////////////////////
//
//GLUFShader Methods:
//
//


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

void GLUFShader::Load(const char* shaderText, bool append)
{
	if (!append)
		mTmpShaderText.clear();

	mTmpShaderText = shaderText;
}

bool GLUFShader::LoadFromFile(const wchar_t* filePath, bool append)
{
	if (!append)
		mTmpShaderText.clear();

	std::ifstream inFile(filePath);
	if (inFile)
	{
#pragma warning (disable : 4244)
		inFile.seekg(0, std::ios::end);
		mTmpShaderText.resize(inFile.tellg());
		inFile.seekg(0, std::ios::beg);
		inFile.read(&mTmpShaderText[0], mTmpShaderText.size());
		inFile.close();
	}
	else
	{
		return false;
	}

	return true;
}

void GLUFShader::LoadFromMemory(char* shaderData, size_t length, bool append)
{
	if (!append)
		mTmpShaderText.clear();

	gli::MemStreamBuf *streamData = new gli::MemStreamBuf(shaderData, length);
	std::istream inData(streamData);
	if (inData)
	{
		inData.seekg(0, std::ios::end);
		mTmpShaderText.resize(inData.tellg());
		inData.seekg(0, std::ios::beg);
		inData.read(&mTmpShaderText[0], mTmpShaderText.size());

#pragma warning (default : 4244)
	}
	else
	{
		delete streamData;
		GLUF_ERROR("Failed to initialize stream to load shader data from");
	}

	delete streamData;
}

#define FAILED_COMPILE 'F'
#define FAILED_LINK    'F'

void GLUFShader::Compile(GLUFShaderInfoStruct& returnStruct)
{
	//make sure we aren't trying to recompile with a previously successful one
	if (mShaderId != 0)
	{
		returnStruct.mSuccess = false;
		returnStruct.mLog = "F";
		return;
	}

	mShaderId = glCreateShader(mShaderType);

	//start by adding the strings to glShader Source.  This is done right before the compile
	//process becuase it is hard to remove it if there is any reason to flush the text

	std::string tmpText = mTmpShaderText;

	GLint tmpSize = mTmpShaderText.length();
	tmpSize--; /*BECAUSE OF NULL TERMINATED STRINGS*/

	const GLchar* text = tmpText.c_str();
	glShaderSource(mShaderId, 1, &text, &tmpSize);

	FlushText();

	glCompileShader(mShaderId);

	GLint isCompiled = 0;
	glGetShaderiv(mShaderId, GL_COMPILE_STATUS, &isCompiled);
	returnStruct.mSuccess = (isCompiled == GL_FALSE) ? false : true;

	GLint maxLength = 0;
	glGetShaderiv(mShaderId, GL_INFO_LOG_LENGTH, &maxLength);

	//The maxLength includes the NULL character
	returnStruct.mLog = (char*)malloc(maxLength);
	glGetShaderInfoLog(mShaderId, maxLength, &maxLength, returnStruct.mLog);

	//Provide the infolog in whatever manor you deem best.
	//Exit with failure.

	//if it failed, delete the shader
	if (returnStruct.mSuccess == false)
	{
		glDeleteShader(mShaderId);
		mShaderId = 0;
	}
	return;
}



////////////////////////////////////////
//
//GLUFProgram Methods:
//
//

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

	//for uniforms
	glGenBuffers(1, &mUniformBuffId);
}

void GLUFProgram::AttachShader(GLUFShaderPtr shader)
{
	mShaderBuff.insert(GLUFShaderP(shader->mShaderType, shader));
	glAttachShader(mProgramId, shader->mShaderId);
}

void GLUFProgram::FlushShaders(void)
{
	for (auto it : mShaderBuff)
	{
		glDetachShader(mProgramId, it.second->mShaderId);
	}
	mShaderBuff.clear();
}

void GLUFProgram::Build(GLUFShaderInfoStruct& retStruct, bool seperate)
{
	//make sure we enable separate shading
	if (seperate){ glProgramParameteri(mProgramId, GL_PROGRAM_SEPARABLE, GL_TRUE); }

	//Link our program
	glLinkProgram(mProgramId);

	//Note the different functions here: glGetProgram* instead of glGetShader*.
	GLint isLinked = 0;
	glGetProgramiv(mProgramId, GL_LINK_STATUS, &isLinked);
	retStruct.mSuccess = (isLinked == GL_FALSE) ? false : true;

	GLint maxLength = 0;
	glGetProgramiv(mProgramId, GL_INFO_LOG_LENGTH, &maxLength);

	//The maxLength includes the NULL character
	retStruct.mLog = (char*)malloc(maxLength);
	glGetProgramInfoLog(mProgramId, maxLength, &maxLength, retStruct.mLog);

	if (!retStruct.mSuccess)
	{
		//in the case of failure, DO NOT DELETE ANYTHING.
	}
	else
	{
		FlushShaders();//this removes the references to them from the program, but they will still exist unless they are 'common'
	}
}

////////////////////////////////////////
//
//GLUFSeperateProgram Methods:
//
//

/////////////////////////////////////////////////////////////////////////
//
//GLUFShaderManager Methods:
//
//
//
//
//

//for creating things

/*
GLUFShaderPtr GLUFShaderManager::CreateShader(std::wstring shad, GLUFShaderType type, bool file, bool seperate)
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

GLUFShaderPtr GLUFShaderManager::CreateShaderFromFile(std::wstring filePath, GLUFShaderType type)
{
	//return CreateShader(filePath, type, true);

	GLUFShaderPtr shader(new GLUFShader());
	shader->Init(type);

	//(file) ? shader->LoadFromFile(shad.c_str()) : shader->Load(shad.c_str());
	shader->LoadFromFile(filePath.c_str());

	GLUFShaderInfoStruct output;
	shader->Compile(output);
	mCompileLogs.insert(std::pair<GLUFShaderPtr, GLUFShaderInfoStruct>(shader, output));

	//log it if it failed
	if (!output)
	{
		std::stringstream ss;
		ss << "Shader Compilation Failed: \n" << output.mLog;
		GLUF_ERROR(ss.str().c_str());
	}

	return shader;
}


GLUFShaderPtr GLUFShaderManager::CreateShaderFromMemory(const char* text, GLUFShaderType type)
{
	//return CreateShader(text, type, false);
	GLUFShaderPtr shader(new GLUFShader());
	shader->Init(type);

	//(file) ? shader->LoadFromFile(shad.c_str()) : shader->Load(shad.c_str());
	shader->Load(text);

	GLUFShaderInfoStruct output;
	shader->Compile(output);
	mCompileLogs.insert(std::pair<GLUFShaderPtr, GLUFShaderInfoStruct>(shader, output));

	//log it if it failed
	if (!output)
	{
		std::stringstream ss;
		ss << "Shader Compilation Failed: \n" << output.mLog;
		GLUF_ERROR(ss.str().c_str());
	}

	return shader;
}

GLUFProgramPtr GLUFShaderManager::CreateProgram(GLUFShaderPtrList shaders, bool seperate)
{
	GLUFProgramPtr program(new GLUFProgram());
	program->Init();

	for (auto it : shaders)
	{
		program->AttachShader(it);
	}

	GLUFShaderInfoStruct out;
	program->Build(out, seperate);
	mLinklogs.insert(std::pair<GLUFProgramPtr, GLUFShaderInfoStruct>(program, out));

	if (!out)
	{
		std::stringstream ss;
		ss << "Program Link Failed: \n" << out.mLog;
		GLUF_ERROR(ss.str().c_str());
	}

	return program;
}


GLUFProgramPtr GLUFShaderManager::CreateProgram(GLUFShaderSourceList shaderSources, bool seperate)
{
	GLUFShaderPtrList shaders;
	for (auto it : shaderSources)
	{
		//use the counter global to get a unique name  This is temperary anyway
		shaders.push_back(CreateShaderFromMemory(it.second, it.first));

		//make sure it didn't fail
		if (!GetShaderLog(shaders[shaders.size()-1]))
		{
			GLUF_ERROR("Program Creation Failed, Reason: Shader Compilation Failed");
			return nullptr;
		}
	}

	return CreateProgram(shaders);
}


GLUFProgramPtr GLUFShaderManager::CreateProgram(GLUFShaderPathList shaderPaths, bool seperate)
{
	GLUFShaderPtrList shaders;
	for (auto it : shaderPaths)
	{

		shaders.push_back(CreateShaderFromFile(it.second, it.first));

		//make sure it didn't fail
		if (!GetShaderLog(shaders[shaders.size() - 1]))
		{
			GLUF_ERROR("Program Creation Failed, Reason: Shader Compilation Failed");
			return nullptr;
		}
	}

	return CreateProgram(shaders);
}




//for removing things

void GLUFShaderManager::DeleteShader(GLUFShaderPtr shader)
{
	delete shader.get();
}

void GLUFShaderManager::DeleteProgram(GLUFProgramPtr program)
{
	delete program.get();
}

void GLUFShaderManager::FlushLogs()
{
	mLinklogs.clear();
	mCompileLogs.clear();
}


//for accessing things

const GLuint GLUFShaderManager::GetShaderId(GLUFShaderPtr shader) const
{
	GLUF_ASSERT(shader);

	return shader->mShaderId;
}


const GLUFShaderType GLUFShaderManager::GetShaderType(GLUFShaderPtr shader) const
{
	GLUF_ASSERT(shader);

	return shader->mShaderType;
}


const GLuint GLUFShaderManager::GetProgramId(GLUFProgramPtr program) const
{
	GLUF_ASSERT(program);

	return program->mProgramId;
}

const GLUFCompileOutputStruct GLUFShaderManager::GetShaderLog(GLUFShaderPtr shaderPtr) const
{
	return mCompileLogs.find(shaderPtr)->second;
}


const GLUFLinkOutputStruct GLUFShaderManager::GetProgramLog(GLUFProgramPtr programPtr) const
{
	return mLinklogs.find(programPtr)->second;
}

//for using things

void GLUFShaderManager::UseProgram(GLUFProgramPtr program)
{
	glUseProgram(program->mProgramId);
}

void GLUFShaderManager::UseProgramNull()
{
	glUseProgram(0);
	glBindProgramPipeline(0);//juse in case we are using pipelines
}

GLUFSepProgramPtr GLUFShaderManager::CreateSeperateProgram(GLUFProgramPtrStagesMap programs)
{
	GLUFSepProgramPtr ret(new GLUFSeperateProgram);
	ret->Init();

	for (auto it : programs)
	{
		ret->AttachProgram(it.second, it.first);
	}
	return ret;
}

void GLUFShaderManager::AttachProgram(GLUFSepProgramPtr ppo, GLbitfield stages, GLUFProgramPtr program)
{
	ppo->AttachProgram(program, stages);
}

void GLUFShaderManager::AttachPrograms(GLUFSepProgramPtr ppo, GLUFProgramPtrStagesMap programs)
{
	for (auto it : programs)
	{
		ppo->AttachProgram(it.second, it.first);
	}
}

void GLUFShaderManager::ClearPrograms(GLUFSepProgramPtr ppo, GLbitfield stages)
{
	glUseProgramStages(ppo->mPPOId, stages, 0);
}

void GLUFShaderManager::UseProgram(GLUFSepProgramPtr program)
{
	glBindProgramPipeline(program->mPPOId);
}





GLUFVertexArrayBase::GLUFVertexArrayBase(GLenum PrimType, GLenum buffUsage, bool index) : mUsageType(buffUsage), mPrimitiveType(PrimType)
{
	glGenVertexArrayBindVertexArray(&mVertexArrayId);

	if (index)
		glGenBuffers(1, &mIndexBuffer);
}

GLUFVertexArrayBase::~GLUFVertexArrayBase()
{
	BindVertexArray();
	glDeleteBuffers(1, &mIndexBuffer);
	glDeleteVertexArrays(1, &mVertexArrayId);

	glBindVertexArray(0);
}

void GLUFVertexArrayBase::BindVertexArray()
{
	glBindVertexArray(mVertexArrayId);
}

void GLUFVertexArrayBase::AddVertexAttrib(GLUFVertexAttribInfo info)
{
	BindVertexArray();

	mAttribInfos.push_back(info);

	RefreshDataBufferAttribute();
}

void GLUFVertexArrayBase::RemoveVertexAttrib(GLUFAttribLoc loc)
{
	for (std::vector<GLUFVertexAttribInfo>::iterator it = mAttribInfos.begin(); it != mAttribInfos.end(); ++it)
	{
		if (it->VertexAttribLocation == loc)
		{
			mAttribInfos.erase(it);
			break;
		}
	}
}

GLUFVertexAttribInfo GLUFVertexArrayBase::GetAttribInfoFromLoc(GLUFAttribLoc loc)
{
	GLUFVertexAttribInfo ret = { 0, 0, 0, GL_FLOAT };
	for (auto it : mAttribInfos)
	{
		if (it.VertexAttribLocation == loc)
			return it;
	}

	return ret;
}

void GLUFVertexArrayBase::BufferIndices(GLuint* indices, unsigned int count)
{
	BindVertexArray();
	mIndexCount = count;

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIndexBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * count, indices, mUsageType);
}

void GLUFVertexArrayBase::Draw()
{
	BindVertexArray();
	EnableVertexAttributes();
	
	if (mIndexBuffer != 0)
	{
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIndexBuffer);
		glDrawElements(mPrimitiveType, mIndexCount, GL_UNSIGNED_INT, nullptr);
	}
	else
	{
		glDrawArrays(mPrimitiveType, 0, mVertexCount);
	}

	DisableVertexAttributes();
}

void GLUFVertexArrayBase::DrawInstanced(GLuint instances)
{
	BindVertexArray();
	EnableVertexAttributes();

	if (mIndexBuffer != 0)
	{
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIndexBuffer);
		glDrawElementsInstanced(mPrimitiveType, mIndexCount, GL_UNSIGNED_INT, nullptr, instances);
	}
	else
	{
		glDrawArraysInstanced(mPrimitiveType, 0, mVertexCount, instances);
	}

	DisableVertexAttributes();
}

void GLUFVertexArrayBase::EnableVertexAttributes()
{
	for (auto it : mAttribInfos)
	{
		glEnableVertexAttribArray(it.VertexAttribLocation);
	}
}

void GLUFVertexArrayBase::DisableVertexAttributes()
{
	for (auto it : mAttribInfos)
	{
		glDisableVertexAttribArray(it.VertexAttribLocation);
	}
}



int RoundNearestMultiple(unsigned int num, unsigned int multiple)
{
	unsigned int nearestMultiple = 0;
	for (unsigned int i = 0; i < num; i += multiple)
	{
		nearestMultiple = i;
	}

	nearestMultiple += multiple;
	return nearestMultiple;
}

void GLUFVertexArrayAoS::RefreshDataBufferAttribute()
{
	BindVertexArray();

	glBindBuffer(GL_ARRAY_BUFFER, mDataBuffer);

	unsigned int stride = GetVertexSize();


	for (auto it : mAttribInfos)
	{
		glVertexAttribPointer(it.VertexAttribLocation, it.ElementsPerValue, it.Type, GL_FALSE, stride, nullptr);
	}
}

GLUFVertexArrayAoS::GLUFVertexArrayAoS(GLenum PrimType, GLenum buffUsage, bool indexed) : GLUFVertexArrayBase(PrimType, buffUsage, indexed)
{
	//the VAO is already bound

	glGenBuffers(1, &mDataBuffer);
}

GLUFVertexArrayAoS::~GLUFVertexArrayAoS()
{
	BindVertexArray();

	glDeleteBuffers(1, &mDataBuffer);
}

GLUFVertexArrayAoS::GLUFVertexArrayAoS(const GLUFVertexArrayAoS& other)
{
	mVertexArrayId = other.mVertexArrayId;
	mVertexCount = other.mVertexCount;
	mUsageType = other.mUsageType;
	mPrimitiveType = other.mPrimitiveType;
	mAttribInfos = other.mAttribInfos;
	mIndexBuffer = other.mIndexBuffer;
	mIndexCount = other.mIndexCount;

	mDataBuffer = other.mDataBuffer;
}

unsigned int GLUFVertexArrayAoS::GetVertexSize()
{
	unsigned int stride = 0;
	char* data = nullptr;
	for (auto it : mAttribInfos)
	{
		//round to the 4 bytes bounderies
		data = (char*)malloc(RoundNearestMultiple(it.BytesPerElement, 4));
		stride += it.ElementsPerValue * sizeof(data);
	}

	return stride;
}

void GLUFVertexArrayAoS::BufferData(void* data, unsigned int NumVertices)
{
	BindVertexArray();
	glBindBuffer(GL_ARRAY_BUFFER, mDataBuffer);

	glBufferData(GL_ARRAY_BUFFER, NumVertices * GetVertexSize(), data, mUsageType);

	mVertexCount = NumVertices;
}

void GLUFVertexArrayAoS::BufferSubData(unsigned int ValueOffsetCount, unsigned int NumValues, void* data)
{
	BindVertexArray();
	glBindBuffer(GL_ARRAY_BUFFER, mDataBuffer);

	unsigned int size = GetVertexSize();
	glBufferSubData(GL_ARRAY_BUFFER, ValueOffsetCount * size, NumValues * size, data);
}




GLuint GLUFVertexArraySoA::GetBufferIdFromAttribLocation(GLUFAttribLoc loc)
{
	return mDataBuffers.find(loc)->second;
}

GLUFVertexArraySoA::GLUFVertexArraySoA(GLenum PrimType, GLenum buffUsage, bool indexed) : GLUFVertexArrayBase(PrimType, buffUsage, indexed)
{
}

GLUFVertexArraySoA::GLUFVertexArraySoA(const GLUFVertexArraySoA& other)
{
	mVertexArrayId = other.mVertexArrayId;
	mVertexCount = other.mVertexCount;
	mUsageType = other.mUsageType;
	mPrimitiveType = other.mPrimitiveType;
	mAttribInfos = other.mAttribInfos;
	mIndexBuffer = other.mIndexBuffer;
	mIndexCount = other.mIndexCount;

	mDataBuffers = other.mDataBuffers;
}

GLUFVertexArraySoA::~GLUFVertexArraySoA()
{
	BindVertexArray();
	for (auto it : mDataBuffers)
		glDeleteBuffers(1, &it.second);
}

void GLUFVertexArraySoA::BufferData(GLUFAttribLoc loc, GLuint VertexCount, void* data)
{
	BindVertexArray();
	glBindBuffer(GL_ARRAY_BUFFER, GetBufferIdFromAttribLocation(loc));
	mVertexCount = VertexCount;

	GLUFVertexAttribInfo info = GetAttribInfoFromLoc(loc);
	glBufferData(GL_ARRAY_BUFFER, VertexCount * info.BytesPerElement * info.ElementsPerValue, data, mUsageType);
}

void GLUFVertexArraySoA::BufferSubData(GLUFAttribLoc loc, GLuint VertexOffsetCount, GLuint VertexCount, void* data)
{
	BindVertexArray();
	glBindBuffer(GL_ARRAY_BUFFER, GetBufferIdFromAttribLocation(loc));
	
	GLUFVertexAttribInfo info = GetAttribInfoFromLoc(loc);
	glBufferSubData(GL_ARRAY_BUFFER, VertexOffsetCount * info.BytesPerElement * info.ElementsPerValue, VertexCount * info.BytesPerElement * info.ElementsPerValue, data);
}

void GLUFVertexArraySoA::AddVertexAttrib(GLUFVertexAttribInfo info)
{
	BindVertexArray();

	mAttribInfos.push_back(info);

	GLuint newBuff = 0;
	glGenBuffers(1, &newBuff);
	mDataBuffers.insert(std::pair<GLUFAttribLoc, GLuint>(info.VertexAttribLocation, newBuff));
	
	RefreshDataBufferAttribute();
}

void GLUFVertexArraySoA::RemoveVertexAttrib(GLUFAttribLoc loc)
{
	BindVertexArray();

	std::map<GLUFAttribLoc, GLuint>::iterator it = mDataBuffers.find(loc);

	glDeleteBuffers(1, &(it->second));
	mDataBuffers.erase(it);

	/*for (unsigned int i = 0; i < mAttribInfos.size(); ++i)
	{
		if (mAttribInfos[i].VertexAttribLocation == loc)
		{
			glDeleteBuffers(1, &mDataBuffers[i]);
			mDataBuffers.erase(mDataBuffers.begin() + i);
		}
	}
	*/

	GLUFVertexArrayBase::RemoveVertexAttrib(loc);
}

void GLUFVertexArraySoA::RefreshDataBufferAttribute()
{
	BindVertexArray();
	for (unsigned int i = 0; i < mAttribInfos.size(); ++i)
	{
		glBindBuffer(GL_ARRAY_BUFFER, mDataBuffers[mAttribInfos[i].VertexAttribLocation]);
		glVertexAttribPointer(mAttribInfos[i].VertexAttribLocation, mAttribInfos[i].ElementsPerValue, mAttribInfos[i].Type, GL_FALSE, 0, nullptr);
	}
}

GLUFVertexArray *LoadVertexArrayFromScene(const aiScene* scene, unsigned int meshNum)
{
	if (meshNum > scene->mNumMeshes)
		return nullptr;

	const aiMesh* mesh = scene->mMeshes[meshNum];

	GLUFVertexArray* vertexData = new GLUFVertexArray(GL_TRIANGLES, GL_STATIC_DRAW, mesh->HasFaces());

	if (mesh->HasPositions())
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
		vertexData->BufferData(VERTEX_ATTRIB_POSITION, mesh->mNumVertices, mesh->mVertices);
	if (mesh->HasNormals())
		vertexData->BufferData(VERTEX_ATTRIB_NORMAL, mesh->mNumVertices, mesh->mNormals);
	if (mesh->HasTextureCoords(0))
		vertexData->BufferData(VERTEX_ATTRIB_UV0, mesh->mNumVertices, AssimpToGlm3_2(mesh->mTextureCoords[0], mesh->mNumVertices));
	if (mesh->HasTextureCoords(1))
		vertexData->BufferData(VERTEX_ATTRIB_UV1, mesh->mNumVertices, AssimpToGlm3_2(mesh->mTextureCoords[1], mesh->mNumVertices));
	if (mesh->HasTextureCoords(2))
		vertexData->BufferData(VERTEX_ATTRIB_UV2, mesh->mNumVertices, AssimpToGlm3_2(mesh->mTextureCoords[2], mesh->mNumVertices));
	if (mesh->HasTextureCoords(3))
		vertexData->BufferData(VERTEX_ATTRIB_UV3, mesh->mNumVertices, AssimpToGlm3_2(mesh->mTextureCoords[3], mesh->mNumVertices));
	if (mesh->HasTextureCoords(4))
		vertexData->BufferData(VERTEX_ATTRIB_UV4, mesh->mNumVertices, AssimpToGlm3_2(mesh->mTextureCoords[4], mesh->mNumVertices));
	if (mesh->HasTextureCoords(5))
		vertexData->BufferData(VERTEX_ATTRIB_UV5, mesh->mNumVertices, AssimpToGlm3_2(mesh->mTextureCoords[5], mesh->mNumVertices));
	if (mesh->HasTextureCoords(6))
		vertexData->BufferData(VERTEX_ATTRIB_UV6, mesh->mNumVertices, AssimpToGlm3_2(mesh->mTextureCoords[6], mesh->mNumVertices));
	if (mesh->HasTextureCoords(7))
		vertexData->BufferData(VERTEX_ATTRIB_UV7, mesh->mNumVertices, AssimpToGlm3_2(mesh->mTextureCoords[7], mesh->mNumVertices));

	if (mesh->HasVertexColors(0))
		vertexData->BufferData(VERTEX_ATTRIB_COLOR0, mesh->mNumVertices, mesh->mColors[0]);
	if (mesh->HasVertexColors(1))
		vertexData->BufferData(VERTEX_ATTRIB_COLOR1, mesh->mNumVertices, mesh->mColors[1]);
	if (mesh->HasVertexColors(2))
		vertexData->BufferData(VERTEX_ATTRIB_COLOR2, mesh->mNumVertices, mesh->mColors[2]);
	if (mesh->HasVertexColors(3))
		vertexData->BufferData(VERTEX_ATTRIB_COLOR3, mesh->mNumVertices, mesh->mColors[3]);
	if (mesh->HasVertexColors(4))
		vertexData->BufferData(VERTEX_ATTRIB_COLOR4, mesh->mNumVertices, mesh->mColors[4]);
	if (mesh->HasVertexColors(5))
		vertexData->BufferData(VERTEX_ATTRIB_COLOR5, mesh->mNumVertices, mesh->mColors[5]);
	if (mesh->HasVertexColors(6))
		vertexData->BufferData(VERTEX_ATTRIB_COLOR6, mesh->mNumVertices, mesh->mColors[6]);
	if (mesh->HasVertexColors(7))
		vertexData->BufferData(VERTEX_ATTRIB_COLOR7, mesh->mNumVertices, mesh->mColors[7]);
	if (mesh->HasTangentsAndBitangents())
	{
		vertexData->BufferData(VERTEX_ATTRIB_BITAN, mesh->mNumVertices, mesh->mBitangents);
		vertexData->BufferData(VERTEX_ATTRIB_TAN, mesh->mNumVertices, mesh->mTangents);
	}

	std::vector<GLuint> indices;
	for (unsigned int i = 0; i < mesh->mNumFaces; ++i)
	{
		aiFace curr = mesh->mFaces[i];
		indices.push_back(curr.mIndices[0]);
		indices.push_back(curr.mIndices[1]);
		indices.push_back(curr.mIndices[2]);
	}
	vertexData->BufferIndices(&indices[0], indices.size());

	return vertexData;
}

std::vector<GLUFVertexArray*> LoadVertexArraysFromScene(const aiScene* scene, unsigned int numMeshes)
{
	std::vector<GLUFVertexArray*> arrays;

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
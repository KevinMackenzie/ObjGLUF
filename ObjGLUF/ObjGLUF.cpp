// ObjGLUFUF.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "ObjGLUF.h"
#include <fstream>
#include <sstream>
#include <GLFW/glfw3.h>

GLUFErrorMethod ErrorMethod;
//GLUFBufferManager g_BufferManager;
GLUFShaderManager g_ShaderManager;

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
/*
class GLUFUniformBuffer
{

	friend GLUFBufferManager;

	//GLUFProgramPtrWeak programRef;
	GLuint mTransformBufferId;
	//TODO: impliment this further with a std::vector

	GLUFMatrixTransformBlock mTransformData;//this will be used lator for mapping

public:

	void RefreshTransformData();

	GLUFUniformBuffer(){ glGenBuffers(1, &mTransformBufferId); glBindBuffer(GL_UNIFORM_BUFFER, mTransformBufferId); glBufferData(GL_UNIFORM_BUFFER, sizeof(glm::mat4) * 5, nullptr, GL_DYNAMIC_DRAW); }
	~GLUFUniformBuffer(){ glDeleteBuffers(1, &mTransformBufferId); mTransformData.TrueDelete(); }
};

class GLUFVertexArrayObject
{
	friend GLUFBufferManager;

	//make sure to check that each element has the SAME number of elements (except for indices)

	std::vector<GLUFBufferType> m_BufferInformation;
	std::vector<GLuint>         m_BufferLocations;//each of these corresponds on one of the above

	GLuint mIndexLocation = 0;

	bool mInitialiazed[3];


public:

	void Refresh();//this reloads the data from the memory to openGL
	void EnableActive();
	void DisableAll();

	GLUFVertexArrayObject(std::vector<GLUFBufferType> BufferInformation);
	~GLUFVertexArrayObject();

};

class GLUFTextureBuffer
{
	friend GLUFBufferManager;
	GLuint mTextureId;

public:

	GLUFTextureBuffer(){ glGenTextures(1, &mTextureId); }
	~GLUFTextureBuffer(){ glDeleteTextures(1, &mTextureId); };
};

void GLUFVertexArrayObject::Refresh()
{
	glBindVertexArray(mVAOId);
	
	if (mIndicies.size())
	{
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIndexId);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, mIndicies.size() * sizeof(GLushort), &mIndicies[0], GL_DYNAMIC_DRAW);
	}

	if (mPositions.size())
	{
		glBindBuffer(GL_ARRAY_BUFFER, mPositionId);
		glBufferData(GL_ARRAY_BUFFER, mPositions.size() * sizeof(glm::vec3), &mPositions[0], GL_DYNAMIC_DRAW);
		if (!mInitialiazed[IN_POSITIONS])
		{
			glEnableVertexAttribArray(IN_POSITIONS);
			mInitialiazed[IN_POSITIONS] = true;
		}
	}
	else if (mInitialiazed[IN_POSITIONS])
	{
		//if the array is EMPTY, and it IS initialized, then remove it
		glBindBuffer(GL_ARRAY_BUFFER, mPositionId);
		glDisableVertexAttribArray(IN_POSITIONS);
		mInitialiazed[IN_POSITIONS] = false;
	}

	if (mNormals.size())
	{
		glBindBuffer(GL_ARRAY_BUFFER, mNormalId);
		glBufferData(GL_ARRAY_BUFFER, mNormals.size() * sizeof(glm::vec3), &mNormals[0], GL_DYNAMIC_DRAW);

		if (!mInitialiazed[IN_NORMALS])
		{
			glEnableVertexAttribArray(IN_NORMALS);
			mInitialiazed[IN_NORMALS] = true;
		}
	}
	else if (mInitialiazed[IN_NORMALS])
	{
		//if the array is EMPTY, and it IS initialized, then remove it
		glBindBuffer(GL_ARRAY_BUFFER, mNormalId);
		glDisableVertexAttribArray(IN_NORMALS);
		mInitialiazed[IN_NORMALS] = false;
	}


	if (mUVCoords.size())
	{
		glBindBuffer(GL_ARRAY_BUFFER, mUVCoordId);
		glBufferData(GL_ARRAY_BUFFER, mUVCoords.size() * sizeof(glm::vec2), &mUVCoords[0], GL_DYNAMIC_DRAW);

		if (!mInitialiazed[IN_UVCOORDS])
		{
			glEnableVertexAttribArray(IN_UVCOORDS);

			mInitialiazed[IN_UVCOORDS] = true;
		}
	}
	else if (mInitialiazed[IN_UVCOORDS])
	{
		//if the array is EMPTY, and it IS initialized, then remove it
		glBindBuffer(GL_ARRAY_BUFFER, mUVCoordId);
		glDisableVertexAttribArray(IN_UVCOORDS);

		mInitialiazed[IN_UVCOORDS] = false;
	}

}

void GLUFVertexArrayObject::EnableActive()
{
	if (mInitialiazed[IN_POSITIONS])
		glEnableVertexAttribArray(IN_POSITIONS);
	if (mInitialiazed[IN_NORMALS])
		glEnableVertexAttribArray(IN_NORMALS);
	if (mInitialiazed[IN_UVCOORDS])
		glEnableVertexAttribArray(IN_UVCOORDS);

}

void GLUFVertexArrayObject::DisableAll()
{
	glDisableVertexAttribArray(IN_POSITIONS);
	glDisableVertexAttribArray(IN_NORMALS);
	glDisableVertexAttribArray(IN_UVCOORDS);
}

GLUFVertexArrayObject::~GLUFVertexArrayObject()
{
	glDeleteVertexArrays(1, &mVAOId);
	GLuint mBuffIds[4] = { mPositionId, mNormalId, mUVCoordId, mIndexId };
	glDeleteBuffers(4, mBuffIds);
}

GLUFVertexArrayObject::GLUFVertexArrayObject(std::vector<GLUFBufferType> BufferInformation)
{
	mInitialiazed[0] = false;
	mInitialiazed[1] = false;
	mInitialiazed[2] = false;

	glGenVertexArrays(1, &mVAOId);
	glBindVertexArray(mVAOId);//THIS WAS A HUGE THING TO MISS, THE VAO MUST BE BOUND

	glGenBuffers(1, &mPositionId);
	glGenBuffers(1, &mNormalId);
	glGenBuffers(1, &mUVCoordId);
	glGenBuffers(1, &mIndexId);

	glBindBuffer(GL_ARRAY_BUFFER, mPositionId);
	glVertexAttribPointer(IN_POSITIONS, 3, GL_FLOAT, GL_FALSE, 0, NULL);

	glBindBuffer(GL_ARRAY_BUFFER, mNormalId);
	glVertexAttribPointer(IN_NORMALS, 3, GL_FLOAT, GL_FALSE, 0, NULL);

	glBindBuffer(GL_ARRAY_BUFFER, mUVCoordId);
	glVertexAttribPointer(IN_UVCOORDS, 2, GL_FLOAT, GL_FALSE, 0, NULL);

	glBindBuffer(GL_ARRAY_BUFFER, 0);

	//we only want to enable vertex arrays if there are ITEMS IN THE BUFFERS
}

void GLUFUniformBuffer::RefreshTransformData()
{
	glBindBufferBase(GL_UNIFORM_BUFFER, GLUF_UNIVERSAL_TRANSFORM_UNIFORM_BLOCK_LOCATION, mTransformBufferId);
	glm::mat4 *pData = (glm::mat4*)glMapBufferRange(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4) * 5, GL_MAP_WRITE_BIT);
	pData[UT_MODEL] = mTransformData[UT_MODEL];
	pData[UT_VIEW] = mTransformData[UT_VIEW];
	pData[UT_PROJ] = mTransformData[UT_PROJ];

	pData[UT_MODELVIEW] = mTransformData[UT_MODELVIEW];
	pData[UT_MVP] = mTransformData[UT_MVP];

	glUnmapBuffer(GL_UNIFORM_BUFFER);
}

///////////////////////////////
//Shader code for uniform block
//
//NOTE: All will be provided unless necisary ( MAYBE FOR NOW )
//
//
/*

layout(std140, binding = 0) uniform MatrixTransformations
{						//base alignment			offset			aligned offset
	mat4 m;				//16						0					0
	mat4 v;				//16						64					64
	mat4 p;				//16						128					128
	mat4 mv;			//16						192					192
	mat4 mvp;			//16						256					256
};

//NOTE: vertex shader input
position -> 0
normal   -> 1
texCoord -> 2



////////////////////////////
//
//Buffer Stuff
///////////////////////////


void GLUFBufferManager::ResetBufferBindings()
{
	glBindVertexArray(0);
	glBindBuffer(GL_VERTEX_ARRAY, 0);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
	glBindBufferBase(GL_UNIFORM_BUFFER, GLUF_UNIVERSAL_TRANSFORM_UNIFORM_BLOCK_LOCATION, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindTexture(GL_TEXTURE_2D, 0);
	//TODO: expand this
}

GLUFVertexArrayPtr GLUFBufferManager::CreateVertexArray(std::vector<GLUFBufferType> DataTypes)
{
	return GLUFVertexArrayPtr(new GLUFVertexArrayObject(DataTypes));
}

GLUFUniformBufferPtr GLUFBufferManager::CreateUniformArray()
{
	return GLUFUniformBufferPtr(new GLUFUniformBuffer);
}

void GLUFBufferManager::DeleteVertexBuffer(GLUFVertexArrayPtr vertArray)
{
	delete vertArray.get();
}

void GLUFBufferManager::DeleteUniformBuffer(GLUFUniformBufferPtr buffer)
{
	delete buffer.get();
}

void GLUFBufferManager::DrawVertexArray(GLUFVertexArrayPtr vertArray, GLenum topology)
{
	glBindVertexArray(vertArray->mVAOId);
	vertArray->EnableActive();
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vertArray->mIndexId);
	glDrawElements(topology, vertArray->mIndicies.size(), GL_UNSIGNED_SHORT, 0);
	vertArray->DisableAll();
	//glDrawArrays(topology, 0, 3);
}

void GLUFBufferManager::DrawVertexArrayInstanced(GLUFVertexArrayPtr vertArray, GLuint instanceCount, GLenum topology)
{
	glBindVertexArray(vertArray->mVAOId);
	glDrawElementsInstanced(topology, vertArray->mPositions.size(), GL_UNSIGNED_SHORT, 0, instanceCount);
}

void GLUFBufferManager::BindUniformArray(GLUFUniformBufferPtr buffer)
{
	//TODO: setup with material and transform data
	glBindBufferBase(GL_UNIFORM_BUFFER, GLUF_UNIVERSAL_TRANSFORM_UNIFORM_BLOCK_LOCATION, buffer->mTransformBufferId);
}

void GLUFBufferManager::ModifyVertexArray(GLUFVertexArrayPtr vertArray, GLUFBufferType type, void* data, int ElementCount)
{
	//when modifying, change the local copy as well
	signed char* rawData = (signed char*)malloc(ElementCount * type.BytePerElement * type.ElementsPerValue);

	if (!data)
	{
		GLUF_ERROR("Attempt to modify vertex data with empty array");
		return;
	}

	/*
	switch (type)
	{
	case VAO_POSITIONS:
		glBindBuffer(GL_ARRAY_BUFFER, vertArray->mPositionId);
		vertArray->mPositions = data;
		break;
	case VAO_NORMALS:
		glBindBuffer(GL_ARRAY_BUFFER, vertArray->mNormalId);
		vertArray->mNormals = data;
		break;
	default:
		GLUF_ERROR("Unsupported Vertex Attribute Type for this Method");
	}

	glBindBuffer()
	glBufferData(GL_ARRAY_BUFFER, data.size() * 3 * sizeof(GLfloat), &data[0], GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

/*void GLUFBufferManager::ModifyVertexArray(GLUFVertexArrayPtr vertArray, GLUFVertexAttributeType type, Vec2Array data)
{
	if (!data.size())
	{
		GLUF_ERROR("Attempt to modify vertex data with empty array");
		return;
	}

	switch (type)
	{
	case VAO_TEXCOORDS:
		glBindBuffer(GL_ARRAY_BUFFER, vertArray->mUVCoordId);
		vertArray->mUVCoords = data;
		break;
	default:
		GLUF_ERROR("Unsupported Vertex Attribute Type for this Method");
	}

	glBufferData(GL_ARRAY_BUFFER, data.size() * 2 * sizeof(GLfloat), &data[0], GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void GLUFBufferManager::MoidfyVertexIncidies(GLUFVertexArrayPtr vertArray, IndexArray data)
{
	if (!data.size())
	{
		GLUF_ERROR("Attempt to modify vertex data with empty array");
		return;
	}

	vertArray->mIndicies = data;
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vertArray->mIndexId);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, data.size() * sizeof(GLushort), &data[0], GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void GLUFBufferManager::ModifyVertexArray(GLUFVertexArrayPtr vertArray, Vec3Array positions, Vec3Array normals, Vec2Array texCoords, IndexArray indicies)
{
	vertArray->mIndicies = indicies;
	vertArray->mPositions = positions;
	vertArray->mNormals = normals;
	vertArray->mUVCoords = texCoords;

	vertArray->Refresh();
}

GLUFVAOData GLUFBufferManager::MapVertexArray(GLUFVertexArrayPtr vertArray)
{
	return GLUFVAOData(&vertArray->mPositions, &vertArray->mNormals, &vertArray->mUVCoords, &vertArray->mIndicies);
}

void GLUFBufferManager::UnMapVertexArray(GLUFVertexArrayPtr vertArray)
{
	vertArray->Refresh();
}

void GLUFBufferManager::ModifyUniformTransformMatrix(GLUFUniformBufferPtr buffer, GLUFTransformUniformType type, glm::mat4 data)
{
	//make sure the data can be accessed
	if (!buffer->mTransformData.ModifyValue(type, data))
		return;
	buffer->RefreshTransformData();//this uses a bit more memory, but is centrailized
}

void GLUFBufferManager::ModifyUniformTransformMatrix(GLUFUniformBufferPtr buffer, GLUFMatrixTransformBlockParam data)
{
	//make sure the data can be accessed
	buffer->mTransformData = data;
	buffer->RefreshTransformData();
}

GLUFMatrixTransformBlock GLUFBufferManager::MapUniformTransform(GLUFUniformBufferPtr buffer)
{
	return buffer->mTransformData;
}

void GLUFBufferManager::UnMapUniformTransform(GLUFUniformBufferPtr buffer)
{
	buffer->RefreshTransformData();
}

GLuint GLUFBufferManager::GetUniformTransformBufferId(GLUFUniformBufferPtr buffer)
{
	return buffer->mTransformBufferId;
}

GLUFTexturePtr GLUFBufferManager::CreateTextureBuffer()
{
	return GLUFTexturePtr(new GLUFTextureBuffer);
}

GLUFTexturePtr GLUFBufferManager::CreateTextureBuffer(GLuint glTexture)
{
	GLUFTexturePtr ptr(new GLUFTextureBuffer);
	ptr->mTextureId = glTexture;
	return ptr;
}

void GLUFBufferManager::UseTexture(GLUFTexturePtr texture, GLuint location, GLenum bindingPoint)
{
	glActiveTexture(bindingPoint);
	glBindTexture(GL_TEXTURE_2D, texture->mTextureId);
#pragma warning(disable : 4244)
	glUniform1f(location, bindingPoint);
#pragma warning(default : 4244)
}

GLuint GLUFBufferManager::GetTextureBufferId(GLUFTexturePtr texture)
{
	return texture->mTextureId;
}

bool GLUFBufferManager::CompareTextures(GLUFTexturePtr texture, GLUFTexturePtr texture1)
{
	return (texture->mTextureId == texture1->mTextureId);
}

GLuint GLUF::LoadTextureFromFile(std::wstring filePath, GLUFTextureFileFormat format)
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


GLuint GLUF::LoadTextureFromMemory(char* data, unsigned int length, GLUFTextureFileFormat format)
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
}*/

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
// ObjGLUFUF.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#define USING_ASSIMP
#include "ObjGLUF.h"
#include <fstream>
#include <sstream>
#include <GLFW/glfw3.h>

namespace GLUF
{

GLUFErrorMethod ErrorMethod;
//GLUFBufferManager g_BufferManager;
GLUFShaderManager g_ShaderManager;

//initialize the standard vertex attributes
//							Name				bytes,	count,	location,				type
const GLUFVertexAttribInfo	g_attribPOS		= { 4,		3,		GLUF_VERTEX_ATTRIB_POSITION,GL_FLOAT };
const GLUFVertexAttribInfo	g_attribNORM	= { 4,		3,		GLUF_VERTEX_ATTRIB_NORMAL,	GL_FLOAT };
const GLUFVertexAttribInfo	g_attribUV0		= { 4,		2,		GLUF_VERTEX_ATTRIB_UV0,		GL_FLOAT };
const GLUFVertexAttribInfo	g_attribUV1		= { 4,		2,		GLUF_VERTEX_ATTRIB_UV1,		GL_FLOAT };
const GLUFVertexAttribInfo	g_attribUV2		= { 4,		2,      GLUF_VERTEX_ATTRIB_UV2,		GL_FLOAT };
const GLUFVertexAttribInfo	g_attribUV3		= { 4,		2,		GLUF_VERTEX_ATTRIB_UV3,		GL_FLOAT };
const GLUFVertexAttribInfo	g_attribUV4		= { 4,		2,		GLUF_VERTEX_ATTRIB_UV4,		GL_FLOAT };
const GLUFVertexAttribInfo	g_attribUV5		= { 4,		2,		GLUF_VERTEX_ATTRIB_UV5,		GL_FLOAT };
const GLUFVertexAttribInfo	g_attribUV6		= { 4,		2,		GLUF_VERTEX_ATTRIB_UV6,		GL_FLOAT };
const GLUFVertexAttribInfo	g_attribUV7		= { 4,		2,		GLUF_VERTEX_ATTRIB_UV7,		GL_FLOAT };
const GLUFVertexAttribInfo	g_attribCOLOR0	= { 4,		4,		GLUF_VERTEX_ATTRIB_COLOR0,	GL_FLOAT };
const GLUFVertexAttribInfo	g_attribCOLOR1	= { 4,		4,		GLUF_VERTEX_ATTRIB_COLOR1,	GL_FLOAT };
const GLUFVertexAttribInfo	g_attribCOLOR2	= { 4,		4,		GLUF_VERTEX_ATTRIB_COLOR2,	GL_FLOAT };
const GLUFVertexAttribInfo	g_attribCOLOR3	= { 4,		4,		GLUF_VERTEX_ATTRIB_COLOR3,	GL_FLOAT };
const GLUFVertexAttribInfo	g_attribCOLOR4	= { 4,		4,		GLUF_VERTEX_ATTRIB_COLOR4,	GL_FLOAT };
const GLUFVertexAttribInfo	g_attribCOLOR5	= { 4,		4,		GLUF_VERTEX_ATTRIB_COLOR5,	GL_FLOAT };
const GLUFVertexAttribInfo	g_attribCOLOR6	= { 4,		4,		GLUF_VERTEX_ATTRIB_COLOR6,	GL_FLOAT };
const GLUFVertexAttribInfo	g_attribCOLOR7	= { 4,		4,		GLUF_VERTEX_ATTRIB_COLOR7,	GL_FLOAT };
const GLUFVertexAttribInfo	g_attribTAN		= { 4,		3,		GLUF_VERTEX_ATTRIB_TAN,		GL_FLOAT };
const GLUFVertexAttribInfo	g_attribBITAN	= { 4,		3,		GLUF_VERTEX_ATTRIB_BITAN,	GL_FLOAT };


GLUFVertexAttribMap g_stdAttrib;

GLuint g_GLVersionMajor = 0;
GLuint g_GLVersionMinor = 0;

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

double g_LastFrame = 0.0;
double g_UpdateInterval = 1.0;//every second
long long  g_FrameCount = 0;//this is since the last update
float g_CurrFps = 0.0f;
wchar_t* g_FrameStats = new wchar_t[MAXLEN];

void GLUFStats_func()
{
	++g_FrameCount;

	double thisFrame = GLUFGetTime();
	if (thisFrame - g_LastFrame < g_UpdateInterval)
		return;//don't update statistics

	double deltaTime = thisFrame - g_LastFrame;

	g_CurrFps = (float)((long double)g_FrameCount / (long double)deltaTime);

	g_FrameCount = 0;//reset the frame count

	//update frame statistic string
	memset(g_FrameStats, 0, MAXLEN);
	swprintf_s(g_FrameStats, MAXLEN, L"%0.2f fps", g_CurrFps);

	//update device statistics
	//TODO:

	g_LastFrame = thisFrame;
}

const wchar_t* GLUFGetFrameStats()
{
	return g_FrameStats;
}

const wchar_t* GLUFGetDeviceStats()
{
	return L"WIP";
}

bool GLUFInitOpenGLExtentions()
{
	GLenum err = glewInit();
	if (err != GLEW_OK)
	{
		GLUF_ERROR("Failed to initialize OpenGL Extensions using GLEW");
		return false;
	}


	//setup global openGL version
	const char* version = (const char*)glGetString(GL_VERSION);
	
	std::vector<std::string> vsVec;
	vsVec = GLUFSplitStr((const char*)version, L'.');//TODO: global openGL version
	g_GLVersionMajor = std::stoi(vsVec[0]);
	g_GLVersionMinor = std::stoi(vsVec[1]);

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

char* GLUFLoadFileIntoMemory(const wchar_t* path, unsigned long* rawSize)
{
	GLUF_ASSERT(path);
	//GLUF_ASSERT(rawSize);
	if (!rawSize)
		rawSize = new unsigned long;

	std::ifstream inFile(path, std::ios::binary);
	if (!inFile)
		return false;

	inFile.seekg(0, std::ios::end);
	*rawSize = (unsigned long)inFile.tellg();
	inFile.seekg(0, std::ios::beg);

	char* rawData = (char*)malloc(*rawSize);
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

long GLUFLoadFileIntoMemory(const wchar_t* path, char* buffer, long len)
{
	GLUF_ASSERT(path);
	GLUF_ASSERT(buffer);

	std::ifstream inFile(path, std::ios::binary);
	if (!inFile)
		return -1;
	
	std::streamsize streamLen = std::streamsize(len);
	if (len == -1)
	{
		inFile.seekg(0, std::ios::end);
		streamLen = inFile.tellg();
		inFile.seekg(0, std::ios::beg);
	}

	//if (sizeof(rawData) != *rawSize)
	//	return false;

	if (!inFile.read(buffer, streamLen))
	{
		GLUF_ERROR("Failed to load file into memory");
	}

	return (unsigned long)streamLen;
}

char* GLUFLoadFileIntoMemory(const char* path, unsigned long* rawSize)
{
	GLUF_ASSERT(path);
	GLUF_ASSERT(rawSize);

	std::ifstream inFile(path, std::ios::binary);
	if (!inFile)
		return false;

	inFile.seekg(0, std::ios::end);
	*rawSize = (unsigned long)inFile.tellg();
	inFile.seekg(0, std::ios::beg);

	char* rawData = new char[*rawSize];
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

long GLUFLoadFileIntoMemory(const char* path, char* buffer, long len)
{
	GLUF_ASSERT(path);
	GLUF_ASSERT(buffer);

	std::ifstream inFile(path, std::ios::binary);
	if (!inFile)
		return -1;

	std::streamsize streamLen = std::streamsize(len);
	if (len == -1)
	{
		inFile.seekg(0, std::ios::end);
		streamLen = inFile.tellg();
		inFile.seekg(0, std::ios::beg);
	}

	//if (sizeof(rawData) != *rawSize)
	//	return false;

	if (!inFile.read(buffer, streamLen))
	{
		GLUF_ERROR("Failed to load file into memory");
	}

	return len;
}

std::string GLUFLoadBinaryArrayIntoString(char* data, long len)
{
	std::istream indata(new MemStreamBuf(data, len));
	char ch = ' ';

	std::string inString;
	while (indata.get(ch))
		inString += ch;

	return inString;
}


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

const glm::mat4& GLUFMatrixStack::Top(void)
{
	//if it is empty, then we want to return the identity
	if (mStack.size() == 0)
		return mIdentity;

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


#define FOURCC_DXT1 0x31545844 // Equivalent to "DXT1" in ASCII
#define FOURCC_DXT3 0x33545844 // Equivalent to "DXT3" in ASCII
#define FOURCC_DXT5 0x35545844 // Equivalent to "DXT5" in ASCII

GLuint LoadTextureDDS(char* rawData, unsigned int size)
{
	//TODO support more compatibiulity, ie RGB, BGR, don't make it dependent on ABGR
	unsigned char header[124];


	/* verify the type of file */
	char filecode[4];
	memcpy(filecode, rawData, 4);
	if (strncmp(filecode, "DDS ", 4) != 0) 
	{
		//fclose(fp);
		return 0;
	}

	/* get the surface desc */
	memcpy(header, rawData + 4/*don't forget to add the offset*/, 124);

	//this is all the data I need, but I just need to load it properly to opengl
	unsigned int height =		*(unsigned int*)&(header[8]);
	unsigned int width =		*(unsigned int*)&(header[12]);
	unsigned int linearSize =	*(unsigned int*)&(header[16]);
	unsigned int mipMapCount =	*(unsigned int*)&(header[24]);
	unsigned int flags =		*(unsigned int*)&(header[76]);
	unsigned int fourCC =		*(unsigned int*)&(header[80]);
	unsigned int RGBBitCount =	*(unsigned int*)&(header[84]);
	unsigned int RBitMask =		*(unsigned int*)&(header[88]);
	unsigned int GBitMask =		*(unsigned int*)&(header[92]);
	unsigned int BBitMask =		*(unsigned int*)&(header[96]);
	unsigned int ABitMask =		*(unsigned int*)&(header[100]);


	char * buffer;
	unsigned int bufsize;
	/* how big is it going to be including all mipmaps? */
	bufsize = mipMapCount > 1 ? linearSize * 2 : linearSize;
	buffer = rawData + 128;
	//memcpy(buffer, rawData + 128/*header size + filecode size*/, bufsize);

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
		unsigned int offset = 0;

		/* load the mipmaps */
		for (unsigned int level = 0; level < mipMapCount && (width || height); ++level)
		{
			unsigned int mipSize = ((width + 3) / 4)*((height + 3) / 4)*blockSize;
			glCompressedTexImage2D(GL_TEXTURE_2D, level, compressedFormat, width, height,
				0, mipSize, buffer + offset);

			offset += mipSize;
			width /= 2;
			height /= 2;

			// Deal with Non-Power-Of-Two textures. This code is not included in the webpage to reduce clutter.
			if (width < 1) width = 1;
			if (height < 1) height = 1;

		}

	}
	else
	{
		unsigned int offset = 0;

		for (unsigned int level = 0; level < mipMapCount && (width || height); ++level)
		{
			glTexImage2D(GL_TEXTURE_2D, level, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, buffer + offset);

			unsigned int mipSize = (width * height * 4);
			offset += mipSize;
			width /= 2;
			height /= 2;

			// Deal with Non-Power-Of-Two textures. This code is not included in the webpage to reduce clutter.
			if (width < 1) width = 1;
			if (height < 1) height = 1;
		}
	}


	return textureID;
}

GLuint LoadTextureCubemapDDS(char* rawData, unsigned int length)
{
	unsigned char header[124];


	/* verify the type of file */
	char filecode[4];
	memcpy(filecode, rawData, 4);
	if (strncmp(filecode, "DDS ", 4) != 0)
	{
		//fclose(fp);
		return 0;
	}

	/* get the surface desc */
	memcpy(header, rawData + 4/*don't forget to add the offset*/, 124);

	//this is all the data I need, but I just need to load it properly to opengl
	unsigned int height = *(unsigned int*)&(header[8]);
	unsigned int width = *(unsigned int*)&(header[12]);
	unsigned int linearSize = *(unsigned int*)&(header[16]);
	unsigned int mipMapCount = *(unsigned int*)&(header[24]);
	unsigned int flags = *(unsigned int*)&(header[76]);
	unsigned int fourCC = *(unsigned int*)&(header[80]);
	unsigned int RGBBitCount = *(unsigned int*)&(header[84]);
	unsigned int RBitMask = *(unsigned int*)&(header[88]);
	unsigned int GBitMask = *(unsigned int*)&(header[92]);
	unsigned int BBitMask = *(unsigned int*)&(header[96]);
	unsigned int ABitMask = *(unsigned int*)&(header[100]);


	char * buffer;
	unsigned int bufsize;
	/* how big is it going to be including all mipmaps? */
	bufsize = mipMapCount > 1 ? linearSize * 2 : linearSize;
	buffer = rawData + 128;
	//memcpy(buffer, rawData + 128/*header size + filecode size*/, bufsize);

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
		unsigned int offset = 0;

		unsigned int pertexSize = width;

		unsigned int mipSize = ((pertexSize + 3) / 4)*((pertexSize + 3) / 4)*blockSize;
		glCompressedTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 0, compressedFormat, pertexSize, pertexSize,
			0, mipSize, buffer + offset);
		offset += mipSize;
		glCompressedTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, 0, compressedFormat, pertexSize, pertexSize,
			0, mipSize, buffer + offset);
		offset += mipSize;
		glCompressedTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, compressedFormat, pertexSize, pertexSize,
			0, mipSize, buffer + offset);
		offset += mipSize;
		glCompressedTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, 0, compressedFormat, pertexSize, pertexSize,
			0, mipSize, buffer + offset);
		offset += mipSize;
		glCompressedTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, 0, compressedFormat, pertexSize, pertexSize,
			0, mipSize, buffer + offset);
		offset += mipSize;
		glCompressedTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 0, compressedFormat, pertexSize, pertexSize,
			0, mipSize, buffer + offset);

	}
	else
	{
		unsigned int offset = 0;
		unsigned int pertexSize = width;
		unsigned int mipSize = (pertexSize * pertexSize * 4);

		glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 0, GL_RGBA, pertexSize, pertexSize, 0, GL_RGBA, GL_UNSIGNED_BYTE, buffer + offset);
		offset += mipSize;
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, 0, GL_RGBA, pertexSize, pertexSize, 0, GL_RGBA, GL_UNSIGNED_BYTE, buffer + offset);
		offset += mipSize;
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, GL_RGBA, pertexSize, pertexSize, 0, GL_RGBA, GL_UNSIGNED_BYTE, buffer + offset);
		offset += mipSize;
		glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, 0, GL_RGBA, pertexSize, pertexSize, 0, GL_RGBA, GL_UNSIGNED_BYTE, buffer + offset);
		offset += mipSize;
		glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, 0, GL_RGBA, pertexSize, pertexSize, 0, GL_RGBA, GL_UNSIGNED_BYTE, buffer + offset);
		offset += mipSize;
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 0, GL_RGBA, pertexSize, pertexSize, 0, GL_RGBA, GL_UNSIGNED_BYTE, buffer + offset);
	}


	return textureID;
}

GLuint LoadTextureFromFile(std::wstring filePath, GLUFTextureFileFormat format)
{
	unsigned long rawSize = 0;
	char* data = GLUFLoadFileIntoMemory(filePath.c_str(), &rawSize);

	GLuint texId = LoadTextureFromMemory(data, rawSize, format);
	free(data);
	return texId;
}


GLuint LoadTextureFromMemory(char* data, unsigned int length, GLUFTextureFileFormat format)
{

	switch (format)
	{
	case TFF_DDS:
		return LoadTextureDDS(data, length);
	case TTF_DDS_CUBEMAP:
		return LoadTextureCubemapDDS(data, length);
	}

	return 0;
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

	//GLuint mUniformBuffId;
	GLuint mProgramId;
	std::map<GLUFShaderType, GLUFShaderPtr > mShaderBuff;
	GLUFVariableLocMap mAttributeLocations;
	GLUFVariableLocMap mUniformLocations;

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
	~GLUFSeperateProgram();

	void Init();
	
	void AttachProgram(GLUFProgramPtr program, GLbitfield stages);

};

////////////////////////////////////////
//
//GLUFSeperateProgram Methods:
//
//

GLUFSeperateProgram::~GLUFSeperateProgram()
{
	glDeleteProgramPipelines(1, &mPPOId); 
}

void GLUFSeperateProgram::Init()
{
	glGenProgramPipelines(1, &mPPOId);
}

void GLUFSeperateProgram::AttachProgram(GLUFProgramPtr program, GLbitfield stages)
{
	m_Programs.insert(GLUFProgramPtrStagesPair(stages, program)); 
	glUseProgramStages(mPPOId, stages, program->GetId());
}

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

	mTmpShaderText.append(shaderText);
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

	MemStreamBuf *streamData = new MemStreamBuf(shaderData, length);
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

	GLint tmpSize = (GLuint)mTmpShaderText.length();
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
	//glGenBuffers(1, &mUniformBuffId);
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
		
		//Load the variable names
		GLint attribCount = 0;
		glGetProgramiv(mProgramId, GL_ACTIVE_ATTRIBUTES, &attribCount);

		GLint maxLength;
		glGetProgramiv(mProgramId, GL_ACTIVE_ATTRIBUTE_MAX_LENGTH, &maxLength);

		GLenum type;

		GLint written, size;
		for (int i = 0; i < attribCount; ++i)
		{
			GLchar* data = (GLchar*)malloc(maxLength);

			glGetActiveAttrib(mProgramId, i, maxLength, &written, &size, &type, data);
			mAttributeLocations.insert(GLUFVariableLocPair(data, glGetAttribLocation(mProgramId, data)));

			free(data);
		}

		GLint uniformCount = 0;
		glGetProgramiv(mProgramId, GL_ACTIVE_UNIFORMS, &uniformCount);

		for (int i = 0; i < uniformCount; ++i)
		{
			GLchar* data = (GLchar*)malloc(maxLength);

			glGetActiveUniform(mProgramId, i, maxLength, &written, &size, &type, data);
			mUniformLocations.insert(GLUFVariableLocPair(data, glGetUniformLocation(mProgramId, data)));

			free(data);
		}
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

GLUFShaderPtr GLUFShaderManager::CreateShaderFromText(const char* str, GLUFShaderType type)
{
	//return CreateShader(text, type, false);
	GLUFShaderPtr shader(new GLUFShader());
	shader->Init(type);

	shader->Load(str);

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

GLUFShaderPtr GLUFShaderManager::CreateShaderFromMemory(char* data, long len, GLUFShaderType type)
{
	return CreateShaderFromText((GLUFLoadBinaryArrayIntoString(data, len) + "\n").c_str(), type);
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
		shaders.push_back(CreateShaderFromText(const_cast<char*>(it.second), it.first));

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

/*GLUFSepProgramPtr GLUFShaderManager::CreateSeperateProgram(GLUFProgramPtrStagesMap programs)
{
	GLUFSepProgramPtr ret(new GLUFSeperateProgram);
	ret->Init();

	for (auto it : programs)
	{
		ret->AttachProgram(it.second, it.first);
	}
	return ret;
}*/

GLuint GLUFShaderManager::GetShaderVariableLocation(GLUFProgramPtr prog, GLUFLocationType locType, std::string varName)
{
	std::map<std::string, GLuint>::iterator it;

	if (locType == GLT_ATTRIB)
	{
		it = prog->mAttributeLocations.find(varName);
	}
	else
	{
		it = prog->mUniformLocations.find(varName);
	}

	return it->second;
	/*if (it)
		return it->second;
	else
		return 0;//make better*/
}

GLUFVariableLocMap GLUFShaderManager::GetShaderAttribLocations(GLUFProgramPtr prog)
{
	return prog->mAttributeLocations;
}

GLUFVariableLocMap GLUFShaderManager::GetShaderUniformLocations(GLUFProgramPtr prog)
{
	return prog->mUniformLocations;
}

GLUFVariableLocMap GLUFShaderManager::GetShaderAttribLocations(GLUFSepProgramPtr prog)
{
	GLUFVariableLocMap ret;

	for (auto it : prog->m_Programs)
	{
		ret.insert(it.second->mAttributeLocations.begin(), it.second->mAttributeLocations.end());
	}

	return ret;
}

GLUFVariableLocMap GLUFShaderManager::GetShaderUniformLocations(GLUFSepProgramPtr prog)
{
	GLUFVariableLocMap ret;

	for (auto it : prog->m_Programs)
	{
		ret.insert(it.second->mUniformLocations.begin(), it.second->mUniformLocations.end());
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
	glUseProgram(0);
	glBindProgramPipeline(program->mPPOId);
}





GLUFVertexArrayBase::GLUFVertexArrayBase(GLenum PrimType, GLenum buffUsage, bool index) : mUsageType(buffUsage), mPrimitiveType(PrimType)
{
	glGenVertexArrayBindVertexArray(&mVertexArrayId);

	if (index)
	{
		glGenBuffers(1, &mIndexBuffer);
		glGenBuffers(1, &mRangedIndexBuffer);
	}
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

void GLUFVertexArrayBase::BufferIndices(std::vector<glm::u32vec2> indices)
{
	BufferIndices(&indices[0][0], indices.size() * 2);
}

void GLUFVertexArrayBase::BufferIndices(std::vector<glm::u32vec3> indices)
{
	BufferIndices(&indices[0][0], indices.size() * 3);
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

void GLUFVertexArrayBase::DrawRange(GLuint start, GLuint count)
{
	BindVertexArray();
	EnableVertexAttributes();

	if (mIndexBuffer != 0)
	{
		//copy the range over an draw
		glBindBuffer(GL_COPY_READ_BUFFER, mIndexBuffer);
		glBindBuffer(GL_COPY_WRITE_BUFFER, mRangedIndexBuffer);

		glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, sizeof(GLuint) * start, 0, sizeof(GLuint) * count);
		
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mRangedIndexBuffer);
		glDrawElements(mPrimitiveType, mIndexCount, GL_UNSIGNED_INT, nullptr);
	}
	else
	{
		glDrawArrays(mPrimitiveType, (start < 0 || start > mVertexCount) ? 0 : start, (count > mVertexCount) ? mVertexCount : count);
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

GLUFMeshBarebones GLUFVertexArraySoA::GetBarebonesMesh()
{
	BindVertexArray();

	std::map<GLUFAttribLoc, GLuint>::iterator it = mDataBuffers.find(GLUF_VERTEX_ATTRIB_POSITION);
	if (mIndexBuffer == 0 || it == mDataBuffers.end())
	{
		return GLUFMeshBarebones();
	}

	GLUFMeshBarebones ret;
	
	glBindBuffer(GL_ARRAY_BUFFER, it->second);
	glm::vec3* pVerts = (glm::vec3*)glMapBuffer(GL_ARRAY_BUFFER, GL_READ_ONLY);

	ret.mVertices = GLUFArrToVec(pVerts, mVertexCount);
	glUnmapBuffer(GL_ARRAY_BUFFER);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIndexBuffer);
	GLuint* pIndices = (GLuint*)glMapBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_READ_ONLY);
	ret.mIndices = GLUFArrToVec(pIndices, mIndexCount);

	return ret;
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

	//const aiMesh* mesh = scene->mMeshes[meshNum];

	GLUFVertexArray* arr = LoadVertexArrayFromScene(scene, g_stdAttrib, meshNum);
	if (!arr)
		return nullptr;

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

GLUFVertexArray* LoadVertexArrayFromScene(const aiScene* scene, GLUFVertexAttribMap inputs, unsigned int meshNum)
{
	if (meshNum > scene->mNumMeshes)
		return false;

	const aiMesh* mesh = scene->mMeshes[meshNum];

	GLUFVertexArray* vertexData = new GLUFVertexArray(GL_TRIANGLES, GL_STATIC_DRAW, mesh->HasFaces());

	GLUFVertexAttribMap::iterator it, itPos, itNorm, itUV0, itUV1, itUV2, itUV3, itUV4, itUV5, itUV6, itUV7, itCol0, itCol1, itCol2, itCol3, itCol4, itCol5, itCol6, itCol7, itTan, itBitan;

	it = itPos = inputs.find(GLUF_VERTEX_ATTRIB_POSITION);
	if (mesh->HasPositions() && it != inputs.end())
		vertexData->AddVertexAttrib(it->second);

	it = itNorm = inputs.find(GLUF_VERTEX_ATTRIB_NORMAL);
	if (mesh->HasNormals() && it != inputs.end())
		vertexData->AddVertexAttrib(it->second);

	it = itUV0 = inputs.find(GLUF_VERTEX_ATTRIB_UV0);
	if (mesh->HasTextureCoords(0) && it != inputs.end())
		vertexData->AddVertexAttrib(it->second);

	it = itUV1 = inputs.find(GLUF_VERTEX_ATTRIB_UV1);
	if (mesh->HasTextureCoords(1) && it != inputs.end())
		vertexData->AddVertexAttrib(it->second);

	it = itUV2 = inputs.find(GLUF_VERTEX_ATTRIB_UV2);
	if (mesh->HasTextureCoords(2) && it != inputs.end())
		vertexData->AddVertexAttrib(it->second);

	it = itUV3 = inputs.find(GLUF_VERTEX_ATTRIB_UV3);
	if (mesh->HasTextureCoords(3) && it != inputs.end())
		vertexData->AddVertexAttrib(it->second);

	it = itUV4 = inputs.find(GLUF_VERTEX_ATTRIB_UV4);
	if (mesh->HasTextureCoords(4) && it != inputs.end())
		vertexData->AddVertexAttrib(it->second);

	it = itUV5 = inputs.find(GLUF_VERTEX_ATTRIB_UV5);
	if (mesh->HasTextureCoords(5) && it != inputs.end())
		vertexData->AddVertexAttrib(it->second);

	it = itUV6 = inputs.find(GLUF_VERTEX_ATTRIB_UV6);
	if (mesh->HasTextureCoords(6) && it != inputs.end())
		vertexData->AddVertexAttrib(it->second);

	it = itUV7 = inputs.find(GLUF_VERTEX_ATTRIB_UV7);
	if (mesh->HasTextureCoords(7) && it != inputs.end())
		vertexData->AddVertexAttrib(it->second);



	it = itCol0 = inputs.find(GLUF_VERTEX_ATTRIB_COLOR0);
	if (mesh->HasVertexColors(0) && it != inputs.end())
		vertexData->AddVertexAttrib(it->second);

	it = itCol1 = inputs.find(GLUF_VERTEX_ATTRIB_COLOR1);
	if (mesh->HasVertexColors(1) && it != inputs.end())
		vertexData->AddVertexAttrib(it->second);

	it = itCol2 = inputs.find(GLUF_VERTEX_ATTRIB_COLOR2);
	if (mesh->HasVertexColors(2) && it != inputs.end())
		vertexData->AddVertexAttrib(it->second);

	it = itCol3 = inputs.find(GLUF_VERTEX_ATTRIB_COLOR3);
	if (mesh->HasVertexColors(3) && it != inputs.end())
		vertexData->AddVertexAttrib(it->second);

	it = itCol4 = inputs.find(GLUF_VERTEX_ATTRIB_COLOR4);
	if (mesh->HasVertexColors(4) && it != inputs.end())
		vertexData->AddVertexAttrib(it->second);
	
	it = itCol5 = inputs.find(GLUF_VERTEX_ATTRIB_COLOR5);
	if (mesh->HasVertexColors(5) && it != inputs.end())
		vertexData->AddVertexAttrib(it->second);

	it = itCol6 = inputs.find(GLUF_VERTEX_ATTRIB_COLOR6);
	if (mesh->HasVertexColors(6) && it != inputs.end())
		vertexData->AddVertexAttrib(it->second);

	it = itCol7 = inputs.find(GLUF_VERTEX_ATTRIB_COLOR7);
	if (mesh->HasVertexColors(7) && it != inputs.end())
		vertexData->AddVertexAttrib(it->second);


	it = itTan = inputs.find(GLUF_VERTEX_ATTRIB_TAN);
	it = itBitan = inputs.find(GLUF_VERTEX_ATTRIB_BITAN);
	if (it != inputs.end())
	{
		if (mesh->HasTangentsAndBitangents() && itTan != inputs.end())
		{
			vertexData->AddVertexAttrib(itTan->second);
			vertexData->AddVertexAttrib(itBitan->second);
		}
	}


	if (mesh->HasPositions() && itPos != inputs.end())
		vertexData->BufferData(itPos->second.VertexAttribLocation, mesh->mNumVertices, mesh->mVertices);
	if (mesh->HasNormals() && itNorm != inputs.end())
		vertexData->BufferData(itNorm->second.VertexAttribLocation, mesh->mNumVertices, mesh->mNormals);
	if (mesh->HasTextureCoords(0) && itUV0 != inputs.end())
		vertexData->BufferData(itUV0->second.VertexAttribLocation, mesh->mNumVertices, AssimpToGlm3_2(mesh->mTextureCoords[0], mesh->mNumVertices));
	if (mesh->HasTextureCoords(1) && itUV1 != inputs.end())
		vertexData->BufferData(itUV1->second.VertexAttribLocation, mesh->mNumVertices, AssimpToGlm3_2(mesh->mTextureCoords[1], mesh->mNumVertices));
	if (mesh->HasTextureCoords(2) && itUV2 != inputs.end())
		vertexData->BufferData(itUV2->second.VertexAttribLocation, mesh->mNumVertices, AssimpToGlm3_2(mesh->mTextureCoords[2], mesh->mNumVertices));
	if (mesh->HasTextureCoords(3) && itUV3 != inputs.end())
		vertexData->BufferData(itUV3->second.VertexAttribLocation, mesh->mNumVertices, AssimpToGlm3_2(mesh->mTextureCoords[3], mesh->mNumVertices));
	if (mesh->HasTextureCoords(4) && itUV4 != inputs.end())
		vertexData->BufferData(itUV4->second.VertexAttribLocation, mesh->mNumVertices, AssimpToGlm3_2(mesh->mTextureCoords[4], mesh->mNumVertices));
	if (mesh->HasTextureCoords(5) && itUV5 != inputs.end())
		vertexData->BufferData(itUV5->second.VertexAttribLocation, mesh->mNumVertices, AssimpToGlm3_2(mesh->mTextureCoords[5], mesh->mNumVertices));
	if (mesh->HasTextureCoords(6) && itUV6 != inputs.end())
		vertexData->BufferData(itUV6->second.VertexAttribLocation, mesh->mNumVertices, AssimpToGlm3_2(mesh->mTextureCoords[6], mesh->mNumVertices));
	if (mesh->HasTextureCoords(7) && itUV7 != inputs.end())
		vertexData->BufferData(itUV7->second.VertexAttribLocation, mesh->mNumVertices, AssimpToGlm3_2(mesh->mTextureCoords[7], mesh->mNumVertices));

	if (mesh->HasVertexColors(0) && itCol0 != inputs.end())
		vertexData->BufferData(itCol0->second.VertexAttribLocation, mesh->mNumVertices, mesh->mColors[0]);
	if (mesh->HasVertexColors(1) && itCol1 != inputs.end())
		vertexData->BufferData(itCol1->second.VertexAttribLocation, mesh->mNumVertices, mesh->mColors[1]);
	if (mesh->HasVertexColors(2) && itCol2 != inputs.end())
		vertexData->BufferData(itCol2->second.VertexAttribLocation, mesh->mNumVertices, mesh->mColors[2]);
	if (mesh->HasVertexColors(3) && itCol3 != inputs.end())
		vertexData->BufferData(itCol3->second.VertexAttribLocation, mesh->mNumVertices, mesh->mColors[3]);
	if (mesh->HasVertexColors(4) && itCol4 != inputs.end())
		vertexData->BufferData(itCol4->second.VertexAttribLocation, mesh->mNumVertices, mesh->mColors[4]);
	if (mesh->HasVertexColors(5) && itCol5 != inputs.end())
		vertexData->BufferData(itCol5->second.VertexAttribLocation, mesh->mNumVertices, mesh->mColors[5]);
	if (mesh->HasVertexColors(6) && itCol6 != inputs.end())
		vertexData->BufferData(itCol6->second.VertexAttribLocation, mesh->mNumVertices, mesh->mColors[6]);
	if (mesh->HasVertexColors(7) && itCol7 != inputs.end())
		vertexData->BufferData(itCol7->second.VertexAttribLocation, mesh->mNumVertices, mesh->mColors[7]);
	if (mesh->HasTangentsAndBitangents() && itBitan != inputs.end() && itTan != inputs.end())
	{
		vertexData->BufferData(itBitan->second.VertexAttribLocation, mesh->mNumVertices, mesh->mBitangents);
		vertexData->BufferData(itTan->second.VertexAttribLocation, mesh->mNumVertices, mesh->mTangents);
	}

	std::vector<GLuint> indices;
	for (unsigned int i = 0; i < mesh->mNumFaces; ++i)
	{
		aiFace curr = mesh->mFaces[i];
		indices.push_back(curr.mIndices[0]);
		indices.push_back(curr.mIndices[1]);
		indices.push_back(curr.mIndices[2]);
	}
	vertexData->BufferIndices(&indices[0], (unsigned int)indices.size());

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

}
#pragma once

//the first int parameter is the "MessageType" enum
using CallbackFuncPtr = bool(*)(int, int, int, int, int); 
using ErrorMethod = void(*)(const char* message, const char* funcName, const char* sourceFile, unsigned int lineNum);

extern "C"
{
	/*
	
	Helping Structures
	
	*/
	struct GLUFMapElement
	{
		const char* key;
		GLuint value;
	};

	/*
	
	Core ObjGLUF Methods

	*/
	__declspec(dllexport) float __cdecl GLUFGetFps();
	__declspec(dllexport) void __cdecl GLUFStatsFunc();
	
	__declspec(dllexport) int __cdecl GLUFGLVersionMajor();
	__declspec(dllexport) int __cdecl GLUFGLVersionMinor();
	__declspec(dllexport) float __cdecl GLUFGLVersion();

	__declspec(dllexport) void __cdecl GLUFInit();
	__declspec(dllexport) void __cdecl GLUFInitGui();
	__declspec(dllexport) void __cdecl GLUFInitOpenGLExtensions();
	__declspec(dllexport) void __cdecl GLUFTerminate();

	__declspec(dllexport) void __cdecl GLUFChangeCallbackFunc(CallbackFuncPtr newCallback);
	__declspec(dllexport) void __cdecl GLUFRegisterErrorMethod(ErrorMethod errorMethod);

	/*
	
	Buffer Methods
	
	*/
	__declspec(dllexport) GLuint __cdecl GLUFCreateVertexArrayAoS();
	__declspec(dllexport) GLuint __cdecl GLUFCreateVertexArraySoA();

	__declspec(dllexport) void __cdecl GLUFBindVertexArray(GLuint id);
	__declspec(dllexport) void __cdecl GLUFUnBindVertexArray(GLuint id);
	__declspec(dllexport) void __cdecl GLUFBufferIndices(GLuint id, const GLvoid* data, GLuint indexCount);
	__declspec(dllexport) void __cdecl GLUFDraw(GLuint id);
	__declspec(dllexport) void __cdecl GLUFDrawInstanced(GLuint id, GLuint instances);
	__declspec(dllexport) void __cdecl GLUFEnableVertexAttributes();
	__declspec(dllexport) void __cdecl GLUFDisableVertexAttributes();
	__declspec(dllexport) void __cdecl GLUFDrawRange(GLuint id, GLuint start, GLuint count);
	__declspec(dllexport) void __cdecl GLUFAddVertexAttribute(GLuint id, unsigned short bytesPerElement, unsigned short elementsPerValue, GLuint vertexAttribLocation, GLenum type, GLuint offset);
	__declspec(dllexport) void __cdecl GLUFRemoveVertexAttribute(GLuint id, GLuint vertexAttribLocation);

	__declspec(dllexport) void __cdecl GLUFBufferData(GLuint id, GLvoid* data, GLuint size);
	__declspec(dllexport) void __cdecl GLUFBufferSubData(GLuint id, GLvoid* data, GLuint size, GLuint* vertexLocations, GLuint vertexLocationSize);
	//__declspec(dllexport) void __cdecl GLUFResizeBuffer(GLsizei numVertices);


	__declspec(dllexport) GLuint __cdecl GLUFLoadFont(GLvoid* data, GLsizei size);
	__declspec(dllexport) GLuint __cdecl GLUFLoadTextureCubemap(GLvoid* data, GLsizei size);
	__declspec(dllexport) GLuint __cdecl GLUFLoadTextureDDS(GLvoid* data, GLsizei size);

	/*
	
	Shader Methods
	
	*/
	__declspec(dllexport) void __cdecl AttachProgram(GLuint ppo, GLuint program);
	__declspec(dllexport) void __cdecl AttachPrograms(GLuint ppo, GLuint* programs, GLuint size);
	__declspec(dllexport) void __cdecl ClearPrograms(GLuint ppo, GLbitfield stages);
	__declspec(dllexport) GLuint __cdecl CreateProgram(GLuint* shaders, GLuint size, GLboolean separate);
	__declspec(dllexport) GLuint __cdecl CreateSeparateProgram(GLuint* programs, GLuint size);
	__declspec(dllexport) GLuint __cdecl CreateShader(const char* text, GLenum shaderType);
	__declspec(dllexport) void __cdecl DeleteProgram(GLuint program);
	__declspec(dllexport) void __cdecl DeleteShader(GLuint shader);
	__declspec(dllexport) void __cdecl GetProgramLog(GLuint program);
	__declspec(dllexport) void __cdecl GetShaderAttribLocations(GLuint program, GLUFMapElement* elements, GLuint* size);
	__declspec(dllexport) void __cdecl GetSepShaderAttribLocations(GLuint program, GLUFMapElement* elements, GLuint* size);
	__declspec(dllexport) void __cdecl GetShaderLog(GLuint shader, const char* text, const GLuint size);
	__declspec(dllexport) GLenum __cdecl GetShaderType(GLuint shader);
	__declspec(dllexport) void __cdecl GetShaderUniformLocations(GLuint program, GLUFMapElement* elements, GLuint* size);
	__declspec(dllexport) void __cdecl GetSepShaderUniformLocations(GLuint ppo, GLUFMapElement* elements, GLuint* size);

	__declspec(dllexport) void __cdecl UseProgram(GLuint program);
	__declspec(dllexport) void __cdecl UseSepProgram(GLuint ppo);


	//non-separable uniform methods
	__declspec(dllexport) void __cdecl Uniform1f(GLuint program, const char* name, GLfloat value);
	__declspec(dllexport) void __cdecl Uniform1f(GLuint program, GLuint loc, GLfloat* value);
	__declspec(dllexport) void __cdecl Uniform2f(GLuint program, const char* name, GLfloat* value);
	__declspec(dllexport) void __cdecl Uniform2f(GLuint program, GLuint loc, GLfloat* value);
	__declspec(dllexport) void __cdecl Uniform3f(GLuint program, const char* name, GLfloat* value);
	__declspec(dllexport) void __cdecl Uniform3f(GLuint program, GLuint loc, GLfloat* value);
	__declspec(dllexport) void __cdecl Uniform4f(GLuint program, const char* name, GLfloat* value);
	__declspec(dllexport) void __cdecl Uniform4f(GLuint program, GLuint loc, GLfloat* value);

	__declspec(dllexport) void __cdecl Uniform1i(GLuint program, const char* name, GLint value);
	__declspec(dllexport) void __cdecl Uniform1i(GLuint program, GLuint loc, GLint* value);
	__declspec(dllexport) void __cdecl Uniform2i(GLuint program, const char* name, GLint* value);
	__declspec(dllexport) void __cdecl Uniform2i(GLuint program, GLuint loc, GLint* value);
	__declspec(dllexport) void __cdecl Uniform3i(GLuint program, const char* name, GLint* value);
	__declspec(dllexport) void __cdecl Uniform3i(GLuint program, GLuint loc, GLint* value);
	__declspec(dllexport) void __cdecl Uniform4i(GLuint program, const char* name, GLint* value);
	__declspec(dllexport) void __cdecl Uniform4i(GLuint program, GLuint loc, GLint* value);

	__declspec(dllexport) void __cdecl Uniform1ui(GLuint program, const char* name, GLuint value);
	__declspec(dllexport) void __cdecl Uniform1ui(GLuint program, GLuint loc, GLuint* value);
	__declspec(dllexport) void __cdecl Uniform2ui(GLuint program, const char* name, GLuint* value);
	__declspec(dllexport) void __cdecl Uniform2ui(GLuint program, GLuint loc, GLuint* value);
	__declspec(dllexport) void __cdecl Uniform3ui(GLuint program, const char* name, GLuint* value);
	__declspec(dllexport) void __cdecl Uniform3ui(GLuint program, GLuint loc, GLuint* value);
	__declspec(dllexport) void __cdecl Uniform4ui(GLuint program, const char* name, GLuint* value);
	__declspec(dllexport) void __cdecl Uniform4ui(GLuint program, GLuint loc, GLuint* value);

	__declspec(dllexport) void __cdecl UniformMatrix2f(GLuint program, const char* name, GLfloat* value);
	__declspec(dllexport) void __cdecl UniformMatrix2f(GLuint program, GLuint loc, GLfloat* value);
	__declspec(dllexport) void __cdecl UniformMatrix2x3f(GLuint program, const char* name, GLfloat* value);
	__declspec(dllexport) void __cdecl UniformMatrix2x3f(GLuint program, GLuint loc, GLfloat* value);
	__declspec(dllexport) void __cdecl UniformMatrix2x4f(GLuint program, const char* name, GLfloat* value);
	__declspec(dllexport) void __cdecl UniformMatrix2x4f(GLuint program, GLuint loc, GLfloat* value);

	__declspec(dllexport) void __cdecl UniformMatrix3f(GLuint program, const char* name, GLfloat* value);
	__declspec(dllexport) void __cdecl UniformMatrix3f(GLuint program, GLuint loc, GLfloat* value);
	__declspec(dllexport) void __cdecl UniformMatrix3x2f(GLuint program, const char* name, GLfloat* value);
	__declspec(dllexport) void __cdecl UniformMatrix3x2f(GLuint program, GLuint loc, GLfloat* value);
	__declspec(dllexport) void __cdecl UniformMatrix3x4f(GLuint program, const char* name, GLfloat* value);
	__declspec(dllexport) void __cdecl UniformMatrix3x4f(GLuint program, GLuint loc, GLfloat* value);

	__declspec(dllexport) void __cdecl UniformMatrix4f(GLuint program, const char* name, GLfloat* value);
	__declspec(dllexport) void __cdecl UniformMatrix4f(GLuint program, GLuint loc, GLfloat* value);
	__declspec(dllexport) void __cdecl UniformMatrix4x2f(GLuint program, const char* name, GLfloat* value);
	__declspec(dllexport) void __cdecl UniformMatrix4x2f(GLuint program, GLuint loc, GLfloat* value);
	__declspec(dllexport) void __cdecl UniformMatrix4x3f(GLuint program, const char* name, GLfloat* value);
	__declspec(dllexport) void __cdecl UniformMatrix4x3f(GLuint program, GLuint loc, GLfloat* value);

	//now for separable ones

	__declspec(dllexport) void __cdecl Uniform1f(GLuint ppo, const char* name, GLfloat value);
	__declspec(dllexport) void __cdecl Uniform1f(GLuint ppo, GLuint loc, GLfloat* value);
	__declspec(dllexport) void __cdecl Uniform2f(GLuint ppo, const char* name, GLfloat* value);
	__declspec(dllexport) void __cdecl Uniform2f(GLuint ppo, GLuint loc, GLfloat* value);
	__declspec(dllexport) void __cdecl Uniform3f(GLuint ppo, const char* name, GLfloat* value);
	__declspec(dllexport) void __cdecl Uniform3f(GLuint ppo, GLuint loc, GLfloat* value);
	__declspec(dllexport) void __cdecl Uniform4f(GLuint ppo, const char* name, GLfloat* value);
	__declspec(dllexport) void __cdecl Uniform4f(GLuint ppo, GLuint loc, GLfloat* value);

	__declspec(dllexport) void __cdecl Uniform1i(GLuint ppo, const char* name, GLint value);
	__declspec(dllexport) void __cdecl Uniform1i(GLuint ppo, GLuint loc, GLint* value);
	__declspec(dllexport) void __cdecl Uniform2i(GLuint ppo, const char* name, GLint* value);
	__declspec(dllexport) void __cdecl Uniform2i(GLuint ppo, GLuint loc, GLint* value);
	__declspec(dllexport) void __cdecl Uniform3i(GLuint ppo, const char* name, GLint* value);
	__declspec(dllexport) void __cdecl Uniform3i(GLuint ppo, GLuint loc, GLint* value);
	__declspec(dllexport) void __cdecl Uniform4i(GLuint ppo, const char* name, GLint* value);
	__declspec(dllexport) void __cdecl Uniform4i(GLuint ppo, GLuint loc, GLint* value);

	__declspec(dllexport) void __cdecl Uniform1ui(GLuint ppo, const char* name, GLuint value);
	__declspec(dllexport) void __cdecl Uniform1ui(GLuint ppo, GLuint loc, GLuint* value);
	__declspec(dllexport) void __cdecl Uniform2ui(GLuint ppo, const char* name, GLuint* value);
	__declspec(dllexport) void __cdecl Uniform2ui(GLuint ppo, GLuint loc, GLuint* value);
	__declspec(dllexport) void __cdecl Uniform3ui(GLuint ppo, const char* name, GLuint* value);
	__declspec(dllexport) void __cdecl Uniform3ui(GLuint ppo, GLuint loc, GLuint* value);
	__declspec(dllexport) void __cdecl Uniform4ui(GLuint ppo, const char* name, GLuint* value);
	__declspec(dllexport) void __cdecl Uniform4ui(GLuint ppo, GLuint loc, GLuint* value);

	__declspec(dllexport) void __cdecl UniformMatrix2f(GLuint ppo, const char* name, GLfloat* value);
	__declspec(dllexport) void __cdecl UniformMatrix2f(GLuint ppo, GLuint loc, GLfloat* value);
	__declspec(dllexport) void __cdecl UniformMatrix2x3f(GLuint ppo, const char* name, GLfloat* value);
	__declspec(dllexport) void __cdecl UniformMatrix2x3f(GLuint ppo, GLuint loc, GLfloat* value);
	__declspec(dllexport) void __cdecl UniformMatrix2x4f(GLuint ppo, const char* name, GLfloat* value);
	__declspec(dllexport) void __cdecl UniformMatrix2x4f(GLuint ppo, GLuint loc, GLfloat* value);

	__declspec(dllexport) void __cdecl UniformMatrix3f(GLuint ppo, const char* name, GLfloat* value);
	__declspec(dllexport) void __cdecl UniformMatrix3f(GLuint ppo, GLuint loc, GLfloat* value);
	__declspec(dllexport) void __cdecl UniformMatrix3x2f(GLuint ppo, const char* name, GLfloat* value);
	__declspec(dllexport) void __cdecl UniformMatrix3x2f(GLuint ppo, GLuint loc, GLfloat* value);
	__declspec(dllexport) void __cdecl UniformMatrix3x4f(GLuint ppo, const char* name, GLfloat* value);
	__declspec(dllexport) void __cdecl UniformMatrix3x4f(GLuint ppo, GLuint loc, GLfloat* value);

	__declspec(dllexport) void __cdecl UniformMatrix4f(GLuint ppo, const char* name, GLfloat* value);
	__declspec(dllexport) void __cdecl UniformMatrix4f(GLuint ppo, GLuint loc, GLfloat* value);
	__declspec(dllexport) void __cdecl UniformMatrix4x2f(GLuint ppo, const char* name, GLfloat* value);
	__declspec(dllexport) void __cdecl UniformMatrix4x2f(GLuint ppo, GLuint loc, GLfloat* value);
	__declspec(dllexport) void __cdecl UniformMatrix4x3f(GLuint ppo, const char* name, GLfloat* value);
	__declspec(dllexport) void __cdecl UniformMatrix4x3f(GLuint ppo, GLuint loc, GLfloat* value);
	/*
	
	GLUFGui Methods
	
	*/
	__declspec(dllexport) void __cdecl GLUFSetDefaultFont();


}
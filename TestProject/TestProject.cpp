// TestProject.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#define USING_ASSIMP
#define SUPPRESS_RADIAN_ERROR
#include "../ObjGLUF/ObjGLUF.h"
#include <GLFW/glfw3.h>
#include <stdlib.h>
#include <stdio.h>

static void error_callback(int error, const char* description)
{
	fputs(description, stderr);
}
static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);
}
void ErrorMethod(const char* message, const char* func, const char* file, unsigned int line)
{
	printf(message);
}

int main(void)
{
	GLFWwindow* window;
	glfwSetErrorCallback(error_callback);
	if (!glfwInit())
		exit(EXIT_FAILURE);

	//glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	//glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
	window = glfwCreateWindow(800, 800, "Simple example", NULL, NULL);
	if (!window)
	{
		glfwTerminate();
		exit(EXIT_FAILURE);
	}
	glfwMakeContextCurrent(window);
	glfwSetKeyCallback(window, key_callback);

	GLenum err = glewInit();
	if (GLEW_OK != err)
	{
		exit(EXIT_FAILURE);
	}
	std::string filePaths[5] = {"vert.glsl", "", "", "", "frag.glsl"};
	GLUF::GLUFInit(filePaths);

	GLUFRegisterErrorMethod(ErrorMethod);

	GLUFBufferManager buffManager;

	//load the test object
	Assimp::Importer importer;
	// And have it read the given file with some example postprocessing
	// Usually - if speed is not the most important aspect for you - you'll
	// propably to request more postprocessing than we do in this example.
	const aiScene* scene = importer.ReadFile("suzanne.obj",
		aiProcess_CalcTangentSpace |
		aiProcess_Triangulate |
		aiProcess_JoinIdenticalVertices |
		aiProcess_SortByPType);

	const aiMesh* mesh = scene->mMeshes[0];

	GLUFShaderManager shadMan;
	GLUFBufferManager buffMan;

	// Dark blue background
	glClearColor(0.0f, 0.0f, 0.4f, 0.0f);

	// Enable depth test
	glEnable(GL_DEPTH_TEST);
	// Accept fragment if it closer to the camera than the former one
	glDepthFunc(GL_LESS);

	// Cull triangles which normal is not towards the camera
	glEnable(GL_CULL_FACE);

	//GLuint VertexArrayID;
	//glGenVertexArrays(1, &VertexArrayID);
	//glBindVertexArray(VertexArrayID);

	// Create and compile our GLSL program from the shaders
	GLUFShaderPathList shaderSources;
	shaderSources.insert(std::pair<GLUFShaderType, std::string>(SH_VERTEX_SHADER, "StandardShadingVert.glsl"));
	shaderSources.insert(std::pair<GLUFShaderType, std::string>(SH_FRAGMENT_SHADER, "StandardShadingFrag.glsl"));
	GLUFProgramPtr program = shadMan.CreateProgram(shaderSources);

	// Get a handle for our "MVP" uniform
	//GLuint MatrixID = glGetUniformLocation(shadMan.GetProgramId(program), "MVP");
	//GLuint ViewMatrixID = glGetUniformLocation(shadMan.GetProgramId(program), "V");
	//GLuint ModelMatrixID = glGetUniformLocation(shadMan.GetProgramId(program), "M");

	// Load the texture
	//GLuint Texture = loadDDS("uvmap.DDS");
	GLUFTexturePtr texture = buffMan.CreateTextureBuffer();
	buffMan.LoadTextureFromFile(texture, "uvmap.dds", TFF_DDS);

	// Get a handle for our "myTextureSampler" uniform
	//GLuint TextureID = glGetUniformLocation(programID, "myTextureSampler");

	// Read our .obj file
	std::vector<glm::vec3> vertices;
	std::vector<glm::vec2> uvs;
	std::vector<glm::vec3> normals;
	std::vector<GLushort> indicies;

	for (unsigned int i = 0; i < mesh->mNumVertices; ++i)
	{
		vertices.push_back(AssimpToGlm(mesh->mVertices[i]));
		normals.push_back(AssimpToGlm(mesh->mNormals[i]));
		aiVector3D uv = mesh->mTextureCoords[0][i];
		uvs.push_back(glm::vec2(uv.x, uv.y));
	}

	for (unsigned int i = 0; i < mesh->mNumFaces; ++i)
	{
		aiFace tmpFace = mesh->mFaces[i];
		indicies.push_back(tmpFace.mIndices[0]);
		indicies.push_back(tmpFace.mIndices[1]);
		indicies.push_back(tmpFace.mIndices[2]);
	}

	// Load it into a VBO

	GLUFVertexArrayPtr vertArray = buffMan.CreateVertexArray();
	buffMan.ModifyVertexArray(vertArray, vertices, normals, uvs, indicies);
	GLUFUniformBufferPtr uniBuf = buffMan.CreateUniformArray();


	// Get a handle for our "LightPosition" uniform
	glUseProgram(shadMan.GetProgramId(program));
	GLuint LightID = glGetUniformLocation(shadMan.GetProgramId(program), "LightPosition_worldspace");


	do{

		// Clear the screen
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


		float ratio;
		int width, height;
		glfwGetFramebufferSize(window, &width, &height);
		ratio = width / (float)height;
		glViewport(0, 0, width, height);

		glm::mat4 ProjectionMatrix = glm::infinitePerspective(50.0f, ratio, 0.1f);
		glm::mat4 ViewMatrix;
		glm::mat4 translate = glm::translate(glm::mat4(), glm::vec3(0.0f, 0.0f, -20.0f));
		glm::mat4 rotation = glm::toMat4(glm::quat(glm::vec3((float)glfwGetTime() / 2.0f, 0.0f, (float)glfwGetTime())));
		glm::mat4 ModelMatrix = translate * rotation;
		//glm::mat4 MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;

		// Send our transformation to the currently bound shader, 
		// in the "MVP" uniform

		GLUFMatrixTransformBlock matData = buffMan.MapUniformTransform(uniBuf);

		matData.ModifyValue(UT_MODEL, ModelMatrix);
		matData.ModifyValue(UT_PROJ, ProjectionMatrix);

		buffMan.UnMapUniformTransform(uniBuf);

		// Use our shader
		shadMan.UseProgram(program);

		//just to make is easier to see things
		glm::vec3 lightPos = glm::vec3(4, 4, 4);
		glUniform3f(LightID, lightPos.x, lightPos.y, lightPos.z);

		buffMan.UseTexture(texture);

		buffManager.DrawVertexArray(vertArray);


		// Swap buffers
		glfwSwapBuffers(window);
		glfwPollEvents();

	} // Check if the ESC key was pressed or the window was closed
	while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS &&
	glfwWindowShouldClose(window) == 0);


	// Close OpenGL window and terminate GLFW
	glfwTerminate();

	glfwDestroyWindow(window);
	glfwTerminate();
	exit(EXIT_SUCCESS);
}
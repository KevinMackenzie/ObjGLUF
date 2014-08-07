// TestProject.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#define USING_ASSIMP
#define SUPPRESS_RADIAN_ERROR
#include "../ObjGLUF/GLUFGui.h"
#include <stdlib.h>
#include <stdio.h>


GLFWwindow* window;

GLUFDialogResourceManager *resMan;
GLUFDialog *dlg;

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

bool MsgProc(GLUF_GUI_CALLBACK_PARAM)
{
	if (msg == GM_KEY)
	{
		if (param1 == GLFW_KEY_ESCAPE && param3 == GLFW_PRESS)
			glfwSetWindowShouldClose(window, GL_TRUE);
		//return true;
	}

	resMan->MsgProc(GLUF_PASS_CALLBACK_PARAM);
	dlg->MsgProc(GLUF_PASS_CALLBACK_PARAM);

	return false;
}

void CtrlMsgProc(GLUF_EVENT evt, int controlId, GLUFControl* pControl)
{
	if (evt == GLUF_EVENT_BUTTON_CLICKED)
	{
		printf("HORRAY\n");
	}
}


int main(void)
{
	GLUFRegisterErrorMethod(ErrorMethod);
	GLUFInit();


	//glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	//glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
	window = glfwCreateWindow(800, 800, "Simple example", NULL, NULL);
	if (!window)
	{
		glfwTerminate();
		exit(EXIT_FAILURE);
	}

	glfwMakeContextCurrent(window);

	GLUFInitOpenGLExtentions();


	GLUFTexturePtr texPtr = GLUFBUFFERMANAGER.CreateTextureBuffer();
	GLUFBUFFERMANAGER.LoadTextureFromFile(texPtr, "dxutcontrolstest.dds", TFF_DDS);
	GLUFInitGui(window, MsgProc, texPtr);

	//glfwSetKeyCallback(window, key_callback);

	//printf((const char*)glGetString(GL_VERSION));


	//GLUFRegisterErrorMethod(ErrorMethod);

	/*GLUFBufferManager buffManager;

	//load the test object
	Assimp::Importer importer;
	// And have it read the given file with some example postprocessing
	// Usually - if speed is not the most important aspect for you - you'll
	// propably to request more postprocessing than we do in this example.
	const aiScene* scene = importer.ReadFile("suzanne.obj.model",
		aiProcess_CalcTangentSpace |
		aiProcess_Triangulate |
		aiProcess_JoinIdenticalVertices |
		aiProcess_SortByPType);

	const aiMesh* mesh = scene->mMeshes[0];

	GLUFShaderManager shadMan;
	GLUFBufferManager buffMan;*/



	//GLuint VertexArrayID;
	//glGenVertexArrays(1, &VertexArrayID);
	//glBindVertexArray(VertexArrayID);

	static const char * vs_source =
	{
		"#version 420 core                                                  \n"
		"                                                                   \n"
		"in vec4 position;                                                  \n"
		"                                                                   \n"
		"out VS_OUT                                                         \n"
		"{                                                                  \n"
		"    vec4 color;                                                    \n"
		"} vs_out;                                                          \n"
		"                                                                   \n"
		"uniform mat4 mv_matrix;                                            \n"
		"uniform mat4 proj_matrix;                                          \n"
		"                                                                   \n"
		"void main(void)                                                    \n"
		"{                                                                  \n"
		"    gl_Position = proj_matrix * mv_matrix * position;              \n"
		"    vs_out.color = position * 2.0 + vec4(0.5, 0.5, 0.5, 0.0);      \n"
		"}                                                                  \n"
	};

	static const char * fs_source =
	{
		"#version 420 core                                                  \n"
		"                                                                   \n"
		"out vec4 color;                                                    \n"
		"                                                                   \n"
		"in VS_OUT                                                          \n"
		"{                                                                  \n"
		"    vec4 color;                                                    \n"
		"} fs_in;                                                           \n"
		"                                                                   \n"
		"void main(void)                                                    \n"
		"{                                                                  \n"
		"    color = fs_in.color;                                           \n"
		"}                                                                  \n"
	};

	// Create and compile our GLSL program from the shaders
	/*GLUFShaderSourceList shaderSources;
	shaderSources.insert(std::pair<GLUFShaderType, const char*>(SH_VERTEX_SHADER, vs_source));
	shaderSources.insert(std::pair<GLUFShaderType, const char*>(SH_FRAGMENT_SHADER, fs_source));
	GLUFProgramPtr program = GLUFSHADERMANAGER.CreateProgram(shaderSources);

	// Get a handle for our "MVP" uniform
	//GLuint MatrixID = glGetUniformLocation(shadMan.GetProgramId(program), "MVP");
	//GLuint ViewMatrixID = glGetUniformLocation(shadMan.GetProgramId(program), "V");
	//GLuint ModelMatrixID = glGetUniformLocation(shadMan.GetProgramId(program), "M");

	// Load the texture
	//GLuint Texture = loadDDS("uvmap.DDS");
	//GLUFTexturePtr texture = buffMan.CreateTextureBuffer();
	//buffMan.LoadTextureFromFile(texture, "uvmap.dds", TFF_DDS);

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
	GLuint LightID = glGetUniformLocation(shadMan.GetProgramId(program), "LightPosition_worldspace");*/

	resMan = new GLUFDialogResourceManager();
	dlg = new GLUFDialog();
	dlg->Init(resMan);
	dlg->SetCallback(CtrlMsgProc);
	dlg->SetSize(0.8f, 0.8f);
	dlg->SetLocation(0.1f, 0.1f);
	dlg->SetBackgroundColors(Color(0, 128, 0, 255));
	dlg->EnableKeyboardInput(true);

	dlg->AddButton(0, "Button", 0.05f, 0.01f, 0.125f, 0.03625f);

	GLUFComboBox* box;
	dlg->AddComboBox(1, 0.2f, 0.2f, 0.175f, 0.03625f, 0, false, &box);
	dlg->AddCheckBox(2, "", 0.3f, 0.1f, 0.03625f, 0.03625f);
	dlg->AddRadioButton(3, 0, "Test", 0.4f, 0.4f, 0.03625f, 0.03625f, true);
	dlg->AddRadioButton(4, 0, "", 0.4f, 0.45f, 0.03625f, 0.03625f);
	dlg->AddRadioButton(5, 0, "", 0.4f, 0.5f, 0.03625f, 0.03625f);
	//dlg->AddSlider (6, 0.1f, 0.3f, 0.4f, 0.03625f, 0.0f, 1.0f, 0.25f);

	box->AddItem("Test0", nullptr);
	box->AddItem("Test1", nullptr);
	box->AddItem("Test2", nullptr);
	box->AddItem("Test3", nullptr);
	box->AddItem("Test4", nullptr);
	box->AddItem("Test5", nullptr);
	box->AddItem("Test6", nullptr);
	box->AddItem("Test7", nullptr);
	box->AddItem("Test8", nullptr);
	box->AddItem("Test9", nullptr);
	box->AddItem("Test10", nullptr);
	box->AddItem("Test11", nullptr);
	box->AddItem("Test12", nullptr);
	box->AddItem("Test13", nullptr);
	box->AddItem("Test14", nullptr);

	//TODO: fix blending issues

	// Our vertices. Tree consecutive floats give a 3D vertex; Three consecutive vertices give a triangle.
	// A cube has 6 faces with 2 triangles each, so this makes 6*2=12 triangles, and 12*3 vertices
	/*static const GLfloat g_vertex_buffer_data[] = {
		-1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f, 1.0f,
		-1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, -1.0f,
		-1.0f, -1.0f, -1.0f,
		-1.0f, 1.0f, -1.0f,
		1.0f, -1.0f, 1.0f,
		-1.0f, -1.0f, -1.0f,
		1.0f, -1.0f, -1.0f,
		1.0f, 1.0f, -1.0f,
		1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f, -1.0f,
		-1.0f, 1.0f, 1.0f,
		-1.0f, 1.0f, -1.0f,
		1.0f, -1.0f, 1.0f,
		-1.0f, -1.0f, 1.0f,
		-1.0f, -1.0f, -1.0f,
		-1.0f, 1.0f, 1.0f,
		-1.0f, -1.0f, 1.0f,
		1.0f, -1.0f, 1.0f,
		1.0f, 1.0f, 1.0f,
		1.0f, -1.0f, -1.0f,
		1.0f, 1.0f, -1.0f,
		1.0f, -1.0f, -1.0f,
		1.0f, 1.0f, 1.0f,
		1.0f, -1.0f, 1.0f,
		1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, -1.0f,
		-1.0f, 1.0f, -1.0f,
		1.0f, 1.0f, 1.0f,
		-1.0f, 1.0f, -1.0f,
		-1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, 1.0f,
		-1.0f, 1.0f, 1.0f,
		1.0f, -1.0f, 1.0f
	};

	// Two UV coordinatesfor each vertex. They were created withe Blender.
	static const GLfloat g_uv_buffer_data[] = {
		0.000059f, 1.0f - 0.000004f,
		0.000103f, 1.0f - 0.336048f,
		0.335973f, 1.0f - 0.335903f,
		1.000023f, 1.0f - 0.000013f,
		0.667979f, 1.0f - 0.335851f,
		0.999958f, 1.0f - 0.336064f,
		0.667979f, 1.0f - 0.335851f,
		0.336024f, 1.0f - 0.671877f,
		0.667969f, 1.0f - 0.671889f,
		1.000023f, 1.0f - 0.000013f,
		0.668104f, 1.0f - 0.000013f,
		0.667979f, 1.0f - 0.335851f,
		0.000059f, 1.0f - 0.000004f,
		0.335973f, 1.0f - 0.335903f,
		0.336098f, 1.0f - 0.000071f,
		0.667979f, 1.0f - 0.335851f,
		0.335973f, 1.0f - 0.335903f,
		0.336024f, 1.0f - 0.671877f,
		1.000004f, 1.0f - 0.671847f,
		0.999958f, 1.0f - 0.336064f,
		0.667979f, 1.0f - 0.335851f,
		0.668104f, 1.0f - 0.000013f,
		0.335973f, 1.0f - 0.335903f,
		0.667979f, 1.0f - 0.335851f,
		0.335973f, 1.0f - 0.335903f,
		0.668104f, 1.0f - 0.000013f,
		0.336098f, 1.0f - 0.000071f,
		0.000103f, 1.0f - 0.336048f,
		0.000004f, 1.0f - 0.671870f,
		0.336024f, 1.0f - 0.671877f,
		0.000103f, 1.0f - 0.336048f,
		0.336024f, 1.0f - 0.671877f,
		0.335973f, 1.0f - 0.335903f,
		0.667969f, 1.0f - 0.671889f,
		1.000004f, 1.0f - 0.671847f,
		0.667979f, 1.0f - 0.335851f
	};

	GLuint vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	GLuint vertexbuffer;
	glGenBuffers(1, &vertexbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), g_vertex_buffer_data, GL_STATIC_DRAW);

	GLuint uvbuffer;
	glGenBuffers(1, &uvbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(g_uv_buffer_data), g_uv_buffer_data, GL_STATIC_DRAW);*/

	float ellapsedTime = 0.0f;
	float prevTime = 0.0f;
	float currTime = 0.0f;

	// Dark blue background
	//glClearColor(0.0f, 0.0f, 0.4f, 0.0f);



	// Cull triangles which normal is not towards the camera
	//glEnable(GL_CULL_FACE);


	do{

		currTime = (float)glfwGetTime();
		ellapsedTime = currTime - prevTime;

		// Clear the screen
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		float ratio;
		int width, height;
		glfwGetFramebufferSize(window, &width, &height);
		ratio = width / (float)height;
		glViewport(0, 0, width, height);
		static const GLfloat black[] = { 0.0f, 0.0f, 0.0f, 1.0f };
		static const GLfloat one = 1.0f;

		glClearBufferfv(GL_COLOR, 0, black);

		/*GLUFSHADERMANAGER.UseProgram(g_UIProgram);

		// Bind our texture in Texture Unit 0
		/*glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, Texture);
		// Set our "myTextureSampler" sampler to user Texture Unit 0
		glUniform1i(TextureID, 0);
		GLUFBUFFERMANAGER.UseTexture(texPtr, 1, GL_TEXTURE0);


		glm::mat4 mat = glm::ortho(-1.2f, 1.2f, -1.2f, 1.2f);
		glUniformMatrix4fv(0, 1, GL_FALSE, &mat[0][0]);

		// 1rst attribute buffer : vertices
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
		glVertexAttribPointer(
			0,                  // attribute. No particular reason for 0, but must match the layout in the shader.
			3,                  // size
			GL_FLOAT,           // type
			GL_FALSE,           // normalized?
			0,                  // stride
			(void*)0            // array buffer offset
			);

		// 2nd attribute buffer : UVs
		glEnableVertexAttribArray(2);
		glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
		glVertexAttribPointer(
			2,                                // attribute. No particular reason for 1, but must match the layout in the shader.
			2,                                // size : U+V => 2
			GL_FLOAT,                         // type
			GL_FALSE,                         // normalized?
			0,                                // stride
			(void*)0                          // array buffer offset
			);

		// Draw the triangle !
		glDrawArrays(GL_TRIANGLES, 0, 12 * 3); // 12*3 indices starting at 0 -> 12 triangles

		glDisableVertexAttribArray(0);
		glDisableVertexAttribArray(2);*/

		dlg->OnRender(ellapsedTime);



		/*glm::mat4 ProjectionMatrix = glm::infinitePerspective(50.0f, ratio, 0.1f);
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

		buffMan.UseTexture(texture, 5, GL_TEXTURE0);

		buffManager.DrawVertexArray(vertArray);*/


		// Swap buffers
		glfwSwapBuffers(window);
		glfwPollEvents();

		//prevTime = currTime;

	} // Check if the ESC key was pressed or the window was closed
	while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS &&
	glfwWindowShouldClose(window) == 0);


	glfwDestroyWindow(window);
	glfwTerminate();

	// Close OpenGL window and terminate GLFW
	glfwTerminate();

	return 0;
}
/*
static void error_callback(int error, const char* description)
{
	fputs(description, stderr);
}
static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);
}
int main(void)
{
	GLFWwindow* window;
	glfwSetErrorCallback(error_callback);
	if (!glfwInit())
		exit(EXIT_FAILURE);
	window = glfwCreateWindow(640, 480, "Simple example", NULL, NULL);
	if (!window)
	{
		glfwTerminate();
		exit(EXIT_FAILURE);
	}

	double x, y;
	x = 0.0;
	y = 0.0;

	glfwMakeContextCurrent(window);
	glfwSetKeyCallback(window, key_callback);
	while (!glfwWindowShouldClose(window))
	{
		float ratio;
		int width, height;
		glfwGetFramebufferSize(window, &width, &height);
		ratio = width / (float)height;
		glViewport(0, 0, width, height);
		glClear(GL_COLOR_BUFFER_BIT);
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		glOrtho(-ratio, ratio, -1.f, 1.f, 1.f, -1.f);
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		glRotatef((float)glfwGetTime() * 50.f, 0.f, 0.f, 1.f);
		glBegin(GL_TRIANGLES);
		glColor3f(1.f, 0.f, 0.f);
		glVertex3f(-0.6f, -0.4f, 0.f);
		glColor3f(0.f, 1.f, 0.f);
		glVertex3f(0.6f, -0.4f, 0.f);
		glColor3f(0.f, 0.f, 1.f);
		glVertex3f(0.f, 0.6f, 0.f);
		glEnd();
		glfwSwapBuffers(window);
		glfwPollEvents();

		glfwGetCursorPos(window, &x, &y);
		printf("%f,%f \n", x, y);

		Sleep(200);
	}
	glfwDestroyWindow(window);
	glfwTerminate();
	exit(EXIT_SUCCESS);
}*/
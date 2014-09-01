// TestProject.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#define USING_ASSIMP
#define SUPPRESS_RADIAN_ERROR
#define SUPPRESS_UTF8_ERROR
#include "../ObjGLUF/GLUFGui.h"
#include <stdlib.h>
#include <stdio.h>
#include <fstream>

using namespace GLUF;

GLFWwindow* window;

GLUFDialogResourceManager *resMan;
GLUFDialog *dlg;

static void error_callback(int error, const char* description)
{
	fputs(description, stderr);
}
void ErrorMethod(const char* message, const char* func, const char* file, unsigned int line)
{
	printf("(%s | %i): %s \n", func, line, message);
	//hang
	system("PAUSE");
}

bool MsgProc(GLUF_GUI_CALLBACK_PARAM)
{
	if (msg == GM_KEY)
	{
		if (param1 == GLFW_KEY_ESCAPE && param3 == GLFW_RELEASE)
			//DialogOpened = !DialogOpened;
			dlg->SetMinimized(!dlg->GetMinimized());
		//return true;
	}

	resMan->MsgProc(GLUF_PASS_CALLBACK_PARAM);
	dlg->MsgProc(GLUF_PASS_CALLBACK_PARAM);

	return false;
}

void CtrlMsgProc(GLUF_EVENT evt, int controlId, GLUFControl* pControl, void* pContext)
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


	//glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);
	//glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 1);
	//glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);

	window = glfwCreateWindow(800, 800, "Simple example", NULL, NULL);
	if (!window)
	{
		glfwTerminate();
		exit(EXIT_FAILURE);
	}

	glfwMakeContextCurrent(window);

	GLUFInitOpenGLExtentions();


	GLuint ctrlTex = LoadTextureFromFile(L"dxutcontrolstest.dds", TFF_DDS);
	GLUFInitGui(window, MsgProc, ctrlTex);

	resMan = new GLUFDialogResourceManager();
	dlg = new GLUFDialog();
	dlg->Init(resMan);
	dlg->SetCallback(CtrlMsgProc);//TODO: fix caption
	dlg->SetCaptionText(L"Caption");
	dlg->SetCaptionHeight(50);
	dlg->Lock(false);
	dlg->EnableAutoClamp();
	//dlg->EnableGrabAnywhere();
	//dlg->SetMinimized(true);
	dlg->EnableCaption(true);
	dlg->SetSize(600, 600);
	dlg->SetLocation(50, 50);
	dlg->SetBackgroundColor(Color(0, 128, 0, 128));
	dlg->EnableKeyboardInput(true);
	//GLUFRect rc = { 0, 200, 200, 0 };


	std::wifstream t("text.txt");
	std::wstring str;

	t.seekg(0, std::ios::end);
	str.reserve(t.tellg());
	t.seekg(0, std::ios::beg);

	str.assign((std::istreambuf_iterator<wchar_t>(t)),
		std::istreambuf_iterator<wchar_t>());
	//dlg->AddEditBox(10, str, 100, 100, 400, 400, GT_LEFT | GT_TOP | GT_MULTI_LINE);
	dlg->AddEditBox(10, L"EditBoxEditBoxEditBoxEditBoxEditBox", 100, 100, 400, 35, GT_LEFT | GT_TOP);

	/*dlg->AddStatic(6, L"The Quick Brown Fox Jumped Over The Lazy Dog", 50, 30, 75, 20);

	//dlg->AddCheckBox(2, L"Check Box", 150, 50, 20, 20);
	//dlg->AddRadioButton(3, 0, L"Button 1", 200, 200, 20, 20, true);
	//dlg->AddRadioButton(4, 0, L"Button 2", 200, 250, 20, 20);
	//dlg->AddRadioButton(5, 0, L"Button 3", 200, 300, 20, 20);
	//dlg->AddButton(0, L"Button", 25, 10, 75, 20);

	dlg->AddButton(0, L"Button", 50, 10, 125, 35);

	GLUFComboBox* box;
	dlg->AddComboBox(1, 200, 200, 125, 35, 0, false, &box);
	dlg->AddCheckBox(2, L"Check Box", 300, 100, 35, 35);
	dlg->AddRadioButton(3, 0, L"Button 1", 400, 400, 35, 35, true);
	dlg->AddRadioButton(4, 0, L"Button 2", 400, 450, 35, 35);
	dlg->AddRadioButton(5, 0, L"Button 3", 400, 500, 35, 35);
	dlg->AddStatic(6, L"Static", 100, 600, 175, 35);
	//dlg->AddSlider (6, 0.1f, 0.3f, 0.4f, 0.03625f, 0.0f, 1.0f, 0.25f);

	box->AddItem(L"Item 0", nullptr);
	box->AddItem(L"Item 1",  nullptr);
	box->AddItem(L"Item 2",  nullptr);
	box->AddItem(L"Item 3",  nullptr);
	box->AddItem(L"Item 4",  nullptr);
	box->AddItem(L"Item 5", nullptr);
	box->AddItem(L"Item 6", nullptr);
	box->AddItem(L"Item 7", nullptr);
	box->AddItem(L"Item 8", nullptr);
	box->AddItem(L"Item 9", nullptr);
	box->AddItem(L"Item 10", nullptr);
	box->AddItem(L"Item 11", nullptr);
	box->AddItem(L"Item 12", nullptr);
	box->AddItem(L"Item 13", nullptr);
	box->AddItem(L"Item 14", nullptr);
	box->AddItem(L"Item 15", nullptr);
	box->AddItem(L"Item 16", nullptr);
	box->AddItem(L"Item 17", nullptr);
	box->AddItem(L"Item 18", nullptr);
	box->AddItem(L"Item 19", nullptr);
	box->AddItem(L"Item 20", nullptr);
	box->AddItem(L"Item 21", nullptr);
	box->AddItem(L"Item 22", nullptr);
	box->AddItem(L"Item 23", nullptr);
	box->AddItem(L"Item 24", nullptr);
	box->AddItem(L"Item 25", nullptr);
	box->AddItem(L"Item 26", nullptr);
	box->AddItem(L"Item 27", nullptr);
	box->AddItem(L"Item 28", nullptr);
	box->AddItem(L"Item 29", nullptr);*/


	//load shaders
	//GLUFProgramPtr frag, vert;
	GLUFProgramPtr Prog;

	GLUFShaderPathList paths;
	paths.insert(std::pair<GLUFShaderType, std::wstring>(SH_VERTEX_SHADER, L"Shaders/BasicLighting120.vert.glsl"));
	paths.insert(std::pair<GLUFShaderType, std::wstring>(SH_FRAGMENT_SHADER, L"Shaders/BasicLighting120.frag.glsl"));
	
	Prog = GLUFSHADERMANAGER.CreateProgram(paths);

	GLUFVariableLocMap attribs, uniforms;
	attribs = GLUFSHADERMANAGER.GetShaderAttribLocations(Prog);
	uniforms = GLUFSHADERMANAGER.GetShaderUniformLocations(Prog);

	// Get a handle for our "MVP" uniform
	GLuint MatrixID = uniforms["MVP"];
	GLuint ViewMatrixID = uniforms["V"];
	GLuint ModelMatrixID = uniforms["M"];

	GLuint TextureID = uniforms["myTextureSampler"];
	GLuint LightID = uniforms["LightPosition_worldspace"];

	// Get a handle for our buffers
	GLuint vertexPosition_modelspaceID = attribs["vertexPosition_modelspace"];
	GLuint vertexUVID = attribs["vertexUV"];
	GLuint vertexNormal_modelspaceID = attribs["vertexNormal_modelspace"];

	GLUFVertexAttribMap attributes;
	attributes.insert(GLUFVertexAttribPair(GLUF_VERTEX_ATTRIB_POSITION, GLUFVertAttrib(vertexPosition_modelspaceID, 4, 3, GL_FLOAT)));
	attributes.insert(GLUFVertexAttribPair(GLUF_VERTEX_ATTRIB_UV0, GLUFVertAttrib(vertexUVID, 4, 2, GL_FLOAT)));
	attributes.insert(GLUFVertexAttribPair(GLUF_VERTEX_ATTRIB_NORMAL, GLUFVertAttrib(vertexNormal_modelspaceID, 4, 3, GL_FLOAT)));

	std::string path = "suzanne.obj.model";
	
	//load from assimp
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(path,
		aiProcess_CalcTangentSpace |
		aiProcess_Triangulate |
		aiProcess_JoinIdenticalVertices |
		aiProcess_SortByPType);

	//load up the locations

	GLUFVertexArray* vertexData = LoadVertexArrayFromScene(scene, attributes);
	if (!vertexData)
		EXIT_FAILURE;

	GLUFVertexArray* vertexData2 = LoadVertexArrayFromScene(scene, attributes);
	if (!vertexData2)
		EXIT_FAILURE;


	//load texture
	GLuint texture = LoadTextureFromFile(L"uvmap.dds", TFF_DDS);


	float ellapsedTime = 0.0f;
	float prevTime = 0.0f;
	float currTime = 0.0f;


	printf("%i.%i", g_GLVersionMajor, g_GLVersionMinor);

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
		
		
		// Enable depth test
		glEnable(GL_DEPTH_TEST);
		// Accept fragment if it closer to the camera than the former one
		glDepthFunc(GL_LESS);

		// Cull triangles which normal is not towards the camera
		glEnable(GL_CULL_FACE);

		GLUFSHADERMANAGER.UseProgram(Prog);
		glm::mat4 ProjectionMatrix = glm::perspective(DEG_TO_RAD_F(50), ratio, 0.1f, 1000.0f);
		glm::mat4 ViewMatrix = glm::mat4();
		glm::mat4 ModelMatrix = glm::translate(glm::mat4(), glm::vec3(-1.5f, 0.0f, -5.0f)) * glm::toMat4(glm::quat(glm::vec3(0.0f, 2.0f * currTime, 0.0f)));
		glm::mat4 MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;

		// Send our transformation to the currently bound shader, 
		// in the "MVP" uniform
		glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
		glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix[0][0]);
		glUniformMatrix4fv(ViewMatrixID, 1, GL_FALSE, &ViewMatrix[0][0]);

		glm::vec3 lightPos = glm::vec3(4, 4, 4);
		glUniform3f(LightID, lightPos.x, lightPos.y, lightPos.z);

		// Bind our texture in Texture Unit 0
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture);
		// Set our "myTextureSampler" sampler to user Texture Unit 0
		glUniform1i(TextureID, 0);

		vertexData->Draw();

		ModelMatrix = glm::translate(glm::mat4(), glm::vec3(1.5f, 0.0f, -5.0f)) * glm::toMat4(glm::quat(glm::vec3(0.0f, 2.0f * currTime, 0.0f)));
		MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;

		// Send our transformation to the currently bound shader, 
		// in the "MVP" uniform
		glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
		glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix[0][0]);
		glUniformMatrix4fv(ViewMatrixID, 1, GL_FALSE, &ViewMatrix[0][0]);
		
		vertexData2->Draw();

		//render dialog last(overlay)
		//if ((int)currTime % 2)
			dlg->OnRender(ellapsedTime);
			//dlg->DrawRect(rc, GLUF::Color(255, 0, 0, 255));

		// Swap buffers
		glfwSwapBuffers(window);
		glfwPollEvents();

		//prevTime = currTime;

	} // Check if the ESC key was pressed or the window was closed
	while (glfwWindowShouldClose(window) == 0);


	glfwDestroyWindow(window);

	// Close OpenGL window and terminate GLFW
	glfwTerminate();

	GLUFTerminate();

	return 0;
}
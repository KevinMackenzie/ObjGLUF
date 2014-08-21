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


	GLuint texPtr = LoadTextureFromFile(L"dxutcontrolstest.dds", TFF_DDS);
	GLUFInitGui(window, MsgProc, texPtr);

	resMan = new GLUFDialogResourceManager();
	dlg = new GLUFDialog();
	dlg->Init(resMan);
	dlg->SetCallback(CtrlMsgProc);//TODO: fix caption
	dlg->SetCaptionText(L"Caption");
	dlg->SetCaptionHeight(0.03625f);
	dlg->LockPosition(false);
	dlg->EnableCaption(true);
	dlg->SetSize(0.8f, 0.8f);
	dlg->SetLocation(0.1f, 0.1f);
	dlg->SetBackgroundColors(Color(0, 128, 0, 128));
	dlg->EnableKeyboardInput(true);

	dlg->AddButton(0, L"Button", 0.05f, 0.01f, 0.125f, 0.03625f);

	GLUFListBox* box;
	dlg->AddListBox(1, 0.2f, 0.2f, 0.125f, 0.35f, GLUFListBox::MULTISELECTION, &box);
	dlg->AddCheckBox(2, L"Check Box", 0.3f, 0.1f, 0.03625f, 0.03625f);
	dlg->AddRadioButton(3, 0, L"Button 1", 0.4f, 0.4f, 0.03625f, 0.03625f, true);
	dlg->AddRadioButton(4, 0, L"Button 2", 0.4f, 0.45f, 0.03625f, 0.03625f);
	dlg->AddRadioButton(5, 0, L"Button 3", 0.4f, 0.5f, 0.03625f, 0.03625f);
	dlg->AddStatic(6, L"Static", 0.1f, 0.6f, 0.175f, 0.03625f);
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
	box->AddItem(L"Item 29", nullptr);





	std::string path = "suzanne.obj.model";
	
	//load from assimp
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(path,
		aiProcess_CalcTangentSpace |
		aiProcess_Triangulate |
		aiProcess_JoinIdenticalVertices |
		aiProcess_SortByPType);


	GLUFVertexArray* vertexData = LoadVertexArrayFromScene(scene);
	if (!vertexData)
		EXIT_FAILURE;

	//load shaders
	GLUFProgramPtr Prog;

	GLUFShaderPathList paths;
	paths.insert(std::pair<GLUFShaderType, std::wstring>(SH_VERTEX_SHADER,   L"StandardShadingVert.glsl"));
	paths.insert(std::pair<GLUFShaderType, std::wstring>(SH_FRAGMENT_SHADER, L"StandardShadingFrag.glsl"));
	Prog = GLUFSHADERMANAGER.CreateProgram(paths);

	//load texture
	GLuint texture = LoadTextureFromFile(L"uvmap.dds", TFF_DDS);


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
		
		
		// Enable depth test
		glEnable(GL_DEPTH_TEST);
		// Accept fragment if it closer to the camera than the former one
		glDepthFunc(GL_LESS);

		// Cull triangles which normal is not towards the camera
		glEnable(GL_CULL_FACE);

		GLUFSHADERMANAGER.UseProgram(Prog);
		glm::mat4 ProjectionMatrix = glm::perspective(DEG_TO_RAD_F(50), ratio, 0.1f, 1000.0f);
		glm::mat4 ViewMatrix = glm::mat4();
		glm::mat4 ModelMatrix = glm::translate(glm::mat4(), glm::vec3(0.0f, 0.0f, -5.0f)) * glm::toMat4(glm::quat(glm::vec3(0.0f, 2.0f * currTime, 0.0f)));
		glm::mat4 MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;

		// Send our transformation to the currently bound shader, 
		// in the "MVP" uniform
		glUniformMatrix4fv(2, 1, GL_FALSE, &MVP[0][0]);
		glUniformMatrix4fv(0, 1, GL_FALSE, &ModelMatrix[0][0]);
		glUniformMatrix4fv(1, 1, GL_FALSE, &ViewMatrix[0][0]);

		glm::vec3 lightPos = glm::vec3(4, 4, 4);
		glUniform3f(3, lightPos.x, lightPos.y, lightPos.z);

		// Bind our texture in Texture Unit 0
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture);
		// Set our "myTextureSampler" sampler to user Texture Unit 0
		glUniform1i(5, 0);

		vertexData->Draw();

		//render dialog last(overlay)
		dlg->OnRender(ellapsedTime);

		// Swap buffers
		glfwSwapBuffers(window);
		glfwPollEvents();

		//prevTime = currTime;

	} // Check if the ESC key was pressed or the window was closed
	while (glfwWindowShouldClose(window) == 0);


	glfwDestroyWindow(window);

	// Close OpenGL window and terminate GLFW
	glfwTerminate();

	return 0;
}
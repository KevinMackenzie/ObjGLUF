// TestProject.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
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
static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);
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


	GLuint texPtr = GLUF::LoadTextureFromFile(L"dxutcontrolstest.dds", TFF_DDS);
	GLUFInitGui(window, MsgProc, texPtr);

	resMan = new GLUFDialogResourceManager();
	dlg = new GLUFDialog();
	dlg->Init(resMan);
	dlg->SetCallback(CtrlMsgProc);//TODO: fix caption
	dlg->SetCaptionText(L"Caption");
	dlg->SetCaptionHeight(0.03625f);
	dlg->SetSize(0.8f, 0.8f);
	dlg->SetLocation(0.1f, 0.1f);
	dlg->SetBackgroundColors(Color(0, 128, 0, 255));
	dlg->EnableKeyboardInput(true);

	dlg->AddButton(0, L"Button", 0.05f, 0.01f, 0.125f, 0.03625f);

	/*std::ifstream t("text.txt");
	std::string str;

	t.seekg(0, std::ios::end);
	str.reserve(t.tellg());
	t.seekg(0, std::ios::beg);

	str.assign((std::istreambuf_iterator<char>(t)),
		std::istreambuf_iterator<char>());

	dlg->AddEditBox(1, str, 0.1f, 0.1f, 0.5f, 0.5f);*/

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

		dlg->OnRender(ellapsedTime);

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
// TestProject.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#define USING_ASSIMP
#define GLUF_DEBUG
#define SUPPRESS_RADIAN_ERROR
#define SUPPRESS_UTF8_ERROR
#include "../ObjGLUF/GLUFGui.h"
#include <stdlib.h>
#include <stdio.h>
#include <fstream>
#include <iostream>
#include <vector>

using namespace GLUF;

GLFWwindow* window;

DialogResourceManagerPtr resMan;
DialogPtr dlg;

//extern FontPtr g_ArielDefault;

static void error_callback(int error, const char* description)
{
	fputs(description, stderr);
}
void MyErrorMethod(const std::string& message, const char* func, const char* file, unsigned int line)
{
    std::cout << "(" << func << " | " << line << "): " << message << std::endl;
	//pause
    char c;
    std::cin >> c;
}

bool MsgProc(_GUI_CALLBACK_PARAM)
{
	if (msg == KEY)
	{
		if (param1 == GLFW_KEY_ESCAPE && param3 == GLFW_RELEASE)
			//DialogOpened = !DialogOpened;
			dlg->SetMinimized(!dlg->GetMinimized());
		//return true;
	}

    /*if (msg != MessageType::CURSOR_ENTER || msg != MessageType::CURSOR_POS || msg != MessageType::FOCUS)
    {
        
    }*/

	resMan->MsgProc(_PASS_CALLBACK_PARAM);
	dlg->MsgProc(_PASS_CALLBACK_PARAM);

	return false;
}

void ControlEventCallback(Event evt, ControlPtr&, const EventCallbackReceivablePtr&) noexcept
{
	if (evt == EVENT_BUTTON_CLICKED)
	{
		printf("HORRAY\n");
	}
}

struct JustPositions : VertexStruct
{
    glm::vec3 pos;

    JustPositions(){};

    virtual void* operator&() const override
    {
        return (void*)&pos;
    }

    virtual size_t size() const override
    {
        return 12;
    }

    virtual size_t n_elem_size(size_t element)
    {
        return 12;
    }

    virtual void buffer_element(void* data, size_t element) override
    {
        pos = *static_cast<glm::vec3*>(data);
    }

    static GLVector<JustPositions> MakeMany(size_t howMany)
    {
        GLVector<JustPositions> ret;
        ret.reserve(howMany);

        for (size_t i = 0; i < howMany; ++i)
        {
            ret.push_back(JustPositions());
        }

        return ret;
    }

    JustPositions& operator=(const glm::vec3& other)
    {
        pos = other;
        return *this;
    }

    JustPositions& operator=(const JustPositions& other)
    {
        pos = other.pos;
        return *this;
    }
    JustPositions& operator=(JustPositions&& other)
    {
        pos = other.pos;
        return *this;
    }
};

#define USE_SEPARATE

int main(void)
{
    RegisterErrorMethod(MyErrorMethod);
	Init();


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

	InitOpenGLExtensions();
    
    //for testing purposes
    /*gGLVersionMajor = 2;
    gGLVersionMinor = 0;
    gGLVersion2Digit = 20;*/

	GLuint ctrlTex = LoadTextureFromFile(L"dxutcontrolstest.dds", TFF_DDS);
    InitGui(window, MsgProc, ctrlTex);

    resMan = std::make_shared<DialogResourceManager>();
    dlg = CreateDialog();
	dlg->Init(resMan);
    dlg->SetCallback(ControlEventCallback);//TODO: fix caption
	//dlg->SetCaptionText(L"Caption");
    //dlg->SetCaptionHeight(50);
    //dlg->EnableCaption(false);
	dlg->Lock(false);
	dlg->EnableAutoClamp();
	dlg->EnableGrabAnywhere();
	dlg->SetMinimized(false);
	//dlg->EnableCaption(false);
	dlg->SetSize(600, 600);
	dlg->SetLocation(50, 50);
	dlg->SetBackgroundColor(Color(0, 128, 0, 128));
	dlg->EnableKeyboardInput(true);
	Rect rc = { 0, 200, 200, 0 };


	std::wifstream t("text.txt");
	std::wstring str;

	t.seekg(0, std::ios::end);
	str.reserve(static_cast<uint32_t>(t.tellg()));
	t.seekg(0, std::ios::beg);

	str.assign((std::istreambuf_iterator<wchar_t>(t)),
		std::istreambuf_iterator<wchar_t>());
	//dlg->AddEditBox(10, str, 100, 100, 400, 400, GT_LEFT | GT_TOP | GT_MULTI_LINE);
	//dlg->AddEditBox(10, L"EditBoxEditBoxEditBoxEditBoxEditBox", 100, 100, 400, 35, Charset::Unicode, GT_LEFT | GT_TOP);

    //dlg->AddStatic(6, L"The Quick Brown Fox Jumped Over The Lazy Dog", { { 50 }, 500, 350, { 480 } });

    dlg->AddCheckBox(2, L"Check Box", { { 150 }, 50, 170, { 20 } });
    dlg->AddRadioButton(3, 0, L"Button 1", { { 200 }, 200, 220, { 180 } }, true);
    dlg->AddRadioButton(4, 0, L"Button 2", { { 200 }, 250, 220, { 230 } });
    dlg->AddRadioButton(5, 0, L"Button 3", { { 200 }, 300, 220, { 280 } });

    //dlg->AddSlider(1, { { 100 }, 100, 400, { 50 } }, 0, 15, 5);
    dlg->AddButton(0, L"Button", { { 25 }, 20, 75, { 10 } });

    std::shared_ptr<ListBoxPtr> boxBase = std::make_shared<ListBoxPtr>(nullptr);
    dlg->AddListBox(7, { { 0 }, 500, 180, { 200 } }, ListBox::MULTISELECTION, boxBase);

    auto box = *boxBase;
	box->AddItem(L"Item 0");
	box->AddItem(L"Item 1");
	box->AddItem(L"Item 2");
	box->AddItem(L"Item 3");
	box->AddItem(L"Item 4");
	box->AddItem(L"Item 5");
	box->AddItem(L"Item 6");
	box->AddItem(L"Item 7");
	box->AddItem(L"Item 8");
	box->AddItem(L"Item 9");
	box->AddItem(L"Item 10");
	box->AddItem(L"Item 11");
	box->AddItem(L"Item 12");
	box->AddItem(L"Item 13");
	box->AddItem(L"Item 14");
	box->AddItem(L"Item 15");
	box->AddItem(L"Item 16");
	box->AddItem(L"Item 17");
	box->AddItem(L"Item 18");
	box->AddItem(L"Item 19");
	box->AddItem(L"Item 20");
	box->AddItem(L"Item 21");
	box->AddItem(L"Item 22");
	box->AddItem(L"Item 23");
	box->AddItem(L"Item 24");
	box->AddItem(L"Item 25");
	box->AddItem(L"Item 26");
	box->AddItem(L"Item 27");
	box->AddItem(L"Item 28");
	box->AddItem(L"Item 29");


	//load shaders
	//ProgramPtr frag, vert;
	/*ProgramPtr Prog;

	//ShaderPathList paths;
	//paths.insert(std::pair<ShaderType, std::wstring>(SH_VERTEX_SHADER, L"Shaders/BasicLighting120.vert.glsl"));
	//paths.insert(std::pair<ShaderType, std::wstring>(SH_FRAGMENT_SHADER, L"Shaders/BasicLighting120.frag.glsl"));
	
	ShaderSourceList sources;
	unsigned long len = 0;

    std::string text;
    std::vector<char> rawMem;
    LoadFileIntoMemory(L"Shaders/BasicLighting120.vert.glsl", rawMem);
    LoadBinaryArrayIntoString(rawMem, text);

	text += '\n';
	sources.insert(std::pair<ShaderType, const char*>(SH_VERTEX_SHADER, text.c_str()));

    std::string text1;
    LoadFileIntoMemory(L"Shaders/BasicLighting120.frag.glsl", rawMem);
    LoadBinaryArrayIntoString(rawMem, text1);
	text1 += '\n';
	sources.insert(std::pair<ShaderType, const char*>(SH_FRAGMENT_SHADER, text1.c_str()));

	SHADERMANAGER.CreateProgram(Prog, sources);*/

#ifdef USE_SEPARATE
    ProgramPtrList Progs;

    //ShaderPathList paths;
    //paths.insert(std::pair<ShaderType, std::wstring>(SH_VERTEX_SHADER, L"Shaders/BasicLighting120.vert.glsl"));
    //paths.insert(std::pair<ShaderType, std::wstring>(SH_FRAGMENT_SHADER, L"Shaders/BasicLighting120.frag.glsl"));

    ShaderSourceList VertSources, FragSources;
    unsigned long len = 0;

    std::string text;
    std::vector<char> rawMem;
    LoadFileIntoMemory(L"Shaders/BasicLighting120.vert.glsl", rawMem);
    LoadBinaryArrayIntoString(rawMem, text);

    text += '\n';
    VertSources.insert(std::pair<ShaderType, const char*>(SH_VERTEX_SHADER, text.c_str()));

    ProgramPtr Vert;
    SHADERMANAGER.CreateProgram(Vert, VertSources, true);

    std::string text1;
    LoadFileIntoMemory(L"Shaders/BasicLighting120.frag.glsl", rawMem);
    LoadBinaryArrayIntoString(rawMem, text1);
    text1 += '\n';
    FragSources.insert(std::pair<ShaderType, const char*>(SH_FRAGMENT_SHADER, text1.c_str()));

    ProgramPtr Frag;
    SHADERMANAGER.CreateProgram(Frag, FragSources, true);

    ProgramPtrList programs;
    programs.push_back(Vert);
    programs.push_back(Frag);

    SepProgramPtr Prog;

    SHADERMANAGER.CreateSeparateProgram(Prog, programs);

#else
    //ShaderPathList paths;
    //paths.insert(std::pair<ShaderType, std::wstring>(SH_VERTEX_SHADER, L"Shaders/BasicLighting120.vert.glsl"));
    //paths.insert(std::pair<ShaderType, std::wstring>(SH_FRAGMENT_SHADER, L"Shaders/BasicLighting120.frag.glsl"));

    ProgramPtr Prog;

    ShaderSourceList Sources;
    unsigned long len = 0;

    std::string text;
    std::vector<char> rawMem;
    LoadFileIntoMemory(L"Shaders/BasicLighting120.vert.glsl", rawMem);
    LoadBinaryArrayIntoString(rawMem, text);

    text += '\n';
    Sources.insert(std::pair<ShaderType, const char*>(SH_VERTEX_SHADER, text.c_str()));

    std::string text1;
    LoadFileIntoMemory(L"Shaders/BasicLighting120.frag.glsl", rawMem);
    LoadBinaryArrayIntoString(rawMem, text1);
    text1 += '\n';
    Sources.insert(std::pair<ShaderType, const char*>(SH_FRAGMENT_SHADER, text1.c_str()));

    SHADERMANAGER.CreateProgram(Prog, Sources);
#endif


	VariableLocMap attribs, uniforms;
	attribs = SHADERMANAGER.GetShaderAttribLocations(Prog);
	uniforms = SHADERMANAGER.GetShaderUniformLocations(Prog);

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

	VertexAttribMap attributes;
	attributes.insert(VertexAttribPair(GLUF_VERTEX_ATTRIB_POSITION, VertAttrib(vertexPosition_modelspaceID, 4, 3, GL_FLOAT)));
	attributes.insert(VertexAttribPair(GLUF_VERTEX_ATTRIB_UV0, VertAttrib(vertexUVID, 4, 2, GL_FLOAT)));
	attributes.insert(VertexAttribPair(GLUF_VERTEX_ATTRIB_NORMAL, VertAttrib(vertexNormal_modelspaceID, 4, 3, GL_FLOAT)));

	std::string path = "suzanne.obj.model";
	
	//load from assimp
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(path,
		aiProcess_CalcTangentSpace |
		aiProcess_Triangulate |
		aiProcess_JoinIdenticalVertices |
		aiProcess_SortByPType);

	//load up the locations

	/*std::shared_ptr<VertexArray> vertexData = LoadVertexArrayFromScene(scene, attributes);
	if (!vertexData)
		EXIT_FAILURE;*/

	std::shared_ptr<VertexArray> vertexData2 = LoadVertexArrayFromScene(scene, attributes);
	if (!vertexData2)
		EXIT_FAILURE;


	//load texture
	GLuint texture = LoadTextureFromFile(L"uvmap.dds", TFF_DDS);


	float ellapsedTime = 0.0f;
	float prevTime = 0.0f;
	float currTime = 0.0f;


	printf("%i.%i", GetGLVersionMajor(), GetGLVersionMinor);

	// Cull triangles which normal is not towards the camera
	//glEnable(GL_CULL_FACE);

	//::TextHelper *textHelper = new ::TextHelper(resMan);
	//textHelper->Init(20);


	/*GLuint skycubemap = ::LoadTextureFromFile(L"afternoon_sky.cubemap.dds", TTF_DDS_CUBEMAP);

    sources.clear();
    LoadFileIntoMemory(L"Shaders/BasicLighting120.vert.glsl", rawMem);
    LoadBinaryArrayIntoString(rawMem, text);
	text += '\n';
	sources.insert(std::pair<ShaderType, const char*>(SH_VERTEX_SHADER, text.c_str()));

    LoadFileIntoMemory(L"Shaders/BasicLighting120.frag.glsl", rawMem);
    LoadBinaryArrayIntoString(rawMem, text1);
	text1 += '\n';
	sources.insert(std::pair<ShaderType, const char*>(SH_FRAGMENT_SHADER, text1.c_str()));

    ProgramPtr sky;
    SHADERMANAGER.CreateProgram(sky, sources);

	VariableLocMap attribLocations = SHADERMANAGER.GetShaderAttribLocations(sky);
	VariableLocMap unifLocations = SHADERMANAGER.GetShaderUniformLocations(sky);

	GLuint sampLoc = unifLocations["m_tex0"];
	GLuint mvpLoc = unifLocations["_mvp"];

	VariableLocMap::iterator it;

	VertexArray m_pVertexArray;
	GLuint mPositionLoc = 0;

	it = attribLocations.find("_position");
	if (it != attribLocations.end())
	{
		m_pVertexArray.AddVertexAttrib(VertAttrib(it->second, 4, 3, GL_FLOAT));
		mPositionLoc = it->second;
	}

    GLVector<JustPositions> verts = JustPositions::MakeMany(8);
	float depth;
	float val = depth = 10.0f;
    verts[0] = glm::vec3(val, -val, -depth);
    verts[1] = glm::vec3(-val, -val, -depth);
    verts[2] = glm::vec3(-val, val, -depth);
    verts[3] = glm::vec3(val, val, -depth);

	//front
    verts[4] = glm::vec3(-val, -val, depth);
    verts[5] = glm::vec3(val, -val, depth);
    verts[6] = glm::vec3(val, val, depth);
    verts[7] = glm::vec3(-val, val, depth);

	std::vector<glm::u32vec3> mTriangles;

	//north
	mTriangles.push_back(glm::u32vec3(4, 5, 6));
	mTriangles.push_back(glm::u32vec3(6, 7, 4));

	//south
	mTriangles.push_back(glm::u32vec3(0, 1, 2));
	mTriangles.push_back(glm::u32vec3(2, 3, 0));

	//east
	mTriangles.push_back(glm::u32vec3(1, 4, 7));
	mTriangles.push_back(glm::u32vec3(7, 2, 1));

	//west
	mTriangles.push_back(glm::u32vec3(5, 0, 3));
	mTriangles.push_back(glm::u32vec3(3, 6, 5));

	//top
	mTriangles.push_back(glm::u32vec3(7, 6, 3));
	mTriangles.push_back(glm::u32vec3(3, 2, 7));

	//bottom
	mTriangles.push_back(glm::u32vec3(1, 0, 5));
	mTriangles.push_back(glm::u32vec3(5, 4, 1));

	m_pVertexArray.BufferData(verts);
	m_pVertexArray.BufferIndices(mTriangles);*/

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

		//SHADERMANAGER.UseProgram(Prog); 
		glm::vec3 pos(3, 5, 6);
		glm::mat4 ProjectionMatrix = glm::perspective(DEG_TO_RAD_F(70), ratio, 0.1f, 1000.f);
		glm::mat4 ViewMatrix = glm::translate(glm::mat4(), -pos);
		glm::mat4 ModelMatrix = glm::translate(glm::mat4(), pos) * glm::toMat4(glm::quat(glm::vec3(0, DEG_TO_RAD_F(30) * currTime, 0)));// glm::translate(glm::mat4(), glm::vec3(-1.5f, 0.0f, -5.0f)) * glm::toMat4(glm::quat(glm::vec3(_PI_F / 2, 2.0f * currTime, 0.0f)));
		glm::mat4 MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;

		// Send our transformation to the currently bound shader, 
		// in the "MVP" uniform
		SHADERMANAGER.UseProgram(Prog);

#ifndef USE_SEPARATE

		glm::vec3 lightPos = glm::vec3(4, 4, 4);
		SHADERMANAGER.GLUniform3f(LightID, lightPos);

		// Bind our texture in Texture Unit 0
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture);
		// Set our "myTextureSampler" sampler to user Texture Unit 0
		SHADERMANAGER.GLUniform1i(TextureID, 0);

		ModelMatrix = glm::translate(glm::mat4(), glm::vec3(1.5f, 0.0f, -5.0f)) * glm::toMat4(glm::quat(glm::vec3(0.0f, 2.0f * currTime, 0.0f)));
		MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;

		// Send our transformation to the currently bound shader, 
		// in the "MVP" uniform
		SHADERMANAGER.GLUniformMatrix4f(MatrixID, MVP);
        SHADERMANAGER.GLUniformMatrix4f(ModelMatrixID, ModelMatrix);
        SHADERMANAGER.GLUniformMatrix4f(ViewMatrixID, ViewMatrix);
		
		vertexData2->Draw();

#else

        SHADERMANAGER.GLActiveShaderProgram(Prog, SH_FRAGMENT_SHADER);

        // Bind our texture in Texture Unit 0
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture);
        // Set our "myTextureSampler" sampler to user Texture Unit 0
        SHADERMANAGER.GLProgramUniform1i(Prog, TextureID, 0);

        ModelMatrix = glm::translate(glm::mat4(), glm::vec3(1.5f, 0.0f, -5.0f)) * glm::toMat4(glm::quat(glm::vec3(0.0f, 2.0f * currTime, 0.0f)));
        MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;

        SHADERMANAGER.GLActiveShaderProgram(Prog, SH_VERTEX_SHADER);

        // Send our transformation to the currently bound shader, 
        // in the "MVP" uniform
        SHADERMANAGER.GLProgramUniformMatrix4f(Prog, MatrixID, MVP);
        SHADERMANAGER.GLProgramUniformMatrix4f(Prog, ModelMatrixID, ModelMatrix);
        SHADERMANAGER.GLProgramUniformMatrix4f(Prog, ViewMatrixID, ViewMatrix);

        glm::vec3 lightPos = glm::vec3(4, 4, 4);
        SHADERMANAGER.GLProgramUniform3f(Prog, LightID, lightPos);

        vertexData2->Draw();

#endif

		//render dialog last(overlay)
		//if ((int)currTime % 2)
			dlg->OnRender(ellapsedTime);
			//dlg->DrawRect(rc, ::Color(255, 0, 0, 255));

        //sky box rendering stuff
		/*SHADERMANAGER.UseProgram(sky);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, skycubemap);
		glUniform1i(sampLoc, 0);
		glUniformMatrix4fv(mvpLoc, 1, GL_FALSE, &MVP[0][0]);
		glDisable(GL_DEPTH_CLAMP);
		glDepthMask(GL_FALSE);
		glDisable(GL_CULL_FACE);
		m_pVertexArray.Draw();
		glEnable(GL_CULL_FACE);
		glDepthMask(GL_TRUE);
		glEnable(GL_DEPTH_CLAMP);*/

		//glEnable(GL_BLEND);
		//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		/*textHelper->Begin(0);
		textHelper->SetInsertionPos(Point(100, 100));
		textHelper->DrawTextLine(L"TESTING");

		SHADERMANAGER.UseProgram(linesprog);
		glUniformMatrix4fv(0, 1, GL_FALSE, &MVP[0][0]);
		m_Squares.Draw();*/

		// Swap buffers
        glfwSwapBuffers(window);
		glfwPollEvents();

		//prevTime = currTime;

	} // Check if the ESC key was pressed or the window was closed
	while (glfwWindowShouldClose(window) == 0);


	glfwDestroyWindow(window);

	// Close OpenGL window and terminate GLFW
	glfwTerminate();

	Terminate();

	return 0;
}
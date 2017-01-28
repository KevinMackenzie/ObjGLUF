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

#include <glm/gtc/matrix_transform.hpp>
#include <glm\gtx\transform.hpp>

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
    //char c;
    //std::cin >> c;
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
		char* copy = new char[sizeof(glm::vec3)];
		memcpy(copy, &pos[0], sizeof(glm::vec3));
		return (void*)&copy[0];
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

void myunexpected()
{
    int i = 0;
}

std::string RemoveChar(std::string str, char ch)
{
	std::stringstream ss;
	for (auto s : str)
	{
		if (s == ch)
			continue;
		ss << s;
	}

	return ss.str();
}

GLUF::GLVector<JustPositions> csvToArray(std::string text)
{
	auto lines = GLUF::SplitStr(text, '\n', false, true);
	auto ret = JustPositions::MakeMany(lines.size());
	int j = 0;
	for (auto it : lines)
	{
		auto columns = GLUF::SplitStr(it, ',');
		glm::vec3 row;
		for (int i = 0; i < columns.size(); ++i)
		{
			columns[i] = RemoveChar(columns[i], ' ');
			row[i] = (atof(columns[i].c_str()));
		}
		ret[j] = row;
		j++;
	}
	return ret;
}



class DataGrapher
{
	void MakeLinesArray(std::string csvFilePath, GLUF::VertexArrayAoS& vao)
	{
		std::string csvText;
		try
		{
			GLUF::LoadFileIntoMemory(csvFilePath, csvText);
		}
		catch (...) {}
		auto data = csvToArray(csvText);

		std::vector<GLuint> indices;
		for (int i = 1; i < data.size(); ++i)
		{
			indices.push_back(i - 1);
			indices.push_back(i);
		}

		vao.BufferData(data);
		vao.BufferIndices(indices);
	}
	
	void MakeAxesArray(glm::vec2 axisMins, glm::vec2 axisMaxs, float tallyWidth, float tallyHeight, GLUF::VertexArrayAoS& vao)
	{
		int xTallyCountx2 = ((axisMaxs.x - axisMins.x) / tallyWidth) * 2;
		int yTallyCountx2 = ((axisMaxs.y - axisMins.y) / tallyWidth) * 2;
		auto ret = JustPositions::MakeMany(xTallyCountx2 + yTallyCountx2 + 3);

		std::vector<GLuint> indices;
		indices.resize(xTallyCountx2 + yTallyCountx2 + 4);

		//the first three vertices are the axes points
		ret[0] = glm::vec3(0, axisMins.x, axisMins.y);
		ret[1] = glm::vec3(0, axisMins.x, axisMaxs.y);
		ret[2] = glm::vec3(0, axisMaxs.x, axisMins.y);

		indices[0] = 0;
		indices[1] = 1;
		indices[2] = 0;
		indices[3] = 2;

		for (int i = 0; i < xTallyCountx2; i += 2)
		{
			int j = i / 2;
			ret[i + 3] = glm::vec3(0, axisMins.x + j*tallyWidth, axisMins.y + tallyHeight / 2);
			ret[i + 4] = glm::vec3(0, axisMins.x + j*tallyWidth, axisMins.y - tallyHeight / 2);

			indices[i + 4] = i + 3;
			indices[i + 5] = i + 4;
		}

		for (int i = 0; i < yTallyCountx2; i += 2)
		{
			int j = i / 2;
			ret[i + 3 + xTallyCountx2] = glm::vec3(0, axisMins.x + tallyHeight / 2, axisMins.y + j*tallyWidth);
			ret[i + 4 + xTallyCountx2] = glm::vec3(0, axisMins.x - tallyHeight / 2, axisMins.y + j*tallyWidth);

			indices[i + 4 + xTallyCountx2] = i + 3 + xTallyCountx2;
			indices[i + 5 + xTallyCountx2] = i + 4 + xTallyCountx2;
		}

		vao.BufferData(ret);
		vao.BufferIndices(indices);
	}
	
	std::vector<GLUF::VertexArrayAoS> dataVAOs;
	GLUF::VertexArrayAoS axesVAO;


	std::vector<glm::vec3> colors;
	ProgramPtr prog;


	// Get a handle for our "MVP" uniform
	GLuint MatrixID = 0;
	GLuint TimeID = 0;
	GLuint ColorID = 0;
	GLuint TimeRangeID = 0;

public:
	DataGrapher(const std::map<std::string, glm::vec3>& dataFilePathsAndColors, const std::string& vertShader, const std::string& fragShader)
	{
		//TODO: automate this better


		ShaderSourceList Sources;
		unsigned long len = 0;

		std::string text;
		std::vector<char> rawMem;
		LoadFileIntoMemory(vertShader, rawMem);
		LoadBinaryArrayIntoString(rawMem, text);

		text += '\n';
		Sources.insert(std::pair<ShaderType, const char*>(SH_VERTEX_SHADER, text.c_str()));

		std::string text1;
		LoadFileIntoMemory(fragShader, rawMem);
		LoadBinaryArrayIntoString(rawMem, text1);
		text1 += '\n';
		Sources.insert(std::pair<ShaderType, const char*>(SH_FRAGMENT_SHADER, text1.c_str()));

		SHADERMANAGER.CreateProgram(prog, Sources);

		VariableLocMap attribs, uniforms;
		attribs = SHADERMANAGER.GetShaderAttribLocations(prog);
		uniforms = SHADERMANAGER.GetShaderUniformLocations(prog);

		// Get a handle for our "MVP" uniform
		MatrixID = uniforms["MVP"];
		TimeID = uniforms["Time"];
		ColorID = uniforms["Color"];
		TimeRangeID = uniforms["TimeRange"];

		// Get a handle for our buffers
		GLuint vertexPosition_modelspaceID = attribs["vertexPosition_modelspace"];

		VertexAttribInfo info = VertAttrib(vertexPosition_modelspaceID, 4, 3, GL_FLOAT);

		int i = 0;
		for (std::pair<std::string, glm::vec3> it : dataFilePathsAndColors)
		{
			dataVAOs.push_back(VertexArrayAoS(GL_LINES, GL_DYNAMIC_DRAW));
			dataVAOs[i].AddVertexAttrib(info);
			MakeLinesArray(it.first, dataVAOs[i]);
			colors.push_back(it.second);

			++i;
		}

		axesVAO = VertexArrayAoS(GL_LINES, GL_DYNAMIC_DRAW);
		axesVAO.AddVertexAttrib(info);
		MakeAxesArray(glm::vec2(0, 0), glm::vec2(200, 200), 10, 10, axesVAO);
	}

	void Draw(int width, int height, float time)
	{
		glm::mat4 ProjectionMatrix = glm::ortho<float>(-20, width / 2 - 20, -20, height / 2 - 20);
		glm::mat4 DataModelMatrix;// = glm::translate(glm::vec3(20, 20, 0));
		glm::mat4 ScaleMatrix;// = glm::scale(glm::vec3(4.0f, 4.0f, 1.0));

		glm::mat4 DataMVP = ProjectionMatrix * DataModelMatrix * ScaleMatrix;
		glm::mat4 AxesMVP = ProjectionMatrix * ScaleMatrix;

		// Send our transformation to the currently bound shader, 
		// in the "MVP" uniform
		SHADERMANAGER.UseProgram(prog);

		// Send our transformation to the currently bound shader, 
		// in the "MVP" uniform
		SHADERMANAGER.GLUniformMatrix4f(MatrixID, DataMVP);
		SHADERMANAGER.GLUniform1f(TimeRangeID, 2);
		SHADERMANAGER.GLUniform1f(TimeID, time);

		for (int i = 0; i < dataVAOs.size(); ++i)
		{
			SHADERMANAGER.GLUniform3f(ColorID, colors[i]);

			dataVAOs[i].Draw();
		}

		SHADERMANAGER.GLUniformMatrix4f(MatrixID, AxesMVP);
		SHADERMANAGER.GLUniform1f(TimeID, 1);
		SHADERMANAGER.GLUniform3f(ColorID, glm::vec3(1, 1, 1));
		axesVAO.Draw();
	}
};

//#define USE_SEPARATE

int main(void)
{

    std::set_unexpected(myunexpected);

    RegisterErrorMethod(MyErrorMethod);
	Init();


	//glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);
	//glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 1);
	//glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);

	window = glfwCreateWindow(800, 400, "Simple example", NULL, NULL);
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
	GLUF::InitGui(window, MsgProc, ctrlTex);

	resMan = std::make_shared<GLUF::DialogResourceManager>();

	FontPtr _font;
	std::vector<char> fontMem;
	LoadFileIntoMemory("arial.ttf", fontMem);
	LoadFont(_font, fontMem, 12);
	resMan->AddFont(_font, 16, GLUF::FontWeight::FONT_WEIGHT_NORMAL);
	dlg = CreateDialog();
	dlg->Init(resMan);
	resMan->RegisterDialog(dlg);

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

    /*ProgramPtr Prog;

    ShaderSourceList Sources;
    unsigned long len = 0;

    std::string text;
    std::vector<char> rawMem;
    LoadFileIntoMemory(L"Shaders/Lines.vert.glsl", rawMem);
    LoadBinaryArrayIntoString(rawMem, text);

    text += '\n';
    Sources.insert(std::pair<ShaderType, const char*>(SH_VERTEX_SHADER, text.c_str()));

    std::string text1;
    LoadFileIntoMemory(L"Shaders/Lines.frag.glsl", rawMem);
    LoadBinaryArrayIntoString(rawMem, text1);
    text1 += '\n';
    Sources.insert(std::pair<ShaderType, const char*>(SH_FRAGMENT_SHADER, text1.c_str()));

    SHADERMANAGER.CreateProgram(Prog, Sources);*/
#endif


	/*VariableLocMap attribs, uniforms;
	attribs = SHADERMANAGER.GetShaderAttribLocations(Prog);
	uniforms = SHADERMANAGER.GetShaderUniformLocations(Prog);

	// Get a handle for our "MVP" uniform
	GLuint MatrixID = uniforms["MVP"];
	GLuint TimeID = uniforms["Time"];
	GLuint ColorID = uniforms["Color"];
	GLuint TimeRangeID = uniforms["TimeRange"];

	// Get a handle for our buffers
	GLuint vertexPosition_modelspaceID = attribs["vertexPosition_modelspace"];

	auto axes = VertexArrayAoS(GL_LINES, GL_DYNAMIC_DRAW);
	axes.AddVertexAttrib(VertAttrib(vertexPosition_modelspaceID, 4, 3, GL_FLOAT));
	MakeAxesArray(glm::vec2(20, 20), glm::vec2(200, 200), 10, 10, axes);

	std::vector<VertexArrayAoS> linesVector;
	for (int i = 0; i < 10; ++i)
	{
		linesVector.push_back(VertexArrayAoS(GL_LINES, GL_DYNAMIC_DRAW));
		linesVector[i].AddVertexAttrib(VertAttrib(vertexPosition_modelspaceID, 4, 3, GL_FLOAT));
	}

	MakeLinesArray("lines/Step1.csv", linesVector[0]);
	MakeLinesArray("lines/Step.9.csv", linesVector[1]);
	MakeLinesArray("lines/Step.8.csv", linesVector[2]);
	MakeLinesArray("lines/Step.7.csv", linesVector[3]);
	MakeLinesArray("lines/Step.6.csv", linesVector[4]);
	MakeLinesArray("lines/Step.5.csv", linesVector[5]);
	MakeLinesArray("lines/Step.4.csv", linesVector[6]);
	MakeLinesArray("lines/Step.3.csv", linesVector[7]);
	MakeLinesArray("lines/Step.2.csv", linesVector[8]);
	MakeLinesArray("lines/Step.1.csv", linesVector[9]);

	std::vector<glm::vec3> colors;
	colors.push_back({ 0,0,1 });
	colors.push_back({ 0,1,0 });
	colors.push_back({ 0,1,1 });
	colors.push_back({ 1,0,0 });
	colors.push_back({ 1,0,1 });
	colors.push_back({ 1,1,0 });
	colors.push_back({ 1,1,1 });
	colors.push_back({ .5,0,0 });
	colors.push_back({ 1,.5,.5 });
	colors.push_back({ .5,.5,1 });*/

	auto tHelper = CreateTextHelper(resMan);

	//load up the locations

	/*std::shared_ptr<VertexArray> vertexData = LoadVertexArrayFromScene(scene, attributes);
	if (!vertexData)
		EXIT_FAILURE;*/

	std::map<std::string, glm::vec3> paths;
	paths["lines/Step1.csv"] = { 0,0,1 };
	paths["lines/Step.9.csv"] = { 0,1,0 };
	paths["lines/Step.8.csv"] = { 0,1,1 };
	paths["lines/Step.7.csv"] = { 1,0,0 };
	paths["lines/Step.6.csv"] = { 1,0,1 };
	paths["lines/Step.5.csv"] = { 1,1,0 };
	paths["lines/Step.4.csv"] = { 1,1,1 };
	paths["lines/Step.3.csv"] = { .5,0,0 };
	paths["lines/Step.2.csv"] = { 1,.5,.5 };
	paths["lines/Step.1.csv"] = { .5,.5,1 };

	auto dataHelper = DataGrapher(paths, "Shaders/Lines.vert.glsl", "Shaders/Lines.frag.glsl");


	float ellapsedTime = 0.0f;
	float prevTime = 0.0f;
	float currTime = 0.0f;


	printf("%i.%i", GetGLVersionMajor(), GetGLVersionMinor);

	// Cull triangles which normal is not towards the camera
	//glEnable(GL_CULL_FACE);

	//::TextHelper *textHelper = new ::TextHelper(resMan);
	//textHelper->Init(20);

	//glEnable(GL_LINE_SMOOTH);
	glLineWidth(1);


	glm::vec2 axisMins(0,0);
	glm::vec2 axisMaxs(150,50);
	float tallyMarkWidth = 10;

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

	float timeStart = -1;
	do{
		if (timeStart == -1)
			timeStart = (float)glfwGetTime();

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

		//make sure to only draw what is in view
		glDisable(GL_DEPTH_CLAMP);

		dataHelper.Draw(width, height, currTime - timeStart);

		//SHADERMANAGER.UseProgram(Prog); 
		/*float dataSpaceHeight = height / 5;
		glm::vec3 pos(0, 0, 80);
		glm::mat4 ProjectionMatrix = glm::ortho<float>(-20, width/2 - 20, -20, height/2 - 20);
		glm::mat4 DataModelMatrix = glm::translate(glm::vec3(20, 20, 0));
		glm::mat4 ScaleMatrix;// = glm::scale(glm::vec3(4.0f, 4.0f, 1.0));

		glm::mat4 DataMVP = ProjectionMatrix * DataModelMatrix*ScaleMatrix;
		glm::mat4 AxesMVP = ProjectionMatrix * ScaleMatrix;

		// Send our transformation to the currently bound shader, 
		// in the "MVP" uniform
		SHADERMANAGER.UseProgram(Prog);*/

#ifndef USE_SEPARATE
		
		// Send our transformation to the currently bound shader, 
		// in the "MVP" uniform
		/*SHADERMANAGER.GLUniformMatrix4f(MatrixID, DataMVP);
		SHADERMANAGER.GLUniform1f(TimeRangeID, 2);
		SHADERMANAGER.GLUniform1f(TimeID, currTime - timeStart);
		
		for (int i = 0; i < linesVector.size(); ++i)
		{
			SHADERMANAGER.GLUniform3f(ColorID, colors[i]);

			linesVector[i].Draw();
		}

		SHADERMANAGER.GLUniformMatrix4f(MatrixID, AxesMVP);
		SHADERMANAGER.GLUniform1f(TimeID, 1);
		SHADERMANAGER.GLUniform3f(ColorID, glm::vec3(1, 1, 1));
		axes.Draw();*/

		//vertexData2->Draw();

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

		tHelper->mColor = Color(255, 0, 0, 255);
		tHelper->Begin(0, 14, 12);
		tHelper->DrawTextLineBase({ {50},20,500,{5} }, GT_TOP | GT_LEFT, L"Hare Population");
		tHelper->DrawTextLineBase({ { 5 },250,6,{ 5 } }, GT_TOP | GT_LEFT, L"Lynx Population");
		tHelper->End();

		//render dialog last(overlay)
		//if ((int)currTime % 2)
			//dlg->OnRender(ellapsedTime);
			//dlg->DrawRect(rc, ::Color(255, 0, 0, 255));

            /*textHelper->Begin(0, 20, 15);
            textHelper->DrawFormattedTextLineBase({ { 500 }, 300, 700, { 280 } }, GT_TOP | GT_LEFT, L"Val1 = %; Val2 = %; Val3 = %", 36, 29.334f, "String");
            textHelper->End();*/

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
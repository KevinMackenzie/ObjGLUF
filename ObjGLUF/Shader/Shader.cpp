#include "Shader.h"

namespace GLUF {

ShaderManager g_ShaderManager;

/*
======================================================================================================================================================================================================
Shader API (Alpha)

Note:
    Attaching shaders to programs and programs to ppo's does not check if shader compilation failed, or if program linking failed respectively

*/


class CompileShaderException : public Exception {
public:
    virtual const char *what() const noexcept override {
        return "Failed to Compile Shader!";
    }

    EXCEPTION_CONSTRUCTOR(CompileShaderException)
};

class CreateGLShaderException : public Exception {
public:
    virtual const char *what() const noexcept override {
        return "OpenGL Failed to Create Shader Instance!";
    }

    EXCEPTION_CONSTRUCTOR(CreateGLShaderException)
};

class LinkProgramException : public Exception {
public:
    virtual const char *what() const noexcept override {
        return "Failed to Link Program!";
    }

    EXCEPTION_CONSTRUCTOR(LinkProgramException)
};

class CreateGLProgramException : public Exception {
public:
    virtual const char *what() const noexcept override {
        return "OpenGL Failed to Create Program Instance!";
    }

    EXCEPTION_CONSTRUCTOR(CreateGLProgramException)
};

class CreateGLPPOException : public Exception {
public:
    virtual const char *what() const noexcept override {
        return "OpenGL Failed to Create PPO Instance!";
    }

    EXCEPTION_CONSTRUCTOR(CreateGLPPOException)
};



/*

ShaderTypeToProgramStage

*/


//--------------------------------------------------------------------------------------
ProgramStage ShaderTypeToProgramStage(ShaderType type) {
    switch (type) {
        case SH_VERTEX_SHADER:
            return PPO_VERTEX_SHADER_BIT;
        case SH_TESSCONTROL_SHADER:
            return PPO_TESSCONTROL_SHADER_BIT;
        case SH_TESS_EVALUATION_SHADER:
            return PPO_TESS_EVALUATION_SHADER_BIT;
        case SH_GEOMETRY_SHADER:
            return PPO_GEOMETRY_SHADER_BIT;
        case SH_FRAGMENT_SHADER:
            return PPO_FRAGMENT_SHADER_BIT;
        default:
            return PPO_INVALID_SHADER_BIT;
    }
};

/*
Shader

    Data Members:
        'mShaderId': OpenGL-Assigned Shader id
        'mTmpShaderText': shader text cache kept if shaders are appending eachother
        'mShaderType': what shader type is it (i.e. GL_FRAGMENT_SHADER)

*/
class Shader {
    friend ShaderManager;
    friend Program;

    GLuint mShaderId;

    std::string mTmpShaderText;

    ShaderType mShaderType;

    //disallow copy and assign, because these must always be refered to by pointers
    Shader(const Shader &other) = delete;
    Shader &operator=(const Shader &other) = delete;
public:

    /*
    Constructor
        Throws:
            no-throw guarantee
    
    */
    Shader() noexcept;
    ~Shader() noexcept;

    //common shader is if the shader will not be deleted after building into a program
    //this is used for things like lighting functions

    /*
    Init

        Parameters:
            'shaderType': which shader type it is

        Throws:
            no-throw guarantee
    */
    void Init(ShaderType shaderType) noexcept;


    /*
    Load
        
        Parameters:
            'shaderText': text to be added
            'append': append 'shaderText' to 'mTmpShaderText' or clear 'mTmpShaderText'

        Throws:
            may throw 'std::bad_alloc' if string allocation fails
    */
    void Load(const std::string &shaderText, bool append = false);

    /*
    LoadFromMemory

        Parameters:
            'shaderData': raw memory to read shader text from
            'append': append text loaded from 'shaderData' to 'mTmpShaderText' or clear 'mTmpShaderText'

        Throws:
            may throw 'std::bad_alloc' if string allocation fails            
    */
    //void LoadFromMemory(const std::vector<char>& shaderData, bool append = false);

    /*
    LoadFromFile

        Parameters:
            'filePath': path of the file to open
            'append': append 'filePath' to 'MTmpShaderText' or clear 'mTmpShaderText'

        Throws:
            'std::ios_base::failure': if file fails to open or read
    
    */
    //bool LoadFromFile(const std::wstring& filePath, bool append = false);

    /*
    FlushText

        Throws:
            no-throw guarantee
    */
    void FlushText(void) noexcept { mTmpShaderText.clear(); }


    /*
    Compile
        
        -Take all of the loaded text and compile into a shader

        Parameters:
            'retStruct': the returned information about shader compilation.

        Throws:
            'CompileShaderException': if shader compilation fails
            'CreateGLShaderException': if shader creaetion failes (because shader creation is stalled until compilation)
    
    */
    void Compile(ShaderInfoStruct &retStruct);


    /*
    Destroy

        -equivilent to destructor, except object itself is not destroyed (i.e. it can be reused)

        Throws:
            no-throw guarantee
        
    */
    void Destroy() noexcept;

};

using ShaderP = std::pair<ShaderType, ShaderPtr>;


/*
Program

    Data Members:
        'mProgramId': OpenGL assigned id
        'mShaderBuff': buffer of shaders before linking
        'mAttributeLocations': location of vertex attributes in program
        'mUniformLocations': location of the uniforms in programs
        'mStages': the stages which this program uses; initialized if separable set to true

*/
class Program {
    friend ShaderManager;

    //GLuint mUniformBuffId;
    GLuint mProgramId;
    std::map<ShaderType, ShaderPtr> mShaderBuff;
    VariableLocMap mAttributeLocations;
    VariableLocMap mUniformLocations;
    GLbitfield mStages;

public:

    /*
    Constructor
        Throws:
            no-throw guarantee

    */
    Program() noexcept;
    ~Program() noexcept;

    /*
    Init
        Throws:
            'CreateGLProgramException': OpenGL failed to create program using glCreateProgram
    */
    void Init();

    /*
    AttachShader
        
        Note:
            if a shader already exists for that stage, it will be overwritten

        Parameters:
            'shader': pointer to shader which is being attached

        Throws:
            'std::invalid_argument': if 'shader == nullptr' or 'shader' is invalid
    */
    void AttachShader(ShaderPtr shader);

    /*
    FlushShaders

        -Flush shader cache, but does not explicitly delete shaders

        Throws:
            no-throw guarantee
    */
    void FlushShaders(void) noexcept;


    /*
    Build
        
        Parameters:
            'retStruct': the returned information about program linking.  i.e. 'mSuccess'
            'separate': whether this program will be used 'separately' (see OpenGL docs for definition)

        Throws:
            'LinkProgramException': if program linking fails
    */
    void Build(ShaderInfoStruct &retStruct, bool separate);

    /*
    GetId

        Returns:
            OpenGL Id of the program
    */
    GLuint GetId() { return mProgramId; }


    /*
    Destroy

        -equivilent to destructor, except object itself is not destroyed (i.e. it can be reused)

        Throws:
            no-throw guarantee

    */
    void Destroy() noexcept;

    /*
    GetBitfield()

        Returns:
            GLbitfield of the program's stages; this is used when initializing and adding to PPO's
    */
    GLbitfield GetBitfield() const noexcept;
};

/*
SeparateProgram


    Data Members:
        'mPPOId': id of programmible pipeline object
        'mPrograms': list of programs used; kept here so they do not delete themselves while this pipeline object exists
        'mActiveProgram': the currently active program which is having uniforms buffered

*/
class SeparateProgram {
    friend ShaderManager;
    GLuint mPPOId;

    ProgramPtrList mPrograms;//so the programs don't go deleting themselves until the PPO is destroyed

    ProgramPtr mActiveProgram;//this is used as the 'active program' when assigning uniforms

public:
    SeparateProgram();
    ~SeparateProgram();

    /*
    Init

        Throws:
            'CreateGLPPOException': if opengl creation of PPO failed
    */
    void Init();

    /*
    AttachProgram

        Parameters:
            'program': program to attach

        Throws:
            no-throw guarantee

    */
    void AttachProgram(const ProgramPtr &program);

    /*
    ClearStages

        -Removes the give stages

        Parameters:
            'stages': bitfield of stages, default value is all of them

        Throws:
            no-throw guarantee
    
    */
    void ClearStages(GLbitfield stages = GL_ALL_SHADER_BITS);

    /*
    SetActiveShaderProgram

        Parameters:
            'stage': the stage of which program will be active

        Throws:
            no-throw guarantee
    
    */
    void SetActiveShaderProgram(ProgramStage stage);


    /*
    GetId

        Returns:
            id of ppo object
    
    */
    GLuint GetId() const noexcept { return mPPOId; }

    /*
    GetActiveProgram

        Returns:
            the active program

        Throws:
            'std::exception' if there is no active program

    */
    const ProgramPtr &GetActiveProgram() const;

};



/*
===================================================================================================
SeparateProgram Methods


*/

//--------------------------------------------------------------------------------------
SeparateProgram::SeparateProgram() {
    //prevents from attempting to create separate shaders w/o the extension
    ASSERT_EXTENTION(GL_ARB_separate_shader_objects);
}

//--------------------------------------------------------------------------------------
SeparateProgram::~SeparateProgram() {
    glDeleteProgramPipelines(1, &mPPOId);
}

//--------------------------------------------------------------------------------------
void SeparateProgram::Init() {
    glGenProgramPipelines(1, &mPPOId);

    if (mPPOId == 0) {
        GLUF_CRITICAL_EXCEPTION(CreateGLPPOException());
    }
}

//--------------------------------------------------------------------------------------
void SeparateProgram::AttachProgram(const ProgramPtr &program) {
    glBindProgramPipeline(mPPOId);
    mPrograms.push_back(program);
    glUseProgramStages(mPPOId, program->GetBitfield(), program->GetId());
}

//--------------------------------------------------------------------------------------
void SeparateProgram::ClearStages(GLbitfield stages) {
    glUseProgramStages(mPPOId, stages, 0);

    //remove the programs that are affected by this clear
    for (auto it = mPrograms.begin(); it != mPrograms.end(); ++it) {
        //if any of the bits of this bitfield are the same, then remove the program.  NOTE: IT IS BAD PRACTICE TO HAVE PROGRAMS HAVE MULTIPLE STAGES, AND ONLY REMOVE ONE OF THEM
        if (((*it)->GetBitfield() & stages) != 0)
            mPrograms.erase(it);
    }
}

//--------------------------------------------------------------------------------------
void SeparateProgram::SetActiveShaderProgram(ProgramStage stage) {
    glBindProgramPipeline(mPPOId);

    //does the currently bound program have this stage?
    if (mActiveProgram)
        if ((mActiveProgram->GetBitfield() & stage) != 0)
            return;//do nothing

    //find which program contains this stage
    bool success = false;
    for (auto it : mPrograms) {
        if ((it->GetBitfield() & stage) != 0) {
            mActiveProgram = it;
            success = true;
            break;
        }
    }

    if (!success) {
        //if the stage does not exist, reset to default (null)
        mActiveProgram = nullptr;
        glActiveShaderProgram(mPPOId, 0);

    } else {
        glActiveShaderProgram(mPPOId, mActiveProgram->GetId());
    }
}

//--------------------------------------------------------------------------------------
const ProgramPtr &SeparateProgram::GetActiveProgram() const {
    if (mActiveProgram == nullptr) {
        GLUF_NON_CRITICAL_EXCEPTION(NoActiveProgramUniformException());
    }

    return mActiveProgram;
}


/*
===================================================================================================
Shader Methods


*/


//--------------------------------------------------------------------------------------
Shader::Shader() noexcept {
    mShaderId = 0;
}

//--------------------------------------------------------------------------------------
Shader::~Shader() noexcept {
    if (mShaderId != 0) {
        glDeleteShader(mShaderId);
    }
    mTmpShaderText.clear();
}

//--------------------------------------------------------------------------------------
void Shader::Init(ShaderType shaderType) noexcept {
    mShaderType = shaderType;
    mShaderId = 0;
}

//--------------------------------------------------------------------------------------
void Shader::Load(const std::string &shaderText, bool append) {
    if (!append)
        mTmpShaderText.clear();

    mTmpShaderText.append(shaderText);
}

//--------------------------------------------------------------------------------------
void Shader::Destroy() noexcept {
    if (mShaderId != 0) {
        glDeleteShader(mShaderId);
        mShaderId = 0;
    }
    mTmpShaderText.clear();
}

//--------------------------------------------------------------------------------------
/*void Shader::LoadFromMemory(const std::vector<char>& shaderData, bool append)
{
    if (!append)
        mTmpShaderText.clear();

    std::string loadedText;

    try
    {
        LoadBinaryArrayIntoString(shaderData, loadedText);
    }
    catch (...)
    {
        GLUF_ERROR("(Shader): Failed to load binary memory into shader string");
        throw;
    }

    
    mTmpShaderText += loadedText;

}*/

//--------------------------------------------------------------------------------------
/*bool Shader::LoadFromFile(const std::wstring& filePath, bool append)
{
    if (!append)
        mTmpShaderText.clear();

    std::ifstream inFile(filePath);
    inFile.exceptions(std::ios::badbit | std::ios::failbit);

    try
    {
#pragma warning (disable : 4244)
        std::string newString;

        inFile.seekg(0, std::ios::end);
        newString.resize(inFile.tellg());
        inFile.seekg(0, std::ios::beg);

        //TODO: DOES THIS NEED TO HAVE A +1 SOMEWHERE TO ACCOUNT FOR NULL CHARACTER
        inFile.read(&newString[0], newString.size());
        inFile.close();
    }
    catch (...)
    {
        GLUF_ERROR("(Shader): Failed to load shader text from file!");
        throw;
    }

    return true;
}*/

#define FAILED_COMPILE 'F'
#define FAILED_LINK    'F'


//--------------------------------------------------------------------------------------
void Shader::Compile(ShaderInfoStruct &returnStruct) {
    //if the shader id is not 0, this means the previous compile attempt was successful, because if it was not, the program is deleted and reset to 0
    if (mShaderId != 0) {
        returnStruct.mSuccess = false;
        returnStruct.mLog = "F";
        return;
    }

    //create the shader
    mShaderId = glCreateShader(mShaderType);

    //if shader creation failed, throw an exception
    if (mShaderId == 0)
        GLUF_CRITICAL_EXCEPTION(CreateGLShaderException());

    //start by adding the strings to glShader Source.  This is done right before the compile
    //process becuase it is hard to remove it if there is any reason to flush the text

    std::string tmpText = mTmpShaderText;

    GLint tmpSize = (GLuint) mTmpShaderText.length();
    tmpSize--; /*BECAUSE OF NULL TERMINATED STRINGS*/

    const GLchar *text = tmpText.c_str();
    glShaderSource(mShaderId, 1, &text, &tmpSize);

    //flush text upon compile no matter what (if compile failed, why would we want to keep the error-ridden code)
    FlushText();

    glCompileShader(mShaderId);

    GLint isCompiled = 0;
    glGetShaderiv(mShaderId, GL_COMPILE_STATUS, &isCompiled);
    returnStruct.mSuccess = (isCompiled == GL_FALSE) ? false : true;

    GLint maxLength = 0;
    glGetShaderiv(mShaderId, GL_INFO_LOG_LENGTH, &maxLength);

    //The maxLength includes the NULL character
    returnStruct.mLog.resize(maxLength);
    glGetShaderInfoLog(mShaderId, maxLength, &maxLength, &returnStruct.mLog[0]);

    //Provide the infolog in whatever manor you deem best.
    //Exit with failure.

    //if it failed, delete the shader, to have a universal way of determining failure
    if (returnStruct.mSuccess == false) {
        glDeleteShader(mShaderId);
        mShaderId = 0;
        GLUF_CRITICAL_EXCEPTION(CompileShaderException());
    }
    return;
}





/*
===================================================================================================
Program Methods


*/

//--------------------------------------------------------------------------------------
Program::Program() noexcept {
    mProgramId = 0;
}

//--------------------------------------------------------------------------------------
Program::~Program() noexcept {
    glDeleteProgram(mProgramId);
}

//--------------------------------------------------------------------------------------
void Program::Init() {
    //unlike with the shader, this will be created during initialization
    mProgramId = glCreateProgram();

    if (mProgramId == 0)
        GLUF_CRITICAL_EXCEPTION(CreateGLProgramException());
}

//--------------------------------------------------------------------------------------
void Program::AttachShader(ShaderPtr shader) {
    ShaderP toInsert{shader->mShaderType, shader};

    //does this stage already have a shader assigned to it?
    auto exists = mShaderBuff.find(shader->mShaderType);
    if (exists != mShaderBuff.end()) {
        //if it does, delete it
        mShaderBuff.erase(exists);
    }

    //finally, insert the new one
    mShaderBuff.insert(toInsert);

    //and add it to OpenGL
    glAttachShader(mProgramId, shader->mShaderId);
}

//--------------------------------------------------------------------------------------
void Program::FlushShaders(void) noexcept {
    //simply go through all of the shaders, remove them, and clear the shader buffer
    for (auto it : mShaderBuff) {
        glDetachShader(mProgramId, it.second->mShaderId);
    }
    mShaderBuff.clear();
}

//--------------------------------------------------------------------------------------
void Program::Build(ShaderInfoStruct &retStruct, bool separate) {
    //make sure we enable separate shading
    if (separate) {
        mStages = 0;

        glProgramParameteri(mProgramId, GL_PROGRAM_SEPARABLE, GL_TRUE);


        //also create the 'mStages' list

        for (auto it : mShaderBuff) {
            mStages |= ShaderTypeToProgramStage(it.first);
        }
    }

    //Link our program
    glLinkProgram(mProgramId);

    //Note the different functions here: glGetProgram* instead of glGetShader*.
    GLint isLinked = 0;
    glGetProgramiv(mProgramId, GL_LINK_STATUS, &isLinked);
    retStruct.mSuccess = (isLinked == GL_FALSE) ? false : true;

    GLint maxLength = 0;
    glGetProgramiv(mProgramId, GL_INFO_LOG_LENGTH, &maxLength);

    //The maxLength includes the NULL character
    retStruct.mLog.resize(maxLength);
    glGetProgramInfoLog(mProgramId, maxLength, &maxLength, &retStruct.mLog[0]);

    if (!retStruct.mSuccess) {
        //in the case of failure, DO NOT DELETE ANYTHING, but do throw an error
        GLUF_CRITICAL_EXCEPTION(LinkProgramException());
    } else {
        FlushShaders();//this removes the references to them from the program, but they will still exist unless this is the last reference to them

        /*
        
        This region loads uniform and attribute locations

        */


        //Load the variable names
        GLint attribCount = 0;
        glGetProgramiv(mProgramId, GL_ACTIVE_ATTRIBUTES, &attribCount);

        GLint maxLength;
        glGetProgramiv(mProgramId, GL_ACTIVE_ATTRIBUTE_MAX_LENGTH, &maxLength);

        //TODO: right now, these are just placeholders, but perhaps a more in depth variable information structure is needed
        GLenum type;
        GLint written, size;

        //this is used to trim the null charactors from the end of the string
        const auto TrimString = [](std::string &toTrim) {
            for (unsigned int i = 0; i < toTrim.size(); ++i) {
                if (toTrim[i] == '\0') {
                    toTrim.erase(toTrim.begin() + i, toTrim.end());
                    break;
                }

            }
        };


        std::string data;
        for (int i = 0; i < attribCount; ++i) {
            //resize and clear every time is a bit messy, but is the only way to make sure the string is the right length
            data.resize(maxLength);

            glGetActiveAttrib(mProgramId, i, maxLength, &written, &size, &type, &data[0]);
            TrimString(data);
            mAttributeLocations.insert(VariableLocPair(data, glGetAttribLocation(mProgramId, &data[0])));

            data.clear();
        }

        GLint uniformCount = 0;
        glGetProgramiv(mProgramId, GL_ACTIVE_UNIFORMS, &uniformCount);

        for (int i = 0; i < uniformCount; ++i) {
            //resize and clear every time is a bit messy, but is the only way to make sure the string is the right length
            data.resize(maxLength);

            glGetActiveUniform(mProgramId, i, maxLength, &written, &size, &type, &data[0]);
            TrimString(data);
            mUniformLocations.insert(VariableLocPair(data, glGetUniformLocation(mProgramId, &data[0])));

            data.clear();
        }
    }
}

//--------------------------------------------------------------------------------------
void Program::Destroy() noexcept {
    if (mProgramId != 0) {
        glDeleteProgram(mProgramId);
        mProgramId = 0;
    }
    mShaderBuff.clear();
    mAttributeLocations.clear();
    mUniformLocations.clear();
}

//--------------------------------------------------------------------------------------
GLbitfield Program::GetBitfield() const noexcept {
    return mStages;

    return 0;
}


/*
===================================================================================================
ShaderManager Methods


*/

/*
//--------------------------------------------------------------------------------------
ShaderPtr ShaderManager::CreateShader(std::wstring shad, ShaderType type, bool file, bool separate)
{
    ShaderPtr shader(new Shader());
    shader->Init(type);

    (file) ? shader->LoadFromFile(shad.c_str()) : shader->Load(shad.c_str());

    ShaderInfoStruct output;
    shader->Compile(output);
    mCompileLogs.insert(std::pair<ShaderPtr, ShaderInfoStruct>(shader, output));

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

//--------------------------------------------------------------------------------------
void ShaderManager::AddCompileLog(const ShaderPtr &shader, const ShaderInfoStruct &log) {
    //TODO: vs2013 does not support shared locking
    _TSAFE_SCOPE(mCompLogMutex);

    mCompileLogs.insert(std::pair<ShaderPtr, ShaderInfoStruct>(shader, log));
}

//--------------------------------------------------------------------------------------
void ShaderManager::AddLinkLog(const ProgramPtr &program, const ShaderInfoStruct &log) {
    _TSAFE_SCOPE(mLinkLogMutex);

    mLinklogs.insert(std::pair<ProgramPtr, ShaderInfoStruct>(program, log));
}

//--------------------------------------------------------------------------------------
GLuint ShaderManager::GetUniformIdFromName(const SepProgramPtr &ppo, const std::string &name) const {
    auto activeProgram = ppo->GetActiveProgram();
    auto it = activeProgram->mUniformLocations.find(name);
    if (it == activeProgram->mUniformLocations.end()) {
        GLUF_NON_CRITICAL_EXCEPTION(std::invalid_argument("Uniform Name Not Found!"));
        return 0;//if we are in release mode, and the name does not exist, default to the 0th, however this MAY CREATE UNDESIRED RESULTS
    }

    return it->second;
}

//--------------------------------------------------------------------------------------
GLuint ShaderManager::GetUniformIdFromName(const ProgramPtr &prog, const std::string &name) const {
    auto it = prog->mUniformLocations.find(name);
    if (it == prog->mUniformLocations.end()) {
        GLUF_NON_CRITICAL_EXCEPTION(std::invalid_argument("Uniform Name Not Found!"));
        return 0;//if we are in release mode, and the name does not exist, default to the 0th, however this MAY CREATE UNDESIRED RESULTS
    }

    return it->second;
}


//--------------------------------------------------------------------------------------
void ShaderManager::CreateShaderFromFile(ShaderPtr &outShader, const std::string &filePath, ShaderType type) {
    //create the shader
    outShader = std::make_shared<Shader>();

    //file to open
    std::ifstream inFile;
    inFile.exceptions(std::ios::badbit | std::ios::failbit);

    ShaderInfoStruct output;
    try {
        //Load the text from the file
        std::string newString;

        //open the file
        inFile.open(filePath);

        //get the file length
        inFile.seekg(0, std::ios::end);
        newString.resize(static_cast<unsigned int>(inFile.tellg()));
        inFile.seekg(0, std::ios::beg);

        //TODO: DOES THIS NEED TO HAVE A +1 SOMEWHERE TO ACCOUNT FOR NULL CHARACTER
        inFile.read(&newString[0], newString.size());
        inFile.close();

        //load it from the file
        outShader->Load(newString);

        //compile it
        outShader->Compile(output);
        AddCompileLog(outShader, output);
    }
    catch (const std::ios_base::failure &e) {
        GLUF_ERROR_LONG("(ShaderManager): Shader File Load Failed: " << e.what());
        RETHROW;//rethrow here, because if file loading failed, the it never got to compilation
    }
    catch (const CompileShaderException &e) {
        GLUF_ERROR_LONG("(ShaderManager): " << e.what());
        //add the log if file load failed
        AddCompileLog(outShader, output);

        GLUF_CRITICAL_EXCEPTION(MakeShaderException());
    }
    catch (const CreateGLShaderException &e) {
        GLUF_ERROR_LONG("(ShaderManager): " << e.what());
        GLUF_CRITICAL_EXCEPTION(MakeShaderException());//don't add compile log if the shader could not be created
    }

}

//--------------------------------------------------------------------------------------
void ShaderManager::CreateShaderFromText(ShaderPtr &outShader, const std::string &text, ShaderType type) {
    outShader = std::make_shared<Shader>();

    ShaderInfoStruct out;
    try {
        //initialize the shader
        outShader->Init(type);

        //load from the text
        outShader->Load(text);

        //compile it
        outShader->Compile(out);
        AddCompileLog(outShader, out);
    }
    catch (const CompileShaderException &e) {
        GLUF_ERROR_LONG("(ShaderManager): " << e.what() << "\n ========Log======== \n" << out.mLog);
        //add the log if file load failed
        AddCompileLog(outShader, out);

        GLUF_CRITICAL_EXCEPTION(MakeShaderException());
    }
    catch (const CreateGLShaderException &e) {
        GLUF_ERROR_LONG("(ShaderManager): " << e.what());
        GLUF_CRITICAL_EXCEPTION(MakeShaderException());//don't add compile log if the shader could not be created
    }
}

//--------------------------------------------------------------------------------------
void ShaderManager::CreateShaderFromMemory(ShaderPtr &outShader, const std::vector<char> &memory, ShaderType type) {
    //load the string from the memory
    std::string outString;
    LoadBinaryArrayIntoString(memory, outString);

    //newline here is needed, because end of loaded file might not have one, which is required for the end of a shader for some reason
    return CreateShaderFromText(outShader, outString + "\n", type);
}

//--------------------------------------------------------------------------------------
void ShaderManager::CreateProgram(ProgramPtr &outProgram, ShaderPtrList shaders, bool separate) {
    outProgram = std::make_shared<Program>();

    ShaderInfoStruct out;
    try {
        //initialize
        outProgram->Init();

        //add the shaders
        for (auto it : shaders) {
            outProgram->AttachShader(it);
        }

        //build the program
        outProgram->Build(out, separate);
        AddLinkLog(outProgram, out);
    }
    catch (const CreateGLProgramException &e) {
        GLUF_ERROR_LONG("(ShaderManager): " << e.what());
        GLUF_CRITICAL_EXCEPTION(MakeShaderException());
    }
    catch (const LinkProgramException &e) {
        GLUF_ERROR_LONG("(ShaderManager): " << e.what() << "\n ========Log======== \n" << out.mLog);
        AddLinkLog(outProgram, out);//if linking failed, still add the log
    }
    catch (const std::invalid_argument &e) {
        GLUF_ERROR_LONG("(ShaderManager): " << e.what());
        outProgram->FlushShaders();//if any of the shaders failed to add, flush so it is in a valid state
        GLUF_CRITICAL_EXCEPTION(MakeShaderException());
    }
}


//--------------------------------------------------------------------------------------
void ShaderManager::CreateProgram(ProgramPtr &outProgram, ShaderSourceList shaderSources, bool separate) {
    ShaderPtrList shaders;
    for (auto it : shaderSources) {
        //create the shader from the text
        std::shared_ptr<Shader> nowShader;
        CreateShaderFromText(nowShader, it.second, it.first);

        //add the shader to the list
        shaders.push_back(nowShader);

        //the exception hierarchy handles all errors for this method
    }

    CreateProgram(outProgram, shaders, separate);
}


//--------------------------------------------------------------------------------------
void ShaderManager::CreateProgramFromFiles(ProgramPtr &outProgram, ShaderPathList shaderPaths, bool separate) {
    ShaderPtrList shaders;
    for (auto it : shaderPaths) {
        //create the shader from the text
        auto nowShader = std::make_shared<Shader>();
        CreateShaderFromFile(nowShader, it.second, it.first);

        //add the shader to the list
        shaders.push_back(nowShader);

        //the exception hierarchy handles all errors for this method
    }

    CreateProgram(outProgram, shaders, separate);
}

//for removing things

//--------------------------------------------------------------------------------------
void ShaderManager::DeleteShader(ShaderPtr &shader) noexcept {
    //make sure it exists
    if (shader)
        shader->Destroy();
}

//--------------------------------------------------------------------------------------
void ShaderManager::DeleteProgram(ProgramPtr &program) noexcept {
    //make sure it exists
    if (program)
        program->Destroy();
}

//--------------------------------------------------------------------------------------
void ShaderManager::FlushLogs() {
    mLinklogs.clear();
    mCompileLogs.clear();
}


//for accessing things

//--------------------------------------------------------------------------------------
const GLuint ShaderManager::GetShaderId(const ShaderPtr &shader) const {
    GLUF_NULLPTR_CHECK(shader);

    return shader->mShaderId;
}


//--------------------------------------------------------------------------------------
const ShaderType ShaderManager::GetShaderType(const ShaderPtr &shader) const {
    GLUF_NULLPTR_CHECK(shader);

    return shader->mShaderType;
}


//--------------------------------------------------------------------------------------
const GLuint ShaderManager::GetProgramId(const ProgramPtr &program) const {
    GLUF_NULLPTR_CHECK(program);

    return program->mProgramId;
}

//--------------------------------------------------------------------------------------
const CompileOutputStruct ShaderManager::GetShaderLog(const ShaderPtr &shaderPtr) const {
    GLUF_NULLPTR_CHECK(shaderPtr);
    _TSAFE_SCOPE(mCompLogMutex);

    return mCompileLogs.find(shaderPtr)->second;
}


//--------------------------------------------------------------------------------------
const LinkOutputStruct ShaderManager::GetProgramLog(const ProgramPtr &programPtr) const {
    GLUF_NULLPTR_CHECK(programPtr);
    _TSAFE_SCOPE(mLinkLogMutex);

    return mLinklogs.find(programPtr)->second;
}

//for using things

//--------------------------------------------------------------------------------------
void ShaderManager::UseProgram(const ProgramPtr &program) const {
    GLUF_NULLPTR_CHECK(program);

    //binding a null program means the program is uninitialized or broken, which is an error
    if (program->GetId() == 0)
        GLUF_NON_CRITICAL_EXCEPTION(UseProgramException());

    //make sure this is not bound when using a program
    glBindProgramPipeline(0);

    glUseProgram(program->mProgramId);
}

//--------------------------------------------------------------------------------------
void ShaderManager::UseProgramNull() const noexcept {
    glUseProgram(0);
    glBindProgramPipeline(0);//juse in case we are using pipelines
}

//--------------------------------------------------------------------------------------
void ShaderManager::CreateSeparateProgram(SepProgramPtr &ppo, const ProgramPtrList &programs) const {
    ppo = std::make_shared<SeparateProgram>();

    try {
        //initialize the PPO
        ppo->Init();

        for (auto it : programs) {
            GLUF_NULLPTR_CHECK(it);
            if (it->GetId() == 0)
                GLUF_CRITICAL_EXCEPTION(std::invalid_argument("Uninitialized Program Attempted to be Added to a PPO"));

            ppo->AttachProgram(it);
        }
    }
    catch (const CreateGLPPOException &e) {
        GLUF_ERROR_LONG("(ShaderManager): " << e.what());
        GLUF_CRITICAL_EXCEPTION(MakePPOException());
    }
    catch (const std::invalid_argument &e) {
        GLUF_ERROR_LONG("(ShaderManager): " << e.what());
        RETHROW;
    }
}

//--------------------------------------------------------------------------------------
const GLuint ShaderManager::GetShaderVariableLocation(const ProgramPtr &program, LocationType locType, const std::string &varName) const {
    GLUF_NULLPTR_CHECK(program);

    VariableLocMap::iterator it;

    if (locType == GLT_ATTRIB) {
        it = program->mAttributeLocations.find(varName);

        if (it == program->mAttributeLocations.end()) {
            GLUF_NON_CRITICAL_EXCEPTION(
                    std::invalid_argument("\"varName\" Could not be found when searching program attributes!"));
            return 0;
        }
    } else {
        it = program->mUniformLocations.find(varName);

        if (it == program->mUniformLocations.end()) {
            GLUF_NON_CRITICAL_EXCEPTION(
                    std::invalid_argument("\"varName\" Could not be found when searching program uniforms!"));
            return 0;
        }
    }

    return it->second;
}

//--------------------------------------------------------------------------------------
const VariableLocMap &ShaderManager::GetShaderAttribLocations(const ProgramPtr &program) const {
    GLUF_NULLPTR_CHECK(program);
    return program->mAttributeLocations;
}

//--------------------------------------------------------------------------------------
const VariableLocMap &ShaderManager::GetShaderUniformLocations(const ProgramPtr &program) const {
    GLUF_NULLPTR_CHECK(program);
    return program->mUniformLocations;
}

//--------------------------------------------------------------------------------------
const VariableLocMap ShaderManager::GetShaderAttribLocations(const SepProgramPtr &program) const {
    GLUF_NULLPTR_CHECK(program);
    VariableLocMap ret;

    for (auto it : program->mPrograms) {
        ret.insert(it->mAttributeLocations.begin(), it->mAttributeLocations.end());
    }

    return ret;
}

//--------------------------------------------------------------------------------------
const VariableLocMap ShaderManager::GetShaderUniformLocations(const SepProgramPtr &program) const {
    GLUF_NULLPTR_CHECK(program);

    VariableLocMap ret;

    for (auto it : program->mPrograms) {
        ret.insert(it->mUniformLocations.begin(), it->mUniformLocations.end());
    }

    return ret;
}

//--------------------------------------------------------------------------------------
void ShaderManager::AttachProgram(SepProgramPtr &ppo, const ProgramPtr &program) const {
    GLUF_NULLPTR_CHECK(ppo);
    GLUF_NULLPTR_CHECK(program);

    if (program->GetId() == 0)
        GLUF_CRITICAL_EXCEPTION(std::invalid_argument("Uninitialized Program Attempted to be Added to a PPO"));

    ppo->AttachProgram(program);
}

//--------------------------------------------------------------------------------------
void ShaderManager::AttachPrograms(SepProgramPtr &ppo, const ProgramPtrList &programs) const {
    GLUF_NULLPTR_CHECK(ppo);

    for (auto it : programs) {
        GLUF_NULLPTR_CHECK(it);
        if (it->GetId() == 0)
            GLUF_CRITICAL_EXCEPTION(std::invalid_argument("Uninitialized Program Attempted to be Added to a PPO"));

        ppo->AttachProgram(it);
    }
}

//--------------------------------------------------------------------------------------
void ShaderManager::ClearPrograms(SepProgramPtr &ppo, GLbitfield stages) const {
    GLUF_NULLPTR_CHECK(ppo);
    ppo->ClearStages(stages);
}

//--------------------------------------------------------------------------------------
void ShaderManager::UseProgram(const SepProgramPtr &ppo) const {
    GLUF_NULLPTR_CHECK(ppo);
    glUseProgram(0);
    glBindProgramPipeline(ppo->GetId());
}

//--------------------------------------------------------------------------------------
void ShaderManager::GLActiveShaderProgram(SepProgramPtr &ppo, ShaderType stage) const {
    GLUF_NULLPTR_CHECK(ppo);
    ppo->SetActiveShaderProgram(ShaderTypeToProgramStage(stage));
}

/*

GLUniform*

*/


/*

float's

*/
//--------------------------------------------------------------------------------------
void ShaderManager::GLUniform1f(GLuint loc, const GLfloat &value) const noexcept {
    glUniform1f(loc, value);
}

//--------------------------------------------------------------------------------------
void ShaderManager::GLUniform2f(GLuint loc, const glm::vec2 &value) const noexcept {
    glUniform2fv(loc, 1, &value[0]);
}

//--------------------------------------------------------------------------------------
void ShaderManager::GLUniform3f(GLuint loc, const glm::vec3 &value) const noexcept {
    glUniform3fv(loc, 1, &value[0]);
}

//--------------------------------------------------------------------------------------
void ShaderManager::GLUniform4f(GLuint loc, const glm::vec4 &value) const noexcept {
    glUniform4fv(loc, 1, &value[0]);
}

//--------------------------------------------------------------------------------------
void ShaderManager::GLUniform1f(const ProgramPtr &prog, const std::string &name, const GLfloat &value) const {
    glUniform1f(GetUniformIdFromName(prog, name), value);
}

//--------------------------------------------------------------------------------------
void ShaderManager::GLUniform2f(const ProgramPtr &prog, const std::string &name, const glm::vec2 &value) const {
    glUniform2fv(GetUniformIdFromName(prog, name), 1, &value[0]);
}

//--------------------------------------------------------------------------------------
void ShaderManager::GLUniform3f(const ProgramPtr &prog, const std::string &name, const glm::vec3 &value) const {
    glUniform3fv(GetUniformIdFromName(prog, name), 1, &value[0]);
}

//--------------------------------------------------------------------------------------
void ShaderManager::GLUniform4f(const ProgramPtr &prog, const std::string &name, const glm::vec4 &value) const {
    glUniform4fv(GetUniformIdFromName(prog, name), 1, &value[0]);
}

/*

int's

*/

//--------------------------------------------------------------------------------------
void ShaderManager::GLUniform1i(GLuint loc, const GLint &value) const noexcept {
    glUniform1i(loc, value);
}

//--------------------------------------------------------------------------------------
void ShaderManager::GLUniform2i(GLuint loc, const glm::i32vec2 &value) const noexcept {
    glUniform2iv(loc, 1, &value[0]);
}

//--------------------------------------------------------------------------------------
void ShaderManager::GLUniform3i(GLuint loc, const glm::i32vec3 &value) const noexcept {
    glUniform3iv(loc, 1, &value[0]);
}

//--------------------------------------------------------------------------------------
void ShaderManager::GLUniform4i(GLuint loc, const glm::i32vec4 &value) const noexcept {
    glUniform4iv(loc, 1, &value[0]);
}

//--------------------------------------------------------------------------------------
void ShaderManager::GLUniform1i(const ProgramPtr &prog, const std::string &name, const GLint &value) const {
    glUniform1i(GetUniformIdFromName(prog, name), value);
}

//--------------------------------------------------------------------------------------
void ShaderManager::GLUniform2i(const ProgramPtr &prog, const std::string &name, const glm::i32vec2 &value) const {
    glUniform2iv(GetUniformIdFromName(prog, name), 1, &value[0]);
}

//--------------------------------------------------------------------------------------
void ShaderManager::GLUniform3i(const ProgramPtr &prog, const std::string &name, const glm::i32vec3 &value) const {
    glUniform3iv(GetUniformIdFromName(prog, name), 1, &value[0]);
}

//--------------------------------------------------------------------------------------
void ShaderManager::GLUniform4i(const ProgramPtr &prog, const std::string &name, const glm::i32vec4 &value) const {
    glUniform4iv(GetUniformIdFromName(prog, name), 1, &value[0]);
}


/*

uint's

*/

//--------------------------------------------------------------------------------------
void ShaderManager::GLUniform1ui(GLuint loc, const GLuint &value) const noexcept {
    glUniform1ui(loc, value);
}

//--------------------------------------------------------------------------------------
void ShaderManager::GLUniform2ui(GLuint loc, const glm::u32vec2 &value) const noexcept {
    glUniform2uiv(loc, 1, &value[0]);
}

//--------------------------------------------------------------------------------------
void ShaderManager::GLUniform3ui(GLuint loc, const glm::u32vec3 &value) const noexcept {
    glUniform3uiv(loc, 1, &value[0]);
}

//--------------------------------------------------------------------------------------
void ShaderManager::GLUniform4ui(GLuint loc, const glm::u32vec4 &value) const noexcept {
    glUniform4uiv(loc, 1, &value[0]);
}

//--------------------------------------------------------------------------------------
void ShaderManager::GLUniform1ui(const ProgramPtr &prog, const std::string &name, const GLuint &value) const {
    glUniform1ui(GetUniformIdFromName(prog, name), value);
}

//--------------------------------------------------------------------------------------
void ShaderManager::GLUniform2ui(const ProgramPtr &prog, const std::string &name, const glm::u32vec2 &value) const {
    glUniform2uiv(GetUniformIdFromName(prog, name), 1, &value[0]);
}

//--------------------------------------------------------------------------------------
void ShaderManager::GLUniform3ui(const ProgramPtr &prog, const std::string &name, const glm::u32vec3 &value) const {
    glUniform3uiv(GetUniformIdFromName(prog, name), 1, &value[0]);
}

//--------------------------------------------------------------------------------------
void ShaderManager::GLUniform4ui(const ProgramPtr &prog, const std::string &name, const glm::u32vec4 &value) const {
    glUniform4uiv(GetUniformIdFromName(prog, name), 1, &value[0]);
}

/*

matrices

*/

//--------------------------------------------------------------------------------------
void ShaderManager::GLUniformMatrix2f(GLuint loc, const glm::mat2 &value) const noexcept {
    glUniformMatrix2fv(loc, 1, 0, &value[0][0]);
}

//--------------------------------------------------------------------------------------
void ShaderManager::GLUniformMatrix3f(GLuint loc, const glm::mat3 &value) const noexcept {
    glUniformMatrix3fv(loc, 1, 0, &value[0][0]);
}

//--------------------------------------------------------------------------------------
void ShaderManager::GLUniformMatrix4f(GLuint loc, const glm::mat4 &value) const noexcept {
    glUniformMatrix4fv(loc, 1, 0, &value[0][0]);
}

//--------------------------------------------------------------------------------------
void ShaderManager::GLUniformMatrix2x3f(GLuint loc, const glm::mat2x3 &value) const noexcept {
    glUniformMatrix2x3fv(loc, 1, 0, &value[0][0]);
}

//--------------------------------------------------------------------------------------
void ShaderManager::GLUniformMatrix3x2f(GLuint loc, const glm::mat3x2 &value) const noexcept {
    glUniformMatrix3x2fv(loc, 1, 0, &value[0][0]);
}

//--------------------------------------------------------------------------------------
void ShaderManager::GLUniformMatrix2x4f(GLuint loc, const glm::mat2x4 &value) const noexcept {
    glUniformMatrix2x4fv(loc, 1, 0, &value[0][0]);
}

//--------------------------------------------------------------------------------------
void ShaderManager::GLUniformMatrix4x2f(GLuint loc, const glm::mat4x2 &value) const noexcept {
    glUniformMatrix4x2fv(loc, 1, 0, &value[0][0]);
}

//--------------------------------------------------------------------------------------
void ShaderManager::GLUniformMatrix3x4f(GLuint loc, const glm::mat3x4 &value) const noexcept {
    glUniformMatrix3x4fv(loc, 1, 0, &value[0][0]);
}

//--------------------------------------------------------------------------------------
void ShaderManager::GLUniformMatrix4x3f(GLuint loc, const glm::mat4x3 &value) const noexcept {
    //warning here can be IGNORED
    glUniformMatrix4x3fv(loc, 1, 0, &value[0][0]);
}

//--------------------------------------------------------------------------------------
void ShaderManager::GLUniformMatrix2f(const ProgramPtr &prog, const std::string &name, const glm::mat2 &value) const {
    glUniformMatrix2fv(GetUniformIdFromName(prog, name), 1, 0, &value[0][0]);
}

//--------------------------------------------------------------------------------------
void ShaderManager::GLUniformMatrix3f(const ProgramPtr &prog, const std::string &name, const glm::mat3 &value) const {
    glUniformMatrix3fv(GetUniformIdFromName(prog, name), 1, 0, &value[0][0]);
}

//--------------------------------------------------------------------------------------
void ShaderManager::GLUniformMatrix4f(const ProgramPtr &prog, const std::string &name, const glm::mat4 &value) const {
    glUniformMatrix4fv(GetUniformIdFromName(prog, name), 1, 0, &value[0][0]);
}

//--------------------------------------------------------------------------------------
void ShaderManager::GLUniformMatrix2x3f(const ProgramPtr &prog, const std::string &name, const glm::mat2x3 &value) const {
    glUniformMatrix2x3fv(GetUniformIdFromName(prog, name), 1, 0, &value[0][0]);
}

//--------------------------------------------------------------------------------------
void ShaderManager::GLUniformMatrix3x2f(const ProgramPtr &prog, const std::string &name, const glm::mat3x2 &value) const {
    glUniformMatrix3x2fv(GetUniformIdFromName(prog, name), 1, 0, &value[0][0]);
}

//--------------------------------------------------------------------------------------
void ShaderManager::GLUniformMatrix2x4f(const ProgramPtr &prog, const std::string &name, const glm::mat2x4 &value) const {
    glUniformMatrix2x4fv(GetUniformIdFromName(prog, name), 1, 0, &value[0][0]);
}

//--------------------------------------------------------------------------------------
void ShaderManager::GLUniformMatrix4x2f(const ProgramPtr &prog, const std::string &name, const glm::mat4x2 &value) const {
    glUniformMatrix4x2fv(GetUniformIdFromName(prog, name), 1, 0, &value[0][0]);
}

//--------------------------------------------------------------------------------------
void ShaderManager::GLUniformMatrix3x4f(const ProgramPtr &prog, const std::string &name, const glm::mat3x4 &value) const {
    glUniformMatrix3x4fv(GetUniformIdFromName(prog, name), 1, 0, &value[0][0]);
}

//--------------------------------------------------------------------------------------
void ShaderManager::GLUniformMatrix4x3f(const ProgramPtr &prog, const std::string &name, const glm::mat4x3 &value) const {
    glUniformMatrix4x3fv(GetUniformIdFromName(prog, name), 1, 0, &value[0][0]);
}





/*

GLProgramUniform*


*/

//macro for easier readibility; the purpose of this line is to throw a 'NoActiveProgramUniformException' in debug mode 
#ifdef _DEBUG

#define HAS_ACTIVE_PROGRAM(ppo) ppo->GetActiveProgram();

#else

#define HAS_ACTIVE_PROGRAM(ppo)

#endif

/*

float's

*/

//--------------------------------------------------------------------------------------
void ShaderManager::GLProgramUniform1f(const SepProgramPtr &ppo, GLuint loc, const GLfloat &value) const {
    HAS_ACTIVE_PROGRAM(ppo);

    glProgramUniform1f(ppo->GetActiveProgram()->GetId(), loc, value);
}

//--------------------------------------------------------------------------------------
void ShaderManager::GLProgramUniform2f(const SepProgramPtr &ppo, GLuint loc, const glm::vec2 &value) const {
    HAS_ACTIVE_PROGRAM(ppo);

    glProgramUniform2fv(ppo->GetActiveProgram()->GetId(), loc, 1, &value[0]);
}

//--------------------------------------------------------------------------------------
void ShaderManager::GLProgramUniform3f(const SepProgramPtr &ppo, GLuint loc, const glm::vec3 &value) const {
    HAS_ACTIVE_PROGRAM(ppo);

    glProgramUniform3fv(ppo->GetActiveProgram()->GetId(), loc, 1, &value[0]);
}

//--------------------------------------------------------------------------------------
void ShaderManager::GLProgramUniform4f(const SepProgramPtr &ppo, GLuint loc, const glm::vec4 &value) const {
    HAS_ACTIVE_PROGRAM(ppo);

    glProgramUniform4fv(ppo->GetActiveProgram()->GetId(), loc, 1, &value[0]);
}

//--------------------------------------------------------------------------------------
void ShaderManager::GLProgramUniform1f(const SepProgramPtr &ppo, const std::string &name, const GLfloat &value) const {
    glProgramUniform1f(ppo->GetActiveProgram()->GetId(), GetUniformIdFromName(ppo, name), value);
}

//--------------------------------------------------------------------------------------
void ShaderManager::GLProgramUniform2f(const SepProgramPtr &ppo, const std::string &name, const glm::vec2 &value) const {
    glProgramUniform2fv(ppo->GetActiveProgram()->GetId(), GetUniformIdFromName(ppo, name), 1, &value[0]);
}

//--------------------------------------------------------------------------------------
void ShaderManager::GLProgramUniform3f(const SepProgramPtr &ppo, const std::string &name, const glm::vec3 &value) const {
    glProgramUniform3fv(ppo->GetActiveProgram()->GetId(), GetUniformIdFromName(ppo, name), 1, &value[0]);
}

//--------------------------------------------------------------------------------------
void ShaderManager::GLProgramUniform4f(const SepProgramPtr &ppo, const std::string &name, const glm::vec4 &value) const {
    glProgramUniform4fv(ppo->GetActiveProgram()->GetId(), GetUniformIdFromName(ppo, name), 1, &value[0]);
}


/*

int's

*/

//--------------------------------------------------------------------------------------
void ShaderManager::GLProgramUniform1i(const SepProgramPtr &ppo, GLuint loc, const GLint &value) const {
    HAS_ACTIVE_PROGRAM(ppo);

    glProgramUniform1i(ppo->GetActiveProgram()->GetId(), loc, value);
}

//--------------------------------------------------------------------------------------
void ShaderManager::GLProgramUniform2i(const SepProgramPtr &ppo, GLuint loc, const glm::i32vec2 &value) const {
    HAS_ACTIVE_PROGRAM(ppo);

    glProgramUniform2iv(ppo->GetActiveProgram()->GetId(), loc, 1, &value[0]);
}

//--------------------------------------------------------------------------------------
void ShaderManager::GLProgramUniform3i(const SepProgramPtr &ppo, GLuint loc, const glm::i32vec3 &value) const {
    HAS_ACTIVE_PROGRAM(ppo);

    glProgramUniform3iv(ppo->GetActiveProgram()->GetId(), loc, 1, &value[0]);
}

//--------------------------------------------------------------------------------------
void ShaderManager::GLProgramUniform4i(const SepProgramPtr &ppo, GLuint loc, const glm::i32vec4 &value) const {
    HAS_ACTIVE_PROGRAM(ppo);

    glProgramUniform4iv(ppo->GetActiveProgram()->GetId(), loc, 1, &value[0]);
}

//--------------------------------------------------------------------------------------
void ShaderManager::GLProgramUniform1i(const SepProgramPtr &ppo, const std::string &name, const GLint &value) const {
    glProgramUniform1i(ppo->GetActiveProgram()->GetId(), GetUniformIdFromName(ppo, name), value);
}

//--------------------------------------------------------------------------------------
void ShaderManager::GLProgramUniform2i(const SepProgramPtr &ppo, const std::string &name, const glm::i32vec2 &value) const {
    glProgramUniform2iv(ppo->GetActiveProgram()->GetId(), GetUniformIdFromName(ppo, name), 1, &value[0]);
}

//--------------------------------------------------------------------------------------
void ShaderManager::GLProgramUniform3i(const SepProgramPtr &ppo, const std::string &name, const glm::i32vec3 &value) const {
    glProgramUniform3iv(ppo->GetActiveProgram()->GetId(), GetUniformIdFromName(ppo, name), 1, &value[0]);
}

//--------------------------------------------------------------------------------------
void ShaderManager::GLProgramUniform4i(const SepProgramPtr &ppo, const std::string &name, const glm::i32vec4 &value) const {
    glProgramUniform4iv(ppo->GetActiveProgram()->GetId(), GetUniformIdFromName(ppo, name), 1, &value[0]);
}


/*

uint's

*/

//--------------------------------------------------------------------------------------
void ShaderManager::GLProgramUniform1ui(const SepProgramPtr &ppo, GLuint loc, const GLuint &value) const {
    HAS_ACTIVE_PROGRAM(ppo);

    glProgramUniform1ui(ppo->GetActiveProgram()->GetId(), loc, value);
}

//--------------------------------------------------------------------------------------
void ShaderManager::GLProgramUniform2ui(const SepProgramPtr &ppo, GLuint loc, const glm::u32vec2 &value) const {
    HAS_ACTIVE_PROGRAM(ppo);

    glProgramUniform2uiv(ppo->GetActiveProgram()->GetId(), loc, 1, &value[0]);
}

//--------------------------------------------------------------------------------------
void ShaderManager::GLProgramUniform3ui(const SepProgramPtr &ppo, GLuint loc, const glm::u32vec3 &value) const {
    HAS_ACTIVE_PROGRAM(ppo);

    glProgramUniform3uiv(ppo->GetActiveProgram()->GetId(), loc, 1, &value[0]);
}

//--------------------------------------------------------------------------------------
void ShaderManager::GLProgramUniform4ui(const SepProgramPtr &ppo, GLuint loc, const glm::u32vec4 &value) const {
    HAS_ACTIVE_PROGRAM(ppo);

    glProgramUniform4uiv(ppo->GetActiveProgram()->GetId(), loc, 1, &value[0]);
}

//--------------------------------------------------------------------------------------
void ShaderManager::GLProgramUniform1ui(const SepProgramPtr &ppo, const std::string &name, const GLuint &value) const {
    glProgramUniform1ui(ppo->GetActiveProgram()->GetId(), GetUniformIdFromName(ppo, name), value);
}

//--------------------------------------------------------------------------------------
void ShaderManager::GLProgramUniform2ui(const SepProgramPtr &ppo, const std::string &name, const glm::u32vec2 &value) const {
    glProgramUniform2uiv(ppo->GetActiveProgram()->GetId(), GetUniformIdFromName(ppo, name), 1, &value[0]);
}

//--------------------------------------------------------------------------------------
void ShaderManager::GLProgramUniform3ui(const SepProgramPtr &ppo, const std::string &name, const glm::u32vec3 &value) const {
    glProgramUniform3uiv(ppo->GetActiveProgram()->GetId(), GetUniformIdFromName(ppo, name), 1, &value[0]);
}

//--------------------------------------------------------------------------------------
void ShaderManager::GLProgramUniform4ui(const SepProgramPtr &ppo, const std::string &name, const glm::u32vec4 &value) const {
    glProgramUniform4uiv(ppo->GetActiveProgram()->GetId(), GetUniformIdFromName(ppo, name), 1, &value[0]);
}

/*

matricies

*/

//--------------------------------------------------------------------------------------
void ShaderManager::GLProgramUniformMatrix2f(const SepProgramPtr &ppo, GLuint loc, const glm::mat2 &value) const {
    HAS_ACTIVE_PROGRAM(ppo);

    glProgramUniformMatrix2fv(ppo->GetActiveProgram()->GetId(), loc, 1, 0, &value[0][0]);
}

//--------------------------------------------------------------------------------------
void ShaderManager::GLProgramUniformMatrix3f(const SepProgramPtr &ppo, GLuint loc, const glm::mat3 &value) const {
    HAS_ACTIVE_PROGRAM(ppo);

    glProgramUniformMatrix3fv(ppo->GetActiveProgram()->GetId(), loc, 1, 0, &value[0][0]);
}

//--------------------------------------------------------------------------------------
void ShaderManager::GLProgramUniformMatrix4f(const SepProgramPtr &ppo, GLuint loc, const glm::mat4 &value) const {
    HAS_ACTIVE_PROGRAM(ppo);

    glProgramUniformMatrix4fv(ppo->GetActiveProgram()->GetId(), loc, 1, 0, &value[0][0]);
}

//--------------------------------------------------------------------------------------
void ShaderManager::GLProgramUniformMatrix2x3f(const SepProgramPtr &ppo, GLuint loc, const glm::mat2x3 &value) const {
    HAS_ACTIVE_PROGRAM(ppo);

    glProgramUniformMatrix2x3fv(ppo->GetActiveProgram()->GetId(), loc, 1, 0, &value[0][0]);
}

//--------------------------------------------------------------------------------------
void ShaderManager::GLProgramUniformMatrix3x2f(const SepProgramPtr &ppo, GLuint loc, const glm::mat3x2 &value) const {
    HAS_ACTIVE_PROGRAM(ppo);

    glProgramUniformMatrix3x2fv(ppo->GetActiveProgram()->GetId(), loc, 1, 0, &value[0][0]);
}

//--------------------------------------------------------------------------------------
void ShaderManager::GLProgramUniformMatrix2x4f(const SepProgramPtr &ppo, GLuint loc, const glm::mat2x4 &value) const {
    HAS_ACTIVE_PROGRAM(ppo);

    glProgramUniformMatrix2x4fv(ppo->GetActiveProgram()->GetId(), loc, 1, 0, &value[0][0]);
}

//--------------------------------------------------------------------------------------
void ShaderManager::GLProgramUniformMatrix4x2f(const SepProgramPtr &ppo, GLuint loc, const glm::mat4x2 &value) const {
    HAS_ACTIVE_PROGRAM(ppo);

    glProgramUniformMatrix4x2fv(ppo->GetActiveProgram()->GetId(), loc, 1, 0, &value[0][0]);
}

//--------------------------------------------------------------------------------------
void ShaderManager::GLProgramUniformMatrix3x4f(const SepProgramPtr &ppo, GLuint loc, const glm::mat3x4 &value) const {
    HAS_ACTIVE_PROGRAM(ppo);

    glProgramUniformMatrix3x4fv(ppo->GetActiveProgram()->GetId(), loc, 1, 0, &value[0][0]);
}

//--------------------------------------------------------------------------------------
void ShaderManager::GLProgramUniformMatrix4x3f(const SepProgramPtr &ppo, GLuint loc, const glm::mat4x3 &value) const {
    HAS_ACTIVE_PROGRAM(ppo);

    glProgramUniformMatrix4x3fv(ppo->GetActiveProgram()->GetId(), loc, 1, 0, &value[0][0]);
}

//--------------------------------------------------------------------------------------
void ShaderManager::GLProgramUniformMatrix2f(const SepProgramPtr &ppo, const std::string &name, const glm::mat2 &value) const {
    glProgramUniformMatrix2fv(ppo->GetActiveProgram()->GetId(), GetUniformIdFromName(ppo, name), 1, 0, &value[0][0]);
}

//--------------------------------------------------------------------------------------
void ShaderManager::GLProgramUniformMatrix3f(const SepProgramPtr &ppo, const std::string &name, const glm::mat3 &value) const {
    glProgramUniformMatrix3fv(ppo->GetActiveProgram()->GetId(), GetUniformIdFromName(ppo, name), 1, 0, &value[0][0]);
}

//--------------------------------------------------------------------------------------
void ShaderManager::GLProgramUniformMatrix4f(const SepProgramPtr &ppo, const std::string &name, const glm::mat4 &value) const {
    glProgramUniformMatrix4fv(ppo->GetActiveProgram()->GetId(), GetUniformIdFromName(ppo, name), 1, 0, &value[0][0]);
}

//--------------------------------------------------------------------------------------
void ShaderManager::GLProgramUniformMatrix2x3f(const SepProgramPtr &ppo, const std::string &name, const glm::mat2x3 &value) const {
    glProgramUniformMatrix2x3fv(ppo->GetActiveProgram()->GetId(), GetUniformIdFromName(ppo, name), 1, 0, &value[0][0]);
}

//--------------------------------------------------------------------------------------
void ShaderManager::GLProgramUniformMatrix3x2f(const SepProgramPtr &ppo, const std::string &name, const glm::mat3x2 &value) const {
    glProgramUniformMatrix3x2fv(ppo->GetActiveProgram()->GetId(), GetUniformIdFromName(ppo, name), 1, 0, &value[0][0]);
}

//--------------------------------------------------------------------------------------
void ShaderManager::GLProgramUniformMatrix2x4f(const SepProgramPtr &ppo, const std::string &name, const glm::mat2x4 &value) const {
    glProgramUniformMatrix2x4fv(ppo->GetActiveProgram()->GetId(), GetUniformIdFromName(ppo, name), 1, 0, &value[0][0]);
}

//--------------------------------------------------------------------------------------
void ShaderManager::GLProgramUniformMatrix4x2f(const SepProgramPtr &ppo, const std::string &name, const glm::mat4x2 &value) const {
    glProgramUniformMatrix4x2fv(ppo->GetActiveProgram()->GetId(), GetUniformIdFromName(ppo, name), 1, 0, &value[0][0]);
}

//--------------------------------------------------------------------------------------
void ShaderManager::GLProgramUniformMatrix3x4f(const SepProgramPtr &ppo, const std::string &name, const glm::mat3x4 &value) const {
    glProgramUniformMatrix3x4fv(ppo->GetActiveProgram()->GetId(), GetUniformIdFromName(ppo, name), 1, 0, &value[0][0]);
}

//--------------------------------------------------------------------------------------
void ShaderManager::GLProgramUniformMatrix4x3f(const SepProgramPtr &ppo, const std::string &name, const glm::mat4x3 &value) const {
    glProgramUniformMatrix4x3fv(ppo->GetActiveProgram()->GetId(), GetUniformIdFromName(ppo, name), 1, 0, &value[0][0]);
}

}

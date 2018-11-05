#ifndef OBJGLUF_SHADER_H
#define OBJGLUF_SHADER_H

#include "../ObjGLUF.h"

namespace GLUF {

/*
================================================================
Shader API (Alpha)

Note:
    -Classes and Structs for dealing directly with OpenGL may use some C types

*/


    enum LocationType {
        GLT_ATTRIB = 0,
        GLT_UNIFORM,
    };

    struct OBJGLUF_API ShaderInfoStruct {
        bool mSuccess = false;

        std::string mLog;

        operator bool() const {
            return mSuccess;
        }
    };

    enum ShaderType {
        SH_VERTEX_SHADER = GL_VERTEX_SHADER,
        SH_TESSCONTROL_SHADER = GL_TESS_CONTROL_SHADER,
        SH_TESS_EVALUATION_SHADER = GL_TESS_EVALUATION_SHADER,
        SH_GEOMETRY_SHADER = GL_GEOMETRY_SHADER,
        SH_FRAGMENT_SHADER = GL_FRAGMENT_SHADER,
        SH_COMPUTE_SHADER = GL_COMPUTE_SHADER
    };

    enum ProgramStage {
        PPO_INVALID_SHADER_BIT = 0,//this is 0, since passing a null bitfield signifies not to use this program for any stage
        PPO_VERTEX_SHADER_BIT = GL_VERTEX_SHADER_BIT,
        PPO_TESSCONTROL_SHADER_BIT = GL_TESS_CONTROL_SHADER_BIT,
        PPO_TESS_EVALUATION_SHADER_BIT = GL_TESS_EVALUATION_SHADER_BIT,
        PPO_GEOMETRY_SHADER_BIT = GL_GEOMETRY_SHADER_BIT,
        PPO_FRAGMENT_SHADER_BIT = GL_FRAGMENT_SHADER_BIT
    };

/*
ShaderTypeToProgramStage

    Parameters:
        'type': which shader type it is

    Returns:
        corresponding 'ProgramStage' to 'type'
*/
    ProgramStage ShaderTypeToProgramStage(ShaderType type);

//use type alias because the compile and link output are both the same
    using CompileOutputStruct = ShaderInfoStruct;
    using LinkOutputStruct    = ShaderInfoStruct;


/*

Opaque Shader Classes

*/

    class OBJGLUF_API Shader;

    class OBJGLUF_API Program;

    class OBJGLUF_API SeparateProgram;


/*

Shader Aliases

*/

    using ShaderPtr         = std::shared_ptr<Shader>;
    using ShaderPtrWeak     = std::weak_ptr<Shader>;
    using ProgramPtr        = std::shared_ptr<Program>;
    using ProgramPtrWeak    = std::weak_ptr<Program>;
    using SepProgramPtr     = std::shared_ptr<SeparateProgram>;
    using SepProgramPtrWeak = std::weak_ptr<SeparateProgram>;

    using ShaderSourceList      = std::map<ShaderType, std::string>;
    using ShaderPathList        = std::map<ShaderType, std::string>;
    using ShaderIdList          = std::vector<GLuint>;
    using ProgramIdList         = std::vector<GLuint>;
    using ShaderNameList        = std::vector<std::string>;
    using ProgramNameList       = std::vector<std::string>;
    using ShaderPtrList         = std::vector<ShaderPtr>;
    using ProgramPtrList        = std::vector<ProgramPtr>;
    using ProgramPtrMap         = std::map<ShaderType, ProgramPtr>;
    using ShaderPtrListWeak     = std::vector<ShaderPtrWeak>;
    using ProgramPtrListWeak    = std::vector<ProgramPtrWeak>;

    using VariableLocMap    = std::map<std::string, GLuint>;
    using VariableLocPair = std::pair<std::string, GLuint>;

/*

Shader Exceptions

*/
    class UseProgramException : public Exception {
    public:
        virtual const char *what() const noexcept override {
            return "Failed to Use Program!";
        }

        EXCEPTION_CONSTRUCTOR(UseProgramException)
    };

    class MakeShaderException : public Exception {
    public:
        virtual const char *what() const noexcept override {
            return "Shading Creation Failed!";
        }

        EXCEPTION_CONSTRUCTOR(MakeShaderException)
    };

    class MakeProgramException : public Exception {
    public:
        virtual const char *what() const noexcept override {
            return "Program Creation Failed!";
        }

        EXCEPTION_CONSTRUCTOR(MakeProgramException)
    };

    class MakePPOException : public Exception {
    public:
        virtual const char *what() const noexcept override {
            return "PPO Creation Failed!";
        }

        EXCEPTION_CONSTRUCTOR(MakePPOException)
    };

    class NoActiveProgramUniformException : public Exception {
    public:
        virtual const char *what() const noexcept override {
            return "Attempt to Buffer Uniform to PPO with no Active Program!";
        }

        EXCEPTION_CONSTRUCTOR(NoActiveProgramUniformException)
    };

/*
ShaderManager

    Multithreading:
        -Thread-Safe Functions:
            'CreateShaderFrom*'
            'CreateProgram'
            'GetShader*Location(s)'
            'Get*Log'
            'FlushLogs'
            'GL*'
            'GetUniformIdFromName'


    Data Members:
        'mCompileLogs': a map of logs for each compiled shader
        'mLinkLogs': a map of logs for each linked program
        'mCompLogMutex': mutual exclution protection for compile logs
        'mLinkLogMutex': mutual exclusion protection for link logs

    Note:
        Much of this is pass-by-reference in order to still have valid data members even
            if exceptions are thrown

*/

    class OBJGLUF_API ShaderManager {

        //a list of logs
        std::map<ShaderPtr, ShaderInfoStruct> mCompileLogs;
        std::map<ProgramPtr, ShaderInfoStruct> mLinklogs;

        mutable std::mutex mCompLogMutex;
        mutable std::mutex mLinkLogMutex;

        /*
        AddCompileLog

            Parameters:
                'shader': shader to add to pair
                'log': log to accompany it

            Note:
                This is meant to be use in order to handle thread-safe log access
        */
        void AddCompileLog(const ShaderPtr &shader, const ShaderInfoStruct &log);


        /*
        AddLinkLog

        Parameters:
            'program': program to add to pair
            'log': log to accompany it

        Note:
            This is meant to be use in order to handle thread-safe log access
        */
        void AddLinkLog(const ProgramPtr &program, const ShaderInfoStruct &log);

        /*
        GetUniformIdFromName

            Parameters:
                'prog': the program to access
                'ppo': the ppo to access
                'name': the name of the uniform

            Returns:
                the id of the uniform

        */
        GLuint GetUniformIdFromName(const SepProgramPtr &ppo, const std::string &name) const;

        GLuint GetUniformIdFromName(const ProgramPtr &prog, const std::string &name) const;

    public:

        /*
        CreateShader*

            Parameters:
                'filePath': path to file containing shader
                'text': text to compile
                'memory': raw memory of text file loaded
                'type': self-explanitory
                'outShader': your brand spankin' new shader

            Throws:
                -CreateShaderFromFile: 'std::ios_base::failure': if file had issues reading
                'MakeShaderException' if compilation, loading, or anything else failed

        */

        void CreateShaderFromFile(ShaderPtr &outShader, const std::string &filePath, ShaderType type);

        void CreateShaderFromText(ShaderPtr &outShader, const std::string &text, ShaderType type);

        void CreateShaderFromMemory(ShaderPtr &outShader, const std::vector<char> &memory, ShaderType type);


        /*
        CreateProgram

            Parameters:
                'shaders': simply a list of precompiled shaders to link
                'shaderSources': a list of shader files in strings to be compiled
                'shaderPaths': a list of shader file paths to be loaded into strings and compiled
                'separate': WIP Don't Use
                'outProgram': your brank spankin' new program

            Throws:
                'std::ios_base::failure': if file loading has issues
                'MakeProgramException' if linking failed
                'MakeShaderException' if shaders need to be compiled and fail

        */

        void CreateProgram(ProgramPtr &outProgram, ShaderPtrList shaders, bool separate = false);

        void CreateProgram(ProgramPtr &outProgram, ShaderSourceList shaderSources, bool separate = false);

        void CreateProgramFromFiles(ProgramPtr &outProgram, ShaderPathList shaderPaths, bool separate = false);


        /*
        GetShader*Location

            Parameters:
                'prog': the program to access
                'locType': which location type? (attribute or uniform)
                'varName': name of variable to get id of

            Returns:
                'GLuint': location id of variable
                'VariableLocMap': map of attribute/variable names to their respective id's

            Throws:
                'std::invalid_argument': if 'prog' == nullptr
                'std::invalid_argument': if 'varName' does not exist

        */

        const GLuint
        GetShaderVariableLocation(const ProgramPtr &program, LocationType locType, const std::string &varName) const;

        const VariableLocMap &GetShaderAttribLocations(const ProgramPtr &program) const;

        const VariableLocMap &GetShaderUniformLocations(const ProgramPtr &program) const;

        const VariableLocMap GetShaderAttribLocations(const SepProgramPtr &program) const;

        const VariableLocMap GetShaderUniformLocations(const SepProgramPtr &program) const;


        /*
        Delete*

            Parameters:
                'shader': shader to be deleted
                'program': program to be deleted

            Note:
                -This does not delete the smart pointers, however it does bring the shader to a state
                    in which it can be reused

            Throws:
                no-throw guarantee

        */

        void DeleteShader(ShaderPtr &shader) noexcept;

        void DeleteProgram(ProgramPtr &program) noexcept;


        //self-explanitory
        void FlushLogs();

        /*
        Get

            Parameters:
                'shader': the shader to access
                'program': the program to access

            Returns:
                'GLuint': id of shader/program
                'ShaderType': what type of shader it is
                'CompileOutputStruct': get log file for compiling
                'LinkOutputStruct': get log file for linking

            Throws
                'std::invalid_argument': if shader/program == nullptr
        */
        const GLuint GetShaderId(const ShaderPtr &shader) const;

        const ShaderType GetShaderType(const ShaderPtr &shader) const;

        const GLuint GetProgramId(const ProgramPtr &program) const;

        const CompileOutputStruct GetShaderLog(const ShaderPtr &shaderPtr) const;

        const LinkOutputStruct GetProgramLog(const ProgramPtr &programPtr) const;


        /*
        UseProgram

            Parameters:
                'program': program to bind for drawing or other purposes

            Throws:
                'std::invalid_argument': if program == nullptr
                'UseProgramException': if program id == 0 (For binding the null program, use UseProgramNull() instead)

        */

        void UseProgram(const ProgramPtr &program) const;

        void UseProgram(const SepProgramPtr &ppo) const;

        //bind program id and ppo id 0
        void UseProgramNull() const noexcept;


        /*
        AttachPrograms

            Parameters:
                'ppo': the programmible pipeline object
                'programs': the list of programs to add
                'stages': which stages to add program to
                'program': single program to add

            Throws:
                'std::invalid_argument': if ppo == nullptr
                'std::invalid_argument': if program(s) == nullptr
        */

        void AttachPrograms(SepProgramPtr &ppo, const ProgramPtrList &programs) const;

        void AttachProgram(SepProgramPtr &ppo, const ProgramPtr &program) const;


        /*
        ClearPrograms

            Parameters:
                'ppo': the programmible pipeline objects
                'stages': which stages should be cleared (reset to default)

            Throws:
                'std::invalid_argument': if ppo == nullptr
        */

        void ClearPrograms(SepProgramPtr &ppo, GLbitfield stages = GL_ALL_SHADER_BITS) const;


        /*
        CreateSeparateProgram

            Parameters:
                'ppo': a null pointer reference to the ppo that will be created
                'programs': the list of programs to initialize the ppo with

            Throws:
                'std::invalid_argument': if any of the programs are nullptr or have an id of 0
                'MakePPOException': if ppo creation failed

        */
        void CreateSeparateProgram(SepProgramPtr &ppo, const ProgramPtrList &programs) const;

        /*
        GLUniform*

        Note:
            the 'program' parameter is used as a look-up when the 'name' parameter is used,
                however you still must call 'UseProgram' before calling this

        OpenGL Version Restrictions:
            -GLUniform*ui* requires openGL version 3.0 or lator

        Parameters:
            'program': the program which the uniform will be set to
            'value': the data to set as the uniform
            'loc': the location of the uniform
            'name': the name of the uniform

        Throws:
            'std::invalid_argument': 'name' does not exist

        */

        void GLUniform1f(GLuint loc, const GLfloat &value) const noexcept;

        void GLUniform2f(GLuint loc, const glm::vec2 &value) const noexcept;

        void GLUniform3f(GLuint loc, const glm::vec3 &value) const noexcept;

        void GLUniform4f(GLuint loc, const glm::vec4 &value) const noexcept;

        void GLUniform1i(GLuint loc, const GLint &value) const noexcept;

        void GLUniform2i(GLuint loc, const glm::i32vec2 &value) const noexcept;

        void GLUniform3i(GLuint loc, const glm::i32vec3 &value) const noexcept;

        void GLUniform4i(GLuint loc, const glm::i32vec4 &value) const noexcept;

        void GLUniform1ui(GLuint loc, const GLuint &value) const noexcept;

        void GLUniform2ui(GLuint loc, const glm::u32vec2 &value) const noexcept;

        void GLUniform3ui(GLuint loc, const glm::u32vec3 &value) const noexcept;

        void GLUniform4ui(GLuint loc, const glm::u32vec4 &value) const noexcept;

        void GLUniformMatrix2f(GLuint loc, const glm::mat2 &value) const noexcept;

        void GLUniformMatrix3f(GLuint loc, const glm::mat3 &value) const noexcept;

        void GLUniformMatrix4f(GLuint loc, const glm::mat4 &value) const noexcept;

        void GLUniformMatrix2x3f(GLuint loc, const glm::mat2x3 &value) const noexcept;

        void GLUniformMatrix3x2f(GLuint loc, const glm::mat3x2 &value) const noexcept;

        void GLUniformMatrix2x4f(GLuint loc, const glm::mat2x4 &value) const noexcept;

        void GLUniformMatrix4x2f(GLuint loc, const glm::mat4x2 &value) const noexcept;

        void GLUniformMatrix3x4f(GLuint loc, const glm::mat3x4 &value) const noexcept;

        void GLUniformMatrix4x3f(GLuint loc, const glm::mat4x3 &value) const noexcept;


        void GLUniform1f(const ProgramPtr &prog, const std::string &name, const GLfloat &value) const;

        void GLUniform2f(const ProgramPtr &prog, const std::string &name, const glm::vec2 &value) const;

        void GLUniform3f(const ProgramPtr &prog, const std::string &name, const glm::vec3 &value) const;

        void GLUniform4f(const ProgramPtr &prog, const std::string &name, const glm::vec4 &value) const;

        void GLUniform1i(const ProgramPtr &prog, const std::string &name, const GLint &value) const;

        void GLUniform2i(const ProgramPtr &prog, const std::string &name, const glm::i32vec2 &value) const;

        void GLUniform3i(const ProgramPtr &prog, const std::string &name, const glm::i32vec3 &value) const;

        void GLUniform4i(const ProgramPtr &prog, const std::string &name, const glm::i32vec4 &value) const;

        void GLUniform1ui(const ProgramPtr &prog, const std::string &name, const GLuint &value) const;

        void GLUniform2ui(const ProgramPtr &prog, const std::string &name, const glm::u32vec2 &value) const;

        void GLUniform3ui(const ProgramPtr &prog, const std::string &name, const glm::u32vec3 &value) const;

        void GLUniform4ui(const ProgramPtr &prog, const std::string &name, const glm::u32vec4 &value) const;

        void GLUniformMatrix2f(const ProgramPtr &prog, const std::string &name, const glm::mat2 &value) const;

        void GLUniformMatrix3f(const ProgramPtr &prog, const std::string &name, const glm::mat3 &value) const;

        void GLUniformMatrix4f(const ProgramPtr &prog, const std::string &name, const glm::mat4 &value) const;

        void GLUniformMatrix2x3f(const ProgramPtr &prog, const std::string &name, const glm::mat2x3 &value) const;

        void GLUniformMatrix3x2f(const ProgramPtr &prog, const std::string &name, const glm::mat3x2 &value) const;

        void GLUniformMatrix2x4f(const ProgramPtr &prog, const std::string &name, const glm::mat2x4 &value) const;

        void GLUniformMatrix4x2f(const ProgramPtr &prog, const std::string &name, const glm::mat4x2 &value) const;

        void GLUniformMatrix3x4f(const ProgramPtr &prog, const std::string &name, const glm::mat3x4 &value) const;

        void GLUniformMatrix4x3f(const ProgramPtr &prog, const std::string &name, const glm::mat4x3 &value) const;

        /*
        GLProgramUniform*

            Note:
                this is only useful when using PPO's; if using regular programs, just use 'GLUniform*'
                the stage of the program which is being modified is set by 'GLActiveShaderProgram'

            Parameters:
                'ppo': ppo set the uniform
                'value': the data to set as the uniform
                'loc': the location of the uniform
                'name': the name of the uniform

            Throws:
                'std::invalid_argument': if 'ppo' == nullptr
                'std::invalid_argument': if 'loc' or 'name' does not exist in active program
                'NoActiveProgramUniformException': if 'GLActiveShaderProgram' was not called first

        */

        void GLProgramUniform1f(const SepProgramPtr &ppo, GLuint loc, const GLfloat &value) const;

        void GLProgramUniform2f(const SepProgramPtr &ppo, GLuint loc, const glm::vec2 &value) const;

        void GLProgramUniform3f(const SepProgramPtr &ppo, GLuint loc, const glm::vec3 &value) const;

        void GLProgramUniform4f(const SepProgramPtr &ppo, GLuint loc, const glm::vec4 &value) const;

        void GLProgramUniform1i(const SepProgramPtr &ppo, GLuint loc, const GLint &value) const;

        void GLProgramUniform2i(const SepProgramPtr &ppo, GLuint loc, const glm::i32vec2 &value) const;

        void GLProgramUniform3i(const SepProgramPtr &ppo, GLuint loc, const glm::i32vec3 &value) const;

        void GLProgramUniform4i(const SepProgramPtr &ppo, GLuint loc, const glm::i32vec4 &value) const;

        void GLProgramUniform1ui(const SepProgramPtr &ppo, GLuint loc, const GLuint &value) const;

        void GLProgramUniform2ui(const SepProgramPtr &ppo, GLuint loc, const glm::u32vec2 &value) const;

        void GLProgramUniform3ui(const SepProgramPtr &ppo, GLuint loc, const glm::u32vec3 &value) const;

        void GLProgramUniform4ui(const SepProgramPtr &ppo, GLuint loc, const glm::u32vec4 &value) const;

        void GLProgramUniformMatrix2f(const SepProgramPtr &ppo, GLuint loc, const glm::mat2 &value) const;

        void GLProgramUniformMatrix3f(const SepProgramPtr &ppo, GLuint loc, const glm::mat3 &value) const;

        void GLProgramUniformMatrix4f(const SepProgramPtr &ppo, GLuint loc, const glm::mat4 &value) const;

        void GLProgramUniformMatrix2x3f(const SepProgramPtr &ppo, GLuint loc, const glm::mat2x3 &value) const;

        void GLProgramUniformMatrix3x2f(const SepProgramPtr &ppo, GLuint loc, const glm::mat3x2 &value) const;

        void GLProgramUniformMatrix2x4f(const SepProgramPtr &ppo, GLuint loc, const glm::mat2x4 &value) const;

        void GLProgramUniformMatrix4x2f(const SepProgramPtr &ppo, GLuint loc, const glm::mat4x2 &value) const;

        void GLProgramUniformMatrix3x4f(const SepProgramPtr &ppo, GLuint loc, const glm::mat3x4 &value) const;

        void GLProgramUniformMatrix4x3f(const SepProgramPtr &ppo, GLuint loc, const glm::mat4x3 &value) const;


        void GLProgramUniform1f(const SepProgramPtr &ppo, const std::string &name, const GLfloat &value) const;

        void GLProgramUniform2f(const SepProgramPtr &ppo, const std::string &name, const glm::vec2 &value) const;

        void GLProgramUniform3f(const SepProgramPtr &ppo, const std::string &name, const glm::vec3 &value) const;

        void GLProgramUniform4f(const SepProgramPtr &ppo, const std::string &name, const glm::vec4 &value) const;

        void GLProgramUniform1i(const SepProgramPtr &ppo, const std::string &name, const GLint &value) const;

        void GLProgramUniform2i(const SepProgramPtr &ppo, const std::string &name, const glm::i32vec2 &value) const;

        void GLProgramUniform3i(const SepProgramPtr &ppo, const std::string &name, const glm::i32vec3 &value) const;

        void GLProgramUniform4i(const SepProgramPtr &ppo, const std::string &name, const glm::i32vec4 &value) const;

        void GLProgramUniform1ui(const SepProgramPtr &ppo, const std::string &name, const GLuint &value) const;

        void GLProgramUniform2ui(const SepProgramPtr &ppo, const std::string &name, const glm::u32vec2 &value) const;

        void GLProgramUniform3ui(const SepProgramPtr &ppo, const std::string &name, const glm::u32vec3 &value) const;

        void GLProgramUniform4ui(const SepProgramPtr &ppo, const std::string &name, const glm::u32vec4 &value) const;

        void GLProgramUniformMatrix2f(const SepProgramPtr &ppo, const std::string &name, const glm::mat2 &value) const;

        void GLProgramUniformMatrix3f(const SepProgramPtr &ppo, const std::string &name, const glm::mat3 &value) const;

        void GLProgramUniformMatrix4f(const SepProgramPtr &ppo, const std::string &name, const glm::mat4 &value) const;

        void
        GLProgramUniformMatrix2x3f(const SepProgramPtr &ppo, const std::string &name, const glm::mat2x3 &value) const;

        void
        GLProgramUniformMatrix3x2f(const SepProgramPtr &ppo, const std::string &name, const glm::mat3x2 &value) const;

        void
        GLProgramUniformMatrix2x4f(const SepProgramPtr &ppo, const std::string &name, const glm::mat2x4 &value) const;

        void
        GLProgramUniformMatrix4x2f(const SepProgramPtr &ppo, const std::string &name, const glm::mat4x2 &value) const;

        void
        GLProgramUniformMatrix3x4f(const SepProgramPtr &ppo, const std::string &name, const glm::mat3x4 &value) const;

        void
        GLProgramUniformMatrix4x3f(const SepProgramPtr &ppo, const std::string &name, const glm::mat4x3 &value) const;


        /*
        GLActiveShaderProgram

            Parameters:
                'ppo': the ppo object to look for the shader program in
                'stage': the program stage to bind

            Note:
                this is called before 'GLProgramUniform*'

                this tells OpenGL which program to look in to find the following uniforms

                'stage' may be part of a program with multiple stages, however this will not affect performance

        */

        void GLActiveShaderProgram(SepProgramPtr &ppo, ShaderType stage) const;

    };


/*
======================================================================================================================================================================================================

Global Shader Manager Instance and Usage Example

*/

    extern ShaderManager OBJGLUF_API g_ShaderManager;

#define SHADERMANAGER GLUF::g_ShaderManager

/*
Usage examples


//create the shader
ShaderPtr shad = SHADERMANAGER.CreateShader("shader.glsl", ST_VERTEX_SHADER);

*/

}

#endif //OBJGLUF_SHADER_H

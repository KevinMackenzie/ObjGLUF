#include "GLExtensions.h"

namespace GLUF {
/*

Helpful OpenGL Constants

*/

GLuint gGLVersionMajor = 0;
GLuint gGLVersionMinor = 0;
GLuint gGLVersion2Digit = 0;

//--------------------------------------------------------------------------------------
GLuint GetGLVersionMajor() {
    return gGLVersionMajor;
}

//--------------------------------------------------------------------------------------
GLuint GetGLVersionMinor() {
    return gGLVersionMinor;
}

//--------------------------------------------------------------------------------------
GLuint GetGLVersion2Digit() {
    return gGLVersion2Digit;
}

void GLExtensions::Init(const std::vector<std::string> &extensions) {
    //setup global openGL version
    const char *version = (const char *) glGetString(GL_VERSION);

    std::vector<std::string> vsVec;
    vsVec = SplitStr((const char *) version, '.');//TODO: global openGL version
    gGLVersionMajor = std::stoi(vsVec[0]);
    gGLVersionMinor = std::stoi(vsVec[1]);
    gGLVersion2Digit = gGLVersionMajor * 10 + gGLVersionMinor;

    //if the version is less than 2.1, do not allow to continue
    if ((gGLVersionMajor == 2 && gGLVersionMinor < 1) || gGLVersionMajor < 2) {
        GLUF_ERROR("OpenGL Version To Low!");
        return;
    }
    for (auto it : extensions) {
        mExtensionList.insert({it, true});
    }

    mCachedExtensionVector = extensions;
}

bool GLExtensions::HasExtension(const std::string &str) {
    //first look in the buffered list
    auto buffIt = mBufferedExtensionList.find(str);
    if (buffIt != mBufferedExtensionList.end())
        return buffIt->second;

    bool success = false;
    auto it = mExtensionList.find(str);
    if (it == mExtensionList.end()) {
        //success = false; //defaults to false
    } else {
        success = true;
    }

    //buffer this one
    mBufferedExtensionList.insert({str, success});

    //then return
    return success;
}
}

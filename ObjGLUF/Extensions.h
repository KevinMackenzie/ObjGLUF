#ifndef OBJGLUF_EXTENSIONS_H
#define OBJGLUF_EXTENSIONS_H

#define EXT_TO_TEXT(ext) #ext
#define ASSERT_EXTENTION(ext) if(!gExtensions.HasExtension(#ext)) GLUF_CRITICAL_EXCEPTION(UnsupportedExtensionException(#ext));

//for switch statements based on opengl extensions being present
#define SWITCH_GL_EXT(ext) if(gExtensions.HasExtension(#ext))

//for switch statements based on opengl version
#define SWITCH_GL_VERSION if(false){}
#define GL_VERSION_GREATER(val) else if(gGLVersion2Digit > val)
#define GL_VERSION_GREATER_EQUAL(val) else if(gGLVersion2Digit >= val)
#define GL_VERSION_LESS(val) else if(gGLVersion2Digit < val)
#define GL_VERSION_LESS_EQUAL(val) else if(gGLVersion2Digit <= val)

namespace GLUF {
/*
GLExtensions

    Data Members:
        'mExtensionList': a map of the extensions.  Maps are used for fast lookups O(log n), however only the key is needed
        'mBufferedExtensionList': a map of extensions that have been requested, and true or false depending on whether they are supported
        'mCachedExtensionVector': a variable whose sole purpose is act as a point of data to reference to when calling 'GetGLExtensions()'

    Note:
        even though 'std::map' may have a O(log n) lookup speed, keeping a separate map for extensions that are actually requested only slightly slows down
            randomly accessed extension support

*/
class GLExtensions
{
    std::map<std::string, bool> mExtensionList;

    std::map<std::string, bool> mBufferedExtensionList;

    std::vector<std::string> mCachedExtensionVector;

    //--------------------------------------------------------------------------------------
    void Init(const std::vector<std::string>& extensions)
    {
        for (auto it : extensions)
        {
            mExtensionList.insert({ it, true });
        }

        mCachedExtensionVector = extensions;
    }

    friend bool InitOpenGLExtensions();
public:

    //--------------------------------------------------------------------------------------
    operator const std::vector<std::string>&() const
    {
        return mCachedExtensionVector;
    }

    //--------------------------------------------------------------------------------------
    bool HasExtension(const std::string& str)
    {
        //first look in the buffered list
        auto buffIt = mBufferedExtensionList.find(str);
        if (buffIt != mBufferedExtensionList.end())
            return buffIt->second;

        bool success = false;
        auto it = mExtensionList.find(str);
        if (it == mExtensionList.end())
        {
            //success = false; //defaults to false
        }
        else
        {
            success = true;
        }

        //buffer this one
        mBufferedExtensionList.insert({ str, success });

        //then return
        return success;
    }
};

class UnsupportedExtensionException : public Exception
{
    const std::string mExt;
public:
    virtual const char* what() const noexcept override
    {
        std::stringstream ss;
        ss << "Unsupported Extension: \"" << mExt << "\"";
        return ss.str().c_str();
    }

    UnsupportedExtensionException(const std::string& ext) : mExt(ext)
    {
        EXCEPTION_CONSTRUCTOR_BODY
    }
};

extern GLExtensions gExtensions;
}

#endif //OBJGLUF_EXTENSIONS_H

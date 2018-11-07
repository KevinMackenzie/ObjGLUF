#ifndef OBJGLUF_TEXTURE_H
#define OBJGLUF_TEXTURE_H

#include "../ObjGLUF.h"
#include "../Exceptions.h"

namespace GLUF {
/*

Texture Utilities:

    Note:
        Not much is here, just the barebones, however more is planned for the future

*/

class TextureCreationException : public Exception {
public:
    virtual const char *what() const noexcept override {
        return "Failed to Create Texture!";
    }

    EXCEPTION_CONSTRUCTOR(TextureCreationException)
};

enum TextureFileFormat {
    TFF_DDS = 0,//we will ONLY support dds's, because they are flexible enough, AND have mipmaps
    TTF_DDS_CUBEMAP = 1
};


/*
LoadTextureFrom*

    Parameters:
        'filePath': path to file to open
        'format': texture file format to be loaded
        'data': raw data to load texture from

    Returns:
        OpenGL texture ID of the created texture

    Throws:
        'std::ios_base::failure': if filePath could not be found, or stream failed
        'TextureCreationException': if texture creation failed

    Note:
        All textures currently must be in ABGR format
        When using loaded textures, flip the UV coords of your meshes (the included mesh loaders do that for you) due to the DDS files loading upside-down
        If formats other than ABGR are supported, ABGR will likely be faster at loading

*/
OBJGLUF_API GLuint LoadTextureFromFile(const std::string &filePath, TextureFileFormat format);

// TODO: This is broken
OBJGLUF_API GLuint LoadTextureFromMemory(const std::vector<char> &data, TextureFileFormat format);

}

#endif //OBJGLUF_TEXTURE_H

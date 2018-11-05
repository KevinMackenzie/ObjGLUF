#include "Texture.h"



/*

Texture Utilities:

*/



#define FOURCC_DXT1 0x31545844 // Equivalent to "DXT1" in ASCII
#define FOURCC_DXT3 0x33545844 // Equivalent to "DXT3" in ASCII
#define FOURCC_DXT5 0x35545844 // Equivalent to "DXT5" in ASCII

/*
LoadTextureDDS

    Parameters:
        'rawData': raw data loaded from file to put into OpenGL

    Returns:
        OpenGL Id of texture created

    Throws:
        'std::invalid_argument': if data is too small, is not the correct file format, 

*/
//--------------------------------------------------------------------------------------
GLuint LoadTextureDDS(const std::vector<char>& rawData)
{
    //TODO support more compatibility, ie RGB, BGR, don't make it dependent on ABGR


    //verify size of header
    if (rawData.size() < 128)
    {
        GLUF_CRITICAL_EXCEPTION(std::invalid_argument("(LoadTextureDDS): Raw Data Too Small For Header!"));
    }

    //verify the type of file
    std::string filecode(rawData.begin(), rawData.begin() + 4);
    if (filecode != "DDS ")
    {
        GLUF_CRITICAL_EXCEPTION(std::invalid_argument("(LoadTextureDDS): Incorrect File Format!"));
    }

    //load the header

    //this is all the data I need, but I just need to load it properly to opengl (NOTE: this is all offset by 4 bytes because of the filecode)
    unsigned int height = *(unsigned int*)&(rawData[12]);
    unsigned int width = *(unsigned int*)&(rawData[16]);
    unsigned int linearSize = *(unsigned int*)&(rawData[20]);
    unsigned int mipMapCount = *(unsigned int*)&(rawData[28]);
    unsigned int flags = *(unsigned int*)&(rawData[80]);
    unsigned int fourCC = *(unsigned int*)&(rawData[84]);
    unsigned int RGBBitCount = *(unsigned int*)&(rawData[88]);
    unsigned int RBitMask = *(unsigned int*)&(rawData[92]);
    unsigned int GBitMask = *(unsigned int*)&(rawData[96]);
    unsigned int BBitMask = *(unsigned int*)&(rawData[100]);
    unsigned int ABitMask = *(unsigned int*)&(rawData[104]);

    //how big is it going to be including all mipmaps?
    unsigned int bufsize = mipMapCount > 1 ? linearSize * 2 : linearSize;

    //verify size of data again once header is loaded
                                                            //this does not work when the file is compressed
    /*if (rawData.size() < 128 + bufsize)
    {
        throw std::invalid_argument("(LoadTextureDDS): Raw Data Too Small!");
    }*/


    unsigned int components = (fourCC == FOURCC_DXT1) ? 3 : 4;
    unsigned int compressedFormat;
    switch (fourCC)
    {
    case FOURCC_DXT1:
        compressedFormat = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
        break;
    case FOURCC_DXT3:
        compressedFormat = GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
        break;
    case FOURCC_DXT5:
        compressedFormat = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
        break;
    default:
        compressedFormat = 0;//uncompressed
    }

    // Create one OpenGL texture
    GLuint textureID;
    glGenTextures(1, &textureID);

    //make sure OpenGL successfully created the texture before loading it
    if (textureID == 0)
        GLUF_CRITICAL_EXCEPTION(TextureCreationException());

    // "Bind" the newly created texture : all future texture functions will modify this texture
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, mipMapCount - 1);//REMEMBER it is max mip, NOT mip count
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_R, GL_RED);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_G, GL_GREEN);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_B, GL_BLUE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_A, GL_ALPHA);

    if (compressedFormat != 0)
    {
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

        unsigned int blockSize = (compressedFormat == GL_COMPRESSED_RGBA_S3TC_DXT1_EXT) ? 8 : 16;
        unsigned int offset = 128;// initial offset to compensate for header and file code

        /* load the mipmaps */
        for (unsigned int level = 0; level < mipMapCount/* && (width || height)*/; ++level)
        {
            unsigned int mipSize = ((width + 3) / 4)*((height + 3) / 4)*blockSize;
            glCompressedTexImage2D(GL_TEXTURE_2D, 
                level, compressedFormat, 
                width, height,
                0, mipSize, 
                rawData.data() + offset);

            offset += mipSize;
            width /= 2;
            height /= 2;

            // Deal with Non-Power-Of-Two textures
            if (width < 1) width = 1;
            if (height < 1) height = 1;

        }

    }
    else
    {
        unsigned int offset = 128;// initial offset to compensate for header and file code

        for (unsigned int level = 0; level < mipMapCount/* && (width || height)*/; ++level)
        {
            glTexImage2D(GL_TEXTURE_2D, 
                level, GL_RGBA, 
                width, height, 
                0, GL_RGBA, GL_UNSIGNED_BYTE, 
                rawData.data() + offset);

            unsigned int mipSize = (width * height * 4);
            offset += mipSize;
            width /= 2;
            height /= 2;

            // Deal with Non-Power-Of-Two textures
            if (width < 1) width = 1;
            if (height < 1) height = 1;
        }
    }

    glBindTexture(GL_TEXTURE_2D, 0);

    return textureID;
}

//--------------------------------------------------------------------------------------
GLuint LoadTextureCubemapDDS(const std::vector<char>& rawData)
{    
    //TODO support more compatibility, ie RGB, BGR, don't make it dependent on ABGR


    //verify size of header
    if (rawData.size() < 128)
    {
        GLUF_CRITICAL_EXCEPTION(std::invalid_argument("(LoadTextureDDS): Raw Data Too Small For Header!"));
    }

    //verify the type of file 
    std::string filecode(rawData.begin(), rawData.begin() + 4);
    if (filecode != "DDS ")
    {
        GLUF_CRITICAL_EXCEPTION(std::invalid_argument("(LoadTextureDDS): Incorrect File Format!"));
    }
    
    //load the header

    //this is all the data I need, but I just need to load it properly to opengl (NOTE: this is all offset by 4 bytes because of the filecode)
    unsigned int height = *(unsigned int*)&(rawData[12]);
    unsigned int width = *(unsigned int*)&(rawData[16]);
    unsigned int linearSize = *(unsigned int*)&(rawData[20]);
    unsigned int mipMapCount = *(unsigned int*)&(rawData[28]);
    unsigned int flags = *(unsigned int*)&(rawData[80]);
    unsigned int fourCC = *(unsigned int*)&(rawData[84]);
    unsigned int RGBBitCount = *(unsigned int*)&(rawData[88]);
    unsigned int RBitMask = *(unsigned int*)&(rawData[92]);
    unsigned int GBitMask = *(unsigned int*)&(rawData[96]);
    unsigned int BBitMask = *(unsigned int*)&(rawData[100]);
    unsigned int ABitMask = *(unsigned int*)&(rawData[104]);

    //how big is it going to be including all mipmaps? 
    unsigned int bufsize = mipMapCount > 1 ? linearSize * 2 : linearSize;

    //verify size of data again once header is loaded
    if (rawData.size() < 128 + bufsize)
    {
        GLUF_CRITICAL_EXCEPTION(std::invalid_argument("(LoadTextureDDS): Raw Data Too Small!"));
    }
    

    unsigned int components = (fourCC == FOURCC_DXT1) ? 3 : 4;
    unsigned int compressedFormat;
    switch (fourCC)
    {
    case FOURCC_DXT1:
        compressedFormat = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
        break;
    case FOURCC_DXT3:
        compressedFormat = GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
        break;
    case FOURCC_DXT5:
        compressedFormat = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
        break;
    default:
        compressedFormat = 0;//uncompressed
    }

    // Create one OpenGL texture
    GLuint textureID;
    glGenTextures(1, &textureID);

    //make sure OpenGL successfully created the texture before loading it
    if (textureID == 0)
        GLUF_CRITICAL_EXCEPTION(TextureCreationException());


    // "Bind" the newly created texture : all future texture functions will modify this texture
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_BASE_LEVEL, 0);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAX_LEVEL, 0);//REMEMBER it is max mip, NOT mip count
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_SWIZZLE_R, GL_RED);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_SWIZZLE_G, GL_GREEN);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_SWIZZLE_B, GL_BLUE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_SWIZZLE_A, GL_ALPHA);

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    //this method is not the prettyest, but it is the easiest to load
    if (compressedFormat != 0)
    {
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

        unsigned int blockSize = (compressedFormat == GL_COMPRESSED_RGBA_S3TC_DXT1_EXT) ? 8 : 16;
        unsigned int offset = 128;// initial offset to compensate for header and file code

        unsigned int pertexSize = width;

        unsigned int mipSize = ((pertexSize + 3) / 4)*((pertexSize + 3) / 4)*blockSize;
        glCompressedTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 
            0, compressedFormat, 
            pertexSize, pertexSize,
            0, mipSize, 
            rawData.data() + offset);
        offset += mipSize;
        glCompressedTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, 
            0, compressedFormat, 
            pertexSize, pertexSize,
            0, mipSize, 
            rawData.data() + offset);
        offset += mipSize;
        glCompressedTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 
            0, compressedFormat, 
            pertexSize, pertexSize,
            0, mipSize, 
            rawData.data() + offset);
        offset += mipSize;
        glCompressedTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, 
            0, compressedFormat, 
            pertexSize, pertexSize,
            0, mipSize, 
            rawData.data() + offset);
        offset += mipSize;
        glCompressedTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, 
            0, compressedFormat, 
            pertexSize, pertexSize,
            0, mipSize, 
            rawData.data() + offset);
        offset += mipSize;
        glCompressedTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 
            0, compressedFormat, 
            pertexSize, pertexSize,
            0, mipSize, 
            rawData.data() + offset);

    }
    else
    {
        unsigned int offset = 128;// initial offset to compensate for header and file code
        unsigned int pertexSize = width;
        unsigned int mipSize = (pertexSize * pertexSize * 4);

        glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 
            0, GL_RGBA, 
            pertexSize, pertexSize, 
            0, GL_RGBA, GL_UNSIGNED_BYTE, 
            (const GLvoid*)(rawData.data() + offset));
        offset += mipSize;
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, 
            0, GL_RGBA, 
            pertexSize, pertexSize, 
            0, GL_RGBA, GL_UNSIGNED_BYTE, 
            (const GLvoid*)(rawData.data() + offset));
        offset += mipSize;
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 
            0, GL_RGBA, 
            pertexSize, pertexSize, 
            0, GL_RGBA, GL_UNSIGNED_BYTE, 
            (const GLvoid*)(rawData.data() + offset));
        offset += mipSize;
        glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, 
            0, GL_RGBA, 
            pertexSize, pertexSize, 
            0, GL_RGBA, GL_UNSIGNED_BYTE, 
            (const GLvoid*)(rawData.data() + offset));
        offset += mipSize;
        glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, 
            0, GL_RGBA, 
            pertexSize, pertexSize, 
            0, GL_RGBA, GL_UNSIGNED_BYTE,
            (const GLvoid*)(rawData.data() + offset));
        offset += mipSize;
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 
            0, GL_RGBA, 
            pertexSize, pertexSize, 
            0, GL_RGBA, GL_UNSIGNED_BYTE, 
            (const GLvoid*)(rawData.data() + offset));
    }


    return textureID;
}



//--------------------------------------------------------------------------------------
GLuint LoadTextureFromFile(const std::string& filePath, TextureFileFormat format)
{
    std::vector<char> memory;

    try
    {
        LoadFileIntoMemory(filePath, memory);
    }
    catch (const std::ios_base::failure& e)
    {
        GLUF_ERROR_LONG("(LoadTextureFromFile): " << e.what());
    }

    GLuint texId;
    try
    {
        texId = LoadTextureFromMemory(memory, format);
    }
    catch (const TextureCreationException& e)
    {
        GLUF_ERROR_LONG("(LoadTextureFromFile): " << e.what());
        RETHROW;
    }

    return texId;
}


//--------------------------------------------------------------------------------------
GLuint LoadTextureFromMemory(const std::vector<char>& data, TextureFileFormat format)
{

    try
    {
        switch (format)
        {
        case TFF_DDS:
            return LoadTextureDDS(data);
        case TTF_DDS_CUBEMAP:
            return LoadTextureCubemapDDS(data);
        }
    }
    catch (const TextureCreationException& e)
    {
        GLUF_ERROR_LONG("(LoadTextureFromMemory): " << e.what());
        RETHROW;
    }
    return 0;
}

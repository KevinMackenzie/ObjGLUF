#ifndef OBJGLUF_ASSIMP_H
#define OBJGLUF_ASSIMP_H

#include <assimp/scene.h>
#include "../ObjGLUF.h"
#include "../VertexBuffer/VertexBuffer.h"

namespace GLUF
{

/*
=======================================================================================================================================================================================================
Utility Functions if Assimp is being used

    Note:
        this part of the library does use nakid pointers to match up with Assimp usage examples

*/

/*

Data Members for Assimp Loading

VertexAttribMap:
Data structure of all of the vertex attributes to load from the assimp scene

VertexAttribPair:
Pair of a vertex attribute, and the attribute info to go with it

*/
using VertexAttribMap = std::map<unsigned char, VertexAttribInfo>;
using VertexAttribPair = std::pair<unsigned char, VertexAttribInfo>;


/*
LoadVertexArrayFromScene

    Parameters:
        'scene': assimp 'aiScene': to load from
        'meshNum': which mesh number to load from the scene
        'inputs': which vertex attributes to load

    Returns:
        shared pointer to the loaded vertex array

    Throws:
        'std::invalid_argument': if 'meshNum' is higher than the number of meshes in 'scene'

        TOOD: SEE ASSIMP DOCUMENTATION TO SEE WHAT IT THROWS PLUS WHAT THIS MIGHT THROW

*/
OBJGLUF_API std::shared_ptr<VertexArray> LoadVertexArrayFromScene(const aiScene* scene, GLuint meshNum = 0);
OBJGLUF_API std::shared_ptr<VertexArray> LoadVertexArrayFromScene(const aiScene* scene, const VertexAttribMap& inputs, GLuint meshNum = 0);



/*
LoadVertexArraysFromScene

    Parameters:
        'scene': assimp 'aiScene': to load from
        'meshOffset': how many meshes in to start
        'numMeshes': how many meshes to load, starting at 'meshOffset'
        'inputs': which vertex attributes to load from each mesh; if inputs.size() == 1,
            then that set of attributes is used for each mesh

    Returns:
        array of shared pointers to the loaded vertex arrays

    Throws:
        TOOD: SEE ASSIMP DOCUMENTATION TO SEE WHAT IT THROWS PLUS WHAT THIS MIGHT THROW
*/
OBJGLUF_API std::vector<std::shared_ptr<VertexArray>> LoadVertexArraysFromScene(const aiScene* scene, GLuint meshOffset = 0, GLuint numMeshes = 1);
OBJGLUF_API std::vector<std::shared_ptr<VertexArray>> LoadVertexArraysFromScene(const aiScene* scene, const std::vector<const VertexAttribMap&>& inputs, GLuint meshOffset = 0, GLuint numMeshes = 1);


/*
=======================================================================================================================================================================================================
A Set of Guidelines for Attribute Locations (organized by number of vector elements)
 TODO: is this helpful?
*/
//2 elements

#define GLUF_VERTEX_ATTRIB_UV0      1
#define GLUF_VERTEX_ATTRIB_UV1      2
#define GLUF_VERTEX_ATTRIB_UV2      3
#define GLUF_VERTEX_ATTRIB_UV3      4
#define GLUF_VERTEX_ATTRIB_UV4      5
#define GLUF_VERTEX_ATTRIB_UV5      6
#define GLUF_VERTEX_ATTRIB_UV6      7
#define GLUF_VERTEX_ATTRIB_UV7      8
//3 elements

#define GLUF_VERTEX_ATTRIB_POSITION 9
#define GLUF_VERTEX_ATTRIB_NORMAL   10
#define GLUF_VERTEX_ATTRIB_TAN      11
#define GLUF_VERTEX_ATTRIB_BITAN    12
//4 elements

#define GLUF_VERTEX_ATTRIB_COLOR0   13
#define GLUF_VERTEX_ATTRIB_COLOR1   14
#define GLUF_VERTEX_ATTRIB_COLOR2   15
#define GLUF_VERTEX_ATTRIB_COLOR3   16
#define GLUF_VERTEX_ATTRIB_COLOR4   17
#define GLUF_VERTEX_ATTRIB_COLOR5   18
#define GLUF_VERTEX_ATTRIB_COLOR6   19
#define GLUF_VERTEX_ATTRIB_COLOR7   20


/*
=======================================================================================================================================================================================================
Premade Attribute Info's which comply with Assimp capabilities, but are not exclusive to them

*/
extern const VertexAttribInfo OBJGLUF_API g_attribPOS;
extern const VertexAttribInfo OBJGLUF_API g_attribNORM;
extern const VertexAttribInfo OBJGLUF_API g_attribUV0;
extern const VertexAttribInfo OBJGLUF_API g_attribUV1;
extern const VertexAttribInfo OBJGLUF_API g_attribUV2;
extern const VertexAttribInfo OBJGLUF_API g_attribUV3;
extern const VertexAttribInfo OBJGLUF_API g_attribUV4;
extern const VertexAttribInfo OBJGLUF_API g_attribUV5;
extern const VertexAttribInfo OBJGLUF_API g_attribUV6;
extern const VertexAttribInfo OBJGLUF_API g_attribUV7;
extern const VertexAttribInfo OBJGLUF_API g_attribCOLOR0;
extern const VertexAttribInfo OBJGLUF_API g_attribCOLOR1;
extern const VertexAttribInfo OBJGLUF_API g_attribCOLOR2;
extern const VertexAttribInfo OBJGLUF_API g_attribCOLOR3;
extern const VertexAttribInfo OBJGLUF_API g_attribCOLOR4;
extern const VertexAttribInfo OBJGLUF_API g_attribCOLOR5;
extern const VertexAttribInfo OBJGLUF_API g_attribCOLOR6;
extern const VertexAttribInfo OBJGLUF_API g_attribCOLOR7;
extern const VertexAttribInfo OBJGLUF_API g_attribTAN;
extern const VertexAttribInfo OBJGLUF_API g_attribBITAN;

extern std::map<unsigned char, VertexAttribInfo> OBJGLUF_API g_stdAttrib;

#define VertAttrib(location, bytes, count, type) {bytes, count, location, type}

/*
=======================================================================================================================================================================================================
Global methods for converting assimp and glm data types

*/

inline glm::vec2 AssimpToGlm(aiVector2D v)
{
    return glm::vec2(v.x, v.y);
}
inline glm::vec2 AssimpToGlm3_2(aiVector3D v)
{
    return glm::vec2(v.x, v.y);
}
inline glm::vec2* AssimpToGlm(aiVector2D* v, GLuint count)
{
    glm::vec2* ret = new glm::vec2[count];
    for (GLuint i = 0; i < count; ++i)
        ret[i] = AssimpToGlm(v[i]);
    return ret;
}
inline glm::vec2* AssimpToGlm3_2(aiVector3D* v, GLuint count)
{
    glm::vec2* ret = new glm::vec2[count];
    for (GLuint i = 0; i < count; ++i)
        ret[i] = AssimpToGlm3_2(v[i]);
    return ret;
}



inline glm::vec3 AssimpToGlm(aiVector3D v)
{
    return glm::vec3(v.x, v.y, v.z);
}
inline glm::vec3* AssimpToGlm(aiVector3D* v, GLuint count)
{
    glm::vec3* ret = new glm::vec3[count];
    for (GLuint i = 0; i < count; ++i)
        ret[i] = AssimpToGlm(v[i]);
    return ret;
}
inline glm::vec3 AssimpToGlm(aiColor3D v)
{
    return glm::vec3(v.r, v.g, v.b);
}

inline glm::vec4 AssimpToGlm(aiColor4D v)
{
    return glm::vec4(v.r, v.g, v.b, v.a);
}

inline GLUF::Color AssimpToGlm4_3u8(aiColor4D col)
{
    return GLUF::Color(col.r * 255, col.g * 255, col.b * 255, col.a * 255);
}

}

#endif //OBJGLUF_ASSIMP_H

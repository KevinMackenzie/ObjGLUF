#ifndef OBJGLUF_VERTEXBUFFER_H
#define OBJGLUF_VERTEXBUFFER_H

#include "../ObjGLUF.h"

#include "GLVector.h"

namespace GLUF {


struct MeshBarebones {
    Vec3Array mVertices;
    IndexArray mIndices;
};


/*
VertexAttribInfo

    Member Data:
        'mBytesPerElement': the number of bytes per element of the vertex
        'mElementsPerValue': the number of elements per vector value
        'mVertexAttribLocation': the location of the vertex attribute
        'mType': primitive type of the data
*/

struct OBJGLUF_API VertexAttribInfo {
    unsigned short mBytesPerElement;//int would be 4
    unsigned short mElementsPerValue;//vec4 would be 4
    AttribLoc mVertexAttribLocation;
    GLenum mType;//float would be GL_FLOAT
    GLuint mOffset;//will be 0 in SoA
};

/*

Vertex Array Exceptions


*/

class MakeVOAException : public Exception {
public:
    virtual const char *what() const noexcept override {
        return "VAO Creation Failed!";
    }

    EXCEPTION_CONSTRUCTOR(MakeVOAException)
};

class InvalidSoABufferLenException : public Exception {
public:
    virtual const char *what() const noexcept override {
        return "Buffer Passed Has Length Inconsistent With the Vertex Attributes!";
    }

    EXCEPTION_CONSTRUCTOR(InvalidSoABufferLenException)
};

class MakeBufferException : public Exception {
public:
    virtual const char *what() const noexcept override {
        return "Buffer Creation Failed!";
    }

    EXCEPTION_CONSTRUCTOR(MakeBufferException)
};

class InvalidAttrubuteLocationException : public Exception {
public:
    virtual const char *what() const noexcept override {
        return "Attribute Location Not Found in This Buffer!";
    }

    EXCEPTION_CONSTRUCTOR(InvalidAttrubuteLocationException)
};

/*
VertexArrayBase

    Member Data:
        'mVertexArrayId': The OpenGL assigned id for this vertex array
        'mVertexCount': the number of vertices in the array
        'mUsageType': the OpenGL Usage type (i.e. GL_READ_WRITE)
        'mPrimitiveType': the OpenGL primitive type (i.e. GL_TRIANGLES)
        'mAttribInfos': a map of attribute locations
        'mIndexBuffer': the location of the single index array
        'mRangedIndexBuffer': the location of a dynamic buffer holding a range of indices
        'mIndexCount': the number of indices (number of faces * number of vertices per primitive)
        'mTempVAOId': the temperary id of the VAO; saved before binding this VAO

*/

class OBJGLUF_API VertexArrayBase {
protected:
    GLuint mVertexArrayId = 0;
    GLuint mVertexCount = 0;


    GLenum mUsageType;
    GLenum mPrimitiveType;
    std::map<AttribLoc, VertexAttribInfo> mAttribInfos;

    GLuint mIndexBuffer = 0;
    GLuint mIndexCount = 0;

    GLuint mTempVAOId = 0;

    /*

    Internal Methods:

        RefreshDataBufferAttribute:
            -reassign the OpenGL buffer attributes to VAO

        GetAttribInfoFromLoc:
            -simple map lookup for location
            -throws 'std::invalid_argument' if loc does not exist

        BufferIndicesBase:
            -similer code for each of the 'BufferIndices' functions

    */
    virtual void RefreshDataBufferAttribute() noexcept = 0;
    const VertexAttribInfo &GetAttribInfoFromLoc(AttribLoc loc) const;
    void BufferIndicesBase(GLuint indexCount, const GLvoid *data) noexcept;


    //disallow copy constructor and assignment operator
    VertexArrayBase(const VertexArrayBase &other) = delete;
    VertexArrayBase &operator=(const VertexArrayBase &other) = delete;
public:
    /*

    Constructor:

        Parameters:
            'index': does this VAO use indices, or just vertices
            'primType': which primative type does it use
            'buffUsage': which buffer useage type

        Throws:
            'MakeVAOException': if construction fails
            'MakeBufferException': if index buffer fails to be created (This will not be thrown if 'index' = false)
    */

    VertexArrayBase(GLenum primType = GL_TRIANGLES, GLenum buffUsage = GL_STATIC_DRAW, bool index = true);


    ~VertexArrayBase();

    /*

    Move Copy Constructor and Move Assignment Operator

        Throws:
            May Throw something in Map Copy Constructor
    */

    VertexArrayBase(VertexArrayBase &&other);
    VertexArrayBase &operator=(VertexArrayBase &&other);

    /*
    Add/RemoveVertexAttrib

        Parameters:
            'info': data structure containing the attribute information
            'loc': OpenGL location of the attribute to remove

        Throws:
            'std::invalid_argument': if loc does not exist or if 'mBytesPerElement == 0' of if 'mElementsPerValue == 0'

    */
    //this would be used to add color, or normals, or texcoords, or even positions.  NOTE: this also deletes ALL DATA in this buffer
    virtual void AddVertexAttrib(const VertexAttribInfo &info);
    virtual void RemoveVertexAttrib(AttribLoc loc);

    /*
    BindVertexArray

        Binds this vertex array for modifications.

        Throws:
            no-throw guarantee

        Preconditions:
            Unkown VAO is bound

        Postconditions
            This VAO is bound
    */
    void BindVertexArray() noexcept;

    /*
    UnBindVertexArray

        Binds old vertex array back;

        Note:
            Undefined results if this is called before 'BindVertexArray'

        Throws:
            no-throw guarantee

        Preconditions:
            This VAO is bound

        Postconditions
            Old Unknown VAO is bound
    */
    void UnBindVertexArray() noexcept;


    /*
    Draw*

        -These functions wrap the whole drawing process into one function call;
        -BindVertexArray is automatically called

        Parameters:
            'start': starting index of ranged draw
            'count': number of indices relative to 'start' to draw
            'instances': number of times to draw the object in instanced draw

        Throws:
            no-throw guarantee

        Draw:
            Draws the vertex array with the currently bound shader

        DrawRange:
            Draws the range of indices from 'start' to 'start' + 'count'

        DrawInstanced:
            Draws 'instances' number of the object.  NOTE: this will draw the object EXACTLY the same, so
                this is not very useful without a special instanced shader program
    */
    void Draw() noexcept;
    void DrawRange(GLuint start, GLuint count) noexcept;
    void DrawInstanced(GLuint instances) noexcept;


    /*
    BufferIndices

        Parameters:
            'indices': array of indices to be buffered

        Throws:
            no-throw guarantee

        Note:
            this could be a template, but is intentionally not to ensure the user
                is entering reasonible data to give predictible results
    */

    void BufferIndices(const std::vector<GLuint> &indices) noexcept;
    void BufferIndices(const std::vector<glm::u32vec2> &indices) noexcept;
    void BufferIndices(const std::vector<glm::u32vec3> &indices) noexcept;
    void BufferIndices(const std::vector<glm::u32vec4> &indices) noexcept;
    //void BufferFaces(GLuint* indices, GLuint FaceCount);

    /*
    Enable/DisableVertexAttributes

        -Iterates through all vertex attributes and enables them.
        -This is mostly for internal use, but is exposed as an interface to allow
            for flexibility

        Throws:
            no-throw guarantee

        Preconditions:
            VAO is currently bound

        Note:
            Calling these functions before calling 'BindVertexArray' is not harmful within
                this API, however it will be harmful using raw OpenGL VAO objects.
            Calling these functions before calling 'BindVertexArray' will not be useful

    */

    virtual void EnableVertexAttributes() const noexcept = 0;
    virtual void DisableVertexAttributes() const noexcept = 0;


    /*
    Disable/EnableVertexAttribute

        NOTE:
            THIS IS NOT THE SAME AS 'Disable/EnableVertexAttribute'
            This is designed to be able to disable specific attributes of a vertex

        Parameters:
            'loc': the attribute location to disable

        Throws:
            no-throw guarantee

    */
    //void EnableVertexAttribute(AttribLoc loc) noexcept;
    //void DisableVertexAttribute(AttribLoc loc) noexcept;
};



/*

VertexArrayAoS:

    An Array of Structures approach to OpenGL buffers

    Note:
        All data structures must be on 4 byte bounderies

    Data Members:
        'mDataBuffer': the single buffer that holds the array
        'mCopyBuffer': a buffer to hold old data when resizeing 'mDataBuffer'

*/

class OBJGLUF_API VertexArrayAoS : public VertexArrayBase {


    /*
    RemoveVertexAttrib

        "RemoveVertexAttrib" is disabled, because removing an attribute will mess up the offset and strides of the buffer

    */
    //this would be used to add color, or normals, or texcoords, or even positions.  NOTE: this also deletes ALL DATA in this buffer
    virtual void RemoveVertexAttrib(AttribLoc loc) final {}

protected:
    GLuint mDataBuffer = 0;
    GLuint mCopyBuffer = 0;


    //see 'VertexArrayBase' Docs
    virtual void RefreshDataBufferAttribute() noexcept;

public:

    /*

    Constructor:

        Parameters:
            'index': does this VAO use indices, or just vertices
            'primType': which primative type does it use
            'buffUsage': which buffer useage type

        Throws:
            'MakeVAOException': if construction fails
            'MakeBufferException': if buffer creation specifically fails
    */
    VertexArrayAoS(GLenum primType = GL_TRIANGLES, GLenum buffUsage = GL_STATIC_DRAW, bool indexed = true);

    ~VertexArrayAoS();


    /*

    Move Copy Constructor and Move Assignment Operator

        Throws:
            May Throw something in Map Copy Constructor

    */

    VertexArrayAoS(VertexArrayAoS &&other);
    VertexArrayAoS &operator=(VertexArrayAoS &&other);


    /*
    GetVertexSize

        Returns:
            Size of each vertex, including possible 4 byte padding for each element in the array

        Throws:
            no-throw guarantee
    */
    GLuint GetVertexSize() const noexcept;



    /*
    Add/RemoveVertexAttrib

        Parameters:
            'info': data structure containing the attribute information
            'offset': used as offset parameter when 'info' is a const

        Throws:
            'std::invalid_argument': if 'mBytesPerElement == 0' or if 'mElementsPerValue == 0'

    */
    //this would be used to add color, or normals, or texcoords, or even positions.  NOTE: this also deletes ALL DATA in this buffer
    virtual void AddVertexAttrib(const VertexAttribInfo &info);
    virtual void AddVertexAttrib(const VertexAttribInfo &info, GLuint offset);


    /*
    BufferData

        -To add whole vertex arrays to the VAO. Truncates old data

        Parameters:
            'data': the array of structures
            'T': data structure derived from 'VertexStruct'

        Throws:
            'std::invalid_argument' if derived 'VertexStruct' is the wrong size or is not derived from 'VertexStruct', but this does NOT enforce the layout

    */
    template<typename T>
    void BufferData(const GLVector<T> &data);


    /*
    ResizeBuffer

        Parameters:
            'numVertices': the number of vertices the array will be
            'keepOldData': if the old data in the buffer should be kept
            'newOldDataOffset': the offset of the new data in VERTICES within the new buffer

        Throws:
            no-throw guarantee
    */
    void ResizeBuffer(GLsizei numVertices, bool keepOldData = false, GLsizei newOldDataOffset = 0);


    /*
    BufferSubData

        -To modify vertices

        Parameters:
            'vertexLocations': overwrite these vertices (if size == 1, then that will be the first vertex, then will overwrite sequentially)
            'data': List of Data which derives from AoSStruct
            'isSorted': set this to 'true' if the vertex locations are in order (not necessessarily contiguous though)

        Template Parameters:
            'T': vertex type, derived from VertexStruct, which CANNOT contain any pointers, it must only contain primative types, or structures of primative types

        Throws:
            'std::invalid_argument' if derived 'VertexStruct' is the wrong size, but this does NOT enforce the layout
            'std::invalid_argument' if size of data array is not the size of the vertex locations, and the size of the vertex locations is not equal to 1
            'std::invalid_argument' if 'T' is not the correct base
            'std::invalid_argument' if 'vertexLocations' contains any duplicates

        Note:
            As efficient as possible, but still not fast.  If this is needed frequently, consider using 'VertexArraySoA'

            ****TODO: Possibly one that accepts an array of vertex locations along with chunk sizes****
    */
    template<typename T>
    void BufferSubData(GLVector<T> data, std::vector<GLuint> vertexLocations, bool isSorted = false);

    /*
    Enable/DisableVertexAttrib

        See 'VertexArrayBase' for doc's

    */
    virtual void EnableVertexAttributes() const noexcept override;
    virtual void DisableVertexAttributes() const noexcept override;

};


/*
VertexArraySoA

    Structure of Arrays approach to OpenGL Buffers

    Data Members:
        'mDataBuffers': one OpenGL buffer per vertex attribute

*/
class OBJGLUF_API VertexArraySoA : public VertexArrayBase {
protected:
    //       Attrib Location, buffer location
    std::map<AttribLoc, GLuint> mDataBuffers;

    //see parent docs
    virtual void RefreshDataBufferAttribute() noexcept;

    /*
    GetBufferIdFromAttribLoc

        Parameters:
            'loc': location of the attribute in the shader

        Returns:
            buffer Id which contains data to put in this attrubute

        Throws:
            'InvalidAttrubuteLocationException': if no attrubte has location 'loc'
    */
    GLuint GetBufferIdFromAttribLoc(AttribLoc loc) const;

public:
    /*

    Constructor:

        Parameters:
            'index': does this VAO use indices, or just vertices
            'primType': which primative type does it use
            'buffUsage': which buffer useage type

        Throws:
            'MakeVAOException': if construction fails

    */
    VertexArraySoA(GLenum primType = GL_TRIANGLES, GLenum buffUsage = GL_STATIC_DRAW, bool indexed = true);

    ~VertexArraySoA();

    /*

    Move Copy Constructor and Move Assignment Operator

        Throws:
            May Throw something in Map Copy Constructor

    */

    VertexArraySoA(VertexArraySoA &&other);
    VertexArraySoA &operator=(VertexArraySoA &&other);


    /*
    GetBarebonesMesh

        Parameters:
            'inData': structure to be written to with OpenGL Buffer Data; all data already inside will be deleted

        Throws:
            'InvalidAttrubuteLocationException': if this SoA does not have a position or index buffer

    */

    void GetBarebonesMesh(MeshBarebones &inData);

    /*
    BufferData

        Parameters:
            'loc': the attribute location for the buffer
            'data': the data to buffer

        Throws:
            'InvalidSoABufferLenException': if data.size() != mVertexCount
            'InvalidAttrubuteLocationException': if loc does not exist in this buffer
    */
    template<typename T>
    void BufferData(AttribLoc loc, const std::vector<T> &data);

    /*
    BufferSubData

        Parameters:
            'loc': the attribute location for the buffer
            'vertexOffsetCount': number of vertices to offset by when writing
            'data': the data to buffer

        Throws:
            'InvalidAttrubuteLocationException': if loc does not exist in this buffer
    */
    template<typename T>
    void BufferSubData(AttribLoc loc, GLuint vertexOffsetCount, const std::vector<T> &data);

    //this would be used to add color, or normals, or texcoords, or even positions.  NOTE: this also deletes ALL DATA in this buffer

    /*
    Add/RemoveVertexAttrib

        Parameters:
            'info': information for the vertex attribute to add
            'loc': location of the vertex attribute to remove

        Throws:
            no-throw guarantee

        Note:
            if 'info.mVertexAttribLocation' already exists, the new
                attribute simply is not added
            if 'loc' does not exist, nothing is deleted
    */
    virtual void AddVertexAttrib(const VertexAttribInfo &info) noexcept;
    virtual void RemoveVertexAttrib(AttribLoc loc) noexcept;


    /*
    Enable/DisableVertexAttrib

    See 'VertexArrayBase' for doc's

    */
    virtual void EnableVertexAttributes() const noexcept override;
    virtual void DisableVertexAttributes() const noexcept override;
};


/*

Vertex Array Object Aliases

*/

using VertexArraySoAPtr = std::shared_ptr<VertexArraySoA>;
using VertexArrayAoSPtr = std::shared_ptr<VertexArrayAoS>;
using VertexArray       = VertexArrayAoS;
using VertexArrayPtr    = std::shared_ptr<VertexArray>;

}
#endif //OBJGLUF_VERTEXBUFFER_H

#include "VertexBuffer.h"
#include "../GLExtensions.h"
// TODO: Remove this dependency
#include "../Ext/Assimp.h"
#include "../ObjGLUF.h"

namespace GLUF {

/*

Buffer Utilities

*/

//--------------------------------------------------------------------------------------
const VertexAttribInfo &VertexArrayBase::GetAttribInfoFromLoc(AttribLoc loc) const {
    auto val = mAttribInfos.find(loc);
    if (val == mAttribInfos.end())
        GLUF_NON_CRITICAL_EXCEPTION(std::invalid_argument("\"loc\" not found in attribute list"));

    return val->second;
}

//--------------------------------------------------------------------------------------
VertexArrayBase::VertexArrayBase(GLenum PrimType, GLenum buffUsage, bool index) : mUsageType(buffUsage),
                                                                                  mPrimitiveType(PrimType) {
    SWITCH_GL_VERSION
    GL_VERSION_GREATER_EQUAL(30) {
        glGenVertexArrayBindVertexArray(&mVertexArrayId);

        if (mVertexArrayId == 0)
            GLUF_CRITICAL_EXCEPTION(MakeVOAException());

    }

    if (index) {
        glGenBuffers(1, &mIndexBuffer);
        if (mIndexBuffer == 0) {
            GLUF_ERROR("Failed to create index buffer!");
            GLUF_CRITICAL_EXCEPTION(MakeBufferException());
        }

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIndexBuffer);

    }
}

//--------------------------------------------------------------------------------------
VertexArrayBase::~VertexArrayBase() noexcept {
    BindVertexArray();

    glDeleteBuffers(1, &mIndexBuffer);

    SWITCH_GL_VERSION
    GL_VERSION_GREATER_EQUAL(30)
    glDeleteVertexArrays(1, &mVertexArrayId);

    UnBindVertexArray();
}

//--------------------------------------------------------------------------------------
VertexArrayBase::VertexArrayBase(VertexArrayBase &&other) {
    //set this class
    mVertexArrayId = other.mVertexArrayId;
    mVertexCount = other.mVertexCount;
    mUsageType = other.mUsageType;
    mPrimitiveType = other.mPrimitiveType;
    mAttribInfos = std::move(other.mAttribInfos);
    mIndexBuffer = other.mIndexBuffer;
    mIndexCount = other.mIndexCount;
    mTempVAOId = other.mTempVAOId;//likely will be 0 anyways


    //reset other class
    other.mVertexArrayId = 0;
    other.mVertexCount = 0;
    other.mUsageType = GL_STATIC_DRAW;
    other.mPrimitiveType = GL_TRIANGLES;
    //other.mAttribInfos.clear();
    other.mIndexBuffer = 0;
    other.mIndexCount = 0;
    other.mTempVAOId = 0;//likely will be 0 anyways
}

//--------------------------------------------------------------------------------------
VertexArrayBase &VertexArrayBase::operator=(VertexArrayBase &&other) {
    //set this class
    mVertexArrayId = other.mVertexArrayId;
    mVertexCount = other.mVertexCount;
    mUsageType = other.mUsageType;
    mPrimitiveType = other.mPrimitiveType;
    mAttribInfos = std::move(other.mAttribInfos);
    mIndexBuffer = other.mIndexBuffer;
    mIndexCount = other.mIndexCount;
    mTempVAOId = other.mTempVAOId;//likely will be 0 anyways


    //reset other class
    other.mVertexArrayId = 0;
    other.mVertexCount = 0;
    other.mUsageType = GL_STATIC_DRAW;
    other.mPrimitiveType = GL_TRIANGLES;
    //other.mAttribInfos.clear();
    other.mIndexBuffer = 0;
    other.mIndexCount = 0;
    other.mTempVAOId = 0;//likely will be 0 anyways

    return *this;
}

//--------------------------------------------------------------------------------------
void VertexArrayBase::AddVertexAttrib(const VertexAttribInfo &info) {
    //don't do null checks, because BindVertexArray already does them for us
    BindVertexArray();

    //make sure the attribute contains valid data
    if (info.mBytesPerElement == 0 || info.mElementsPerValue == 0)
        GLUF_CRITICAL_EXCEPTION(std::invalid_argument("Invalid Data in Vertex Attribute Info!"));

    mAttribInfos.insert(std::pair<AttribLoc, VertexAttribInfo>(info.mVertexAttribLocation, info));

    //this is a bit inefficient to refresh every time an attribute is added, but this should not be significant
    RefreshDataBufferAttribute();

    //enable the new attribute
    glEnableVertexAttribArray(info.mVertexAttribLocation);

    UnBindVertexArray();
}

//--------------------------------------------------------------------------------------
void VertexArrayBase::RemoveVertexAttrib(AttribLoc loc) {
    auto val = mAttribInfos.find(loc);
    if (val == mAttribInfos.end())
        GLUF_NON_CRITICAL_EXCEPTION(std::invalid_argument("\"loc\" not found in attribute list"));

    mAttribInfos.erase(val);

    RefreshDataBufferAttribute();
}

//--------------------------------------------------------------------------------------
void VertexArrayBase::BindVertexArray() noexcept {
    SWITCH_GL_VERSION
    GL_VERSION_GREATER_EQUAL(30) {
        //store the old one before binding this one
        GLint tmpVAOId = 0;
        glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &tmpVAOId);
        mTempVAOId = static_cast<GLuint>(tmpVAOId);

        glBindVertexArray(mVertexArrayId);
    }
}

//--------------------------------------------------------------------------------------
void VertexArrayBase::UnBindVertexArray() noexcept {
    SWITCH_GL_VERSION
    GL_VERSION_GREATER_EQUAL(30) {
        glBindVertexArray(mTempVAOId);
        mTempVAOId = 0;
    }
}

//--------------------------------------------------------------------------------------
void VertexArrayBase::Draw() noexcept {
    BindVertexArray();

    SWITCH_GL_VERSION
    GL_VERSION_LESS(30) {
        EnableVertexAttributes();//must disable and re-enable every time with openGL less than 3.0
    }

    if (mIndexBuffer != 0) {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIndexBuffer);
        glDrawElements(mPrimitiveType, mIndexCount, GL_UNSIGNED_INT, nullptr);
    } else {
        glDrawArrays(mPrimitiveType, 0, mVertexCount);
    }

    SWITCH_GL_VERSION
    GL_VERSION_LESS(30) {
        DisableVertexAttributes();
    }

    UnBindVertexArray();
}

//--------------------------------------------------------------------------------------
void VertexArrayBase::DrawRange(GLuint start, GLuint count) noexcept {
    BindVertexArray();

    SWITCH_GL_VERSION
    GL_VERSION_LESS(30) {
        EnableVertexAttributes();//must disable and re-enable every time with openGL less than 3.0
    }

    if (mIndexBuffer != 0) {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIndexBuffer);
        glDrawElements(mPrimitiveType, count, GL_UNSIGNED_INT, static_cast<GLuint *>(nullptr) + start);
    } else {
        glDrawArrays(mPrimitiveType, (start < 0 || start > mVertexCount) ? 0 : start,
                     (count > mVertexCount) ? mVertexCount : count);
    }

    SWITCH_GL_VERSION
    GL_VERSION_LESS(30) {
        DisableVertexAttributes();
    }

    UnBindVertexArray();
}

//--------------------------------------------------------------------------------------
void VertexArrayBase::DrawInstanced(GLuint instances) noexcept {
    BindVertexArray();

    SWITCH_GL_VERSION
    GL_VERSION_LESS(30) {
        EnableVertexAttributes();//must disable and re-enable every time with openGL less than 3.0
    }

    if (mIndexBuffer != 0) {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIndexBuffer);
        glDrawElementsInstanced(mPrimitiveType, mIndexCount, GL_UNSIGNED_INT, nullptr, instances);
    } else {
        glDrawArraysInstanced(mPrimitiveType, 0, mVertexCount, instances);
    }

    SWITCH_GL_VERSION
    GL_VERSION_LESS(30) {
        DisableVertexAttributes();
    }

    UnBindVertexArray();
}

//helper function
//--------------------------------------------------------------------------------------
void VertexArrayBase::BufferIndicesBase(GLuint indexCount, const GLvoid *data) noexcept {
    BindVertexArray();
    mIndexCount = indexCount;
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIndexBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * mIndexCount, data, mUsageType);

    UnBindVertexArray();
}

//--------------------------------------------------------------------------------------
void VertexArrayBase::BufferIndices(const std::vector<GLuint> &indices) noexcept {
    BufferIndicesBase(indices.size(), &indices[0]);
}

//--------------------------------------------------------------------------------------
void VertexArrayBase::BufferIndices(const std::vector<glm::u32vec2> &indices) noexcept {
    BufferIndicesBase(indices.size() * 2, &indices[0]);
}

//--------------------------------------------------------------------------------------
void VertexArrayBase::BufferIndices(const std::vector<glm::u32vec3> &indices) noexcept {
    BufferIndicesBase(indices.size() * 3, &indices[0]);
}

//--------------------------------------------------------------------------------------
void VertexArrayBase::BufferIndices(const std::vector<glm::u32vec4> &indices) noexcept {
    BufferIndicesBase(indices.size() * 4, &indices[0]);
}

//--------------------------------------------------------------------------------------
/*void VertexArrayBase::EnableVertexAttribute(AttribLoc loc)
{
    auto it = mDisabledAttribInfos.find(loc);
    if (it == mDisabledAttribInfos.end())
        return;

    mAttribInfos[loc] = it->second;

    mDisabledAttribInfos.erase(it);
}

//--------------------------------------------------------------------------------------
void VertexArrayBase::DisableVertexAttribute(AttribLoc loc)
{
    auto it = mAttribInfos.find(loc);
    if (it == mAttribInfos.end())
        return;

    mDisabledAttribInfos[loc] = it->second;

    mAttribInfos.erase(it);
}*/



/*
RoundNearestMultiple

    Parameters:
        'num': Number to round
        'multiple': which multiple to round to

    Throws:
        'std::invalid_argument' if multiple == 0
       
    Note:
        This always rounds up to the next multiple
        Usage is to make sure memory is within certain boundaries (i.e. 4 byte boundaries)

*/
//--------------------------------------------------------------------------------------
int RoundNearestMultiple(unsigned int num, unsigned int multiple) {
    if (multiple == 0) {
        GLUF_NON_CRITICAL_EXCEPTION(std::invalid_argument("Multiple Cannot Be 0"));
        return 0;//if the multiple is 0, still needs to return something in release mode
    }

    unsigned int nearestMultiple = 0;

    //loop up to the 
    for (unsigned int i = 0; i < num; i += multiple) {
        nearestMultiple = i;
    }

    nearestMultiple += multiple;
    return nearestMultiple;
}

//--------------------------------------------------------------------------------------
void VertexArrayAoS::RefreshDataBufferAttribute() noexcept {
    SWITCH_GL_VERSION
    GL_VERSION_GREATER_EQUAL(30)//this is done at draw time in opengl less than 3.0
    {

        BindVertexArray();

        glBindBuffer(GL_ARRAY_BUFFER, mDataBuffer);

        GLuint stride = GetVertexSize();
        for (auto it : mAttribInfos) {
            //the last parameter might be wrong
            glVertexAttribPointer(it.second.mVertexAttribLocation, it.second.mElementsPerValue, it.second.mType,
                                  GL_FALSE, stride,
                                  reinterpret_cast<GLvoid *>(static_cast<uintptr_t>(it.second.mOffset)));
        }

        UnBindVertexArray();
    }
}

//--------------------------------------------------------------------------------------
VertexArrayAoS::VertexArrayAoS(GLenum PrimType, GLenum buffUsage, bool indexed) : VertexArrayBase(PrimType, buffUsage,
                                                                                                  indexed) {
    //the VAO is already bound

    glGenBuffers(1, &mDataBuffer);
    glGenBuffers(1, &mCopyBuffer);

    if (mDataBuffer == 0 || mCopyBuffer == 0)
        GLUF_CRITICAL_EXCEPTION(MakeBufferException());
}

//--------------------------------------------------------------------------------------
VertexArrayAoS::~VertexArrayAoS() noexcept {
    BindVertexArray();

    glDeleteBuffers(1, &mDataBuffer);

    UnBindVertexArray();
}

//--------------------------------------------------------------------------------------
VertexArrayAoS::VertexArrayAoS(VertexArrayAoS &&other) : VertexArrayBase(std::move(other)) {
    mDataBuffer = other.mDataBuffer;

    other.mDataBuffer = 0;
}

//--------------------------------------------------------------------------------------
VertexArrayAoS &VertexArrayAoS::operator=(VertexArrayAoS &&other) {
    //since there is no possibility for user error, not catching dynamic cast here is A-OK

    VertexArrayBase *thisParentPtr = dynamic_cast<VertexArrayBase *>(this);
    *thisParentPtr = std::move(other);

    mDataBuffer = other.mDataBuffer;
    other.mDataBuffer = 0;

    return *this;
}

//--------------------------------------------------------------------------------------
GLuint VertexArrayAoS::GetVertexSize() const noexcept {
    GLuint stride = 0;
    //WOW: this before was allocating memory to find the size of the memory, then didn't even delete it
    for (auto it : mAttribInfos) {
        //round to the 4 bytes bounderies
        stride += it.second.mElementsPerValue * RoundNearestMultiple(it.second.mBytesPerElement, 4);
    }

    return stride;
}

//--------------------------------------------------------------------------------------
void VertexArrayAoS::AddVertexAttrib(const VertexAttribInfo &info) {
    //don't do null checks, because BindVertexArray already does them for us
    BindVertexArray();

    //make sure the attribute contains valid data
    if (info.mBytesPerElement == 0 || info.mElementsPerValue == 0)
        GLUF_CRITICAL_EXCEPTION(std::invalid_argument("Invalid Data in Vertex Attribute Info!"));

    mAttribInfos.insert(std::pair<AttribLoc, VertexAttribInfo>(info.mVertexAttribLocation, info));

    //this is a bit inefficient to refresh every time an attribute is added, but this should not be significant
    RefreshDataBufferAttribute();

    //enable the new attribute
    glEnableVertexAttribArray(info.mVertexAttribLocation);//not harmful in opengl less than 3.0

    UnBindVertexArray();
}

//--------------------------------------------------------------------------------------
void VertexArrayAoS::AddVertexAttrib(const VertexAttribInfo &info, GLuint offset) {
    //don't do null checks, because BindVertexArray already does them for us
    BindVertexArray();

    //make sure the attribute contains valid data
    if (info.mBytesPerElement == 0 || info.mElementsPerValue == 0)
        GLUF_CRITICAL_EXCEPTION(std::invalid_argument("Invalid Data in Vertex Attribute Info!"));

    //insert the offset into the data
    VertexAttribInfo tmpCopy = info;
    tmpCopy.mOffset = offset;

    mAttribInfos.insert(std::pair<AttribLoc, VertexAttribInfo>(info.mVertexAttribLocation, tmpCopy));

    //this is a bit inefficient to refresh every time an attribute is added, but this should not be significant
    RefreshDataBufferAttribute();

    //enable the new attribute
    glEnableVertexAttribArray(info.mVertexAttribLocation);//not harmful in opengl less than 3.0

    UnBindVertexArray();
}

//--------------------------------------------------------------------------------------
void VertexArrayAoS::ResizeBuffer(GLsizei numVertices, bool keepOldData, GLsizei newOldDataOffset) {
    //if we are keeping the old data, move it into a new buffer
    GLsizei vertSize = GetVertexSize();
    GLsizei newTotalSize = vertSize * numVertices;
    if (keepOldData) {
        GLsizei totalSize = vertSize * mVertexCount;
        GLsizei newOldDataTotalOffset = vertSize * newOldDataOffset;

        glBindBuffer(GL_COPY_READ_BUFFER, mDataBuffer);
        glBindBuffer(GL_COPY_WRITE_BUFFER, mCopyBuffer);

        //resize the copy buffer
        glBufferData(GL_COPY_WRITE_BUFFER, totalSize, nullptr, GL_STREAM_COPY);

        //copy the data
        glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, 0, 0, totalSize);

        //change binding
        glBindBuffer(GL_COPY_READ_BUFFER, mCopyBuffer);
        glBindBuffer(GL_COPY_WRITE_BUFFER, mDataBuffer);

        //resize the data buffer
        glBufferData(GL_COPY_WRITE_BUFFER, newTotalSize, nullptr, GL_STREAM_COPY);

        //copy the data back
        glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, 0, newOldDataTotalOffset, totalSize);
    } else {
        glBindBuffer(GL_ARRAY_BUFFER, mDataBuffer);
        glBufferData(GL_ARRAY_BUFFER, newTotalSize, nullptr, GL_STREAM_DRAW);
    }

    mVertexCount = numVertices;
}

//--------------------------------------------------------------------------------------
void VertexArrayAoS::EnableVertexAttributes() const noexcept {
    glBindBuffer(GL_ARRAY_BUFFER, mDataBuffer);

    GLuint stride = GetVertexSize();
    for (auto it : mAttribInfos) {
        glEnableVertexAttribArray(it.second.mVertexAttribLocation);
        glVertexAttribPointer(it.second.mVertexAttribLocation, it.second.mElementsPerValue, it.second.mType, GL_FALSE,
                              stride, reinterpret_cast<const GLvoid *>(it.second.mOffset));
    }

}

//--------------------------------------------------------------------------------------
void VertexArrayAoS::DisableVertexAttributes() const noexcept {
    glBindBuffer(GL_ARRAY_BUFFER, mDataBuffer);

    for (auto it : mAttribInfos) {
        glDisableVertexAttribArray(it.second.mVertexAttribLocation);
    }
}


//--------------------------------------------------------------------------------------
void VertexArraySoA::RefreshDataBufferAttribute() noexcept {
    //this is done at draw time less than opengl 3.0
    SWITCH_GL_VERSION
    GL_VERSION_GREATER_EQUAL(30) {
        BindVertexArray();
        for (auto it : mAttribInfos) {
            glBindBuffer(GL_ARRAY_BUFFER, mDataBuffers[it.second.mVertexAttribLocation]);
            glVertexAttribPointer(it.second.mVertexAttribLocation, it.second.mElementsPerValue, it.second.mType,
                                  GL_FALSE, 0, nullptr);
        }
        UnBindVertexArray();
    }
}

//--------------------------------------------------------------------------------------
GLuint VertexArraySoA::GetBufferIdFromAttribLoc(AttribLoc loc) const {
    auto ret = mDataBuffers.find(loc);
    if (ret == mDataBuffers.end())
        GLUF_CRITICAL_EXCEPTION(InvalidAttrubuteLocationException());

    return ret->second;
}

//--------------------------------------------------------------------------------------
VertexArraySoA::VertexArraySoA(GLenum PrimType, GLenum buffUsage, bool indexed) : VertexArrayBase(PrimType, buffUsage,
                                                                                                  indexed) {
}

//--------------------------------------------------------------------------------------
VertexArraySoA::~VertexArraySoA() noexcept {
    BindVertexArray();
    for (auto it : mDataBuffers)
        glDeleteBuffers(1, &it.second);
    UnBindVertexArray();
}

//--------------------------------------------------------------------------------------
VertexArraySoA::VertexArraySoA(VertexArraySoA &&other) : VertexArrayBase(std::move(other)) {
    mDataBuffers = std::move(other.mDataBuffers);
}

//--------------------------------------------------------------------------------------
VertexArraySoA &VertexArraySoA::operator=(VertexArraySoA &&other) {
    //since there is no possibility for user error, not catching dynamic cast here is A-OK

    VertexArrayBase *thisParentPtr = dynamic_cast<VertexArrayBase *>(this);
    *thisParentPtr = std::move(other);

    mDataBuffers = std::move(other.mDataBuffers);

    return *this;
}

//--------------------------------------------------------------------------------------
void VertexArraySoA::GetBarebonesMesh(MeshBarebones &inData) {
    BindVertexArray();

    std::map<AttribLoc, GLuint>::iterator it = mDataBuffers.find(GLUF_VERTEX_ATTRIB_POSITION);
    if (mIndexBuffer == 0 || it == mDataBuffers.end()) {
        GLUF_CRITICAL_EXCEPTION(InvalidAttrubuteLocationException());
    }

    glBindBuffer(GL_ARRAY_BUFFER, it->second);
    glm::vec3 *pVerts = (glm::vec3 *) glMapBuffer(GL_ARRAY_BUFFER, GL_READ_ONLY);

    inData.mVertices = ArrToVec(pVerts, mVertexCount);
    glUnmapBuffer(GL_ARRAY_BUFFER);


    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIndexBuffer);
    GLuint *pIndices = (GLuint *) glMapBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_READ_ONLY);

    inData.mIndices = ArrToVec(pIndices, mIndexCount);
    glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);

    UnBindVertexArray();
}

//--------------------------------------------------------------------------------------
void VertexArraySoA::AddVertexAttrib(const VertexAttribInfo &info) noexcept {
    BindVertexArray();

    mAttribInfos.insert(std::pair<AttribLoc, VertexAttribInfo>(info.mVertexAttribLocation, info));

    GLuint newBuff = 0;
    glGenBuffers(1, &newBuff);
    mDataBuffers.insert(std::pair<AttribLoc, GLuint>(info.mVertexAttribLocation, newBuff));

    RefreshDataBufferAttribute();

    UnBindVertexArray();
}

//--------------------------------------------------------------------------------------
void VertexArraySoA::RemoveVertexAttrib(AttribLoc loc) noexcept {
    BindVertexArray();

    auto it = mDataBuffers.find(loc);

    glDeleteBuffers(1, &(it->second));
    mDataBuffers.erase(it);

    VertexArrayBase::RemoveVertexAttrib(loc);

    UnBindVertexArray();
}

//--------------------------------------------------------------------------------------
void VertexArraySoA::EnableVertexAttributes() const noexcept {
    auto it = mDataBuffers.begin();
    for (auto itAttrib : mAttribInfos) {
        glBindBuffer(GL_ARRAY_BUFFER, it->second);
        glEnableVertexAttribArray(itAttrib.second.mVertexAttribLocation);
        glVertexAttribPointer(itAttrib.second.mVertexAttribLocation, itAttrib.second.mElementsPerValue,
                              itAttrib.second.mType, GL_FALSE, 0, nullptr);
        ++it;
    }
}

//--------------------------------------------------------------------------------------
void VertexArraySoA::DisableVertexAttributes() const noexcept {
    for (auto it : mAttribInfos) {
        glDisableVertexAttribArray(it.second.mVertexAttribLocation);
    }
}
}

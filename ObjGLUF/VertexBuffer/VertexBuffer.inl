#ifndef OBJGLUF_VERTEXBUFFER_INL
#define OBJGLUF_VERTEXBUFFER_INL

namespace GLUF {

/*
===================================================================================================
VertexArrayAoS Template Functions

*/

//--------------------------------------------------------------------------------------
template<typename T>
void VertexArrayAoS::BufferData(const GLVector <T> &data) {
    //a pretty logical first step
    if (data.size() == 0)
        return;

    //next, make sure that 'data' contains data intentended for this buffer operation
    try {
        //this means, if 'T' is not derived from VertexStructBase, then it cannot be used
        T *tTest = new T();//T already needs a default constructor
        dynamic_cast<VertexStruct *>(tTest);
        delete tTest;
    }
    catch (...) {
        throw std::invalid_argument("\"data\" Contains Data of Invalid Base!");
    }

    GLuint vertexSize = GetVertexSize();

    if (data[0].size() != vertexSize)
        throw std::invalid_argument("(VertexArrayAoS::BufferData): data vertex size is not compatible");

    BindVertexArray();
    glBindBuffer(GL_ARRAY_BUFFER, mDataBuffer);

    //pass OpenGL the raw pointers
    glBufferData(GL_ARRAY_BUFFER, data.size() * vertexSize, data.gl_data(), mUsageType);

    mVertexCount = data.size();

    UnBindVertexArray();

}

//--------------------------------------------------------------------------------------
template<typename T>
void VertexArrayAoS::BufferSubData(GLVector <T> data, std::vector <GLuint> vertexLocations, bool isSorted) {
    //a pretty logical first step
    if (data.size() == 0 || vertexLocations.size() == 0)
        return;

    //next, make sure that 'data' contains data intentended for this buffer operation
    try {
        //this means, if 'T' is not derived from VertexStructBase, then it cannot be used
        dynamic_cast<const VertexStruct *>(&data[0]);
    }
    catch (...) {
        throw std::invalid_argument("\"data\" Contains Data of Invalid Base!");
    }

    GLuint vertexSize = GetVertexSize();

    if (data[0].size() != vertexSize)
        throw std::invalid_argument("(VertexArrayAoS::BufferSubData): data vertex size is not compatible");

    if (vertexLocations.size() != 1 && vertexLocations.size() != data.size())
        throw std::invalid_argument("(VertexArrayAoS::BufferSubData): vertex location array size is too small");



    //if the size is 1, do a simple sequential overwrite
    if (vertexLocations.size() == 1) {
        BindVertexArray();
        glBindBuffer(GL_ARRAY_BUFFER, mDataBuffer);
        glBufferSubData(GL_ARRAY_BUFFER, vertexLocations[0] * vertexSize, data.size() * vertexSize, data.gl_data());
    } else//otherwise, split the data into contiguous chunks for best efficiency
    {

        /*

        Overview:

        Sort Vertex Locations
        Check for Duplicates
        Split Into Contiguous Chunks
        Buffer Each Chunk

        */

        //Next, sort the possibly unordered vertex locations with Bubble Sort
        if (!isSorted) {
            GLuint swap = 0;
            for (GLuint c = 0; c < (vertexLocations.size() - 1); c++) {
                for (GLuint d = 0; d < vertexLocations.size() - c - 1; d++) {
                    if (vertexLocations[d] > vertexLocations[d + 1]) /* For decreasing order use < */
                    {
                        //note: std::swap increases efficiency of swapping

                        //swap the vertices
                        std::swap(vertexLocations[d], vertexLocations[d + 1]);

                        //also remember to swap the data elements
                        std::swap(data[d], data[d + 1]);
                    }
                }
            }
        }

        //run through them all to make sure there are no duplicates, if there are, throw an exception
        {
            GLuint prev = 0;
            GLuint curr = 0;
            for (GLuint i = 0; i < vertexLocations.size(); ++i) {
                curr = vertexLocations[i];
                if (i == 0) {
                    prev = curr;
                    continue;
                }

                if (prev == curr) {
                    throw std::invalid_argument("Attempt to Buffer Subdata of Same Vertex Twice!");
                } else {
                    prev = curr;
                }
            }
        }

        //the chunks and offset for each chunk
        std::vector <GLVector<T>>
                chunkedData;
        std::vector <GLuint> chunkOffsets;

        //the working chunk
        GLVector <T> chunk;

        //the previous vertex location
        GLuint prevLocation = vertexLocations[0];

        bool isFirst = true;
        for (GLuint i = 0; i < vertexLocations.size(); ++i) {
            const T &itData = data[i];
            GLuint itLocation = vertexLocations[i];

            //initialize the first round and skip the rest
            if (isFirst) {
                chunk.push_back(itData);
                chunkOffsets.push_back(itLocation);

                isFirst = false;
                continue;
            }

            if (itLocation == vertexLocations.back()) {
                //we have reached the end of the data
                chunkedData.push_back(chunk);
            } else if (itLocation == prevLocation + 1) {
                chunk.push_back(itData);

                prevLocation = itLocation;
            } else {
                //if this one is not contiguous, add the chunk to the list and clear the chunk
                chunkedData.push_back(chunk);
                chunk.clear();

                //reinitialize the chunk if this data is not contiguous
                chunk.push_back(itData);
                chunkOffsets.push_back(itLocation);

                //finally, reset the counter
                prevLocation = itLocation;
            }

        }

        //once it is all split into chunks, buffer the data chunk by chunk
        for (GLuint i = 0; i < chunkedData.size(); ++i) {
            //for OpenGL safe-ness, bind the buffers at the last minute before buffering
            BindVertexArray();
            glBindBuffer(GL_ARRAY_BUFFER, mDataBuffer);
            glBufferSubData(GL_ARRAY_BUFFER, chunkOffsets[i] * vertexSize, chunkedData[i].size() * vertexSize,
                            (chunkedData[i]).gl_data());
        }

    }
}


/*
===================================================================================================
VertexArraySoA Template Functions

*/

template<typename T>
void VertexArraySoA::BufferData(AttribLoc loc, const std::vector <T> &data) {
    auto buffId = GetBufferIdFromAttribLoc(loc);

    BindVertexArray();
    glBindBuffer(GL_ARRAY_BUFFER, buffId);

    if (mVertexCount != 0) {
        if (data.size() != mVertexCount) {
            throw InvalidSoABufferLenException();
        }
    } else {
        mVertexCount = data.size();
    }

    VertexAttribInfo info = GetAttribInfoFromLoc(loc);
    GLuint bytesPerValue = info.mBytesPerElement * info.mElementsPerValue;
    glBufferData(GL_ARRAY_BUFFER, mVertexCount * bytesPerValue, data.data(), mUsageType);
    UnBindVertexArray();
}

template<typename T>
void VertexArraySoA::BufferSubData(AttribLoc loc, GLuint vertexOffsetCount, const std::vector <T> &data) {
    auto buffId = GetBufferIdFromAttribLoc(loc);

    BindVertexArray();
    glBindBuffer(GL_ARRAY_BUFFER, buffId);


    VertexAttribInfo info = GetAttribInfoFromLoc(loc);
    GLuint bytesPerValue = info.mBytesPerElement * info.mElementsPerValue;
    glBufferSubData(GL_ARRAY_BUFFER, vertexOffsetCount * bytesPerValue, mVertexCount * bytesPerValue, data.data());
    UnBindVertexArray();
}

}
#endif //OBJGLUF_VERTEXBUFFER_INL

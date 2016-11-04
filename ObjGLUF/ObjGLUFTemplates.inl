#pragma once

#include "ObjGLUF.h"

namespace GLUF
{

    /*
    ===================================================================================================
    Global Function Implementation

    */

    //--------------------------------------------------------------------------------------
    template<typename T>
    inline std::vector<T> ArrToVec(T* arr, unsigned long len)
    {
        if (!arr)
            throw std::invalid_argument("ArrToVec: \'arr\' == nullptr");

        return std::vector<T>(arr, arr + len);
    }


    //--------------------------------------------------------------------------------------
    template<typename T>
    inline std::vector<T> AdoptArray(T*& arr, unsigned long len) noexcept
    {
        if (arr == nullptr)
        throw std::invalid_argument("AdoptArray: \'arr\' == nullptr");

        NOEXCEPT_REGION_START

            //the return data;
            std::vector<T> ret;

        //the (hidden) internal vector member variables which contain the vector's memory internals
        ret._Myfirst = arr;
        ret._Mylast = arr + len;
        ret._Myend = arr + len;//in this case _Myend and _Mylast will be the same, because the the array is the exact size of all of the elements

        //reset the 'arr' parameter to nullptr in order to prevent the vector from being damaged externally
        arr = nullptr;

        return ret;

        NOEXCEPT_REGION_END

            return std::vector <T>();//to keep the compiler from complaining
    }


    /*
    ===================================================================================================
    GLVector Implementation
    
    */

    template<typename T>
    GLVector<T>::GLVector(GLVector<T>&& other) : std::vector<T>(std::move(other))
    {
        mGLData = other.mGLData;
        other.mGLData = 0;
    }

	//--------------------------------------------------------------------------------------
	template<typename T>
	GLVector<T>::~GLVector()
	{
		this->gl_delete_data();
	}

    //--------------------------------------------------------------------------------------
    template<typename T>
    GLVector<T>& GLVector<T>::operator=(GLVector&& other)
    {
        std::vector<T>* thisParentPtr = dynamic_cast<std::vector<T>*>(this);
        *thisParentPtr = std::move(other);

        mGLData = other.mGLData;
        other.mGLData = 0;

        return *this;
    }

    //--------------------------------------------------------------------------------------
    template<typename T>
    GLVector<T>& GLVector<T>::operator=(const GLVector<T>& other)
    {
        std::vector<T>* thisParentPtr = dynamic_cast<std::vector<T>*>(this);
        *thisParentPtr = other;

        mGLData = 0;//begin at 0, because we don't actually want to copy over the old data, because it will likely be invalidated, otherwise there is no reason to copy
    }

    //--------------------------------------------------------------------------------------
    template<typename T>
    void* GLVector<T>::gl_data() const
    {
        if (size() == 0)
            return nullptr;

        {
            size_t expcetedSize = front().size();
            for (auto it : *this)
            {
                if (it.size() != front().size())
                    throw std::length_error("Inconsistent Lengths Used in GLVector");
            }
        }

        size_t stride = front().size();
        size_t rawSize = stride * size();

        //compile the data into one array of contiguous data

        //recreate the cached data
        gl_delete_data();
        mGLData = new char[rawSize];

        //add each element
        for (size_t i = 0; i < size(); ++i)
        {
            //this doesn't actually take the memory location of the element, because the '&' operator is overloaded to return a contiguous array of bytes containing each element's data
            void* mem = &(*this)[i];

            //may be unsafe, but that's a later problem
            std::memcpy(mGLData + i * stride, mem, stride);

            //safe to delete, because when each element creates the raw data to add, it does not automatically delete it
            delete[] mem;
        }

        return mGLData;
    }

    //--------------------------------------------------------------------------------------
    template<typename T>
    void* GLVector<T>::gl_delete_data() const
    {
        NOEXCEPT_REGION_START

            delete[] mGLData;
        return nullptr;

        NOEXCEPT_REGION_END
    }

    //--------------------------------------------------------------------------------------
    template<typename T>
    void GLVector<T>::buffer_element(void* data, size_t element)
    {
        char* tmpUsableData = static_cast<char*>(data);
        if (element >= front().size())
            return;

        size_t elementSize = front().n_elem_size(element);
        for (unsigned int i = 0; i < size(); ++i)
        {
            (*this)[i].buffer_element(tmpUsableData, element);
            tmpUsableData += elementSize;
        }
    }


    /*
    ===================================================================================================
    VertexArrayAoS Template Functions

    */

    //--------------------------------------------------------------------------------------
    template<typename T>
    void VertexArrayAoS::BufferData(const GLVector<T>& data)
    {
        //a pretty logical first step
        if (data.size() == 0)
            return;

        //next, make sure that 'data' contains data intentended for this buffer operation
        try
        {
            //this means, if 'T' is not derived from VertexStructBase, then it cannot be used
            T* tTest = new T();//T already needs a default constructor
            dynamic_cast<VertexStruct*>(tTest);
            delete tTest;
        }
        catch (...)
        {
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
    void VertexArrayAoS::BufferSubData(GLVector<T> data, std::vector<GLuint> vertexLocations, bool isSorted)
    {
        //a pretty logical first step
        if (data.size() == 0 || vertexLocations.size() == 0)
            return;

        //next, make sure that 'data' contains data intentended for this buffer operation
        try
        {
            //this means, if 'T' is not derived from VertexStructBase, then it cannot be used
            dynamic_cast<const VertexStruct*>(&data[0]);
        }
        catch (...)
        {
            throw std::invalid_argument("\"data\" Contains Data of Invalid Base!");
        }

        GLuint vertexSize = GetVertexSize();

        if (data[0].size() != vertexSize)
            throw std::invalid_argument("(VertexArrayAoS::BufferSubData): data vertex size is not compatible");

        if (vertexLocations.size() != 1 && vertexLocations.size() != data.size())
            throw std::invalid_argument("(VertexArrayAoS::BufferSubData): vertex location array size is too small");



        //if the size is 1, do a simple sequential overwrite
        if (vertexLocations.size() == 1)
        {
            BindVertexArray();
            glBindBuffer(GL_ARRAY_BUFFER, mDataBuffer);
            glBufferSubData(GL_ARRAY_BUFFER, vertexLocations[0] * vertexSize, data.size() * vertexSize, data.gl_data());
        }
        else//otherwise, split the data into contiguous chunks for best efficiency
        {

            /*

            Overview:

            Sort Vertex Locations
            Check for Duplicates
            Split Into Contiguous Chunks
            Buffer Each Chunk

            */

            //Next, sort the possibly unordered vertex locations with Bubble Sort
            if (!isSorted)
            {
                GLuint swap = 0;
                for (GLuint c = 0; c < (vertexLocations.size() - 1); c++)
                {
                    for (GLuint d = 0; d < vertexLocations.size() - c - 1; d++)
                    {
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
            for (GLuint i = 0; i < vertexLocations.size(); ++i)
            {
                curr = vertexLocations[i];
                if (i == 0)
                {
                    prev = curr;
                    continue;
                }

                if (prev == curr)
                {
                    throw std::invalid_argument("Attempt to Buffer Subdata of Same Vertex Twice!");
                }
                else
                {
                    prev = curr;
                }
            }
        }

        //the chunks and offset for each chunk
        std::vector<GLVector<T>> chunkedData;
        std::vector<GLuint> chunkOffsets;

        //the working chunk
        GLVector<T> chunk;

        //the previous vertex location
        GLuint prevLocation = vertexLocations[0];

        bool isFirst = true;
        for (GLuint i = 0; i < vertexLocations.size(); ++i)
        {
            const T& itData = data[i];
            GLuint itLocation = vertexLocations[i];

            //initialize the first round and skip the rest
            if (isFirst)
            {
                chunk.push_back(itData);
                chunkOffsets.push_back(itLocation);

                isFirst = false;
                continue;
            }

            if (itLocation == vertexLocations.back())
            {
                //we have reached the end of the data
                chunkedData.push_back(chunk);
            }
            else if (itLocation == prevLocation + 1)
            {
                chunk.push_back(itData);

                prevLocation = itLocation;
            }
            else
            {
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
        for (GLuint i = 0; i < chunkedData.size(); ++i)
        {
            //for OpenGL safe-ness, bind the buffers at the last minute before buffering
            BindVertexArray();
            glBindBuffer(GL_ARRAY_BUFFER, mDataBuffer);
            glBufferSubData(GL_ARRAY_BUFFER, chunkOffsets[i] * vertexSize, chunkedData[i].size() * vertexSize, (chunkedData[i]).gl_data());
        }

        }
    }


    /*
    ===================================================================================================
    VertexArraySoA Template Functions

    */

    template<typename T>
    void VertexArraySoA::BufferData(AttribLoc loc, const std::vector<T>& data)
    {
        auto buffId = GetBufferIdFromAttribLoc(loc);

        BindVertexArray();
        glBindBuffer(GL_ARRAY_BUFFER, buffId);

        if (mVertexCount != 0)
        {
            if (data.size() != mVertexCount)
            {
                throw InvalidSoABufferLenException();
            }
        }
        else
        {
            mVertexCount = data.size();
        }

        VertexAttribInfo info = GetAttribInfoFromLoc(loc);
        GLuint bytesPerValue = info.mBytesPerElement * info.mElementsPerValue;
        glBufferData(GL_ARRAY_BUFFER, mVertexCount * bytesPerValue, data.data(), mUsageType);
        UnBindVertexArray();
    }

    template<typename T>
    void VertexArraySoA::BufferSubData(AttribLoc loc, GLuint vertexOffsetCount, const std::vector<T>& data)
    {
        auto buffId = GetBufferIdFromAttribLoc(loc);

        BindVertexArray();
        glBindBuffer(GL_ARRAY_BUFFER, buffId);


        VertexAttribInfo info = GetAttribInfoFromLoc(loc);
        GLuint bytesPerValue = info.mBytesPerElement * info.mElementsPerValue;
        glBufferSubData(GL_ARRAY_BUFFER, vertexOffsetCount * bytesPerValue, mVertexCount * bytesPerValue, data.data());
        UnBindVertexArray();
    }


}
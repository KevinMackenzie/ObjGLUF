#ifndef OBJGLUF_GLVECTOR_INL
#define OBJGLUF_GLVECTOR_INL

namespace GLUF {
/*
===================================================================================================
GLVector Implementation

*/

template<typename T>
GLVector<T>::GLVector(GLVector <T> &&other) : std::vector<T>(std::move(other)) {
    mGLData = other.mGLData;
    other.mGLData = 0;
}

//--------------------------------------------------------------------------------------
template<typename T>
GLVector<T>::~GLVector() {
    this->gl_delete_data();
}

//--------------------------------------------------------------------------------------
template<typename T>
GLVector <T> &GLVector<T>::operator=(GLVector &&other) {
    std::vector<T> *thisParentPtr = dynamic_cast<std::vector<T> *>(this);
    *thisParentPtr = std::move(other);

    mGLData = other.mGLData;
    other.mGLData = 0;

    return *this;
}

//--------------------------------------------------------------------------------------
template<typename T>
GLVector <T> &GLVector<T>::operator=(const GLVector <T> &other) {
    std::vector<T> *thisParentPtr = dynamic_cast<std::vector<T> *>(this);
    *thisParentPtr = other;

    mGLData = 0;//begin at 0, because we don't actually want to copy over the old data, because it will likely be invalidated, otherwise there is no reason to copy
}

//--------------------------------------------------------------------------------------
template<typename T>
void *GLVector<T>::gl_data() const {
    if (std::vector<T>::size() == 0)
        return nullptr;

    {
        size_t expcetedSize = std::vector<T>::front().size();
        for (auto it : *this) {
            if (it.size() != std::vector<T>::front().size())
                throw std::length_error("Inconsistent Lengths Used in GLVector");
        }
    }

    size_t stride = std::vector<T>::front().size();
    size_t rawSize = stride * std::vector<T>::size();

    //compile the data into one array of contiguous data

    //recreate the cached data
    gl_delete_data();
    mGLData = new char[rawSize];

    //add each element
    for (size_t i = 0; i < std::vector<T>::size(); ++i) {
        // TODO: just make this a virtual function... what was I thinking
        //this doesn't actually take the memory location of the element, because the '&' operator is overloaded to return a contiguous array of bytes containing each element's data
        char *mem = (*this)[i].get_data();

        //may be unsafe, but that's a later problem
        std::memcpy(mGLData + i * stride, mem, stride);

        //safe to delete, because when each element creates the raw data to add, it does not automatically delete it
        delete[] mem;
    }

    return mGLData;
}

//--------------------------------------------------------------------------------------
template<typename T>
void *GLVector<T>::gl_delete_data() const {
    delete[] mGLData;
    return nullptr;
}

//--------------------------------------------------------------------------------------
template<typename T>
void GLVector<T>::buffer_element(void *data, size_t element) {
    char *tmpUsableData = static_cast<char *>(data);
    if (element >= std::vector<T>::front().size())
        return;

    size_t elementSize = std::vector<T>::front().n_elem_size(element);
    for (unsigned int i = 0; i < std::vector<T>::size(); ++i) {
        (*this)[i].buffer_element(tmpUsableData, element);
        tmpUsableData += elementSize;
    }
}
}

#endif //OBJGLUF_GLVECTOR_INL

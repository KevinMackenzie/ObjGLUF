#ifndef OBJGLUF_GLVECTOR_H
#define OBJGLUF_GLVECTOR_H

#include <vector>
#include <ctype.h>

namespace GLUF {
/*
VertexStruct

    Base struct for data used in 'VertexArrayAoS'

*/

struct VertexStruct {
    virtual char *get_data() const = 0;
    virtual size_t size() const = 0;
    virtual size_t n_elem_size(size_t element) = 0;
    virtual void buffer_element(void *data, size_t element) = 0;
};

/*

GLVector
    TODO: Re-evaluate this class

    -a small excention to the std::vector class;
    -use this just like you would std::vector, except T MUST be derived from 'VertexStruct'
    -T::size() must be the same for every element of the vector, however it will only throw an exception when calling gl_data()

    Data Members:
        'mGLData': the cached data from 'gl_data()', gets destroyed when vector does
*/

template<typename T>
class GLVector : public std::vector<T> {
    mutable char *mGLData = nullptr;
public:

    /*
    Default Constructor
    */
    GLVector() {}

    ~GLVector();

    /*

    Move Copy Constructor and Move Assignment Operator

        Throws:
            May throw something in 'std::vector's move constructor or assignment operator
    */

    GLVector(GLVector &&other);
    GLVector &operator=(GLVector &&other);

    /*
    Copy Constructor and Assignment Operator

        Throws:
            May throw something in 'std::vector's copy constructor or assignment operator
    */
    GLVector(const GLVector &other) : std::vector<T>(other), mGLData(0) {}

    GLVector &operator=(const GLVector &other);

    /*
    gl_data

        Returns:
            contiguous, raw data of each element in the vector

        Throws:
            'std::length_error': if any two elements in the vector are not the same length

    */
    void *gl_data() const;

    /*
    gl_delete_data

        deletes 'mGLData'

        Returns:
            always nullptr; usage: "rawData = vec.gl_delete_data();"

        Throws:
            no-throw guarantee
    */
    void *gl_delete_data() const;

    /*
    buffer_element

        Parameters:
            'data': raw data to buffer
            'element': which element in the Vertex is it modifying

        Throws:
            undefined

    */
    void buffer_element(void *data, size_t element);
};
}

#include "GLVector.inl"
#endif //OBJGLUF_GLVECTOR_H

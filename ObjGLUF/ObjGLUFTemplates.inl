#pragma once

#include "ObjGLUF.h"

namespace GLUF {

/*
===================================================================================================
Global Function Implementation

*/

//--------------------------------------------------------------------------------------
template<typename T>
inline std::vector<T> ArrToVec(T *arr, unsigned long len) {
    if (!arr)
        throw std::invalid_argument("ArrToVec: \'arr\' == nullptr");

    return std::vector<T>(arr, arr + len);
}


//--------------------------------------------------------------------------------------
template<typename T>
inline std::vector<T> AdoptArray(T *&arr, unsigned long len) noexcept {
    if (arr == nullptr)
        throw std::invalid_argument("AdoptArray: \'arr\' == nullptr");
    //the return data;
    std::vector<T> ret;

    //the (hidden) internal vector member variables which contain the vector's memory internals
    ret._Myfirst = arr;
    ret._Mylast = arr + len;
    ret._Myend = arr +
                 len;//in this case _Myend and _Mylast will be the same, because the the array is the exact size of all of the elements

    //reset the 'arr' parameter to nullptr in order to prevent the vector from being damaged externally
    arr = nullptr;

    return ret;
}

}

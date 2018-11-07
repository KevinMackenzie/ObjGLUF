#ifndef OBJGLUF_EXCEPTIONS_H
#define OBJGLUF_EXCEPTIONS_H

#include <exception>
// TODO: remove this dependency
#include "ObjGLUF.h"

//macro for forcing an exception to show up on the log upon contruction
#define EXCEPTION_CONSTRUCTOR_BODY \
    GLUF_ERROR_LONG(" Exception Thrown: \"" << what() << "\"");
#define EXCEPTION_CONSTRUCTOR(class_name) \
class_name() \
{ \
    GLUF_ERROR_LONG(" Exception Thrown: \"" << what() << "\""); \
}

#define RETHROW throw;

/*
Exception

    Serves as base class for all other  exceptions.
    Override MyUniqueMessage in children
*/
class Exception : public std::exception {
public:

    const char *what() const noexcept = 0;
};

#endif //OBJGLUF_EXCEPTIONS_H

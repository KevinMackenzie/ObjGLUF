#ifndef OBJGLUF_EXCEPTIONS_H
#define OBJGLUF_EXCEPTIONS_H

class ControlCreationException : public Exception
{
public:
    virtual const char* what() const noexcept override
    {
        return "Control Falied to be Created!";
    }

    EXCEPTION_CONSTRUCTOR(ControlCreationException);
};

class ControlInitException
{
    virtual const char* what() const noexcept
    {
        return "Control or child failed to initialized correctly!";
    }

    EXCEPTION_CONSTRUCTOR(ControlInitException);
};

class NoItemSelectedException : Exception
{
public:
    virtual const char* what() const noexcept override
    {
        return "No Item Selected In List Box or Combo Box!";
    }

    EXCEPTION_CONSTRUCTOR(NoItemSelectedException);
};

class StringContainsInvalidCharacters : public Exception
{
public:
    virtual const char* what() const noexcept override
    {
        return "String Contained Characters Not In Charset";
    }

    EXCEPTION_CONSTRUCTOR(StringContainsInvalidCharacters)
};
#endif //OBJGLUF_EXCEPTIONS_H

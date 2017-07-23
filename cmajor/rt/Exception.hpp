// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMAJOR_RT_EXCEPTION_INCLUDED
#define CMAJOR_RT_EXCEPTION_INCLUDED

namespace cmajor { namespace rt {

class CmajorException
{
public:
    CmajorException(void* exceptionObject_);
private:
    void* exceptionObject;
};

} }  // namespace cmajor::rt

#endif // CMAJOR_RT_EXCEPTION_INCLUDED

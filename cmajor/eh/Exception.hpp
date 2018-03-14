// =================================
// Copyright (c) 2018 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMAJOR_EH_EXCEPTION_INCLUDED
#define CMAJOR_EH_EXCEPTION_INCLUDED

namespace cmajor { namespace eh {

class Exception
{
};

} } // namespace cmajor::eh

extern "C" void* ehExceptionTypeId;

extern "C" void* EhGetExceptionTypeId();

#endif // CMAJOR_EH_EXCEPTION_INCLUDED

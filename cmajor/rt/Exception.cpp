// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cmajor/rt/Exception.hpp>

namespace cmajor { namespace rt {

CmajorException::CmajorException(void* exceptionObject_) : exceptionObject(exceptionObject_)
{
}

} }  // namespace cmajor::rt
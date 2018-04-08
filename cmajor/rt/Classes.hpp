// =================================
// Copyright (c) 2018 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMAJOR_RT_CLASSES_INCLUDED
#define CMAJOR_RT_CLASSES_INCLUDED
#include <stdint.h>
#include <boost/uuid/uuid.hpp>

namespace cmajor { namespace rt {

void InitClasses();
void DoneClasses();
//uint64_t GetClassId(uint32_t typeId);
uint64_t GetClassId(const boost::uuids::uuid& typeId);

} } // namespace cmajor::rt

#endif // CMAJOR_RT_CLASSES_INCLUDED

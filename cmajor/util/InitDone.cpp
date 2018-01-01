// =================================
// Copyright (c) 2018 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cmajor/util/InitDone.hpp>
#include <cmajor/util/Unicode.hpp>

namespace cmajor { namespace util {

void Init()
{
    cmajor::unicode::UnicodeInit();
}

void Done()
{
    cmajor::unicode::UnicodeDone();
}

} } // namespace cmajor::util


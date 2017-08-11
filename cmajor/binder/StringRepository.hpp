// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMAJOR_BINDER_STRING_REPOSITORY_INCLUDED
#define CMAJOR_BINDER_STRING_REPOSITORY_INCLUDED
#include <unordered_map>

namespace cmajor { namespace binder {

class StringRepository
{
public:
    int Install(const std::string& str);
    const std::string& GetString(int id) const;
private:
    std::unordered_map<std::string, int> stringMap;
    std::vector<std::string> strings;
};

} } // namespace cmajor::binder

#endif // CMAJOR_BINDER_STRING_REPOSITORY_INCLUDED

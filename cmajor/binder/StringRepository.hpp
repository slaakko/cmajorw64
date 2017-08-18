// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMAJOR_BINDER_STRING_REPOSITORY_INCLUDED
#define CMAJOR_BINDER_STRING_REPOSITORY_INCLUDED
#include <cmajor/util/Error.hpp>
#include <unordered_map>

namespace cmajor { namespace binder {

template<class StringT>
class StringRepository
{
public:
    int Install(const StringT& str);
    const StringT& GetString(int id) const;
private:
    std::unordered_map<StringT, int> stringMap;
    std::vector<StringT> strings;
};

template<class StringT>
int StringRepository<StringT>::Install(const StringT& str)
{
    auto it = stringMap.find(str);
    if (it != stringMap.cend())
    {
        return it->second;
    }
    else
    {
        int id = strings.size();
        stringMap[str] = id;
        strings.push_back(str);
        return id;
    }
}

template<class StringT>
const StringT& StringRepository<StringT>::GetString(int id) const
{
    Assert(id >= 0 && id < strings.size(), "invalid string id");
    return strings[id];
}

} } // namespace cmajor::binder

#endif // CMAJOR_BINDER_STRING_REPOSITORY_INCLUDED

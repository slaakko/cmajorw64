// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cmajor/binder/StringRepository.hpp>
#include <cmajor/util/Error.hpp>

namespace cmajor { namespace binder {

int StringRepository::Install(const std::string& str)
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

const std::string& StringRepository::GetString(int id) const
{
    Assert(id >= 0 && id < strings.size(), "invalid string id");
    return strings[id];
}

} } // namespace cmajor::binder

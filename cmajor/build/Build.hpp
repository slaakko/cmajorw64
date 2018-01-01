// =================================
// Copyright (c) 2018 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMAJOR_BUILD_BUILD_INCLUDED
#define CMAJOR_BUILD_BUILD_INCLUDED
#include <string>

namespace cmajor { namespace build {

void BuildProject(const std::string& projectFilePath);
void BuildSolution(const std::string& solutionFilePath);

} } // namespace cmajor::build

#endif // CMAJOR_BUILD_BUILD_INCLUDED

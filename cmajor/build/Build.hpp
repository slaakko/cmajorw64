// =================================
// Copyright (c) 2018 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMAJOR_BUILD_BUILD_INCLUDED
#define CMAJOR_BUILD_BUILD_INCLUDED
#include <cmajor/symbols/Module.hpp>
#include <cmajor/ast/Project.hpp>

namespace cmajor { namespace build {

using namespace cmajor::ast;

void BuildProject(const std::string& projectFilePath);
void BuildProject(Project* project);
void BuildSolution(const std::string& solutionFilePath);
void BuildMsBuildProject(const std::string& projectName, const std::string& projectDirectory, const std::string& target,
    const std::vector<std::string>& sourceFiles, const std::vector<std::string>& referenceFiles);
void GenerateLibrary(const std::vector<std::string>& objectFilePaths, const std::string& libraryFilePath);
void Link(const std::string& executableFilePath, const std::string& libraryFilePath, const std::vector<std::string>& libraryFilePaths, cmajor::symbols::Module& module);
void ReadSystemFileIndex(const std::string& config);
void WriteSystemFileIndex(const std::string& config);
void ReadTypeIdCounter(const std::string& config);
void WriteTypeIdCounter(const std::string& config);
void ReadFunctionIdCounter(const std::string& config);
void WriteFunctionIdCounter(const std::string& config);
void ReadUserFileIndexCounter(const std::string& config);
void WriteUserFiledIndexCounter(const std::string& config);

} } // namespace cmajor::build

#endif // CMAJOR_BUILD_BUILD_INCLUDED

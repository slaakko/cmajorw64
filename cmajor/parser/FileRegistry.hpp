// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMAJOR_PARSER_FILE_REGISTRY_INCLUDED
#define CMAJOR_PARSER_FILE_REGISTRY_INCLUDED
#include <cmajor/util/Error.hpp>
#include <memory>
#include <string>
#include <vector>

namespace cmajor { namespace parser {

class FileRegistry
{
public:
    static void Init();
    static FileRegistry& Instance() { Assert(instance, "file registry not initialized"); return *instance; }
    int RegisterFile(const std::string& filePath);
    const std::string& GetFilePath(int filePathIndex);
    int GetNumberOfFilePaths();
private:
    static std::unique_ptr<FileRegistry> instance;
    std::vector<std::string> filePaths;
};

} } // namespace cmajor::parser

#endif // CMAJOR_PARSER_FILE_REGISTRY_INCLUDED

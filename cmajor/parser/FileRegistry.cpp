// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cmajor/parser/FileRegistry.hpp>
#include <mutex>

namespace cmajor { namespace parser {

std::mutex mtx;

std::unique_ptr<FileRegistry> FileRegistry::instance;

void FileRegistry::Init()
{
    instance.reset(new FileRegistry());
}

int FileRegistry::RegisterFile(const std::string& filePath)
{
    std::lock_guard<std::mutex> lock(mtx);
    int fileIndex = GetNumberOfFilePaths();
    filePaths.push_back(filePath);
    return fileIndex;
}

const std::string& FileRegistry::GetFilePath(int filePathIndex)
{
    std::lock_guard<std::mutex> lock(mtx);
    static std::string emptyFileName;
    if (filePathIndex >= 0 && filePathIndex < GetNumberOfFilePaths())
    {
        return filePaths[filePathIndex];
    }
    return emptyFileName;
}

int FileRegistry::GetNumberOfFilePaths()
{
    return int(filePaths.size());
}

} } // namespace cmajor::parser

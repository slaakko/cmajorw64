// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cmajor/parser/FileRegistry.hpp>
#include <cmajor/ast/SystemFileIndex.hpp>
#include <mutex>

namespace cmajor { namespace parser {

using namespace cmajor::ast;

std::mutex mtx;

std::unique_ptr<FileRegistry> FileRegistry::instance;

void FileRegistry::Init()
{
    instance.reset(new FileRegistry());
}

FileRegistry::FileRegistry() : obtainSystemFileIndeces(false)
{
}

void FileRegistry::PushObtainSystemFileIndeces()
{
    obtainsSystemFileIndecesStack.push(obtainSystemFileIndeces);
    obtainSystemFileIndeces = true;
}

void FileRegistry::PopObtainSystemFileIndeces()
{
    obtainSystemFileIndeces = obtainsSystemFileIndecesStack.top();
    obtainsSystemFileIndecesStack.pop();
}

uint32_t FileRegistry::RegisterFile(const std::string& filePath)
{
    std::lock_guard<std::mutex> lock(mtx);
    if (obtainSystemFileIndeces)
    {
        uint32_t fileIndex = SystemFileIndex::Instance().RegisterSystemSourceFile(filePath);
        return fileIndex;
    }
    else
    {
        uint32_t fileIndex = GetNumberOfFilePaths();
        filePaths.push_back(filePath);
        return fileIndex;
    }
}

std::string FileRegistry::GetFilePath(uint32_t filePathIndex)
{
    std::lock_guard<std::mutex> lock(mtx);
    if (filePathIndex >= firstSystemFileIndex)
    {
        return SystemFileIndex::Instance().GetSystemSourceFilePath(filePathIndex);
    }
    else
    {
        static std::string emptyFileName;
        if (filePathIndex >= 0 && filePathIndex < GetNumberOfFilePaths())
        {
            return filePaths[filePathIndex];
        }
        return emptyFileName;
    }
}

uint32_t FileRegistry::GetNumberOfFilePaths()
{
    return uint32_t(filePaths.size());
}

} } // namespace cmajor::parser

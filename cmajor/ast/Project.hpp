// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMAJOR_AST_PROJECT_INCLUDED
#define CMAJOR_AST_PROJECT_INCLUDED
#include <boost/filesystem.hpp>
#include <string>
#include <vector>
#include <memory>

namespace cmajor { namespace ast {

class ProjectDeclaration
{
public:
    virtual ~ProjectDeclaration();
};

class ReferenceDeclaration : public ProjectDeclaration
{
public:
    ReferenceDeclaration(const std::string& filePath_);
    const std::string& FilePath() const { return filePath; }
private:
    std::string filePath;
};

class SourceFileDeclaration : public ProjectDeclaration
{
public:
    SourceFileDeclaration(const std::string& filePath_);
    const std::string& FilePath() const { return filePath; }
private:
    std::string filePath;
};

enum class Target
{
    program, library
};

class TargetDeclaration : public ProjectDeclaration
{
public:
    TargetDeclaration(Target target_);
    Target GetTarget() const { return target; }
private:
    Target target;
};

class Project
{
public:
    Project(const std::u32string& name_, const std::string& filePath_, const std::string& config_);
    const std::u32string& Name() const { return name; }
    const std::string& FilePath() const { return filePath; }
    const boost::filesystem::path& BasePath() const { return basePath; }
    void AddDeclaration(ProjectDeclaration* declaration);
    void ResolveDeclarations();
    const std::vector<std::string>& References() const { return references; }
    const std::vector<std::string>& SourceFilePaths() const { return sourceFilePaths; }
    Target GetTarget() const { return target; }
    bool DependsOn(Project* that) const;
private:
    std::u32string name;
    std::string filePath;
    std::string config;
    Target target;
    boost::filesystem::path basePath;
    boost::filesystem::path systemLibDir;
    std::vector<std::unique_ptr<ProjectDeclaration>> declarations;
    std::string libraryFilePath;
    std::vector<std::string> references;
    std::vector<std::string> sourceFilePaths;
};

} } // namespace cmajor::ast

#endif // CMAJOR_AST_PROJECT_INCLUDED

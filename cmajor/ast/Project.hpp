// =================================
// Copyright (c) 2018 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMAJOR_AST_PROJECT_INCLUDED
#define CMAJOR_AST_PROJECT_INCLUDED
#include <boost/filesystem.hpp>
#include <string>
#include <vector>
#include <memory>

namespace cmajor { namespace ast {

std::string CmajorRootDir();
std::string CmajorSystemLibDir(const std::string& config);
std::string CmajorSystemModuleFilePath(const std::string& config);

enum class ProjectDeclarationType : uint8_t
{
    referenceDeclaration, sourceFileDeclaration, textFileDeclaration, targetDeclaration
};

class ProjectDeclaration
{
public:
    ProjectDeclaration(ProjectDeclarationType declarationType_);
    virtual ~ProjectDeclaration();
    ProjectDeclarationType GetDeclarationType() const { return declarationType; }
private:
    ProjectDeclarationType declarationType;
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

class TextFileDeclaration : public ProjectDeclaration
{
public:
    TextFileDeclaration(const std::string& filePath_);
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
    const std::string& ModuleFilePath() const { return moduleFilePath; }
    const std::string& LibraryFilePath() const { return libraryFilePath; }
    const std::string& ExecutableFilePath() const { return executableFilePath; }
    const std::vector<std::string>& References() const { return references; }
    const std::vector<std::string>& ReferencedProjectFilePaths() const { return referencedProjectFilePaths; }
    const std::vector<std::string>& SourceFilePaths() const { return sourceFilePaths; }
    Target GetTarget() const { return target; }
    bool DependsOn(Project* that) const;
    bool IsSystemProject() const { return isSystemProject; }
    void SetSystemProject() { isSystemProject = true; }
    void SetModuleFilePath(const std::string& moduleFilePath_);
    void SetLibraryFilePath(const std::string& libraryFilePath_);
    bool IsUpToDate() const;
private:
    std::u32string name;
    std::string filePath;
    std::string config;
    Target target;
    boost::filesystem::path basePath;
    boost::filesystem::path systemLibDir;
    std::vector<std::unique_ptr<ProjectDeclaration>> declarations;
    std::string moduleFilePath;
    std::string libraryFilePath;
    std::string executableFilePath;
    std::vector<std::string> references;
    std::vector<std::string> referencedProjectFilePaths;
    std::vector<std::string> sourceFilePaths;
    bool isSystemProject;
};

} } // namespace cmajor::ast

#endif // CMAJOR_AST_PROJECT_INCLUDED

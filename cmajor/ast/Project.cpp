// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cmajor/ast/Project.hpp>
#include <cmajor/util/Path.hpp>

namespace cmajor { namespace ast {

using namespace cmajor::util;

std::string CmajorRootDir()
{
    char* e = getenv("CMAJOR_ROOT");
    if (e == nullptr || !*e)
    {
        throw std::runtime_error("please set 'CMAJOR_ROOT' environment variable to contain /path/to/cmajorw64/cmajor directory.");
    }
    return std::string(e);
}

std::string GetSystemLibDir(const std::string& config)
{
    boost::filesystem::path sad(CmajorRootDir());
    sad /= "system";
    sad /= "lib";
    sad /= config;
    return GetFullPath(sad.generic_string());
}

ProjectDeclaration::~ProjectDeclaration()
{
}

ReferenceDeclaration::ReferenceDeclaration(const std::string& filePath_) : filePath(filePath_)
{
}

SourceFileDeclaration::SourceFileDeclaration(const std::string& filePath_) : filePath(filePath_)
{
}

TargetDeclaration::TargetDeclaration(Target target_) : target(target_)
{
}

Project::Project(const std::u32string& name_, const std::string& filePath_, const std::string& config_) :
    name(name_), filePath(filePath_), config(config_), target(Target::program), basePath(filePath)
{
    basePath.remove_filename();
    systemLibDir = GetSystemLibDir(config);
    boost::filesystem::path lfp(filePath);
    boost::filesystem::path fn = lfp.filename();
    lfp.remove_filename();
    lfp /= "lib";
    lfp /= config;
    lfp /= fn;
    lfp.replace_extension(".cml");
    libraryFilePath = GetFullPath(lfp.generic_string());
    boost::filesystem::path efp(filePath);
    efp.remove_filename();
    efp /= "bin";
    efp /= config;
    efp /= fn;
    efp.replace_extension(".exe");
    executableFilePath = GetFullPath(efp.generic_string());
}

void Project::AddDeclaration(ProjectDeclaration* declaration)
{
    declarations.push_back(std::unique_ptr<ProjectDeclaration>(declaration));
}

void Project::ResolveDeclarations()
{
    for (const std::unique_ptr<ProjectDeclaration>& declaration : declarations)
    {
        if (ReferenceDeclaration* reference = dynamic_cast<ReferenceDeclaration*>(declaration.get()))
        {
            boost::filesystem::path rp(reference->FilePath());
            boost::filesystem::path fn = rp.filename();
            rp.remove_filename();
            if (rp.is_relative())
            {
                rp = systemLibDir / rp;
            }
            rp /= fn;
            if (rp.extension() == ".cmp")
            {
                rp.replace_extension(".cml");
            }
            if (rp.extension() != ".cml")
            {
                throw std::runtime_error("invalid reference path extension '" + rp.generic_string() + "' (not .cmp or .cml)");
            }
            if (!boost::filesystem::exists(rp))
            {
                rp = reference->FilePath();
                rp.remove_filename();
                if (rp.is_relative())
                {
                    rp = basePath / rp;
                }
                rp /= "lib";
                rp /= config;
                rp /= fn;
                if (rp.extension() == ".cmp")
                {
                    rp.replace_extension(".cml");
                }
                if (rp.extension() != ".cml")
                {
                    throw std::runtime_error("invalid reference path extension '" + rp.generic_string() + "' (not .cmp or .cml)");
                }
            }
            std::string referencePath = GetFullPath(rp.generic_string());
            if (std::find(references.cbegin(), references.cend(), referencePath) == references.cend())
            {
                references.push_back(referencePath);
            }
        }
        else if (SourceFileDeclaration* sourceFileDeclaration = dynamic_cast<SourceFileDeclaration*>(declaration.get()))
        {
            boost::filesystem::path sfp(sourceFileDeclaration->FilePath());
            if (sfp.is_relative())
            {
                sfp = basePath / sfp;
            }
            if (sfp.extension() != ".cm")
            {
                throw std::runtime_error("invalid source file extension '" + sfp.generic_string() + "' (not .cm)");
            }
            if (!boost::filesystem::exists(sfp))
            {
                throw std::runtime_error("source file path '" + GetFullPath(sfp.generic_string()) + "' not found");
            }
            std::string sourceFilePath = GetFullPath(sfp.generic_string());
            if (std::find(sourceFilePaths.cbegin(), sourceFilePaths.cend(), sourceFilePath) == sourceFilePaths.cend())
            {
                sourceFilePaths.push_back(sourceFilePath);
            }
        }
        else if (TargetDeclaration* targetDeclaration = dynamic_cast<TargetDeclaration*>(declaration.get()))
        {
            target = targetDeclaration->GetTarget();
        }
        else
        {
            throw std::runtime_error("unknown project declaration");
        }
    }
}

bool Project::DependsOn(Project* that) const
{
    return std::find(references.cbegin(), references.cend(), that->libraryFilePath) != references.cend();
}

} } // namespace cmajor::ast

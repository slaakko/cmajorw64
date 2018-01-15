#include <cmajor/parser/Project.hpp>
#include <cmajor/util/Unicode.hpp>
#include <cmajor/util/MappedInputFile.hpp>
#include <cmajor/ast/InitDone.hpp>
#include <cmajor/parsing/InitDone.hpp>
#include <cmajor/util/InitDone.hpp>
#include <iostream>
#include <fstream>
#include <stdexcept>

using namespace cmajor::parser;
using namespace cmajor::unicode;
using namespace cmajor::util;

struct InitDone
{
    InitDone()
    {
        cmajor::ast::Init();
        cmajor::parsing::Init();
        cmajor::util::Init();
    }
    ~InitDone()
    {
        cmajor::util::Done();
        cmajor::parsing::Done();
        cmajor::ast::Done();
    }
};

std::vector<std::string> Lines(const std::string& content)
{
    std::vector<std::string> lines;
    int state = 0;
    std::string line;
    for (char c : content)
    {
        switch (state)
        {
            case 0:
            {
                if (c == '\r')
                {
                    state = 1;
                }
                else if (c == '\n')
                {
                    lines.push_back(line);
                    line.clear();
                }
                else
                {
                    line.append(1, c);
                }
                break;
            }
            case 1:
            {
                if (c == '\n')
                {
                    lines.push_back(line);
                    line.clear();
                    state = 0;
                }
                break;
            }
        }
    }
    if (!line.empty())
    {
        lines.push_back(line);
    }
    return lines;
}

int main(int argc, const char** argv)
{
    try
    {
        InitDone initDone;
        ProjectGrammar* projectGrammar = ProjectGrammar::Create();
        for (int i = 1; i < argc; ++i)
        {
            std::string arg = argv[i];
            MappedInputFile projectFile(arg);
            std::u32string p(ToUtf32(std::string(projectFile.Begin(), projectFile.End())));
            std::string config = "debug";
            std::unique_ptr<Project> project(projectGrammar->Parse(&p[0], &p[0] + p.length(), 0, arg, config));
            project->ResolveDeclarations();
            int n = project->SourceFilePaths().size();
            for (int i = 0; i < n; ++i)
            {
                const std::string& sourceFilePath = project->SourceFilePaths()[i];
                std::string sourceFileContent;
                {
                    MappedInputFile sourceFile(sourceFilePath);
                    sourceFileContent = std::string(sourceFile.Begin(), sourceFile.End());
                }
                std::ofstream nobom(sourceFilePath);
                std::vector<std::string> lines = Lines(sourceFileContent);
                for (int i = 0; i < lines.size(); ++i)
                {
                    nobom << lines[i] << std::endl;
                }
            }
        }
    }
    catch (const std::exception& ex)
    {
        std::cerr << ex.what() << std::endl;
        return 1;
    }
    return 0;
}

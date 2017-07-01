// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <iostream>
#include <string>
#include <stdexcept>

const char* version = "2.0.0";

void PrintHelp()
{
    std::cout << "Cmajor compiler version " << version << " for Windows x64" << std::endl;
}

int main(int argc, const char** argv)
{
    try
    {
        if (argc < 2)
        {
            PrintHelp();
        }
        else
        {
            for (int i = 1; i < argc; ++i)
            {
                std::string arg = argv[i];
                if (arg == "--help" || arg == "-h")
                {
                    PrintHelp();
                    return 0;
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

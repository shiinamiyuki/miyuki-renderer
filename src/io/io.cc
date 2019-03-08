//
// Created by Shiina Miyuki on 2019/2/28.
//

#include "io.h"
namespace Miyuki{
    namespace IO{
        void readUnderPath(const std::string &filename, std::function<void(const std::string &)> f){
            cxx::filesystem::path currentPath = cxx::filesystem::current_path();

            cxx::filesystem::path inputFile(filename);
            auto file = inputFile.filename().string();
            auto parent = inputFile.parent_path();
            if (!parent.empty())
                cxx::filesystem::current_path(parent);

            f(file);
            cxx::filesystem::current_path(currentPath);
        }
    }
}
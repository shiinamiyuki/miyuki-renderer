// MIT License
// 
// Copyright (c) 2019 椎名深雪
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all

// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include <core/export.h>
#include <stdexcept>
#include <iostream>
#include <cxxopts.hpp>
#include <api/defs.h>
#include <fstream>
#include <api/graph.h>

int main(int argc, char **argv) {
    using namespace miyuki;
    try {
        cxxopts::Options options("myk-cli", "miyuki-renderer :: Standalone");
        options.add_options()
                ("s,ser", "Read scene file as serialized data")
                ("f,file", "Scene file name", cxxopts::value<std::string>())
                ("o,out", "Output image file name", cxxopts::value<std::string>())
                ("h,help", "Print help and exit.");
        auto result = options.parse(argc, argv);
        if (result.count("help")) {
            std::cout << options.help({""}) << std::endl;
            return 0;
        }
        std::string sceneFile = result["file"].as<std::string>(), outFile = "out.png";
        if (result.count("out") != 0) {
            outFile = result["out"].as<std::string>();
        }
        {
            CurrentPathGuard _guard;
            fs::path scenePath = fs::absolute(fs::path(sceneFile));
            if(!fs::exists(scenePath)){
                MIYUKI_THROW(std::runtime_error, "file doesn't exist");
            }
            fs::path sceneDir = scenePath.parent_path();
            fs::current_path(sceneDir);
            sceneFile = fs::path(sceneFile).filename().string();

            core::Initialize();

            std::ifstream in(sceneFile);
            std::string str((std::istreambuf_iterator<char>(in)),
                            std::istreambuf_iterator<char>());
            json data = json::parse(str);

            auto graph = std::make_shared<core::SceneGraph>();
            graph->initialize(data);

            graph->render(outFile);

            core::Finalize();
        }
        return 0;
    } catch (std::exception &e) {
        std::cerr << "Fatal error: " << e.what() << std::endl;
        std::cerr << "Exited." << std::endl;
        return -1;
    }
}
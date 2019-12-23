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
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.


#include <fstream>
#include <string>
#include "../core/mesh-importers/wavefront-importer.h"
#include "../core/export.h"
#include <iostream>
#include <miyuki.renderer/graph.h>

int main(int argc, char **argv) {
    using namespace miyuki;
    if (argc != 3) {
        printf("Usage: mesh-importer mesh scene\n");
        return 0;
    }
    auto importer = std::make_shared<core::WavefrontImporter>();
    auto result = importer->importMesh(argv[1]);
    if (!result.mesh) {
        std::cerr << "error importing " << argv[1] << std::endl;
    } else {
        auto sceneFile = fs::path(argv[2]);
        core::SceneGraph graph;
        auto ctx = core::Initialize();
        if (!fs::exists(sceneFile)) {

        } else {
            std::ifstream in(sceneFile);
            std::string content((std::istreambuf_iterator<char>(in)),
                                std::istreambuf_iterator<char>());
            auto j = json::parse(content);
            graph = serialize::fromJson<core::SceneGraph>(*ctx, j);
        }

        auto outFile = fs::path(argv[1]).stem().string().append(".mesh");
        result.mesh->writeToFile(outFile);
        graph.shapes.emplace_back(result.mesh);
        std::ofstream out(sceneFile);
        out << serialize::toJson(*ctx, graph).dump(2) << std::endl;
    }
    return 0;
}

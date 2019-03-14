//
// Created by Shiina Miyuki on 2019/3/3.
//

#include "miyuki.h"
#include "renderengine.h"
using namespace Miyuki;

int main(int argc, char **argv) {
    RenderEngine engine;
    try {
        engine.processCommandLine(argc, argv);
        engine.commitScene();
        return engine.exec();
    } catch (std::exception &e) {
        std::cerr << e.what() << std::endl;
    }
}
//
// Created by Shiina Miyuki on 2019/1/12.
//


//#include "../thirdparty/cxxopts.hpp"

#include "rendersystem.hpp"
#include "../core/scene.h"



int32_t main(int32_t argc, char **argv) {
    renderSystem.processOptions(argc, argv);

    return renderSystem.exec();

}
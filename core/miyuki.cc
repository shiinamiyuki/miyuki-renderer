//
// Created by Shiina Miyuki on 2019/1/12.
//


//#include "../lib/cxxopts.hpp"

#include "rendersystem.hpp"




int main(int argc, char **argv) {
    renderSystem.processOptions(argc, argv);

    return renderSystem.exec();
}
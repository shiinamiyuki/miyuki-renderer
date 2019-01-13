//
// Created by xiaoc on 2019/1/12.
//

#include "util.h"

using namespace Miyuki;

class RenderSystem {
public:
    RenderSystem() {
        Init();
    }

    int exec() {
        return 0;
    }

    ~RenderSystem() {
        Exit();
    }
};

int main(int argc, char **argv) {
    RenderSystem renderSystem;
    return renderSystem.exec();
}
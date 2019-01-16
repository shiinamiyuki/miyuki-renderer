//
// Created by xiaoc on 2019/1/12.
//

#include "util.h"
#include "mesh.h"
using namespace Miyuki;

class RenderSystem {
public:
    RenderSystem() {
        Init();
    }

    int exec() {
        MaterialList materialList;
        auto m = Mesh::LoadFromObj(&materialList,"models/gopher.obj");
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
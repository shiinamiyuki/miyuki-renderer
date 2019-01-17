//
// Created by Shiina Miyuki on 2019/1/12.
//

#include "util.h"
#include "mesh.h"
#include "scene.h"

using namespace Miyuki;

class RenderSystem {
public:
    RenderSystem() {
        Init();
    }

    int exec() {
        Scene scene;
        scene.loadObjTrigMesh("models/cornell_box.obj");
        scene.getCamera().moveTo(Vec3f(250, 250, -500));
        scene.renderPreview();
        scene.writeImage("test.png");
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
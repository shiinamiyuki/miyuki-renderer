//
// Created by Shiina Miyuki on 2019/1/12.
//

#include "util.h"
#include "mesh.h"
#include "scene.h"

using namespace Miyuki;

class RenderSystem {
public:
    RenderSystem(int argc, char **argv) {
        Init();
    }

    int exec() {
        Scene scene;
        scene.loadObjTrigMesh("models/cornell_box.obj");
        scene.loadObjTrigMesh("models/nature.obj",Transform(Vec3f(180, 165, 140),Vec3f(0,0,0),1));
        scene.loadObjTrigMesh("models/gopher.obj", Transform(Vec3f(350, 0, 0), Vec3f(M_PI / 2 + 0.3, 0, 0), 20));
        scene.getCamera().moveTo(Vec3f(280, 260, -520));
       // scene.setAmbientLight(Spectrum(1,1,1));
//        scene.getCamera().moveTo(Vec3f(10,3,0));
//        scene.getCamera().rotateTo(Vec3f(-M_PI/2,0,0));
        scene.prepare();
        PathTracer pathTracer;
        pathTracer.render(&scene);
        scene.writeImage("test.png");
        return 0;
    }

    ~RenderSystem() {
        Exit();
    }
};

int main(int argc, char **argv) {
    RenderSystem renderSystem(argc, argv);
    return renderSystem.exec();
}
//
// Created by Shiina Miyuki on 2019/1/12.
//

#include "util.h"
#include "mesh.h"
#include "scene.h"
#include "distribution.h"

using namespace Miyuki;

class RenderSystem {
public:
    RenderSystem(int argc, char **argv) {
        Init();
    }

    int exec() {
        Scene scene;
        scene.loadObjTrigMesh("models/sportsCar.obj", Transform(Vec3f(10, 0, 305), Vec3f(2.3, 0, 0), 1));
        scene.getCamera().moveTo(Vec3f(15, 1.5, 305));
        scene.getCamera().rotateTo(Vec3f(-M_PI/2 ,0.3,0));
      //  scene.getCamera().rotateTo(Vec3f(-M_PI/2 - ,0,0));
        scene.loadObjTrigMesh("models/box.obj");
        //scene.loadObjTrigMesh("models/nature.obj", Transform(Vec3f(180, 165, 140), Vec3f(0, 0, 0), 1));
        //scene.loadObjTrigMesh("models/nature.obj", Transform(Vec3f(280, 400, 250), Vec3f(0, 0, 0), 2.3));
        //scene.loadObjTrigMesh("models/gopher.obj", Transform(Vec3f(350, 0, 0), Vec3f(M_PI / 2 + 0.3, 0, 0), 20));
        //    scene.getCamera().moveTo(Vec3f(280, 260, -520));
        //scene.setAmbientLight(Spectrum(0.8, 0.8, 0.8));
//        scene.getCamera().moveTo(Vec3f(10,3,0));
//        scene.getCamera().rotateTo(Vec3f(-M_PI/2,0,0));
        scene.option.samplesPerPixel = 1024;
        scene.prepare();
        PathTracer pathTracer;
        AOIntegrator aoIntegrator;
        PSSMLTUnidirectional pssmlt;
        pathTracer.render(scene);
        scene.writeImage("test.png");
        return 0;
    }

    int test() {
        std::vector<float> p = {4, 2, 4, 2, 4, 2, 4, 2};
        Distribution1D distribution1D(p.data(), p.size());
        std::vector<int> count;
        count.resize(p.size());
        std::random_device rd;
        std::uniform_real_distribution<double> dist;
        int N = 10000;
        for (int i = 0; i < N; i++) {
            count.at(distribution1D.sampleInt(dist(rd)))++;
        }
        fmt::print("{}\n", distribution1D.cdf(1));
        for (int i = 0; i < count.size(); i++) {
            fmt::print("{} {}\n", (double) i / count.size(), count[i] / (double) N);
        }
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
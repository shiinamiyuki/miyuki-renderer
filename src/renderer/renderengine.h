//
// Created by Shiina Miyuki on 2019/3/3.
//

#ifndef MIYUKI_RENDERENGINE_H
#define MIYUKI_RENDERENGINE_H

#include "core/scene.h"
#include "integrators/integrator.h"

namespace Miyuki {
    class RenderEngine {
        Scene scene;
        std::unique_ptr<Integrator> integrator;
        std::atomic<bool> renderContinue;
        enum Mode {
            commandLine,
            gui
        } mode;
    public:
        std::function<void(void)> updateFunc;

        RenderEngine();

        void processCommandLine(int argc, char **argv);

        void readDescription(Json::JsonObject object);

        int exec();

        void stopRender();

        void startRender();

        void setGuiMode(bool opt) {
            if (opt)
                mode = gui;
            else
                mode = commandLine;
        }

        void readPixelData(std::vector<uint8_t> &pixelData, int &width, int &height);

        void imageSize(int &width, int &height);

        void commitScene() {
            scene.commit();
        }

        void renderPreview(std::vector<uint8_t> &pixelData, int &width, int &height);

        Camera *getMainCamera() const {
            return scene.camera.get();
        }

        void updateCameraInfoToParameterSet() {
            scene.parameterSet.addVec3f("camera.rotation", getMainCamera()->rotation() / PI * 180);
            scene.parameterSet.addVec3f("camera.translation", getMainCamera()->translation());
            fmt::print("Camera translation: {} {} {}\n", getMainCamera()->translation().x(),
                       getMainCamera()->translation().y(),
                       getMainCamera()->translation().z());
            fmt::print("Camera rotation: {} {} {}\n",
                       scene.parameterSet.findVec3f("camera.rotation",{}).x(),
                       scene.parameterSet.findVec3f("camera.rotation",{}).y(),
                       scene.parameterSet.findVec3f("camera.rotation",{}).z());
        }
    };
}

#endif //MIYUKI_RENDERENGINE_H

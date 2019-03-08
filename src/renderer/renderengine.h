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
    public:
        void processCommandLine(int argc, char **argv);

        void readDescription(Json::JsonObject object);

        int exec();
    };
}

#endif //MIYUKI_RENDERENGINE_H

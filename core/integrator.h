//
// Created by Shiina Miyuki on 2019/1/12.
//

#ifndef MIYUKI_INTEGRATOR_H
#define MIYUKI_INTEGRATOR_H

#include "util.h"

namespace Miyuki {
    class Scene;

    class Integrator {
    public:
        virtual ~Integrator() = default;

        virtual void render(Scene *) = 0;
    };

    template<typename T>
    Integrator *createIntegrator() { return nullptr; }
}
#endif //MIYUKI_INTEGRATOR_H

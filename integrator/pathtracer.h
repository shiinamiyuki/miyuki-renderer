//
// Created by Shiina Miyuki on 2019/1/19.
//

#ifndef MIYUKI_PATH_H
#define MIYUKI_PATH_H

#include "../core/integrator.h"

namespace Miyuki {
    class PathTracer : public Integrator {
    public:
        void render(Scene *) override;
    };
}
#endif //MIYUKI_PATH_H

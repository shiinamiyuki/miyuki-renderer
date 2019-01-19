//
// Created by Shiina Miyuki on 2019/1/19.
//

#ifndef MIYUKI_AO_H
#define MIYUKI_AO_H

#include "../core/integrator.h"

namespace Miyuki {
    class AOIntegrator : public Integrator {
    public:
        void render(Scene *) override;
    };
}
#endif //MIYUKI_AO_H

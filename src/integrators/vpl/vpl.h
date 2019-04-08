//
// Created by Shiina Miyuki on 2019/4/5.
//

#ifndef MIYUKI_VPL_H
#define MIYUKI_VPL_H

#include <integrators/integrator.h>


namespace Miyuki {
    class VPL : public Integrator {
        int minDepth, maxDepth;
    public:
        VPL(const ParameterSet &set);

        void render(Scene &scene) override;
    };
}
#endif //MIYUKI_VPL_H

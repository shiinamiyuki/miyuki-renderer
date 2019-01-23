//
// Created by Shiina Miyuki on 2019/1/22.
//

#ifndef MIYUKI_PSSMLT_H
#define MIYUKI_PSSMLT_H

#include "../core/integrator.h"
#include "pathtracer.h"

namespace Miyuki{
    class PrimarySample{

    };
    class MLTSampler : public Sampler{
    public:
        Float nextFloat() override;

        int nextInt() override;

        Float nextFloat(Seed *seed) override;

        int nextInt(Seed *seed) override;
    };
    class PSSMLTUnidirectional : public PathTracer{
    public:

    };
}

#endif //MIYUKI_PSSMLT_H

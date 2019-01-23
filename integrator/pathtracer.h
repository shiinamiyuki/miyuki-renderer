//
// Created by Shiina Miyuki on 2019/1/19.
//

#ifndef MIYUKI_PATH_H
#define MIYUKI_PATH_H

#include "../core/integrator.h"
#include "../core/geometry.h"
#include "../core/sampler.h"

namespace Miyuki {
    class PathTracer : public Integrator {
        //void renderPixel(const Point2i &, Sampler &);

        void iteration(Scene *);

    public:
        void render(Scene *) override;
    };



}
#endif //MIYUKI_PATH_H

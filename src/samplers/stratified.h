//
// Created by Shiina Miyuki on 2019/1/31.
//

#ifndef MIYUKI_STRATIFIED_H
#define MIYUKI_STRATIFIED_H

#include "sampler.h"
#include "../math/geometry.h"

namespace Miyuki {
    class StratifiedSampler : public Sampler {
        //int32_t x,y;
        std::vector<Point2i> coord;
        int32_t N;
        Float delta;
        bool flag;
        int streamIdx;
        void updateXY();
    public:
        explicit StratifiedSampler(Seed *s);

        Float nextFloat() override;

        int32_t nextInt() override;

        Point2f nextFloat2D() override;

        Float nextFloat(Seed *seed) override;

        void start() override;

        int32_t nextInt(Seed *seed) override;
    };
}
#endif //MIYUKI_STRATIFIED_H

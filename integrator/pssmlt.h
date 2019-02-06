//
// Created by Shiina Miyuki on 2019/1/22.
//

#ifndef MIYUKI_PSSMLT_H
#define MIYUKI_PSSMLT_H

#include "../core/integrator.h"
#include "../core/spectrum.h"
#include "../sampler/random.h"
#include "../core/memory.h"
#include "pathtracer.h"


namespace Miyuki {
    namespace MLT {
        struct PrimarySample {
            int32_t modify;
            Float value;

            PrimarySample() = default;

            PrimarySample(Float v, int32_t m) : value(v), modify(m) {}
        };

        struct Sample {
            Point2i pos;
            Float weight;
            Spectrum radiance;

            Sample() : weight(0) {}
        };

        struct PathSeedGenerator : public RandomSampler {
        public:
            std::vector<Float> pathSeed;

            PathSeedGenerator(Seed *s = nullptr) : RandomSampler(s) {}

            Float nextFloat() override;

            Float nextFloat(Seed *seed) override;
        };
    }
    class PSSMLTUnidirectional;

    class MLTSampler : public RandomSampler {
        friend class PSSMLTUnidirectional;

        Float I, oldI, b;
        MLT::Sample oldSample, newSample, contributionSample;
        Float largeStepProb;
        bool largeStep;
        int32_t time;
        int32_t largeStepTime;
        int32_t streamIdx;
        std::vector<MLT::PrimarySample> u;
        std::vector<std::pair<int32_t, MLT::PrimarySample>> stack;

        void start();

        Float primarySample(int32_t i);

        void push(int32_t i, const MLT::PrimarySample &);

        void pop();

    public:
        void assignSeed(const MLT::PathSeedGenerator &);

        MLTSampler(Seed *s);

        Float nextFloat() override;

        int32_t nextInt() override;

        Float nextFloat(Seed *seed) override;

        Point2f nextFloat2D() override;

        int32_t nextInt(Seed *seed) override;

        void update(const Point2i &pos, Spectrum &L);
    };

    class PSSMLTUnidirectional : public PathTracer {
        std::vector<MLTSampler> samples;

        Spectrum trace(MemoryArena&,int32_t, Scene &, Sampler &, Point2i &);

        void bootstrap(Scene &);

        void mutation(Scene &);

    public:
        void render(Scene &) override;
    };



}

#endif //MIYUKI_PSSMLT_H

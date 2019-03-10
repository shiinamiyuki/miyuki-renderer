//
// Created by Shiina Miyuki on 2019/3/9.
//

#ifndef MIYUKI_MLTSAMPLER_H
#define MIYUKI_MLTSAMPLER_H

#include "sampler.h"
namespace Miyuki{
    struct PrimarySample{
        Float value;
        Float backup;
    };
    class MLTSampler : public Sampler{
        int nStream;
        int streamIndex;
        int sampleIndex;
    public:
        MLTSampler(Seed * seed);
        void startStream(int index){
            Assert(index < nStream);
            streamIndex = index;
        }
    };
}
#endif //MIYUKI_MLTSAMPLER_H

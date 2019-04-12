//
// Created by Shiina Miyuki on 2019/4/9.
//

#ifndef MIYUKI_VARIANCE_HPP
#define MIYUKI_VARIANCE_HPP

#include <miyuki.h>
namespace Miyuki{
    template<typename T>
    struct Variance{
        T mean, meanSqr;
        double sampleCount = 0;
        Variance():mean(), meanSqr(){}
        void addSample(const T& sample){
            auto delta = sample - mean;
            sampleCount += 1;
            mean += delta / sampleCount;
            meanSqr += delta * (sample - mean);


        }
        T var()const{
            if(sampleCount < 2){
                return T();
            }
            return meanSqr / (sampleCount - 1);
        }
    };
}
#endif //MIYUKI_VARIANCE_HPP

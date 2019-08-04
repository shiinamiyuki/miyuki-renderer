//
// Created by Shiina Miyuki on 2019/1/22.
//

#ifndef MIYUKI_DISTRIBUTION_H
#define MIYUKI_DISTRIBUTION_H

#include "miyuki.h"

namespace Miyuki {
    // an arbitrary 1d distribution
    // discrete implementation of inverse transform sampling
    class Distribution1D {
        std::vector<Float> cdfArray;

        // solves cdf^(-1)
        int binarySearch(Float x) const;

    public:
        Float funcInt;

        Distribution1D(const Float *data, uint32_t N);

        int sampleDiscrete(Float x, Float *pdf = nullptr) const;

        Float sampleContinuous(Float x, Float* pdf = nullptr, int *offset=nullptr) const;

        Float pdf(int x) const;

        Float cdf(Float x) const;

		size_t count()const { return cdfArray.size() - 1; }
    };
}
#endif //MIYUKI_DISTRIBUTION_H

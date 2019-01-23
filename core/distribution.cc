//
// Created by Shiina Miyuki on 2019/1/22.
//

#include "distribution.h"

using namespace Miyuki;

Miyuki::Distribution1D::Distribution1D(const Float *data, unsigned int N) {
    cdfArray.resize(N + 1);
    cdfArray[0] = 0;
    for (int i = 0; i < N; i++) {
        cdfArray[i + 1] = cdfArray[i] + data[i];
    }
    auto sum = cdfArray[N];
    if (sum > 0) {
        for (int i = 0; i < N; i++) {
            cdfArray[i] /= sum;
        }
    } else {
        for (int i = 0; i < N; i++) {
            cdfArray[i] = 1.0f / N;
        }
    }
}

int Distribution1D::sampleInt(Float x) const {
    return binarySearch(x);
}

Float Distribution1D::sampleFloat(Float x) const {
    return cdfArray[binarySearch(x)];
}

int Distribution1D::binarySearch(Float x) const {
    int lower = 0;
    int higher = (int) cdfArray.size() - 1;
    while (lower < higher) {
        int mid = (lower + higher) / 2;
        if (mid < 1 || (cdfArray[mid - 1] <= x && x < cdfArray[mid])) {
            return mid;
        }
        if (cdfArray[mid] < x) {
            lower = mid;
        } else {
            higher = mid;
        }
    }
    return (lower + higher) / 2;
}

Float Distribution1D::cdf(Float x) const {
    int i = (int)clamp<Float>(x *  cdfArray.size(), 0, cdfArray.size() - 1);
    return cdfArray[i];
}


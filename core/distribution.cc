//
// Created by Shiina Miyuki on 2019/1/22.
//

#include "distribution.h"

using namespace Miyuki;

Miyuki::Distribution1D::Distribution1D(const Float *data, unsigned int N) {
    cdfArray.resize(N + 1);
    cdfArray[0] = 0;
    for (int i = 0; i < N; i++) {
        cdfArray[i + 1] = cdfArray[i] + data[i] / N;
    }
    funcInt = cdfArray[N];
    if (funcInt > 0) {
        for (int i = 0; i < N + 1; i++) {
            cdfArray[i] /= funcInt;
        }
    } else {
        for (int i = 1; i < N + 1; i++) {
            cdfArray[i] = Float(i) / N;
        }
    }
}

int Distribution1D::sampleInt(Float x) const {
    return clamp<int>(binarySearch(x), 1, cdfArray.size() - 2) - 1;
}

Float Distribution1D::sampleFloat(Float x) const {
    return cdfArray[binarySearch(x)];
}

int Distribution1D::binarySearch(Float x) const {
    int lower = 0;
    int higher = (int) cdfArray.size() - 1;
    while (lower <= higher) {
        int mid = (lower + higher) / 2;
        if (mid < 1 || (cdfArray[mid - 1] <= x && x < cdfArray[mid])) {
            return mid;
        }
        if (cdfArray[mid] < x) {
            lower = mid + 1;
        } else {
            higher = mid - 1;
        }
    }
    return (lower + higher) / 2;
}

Float Distribution1D::cdf(Float x) const {
    int i = (int) clamp<Float>(x * cdfArray.size(), 0, cdfArray.size() - 1);
    return cdfArray[i];
}


//
// Created by Shiina Miyuki on 2019/3/19.
//

#ifndef MIYUKI_NLM_H
#define MIYUKI_NLM_H

#include <io/image.h>

#include <utils/thread.h>

namespace Miyuki {
    inline Float EuclideanDistanceSqr(const Spectrum &a, const Spectrum &b) {
        return (a - b).lengthSquared();
    }

    template<typename Texel>
    void NLMeansWeights(const IO::GenericImage<Texel> &in,
                        IO::GenericImage<Texel> &out,
                        int windowSize = 7) {
        IO::GenericImage<Texel> temp(in.width, in.height);
        for (int i = 0; i < in.height; i++) {
            Texel sum = Texel();
            for (int j = 0; j < in.width; j++) {
                sum += in(j, i);
                out(j, i) = sum;
            }
        }

        for (int i = 0; i < in.width; i++) {
            Texel sum = Texel();
            for (int j = 0; j < in.height; j++) {
                sum += out(i, j);
                temp(i, j) = sum;
            }
        }
        int w = (windowSize - 1) / 2;
        for (int i = 0; i < in.width; i++) {
            for (int j = 0; j < in.height; j++) {
                int lowerX = i - w - 1, lowerY = j - w - 1;
                int upperX = i + w, upperY = j + w;
                out(i, j) = temp(upperX, upperY) - temp(upperX, lowerY) - temp(lowerX, upperY) + temp(lowerX, lowerY);
                out(i, j) = out(i, j) / (windowSize * windowSize);
            }
        }
    }

    template<typename Texel>
    void NLMeans(const IO::GenericImage<Texel> &in,
                 IO::GenericImage<Texel> &out,
                 int searchWindowSize = 21,
                 int blockSize = 7,
                 Float h = 1.0f) {
        IO::GenericImage<Texel> B(in.width, in.height);
        out = IO::GenericImage<Texel>(in.width, in.height);
        NLMeansWeights(in, B, blockSize);
        int w = (searchWindowSize - 1) / 2;
        Thread::ParallelFor(0u, in.width, [&](uint32_t i, uint32_t) {
            for (int j = 0; j < in.height; j++) {
                double weightSum = 0;
                auto Bp = B(i, j);
                Texel sum;
                for (int dx = -w; dx <= w; dx++) {
                    for (int dy = -w; dy <= w; dy++) {
                        auto Bq = B(i + dx, j + dy);
                        auto f = exp(-EuclideanDistanceSqr(Bp, Bq) / (h * h));
                        sum += f * in(i + dx, j + dy);
                        weightSum += f;
                    }
                }
                out(i, j) = weightSum != 0 ? sum / weightSum : sum;
            }
        });
    }
}

#endif //MIYUKI_NLM_H

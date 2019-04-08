//
// Created by Shiina Miyuki on 2019/3/19.
//

#ifndef MIYUKI_NLM_H
#define MIYUKI_NLM_H

#include <io/image.h>

#include <utils/thread.h>

namespace Miyuki {
    inline Float NLMDistanceSqr(const Spectrum &a, const Spectrum &b) {
        return (std::pow(a.r() - b.r(), 2.0f)
                + std::pow(a.g() - b.g(), 2.0f)
                + std::pow(a.b() - b.b(), 2.0f)) / 3.0f;
    }

    template<typename Texel>
    void BoxFilter(const IO::GenericImage<Texel> &in,
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
                 Float h = 1.0f,
                 Float sigma = 2.0f) {
        out = IO::GenericImage<Texel>(in.width, in.height);
        std::vector<IO::GenericImage<Float>> u;
        std::vector<IO::GenericImage<Float>> v;
        for (int i = 0; i < searchWindowSize * searchWindowSize; i++) {
            u.emplace_back(in.width, in.height);
            v.emplace_back(in.width, in.height);
        }
        auto mapS2 = [=](int i) {
            const int S = (searchWindowSize - 1) / 2;
            return Point2i(i % searchWindowSize, i / searchWindowSize) - Point2i(S, S);
        };

        auto mapU = [=](int i) {
            return Point2i(i % in.width, i / in.width);
        };

        // u_n[k] = (y[k+n] - y[n])^2
        Thread::ParallelFor(0u, in.width * in.height, [&](uint32_t k, uint32_t) {
            auto p = mapU(k);
            for (int n = 0; n < searchWindowSize * searchWindowSize; n++) {
                auto offset = mapS2(n);
                auto ykn = in(p + offset);
                auto yk = in(p);
                auto dist = (NLMDistanceSqr(ykn, yk));
                u[n][k] = dist;
            }
        }, 4096);
        Thread::ParallelFor(0u, searchWindowSize * searchWindowSize, [&](uint32_t n, uint32_t) {
            BoxFilter(u[n], v[n], blockSize);
        });
        Thread::ParallelFor(0u, in.width * in.height, [&](uint32_t k, uint32_t) {
            Float weightSum = 0;
            Texel sum;
            auto p = mapU(k);
            for (int n = 0; n < searchWindowSize * searchWindowSize; n++) {
                Float weight = std::exp(-v[n][k] / (h * h));
                auto offset = mapS2(n);
                auto yn = in(p + offset);
                sum += in(p + offset) * weight;
                weightSum += weight;
            }
            if (weightSum != 0) {
                sum /= weightSum;
            }
            out[k] = sum;
        }, 4096);
    }
}

#endif //MIYUKI_NLM_H

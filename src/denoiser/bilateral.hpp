//
// Created by Shiina Miyuki on 2019/4/8.
//

#ifndef MIYUKI_BILATERAL_HPP
#define MIYUKI_BILATERAL_HPP

#include <io/image.h>

#include <utils/thread.h>
#include <denoiser/nlm.h>

namespace Miyuki {
    namespace Denoising {

        template<typename Texel>
        void GaussianBlur(const IO::GenericImage <Texel> &in,
                          IO::GenericImage <Texel> &out, Float sigma) {
            out = IO::GenericImage<Texel>(in.width, in.height);
            int w = std::ceil(6 * sigma) / 2;
            Thread::ParallelFor(0u, in.width, [&](uint32_t i, uint32_t) {
                for (int j = 0; j < in.height; j++) {
                    double weightSum = 0;
                    Texel sum;
                    for (int x = -w; x <= w; x++) {
                        for (int y = -w; y <= w; y++) {
                            double weight = std::exp(-(x * x + y * y) / (2 * sigma * sigma));
                            weightSum += weight;
                            sum += weight * in(i + x, j + y);
                        }
                    }
                    if (weightSum != 0)
                        sum /= weightSum;
                    out(i, j) = sum;
                }
            }, 32);
        }

        template<typename Texel>
        void Bilateral(const IO::GenericImage <Texel> &in,
                       IO::GenericImage <Texel> &out,
                       int windowSize,
                       Float sigma_d,
                       Float sigma_r) {
            out = IO::GenericImage<Texel>(in.width, in.height);
            int w = (windowSize - 1) / 2;
            Thread::ParallelFor(0u, in.width, [&](uint32_t i, uint32_t) {
                for (int j = 0; j < in.height; j++) {
                    double weightSum = 0;
                    Texel sum;
                    for (int x = -w; x <= w; x++) {
                        for (int y = -w; y <= w; y++) {
                            double weight = std::exp(
                                    -(x * x + y * y) / (2 * sigma_d * sigma_d)
                                    - NLMDistanceSqr(in(i, j), in(i + x, j + y)) / (2 * sigma_r * sigma_r)
                            );
                            weightSum += weight;
                            sum += weight * in(i + x, j + y);
                        }
                    }
                    if (weightSum != 0)
                        sum /= weightSum;
                    out(i, j) = sum;
                }
            }, 32);
        }
    }
}
#endif //MIYUKI_BILATERAL_HPP

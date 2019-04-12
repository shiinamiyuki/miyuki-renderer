//
// Created by Shiina Miyuki on 2019/4/8.
//

#include "denoiser.h"
#include <denoiser/nlm.h>
#include <utils/thread.h>

namespace Miyuki {


    struct DenoiserInternal {
        const Denoiser::FeatureBuffer *buffer;
        IO::Image color;
        IO::Image normal;
        IO::Image albedo;
        IO::Image colorVariance;

        DenoiserInternal(const Denoiser::FeatureBuffer *buffer)
                : buffer(buffer),
                  color(buffer->width, buffer->height),
                  normal(buffer->width, buffer->height),
                  albedo(buffer->width, buffer->height),
                  colorVariance(buffer->width, buffer->height) {
            auto &_buffer = *(this->buffer);
            for (int i = 0; i < _buffer.width; i++) {
                for (int j = 0; j < _buffer.height; j++) {
                    CHECK(_buffer(i, j).color.valid);
                    albedo(i, j) = _buffer(i, j).albedo.eval();
                    color(i, j) = _buffer(i, j).color.eval();
                    normal(i, j) = removeNaNs(_buffer(i, j).normal.eval().normalized());

                    colorVariance(i, j) = _buffer(i, j).color.variance.var();
                }
            }
        }

        void DenoisePass(IO::Image &out,
                         int searchWindowSize,
                         int blockSize,
                         Float alpha,
                         Float beta,
                         Float gamma) {
            out = IO::Image(color.width, color.height);
            const int S = (searchWindowSize - 1) / 2;
            auto mapS2 = [=](int i) {

                return Point2i(i % searchWindowSize, i / searchWindowSize) - Point2i(S, S);
            };

            auto mapU = [=](int i) {
                return Point2i(i % color.width, i / color.width);
            };
            Float sigma = 0;
            for (auto &i:colorVariance.pixelData) {
                sigma += i.max();
            }
            sigma /= colorVariance.pixelData.size();
            alpha *= alpha;
            beta *= beta;
            gamma *= gamma;
            Thread::ParallelFor(0u, color.width * color.height, [&](uint32_t k, uint32_t) {
                auto p = mapU(k);
                Spectrum sum;
                double weight, weightSum = 0;
                for (int n = 0; n < searchWindowSize * searchWindowSize; n++) {
                    auto offset = mapS2(n);
                    auto ykn = color(p + offset);
                    auto yk = color(p);
                    auto var = colorVariance(p).max() + 1e-5;
                    auto dist = Denoising::NLMDistanceSqr(ykn, yk) / (alpha * var);
                    dist += Denoising::NLMDistanceSqr(normal(p + offset), normal(p)) / beta;
                    dist += Denoising::NLMDistanceSqr(albedo(p + offset), albedo(p)) / gamma;
                    dist += (offset[0] * offset[0] + offset[1] * offset[1]) / (2 * sigma * sigma);
                    weight = std::exp(-dist);
                    sum += ykn * weight;
                    weightSum += weight;
                }
                CHECK(!std::isnan(weightSum));
                if (weightSum != 0)
                    sum /= weightSum;
                out(p) = sum;
            }, 4096);
        }
    };

    void Denoiser::denoise(Film &film) {
        CHECK(film.auxBuffer);
        if (!film.auxBuffer)return;
        FeatureBuffer &buffer = *film.auxBuffer;
        IO::Image out(buffer.width, buffer.height);
        DenoiserInternal(film.auxBuffer.get()).DenoisePass(out, 21, 5, 50, 0.1, 0.05);
        film.clear();

        for (int i = 0; i < out.width; i++) {
            for (int j = 0; j < out.height; j++) {
                film.addSample(Point2f(i, j), out(i, j));
            }
        }
    }
}
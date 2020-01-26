// MIT License
// 
// Copyright (c) 2019 椎名深雪
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#ifndef MIYUKIRENDERER_MLT_SAMPLER_H
#define MIYUKIRENDERER_MLT_SAMPLER_H

#include <miyuki.renderer/sampler.h>
#include <miyuki.foundation/rng.h>

namespace miyuki::core {
    class MLTSampler : public Sampler {
        Point2i dimension{};

        struct PrimarySample {
            Float value;
            Float _backup;
            uint64_t lastModificationIteration;
            uint64_t lastModifiedBackup;

            void backup() {
                _backup = value;
                lastModifiedBackup = lastModificationIteration;
            }

            void restore() {
                value = _backup;
                lastModificationIteration = lastModifiedBackup;
            }
        };

        Rng rng;
        uint32_t sampleIndex = 0;
        uint64_t currentIteration = 0;
        uint64_t lastLargeStepIteration = 0;
        bool largeStep{};
        Float largeStepProb{};
        std::vector<PrimarySample> X;

    public:
        MYK_DECL_CLASS(MLTSampler, "MLTSampler", interface = "Sampler")

        MLTSampler() = default;

        MLTSampler(Float largeStepProb, const Point2i &filmDimension, const uint64_t &seed)
                : largeStepProb(largeStepProb), dimension(filmDimension), rng(seed) {}

        void startPixel(const Point2i &i, const Point2i &filmDimension) override {
            dimension = filmDimension;
        }

        Float next1D() override {
            if (sampleIndex >= X.size()) {
                X.resize(sampleIndex + 1u);
            }
            auto &Xi = X[sampleIndex];
            mutate(Xi, sampleIndex);
            sampleIndex += 1;
            return Xi.value;
        }

        void mutate(PrimarySample &Xi, int index) {
            double s1, s2;
            if (index >= 2) {
                s1 = 1.0 / 1024.0, s2 = 1.0 / 64.0;
            } else if (index == 1) {
                s1 = 1.0 / dimension[1], s2 = 0.1;
            } else {
                s1 = 1.0 / dimension[0], s2 = 0.1;
            }
            if (Xi.lastModificationIteration < lastLargeStepIteration) {
                Xi.value = rng.uniformFloat();
                Xi.lastModificationIteration = lastLargeStepIteration;
            }

            if (largeStep) {
                Xi.backup();
                Xi.value = rng.uniformFloat();
            } else {
                int64_t nSmall = currentIteration - Xi.lastModificationIteration;

                auto nSmallMinus = nSmall - 1;
                if (nSmallMinus > 0) {
                    auto x = Xi.value;
                    while (nSmallMinus > 0) {
                        nSmallMinus--;
                        x = mutate(x, s1, s2);
                    }
                    Xi.value = x;
                    Xi.lastModificationIteration = currentIteration - 1;
                }
                Xi.backup();
                Xi.value = mutate(Xi.value, s1, s2);
            }

            Xi.lastModificationIteration = currentIteration;
        }

        Float mutate(Float x, Float s1, Float s2) {
            auto r = rng.uniformFloat();
            if (r < 0.5) {
                r = r * 2.0;
                x = x + s2 * exp(-log(s2 / s1) * r);
                if (x > 1.0) x -= 1.0;
            } else {
                r = (r - 0.5) * 2.0;
                x = x - s2 * exp(-log(s2 / s1) * r);
                if (x < 0.0) x += 1.0;
            }
            return x;
        }

        void startNextSample() override {
            sampleIndex = 0;
            currentIteration++;
            largeStep = rng.uniformFloat() < largeStepProb;
        }

        [[nodiscard]] std::shared_ptr<Sampler> clone() const override {
            return std::make_shared<MLTSampler>(*this);
        }

    };
}
#endif //MIYUKIRENDERER_MLT_SAMPLER_H

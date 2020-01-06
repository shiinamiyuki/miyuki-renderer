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

#ifndef MIYUKIRENDERER_MIXBSDF_TEMPLATE_H
#define MIYUKIRENDERER_MIXBSDF_TEMPLATE_H

#include <miyuki.renderer/bsdf.h>
#include <miyuki.renderer/shader.h>

namespace miyuki::core {
    template<class BSDFA, class BSDFB>
    struct TMixBSDF : public BSDF {
        std::shared_ptr<BSDFA> bsdfA;
        std::shared_ptr<BSDFB> bsdfB;
        std::shared_ptr<Shader> fraction;

    public:
        TMixBSDF() = default;

        TMixBSDF(const std::shared_ptr<Shader> &fraction,
                 const std::shared_ptr<BSDFA> &bsdfA,
                 const std::shared_ptr<BSDFB> &bsdfB) : fraction(fraction), bsdfA(bsdfA), bsdfB(bsdfB) {}

        MYK_DECL_CLASS(MixBSDF, "MixBDSF", interface = "BSDF")

        MYK_SER(fraction, bsdfA, bsdfB)

        [[nodiscard]] BSDF::Type getBSDFType() const override {
            return BSDF::Type(bsdfA->getBSDFType() | bsdfB->getBSDFType());
        }

        [[nodiscard]] Spectrum evaluate(const ShadingPoint &sp, const Vec3f &wo, const Vec3f &wi) const override {
            auto fA = bsdfA->evaluate(sp, wo, wi);
            auto fB = bsdfB->evaluate(sp, wo, wi);
            auto frac = fraction->evaluate(sp)[0];
            MIYUKI_CHECK(minComp(fA) >= 0.0f);
            MIYUKI_CHECK(minComp(fB) >= 0.0f);
            MIYUKI_CHECK(frac >= 0.0f && frac <= 1.0f);
            return lerp<Spectrum>(fA, fB, Vec3f(frac));
        }

        [[nodiscard]] Float evaluatePdf(const ShadingPoint &sp, const Vec3f &wo, const Vec3f &wi) const override {
            auto pdfA = bsdfA->evaluatePdf(sp, wo, wi);
            auto pdfB = bsdfB->evaluatePdf(sp, wo, wi);
            auto frac = fraction->evaluate(sp)[0];
            MIYUKI_CHECK(pdfA >= 0.0f);
            MIYUKI_CHECK(pdfB >= 0.0f);
            MIYUKI_CHECK(frac >= 0.0f && frac <= 1.0f);
            return lerp(pdfA, pdfB, frac);
        }

        void sample(Point2f u, const ShadingPoint &, BSDFSample &sample) const override {
            auto frac = fraction->evaluate(sp)[0];
            MIYUKI_CHECK(frac >= 0.0);
            BSDF *first, *second;
            auto p = 1.0f - frac;
            if (u[0] < p) {
                u[0] /= p;
                first = bsdfA.get();  // p = 1 - frac
                second = bsdfB.get();
            } else {
                u[0] = (u[0] - p) / (1.0f - p);
                first = bsdfB.get();
                second = bsdfA.get();
                frac = 1.0f - frac;
            }
            first->sample(u, sp, sample);
            MIYUKI_CHECK(0 <= frac && frac <= 1);
            MIYUKI_CHECK(minComp(sample.f) >= 0.0f);
            // evaluate whole bsdf if not sampled specular
            if ((sample.sampledType & BSDF::ESpecular) == 0) {
                sample.f = lerp<Spectrum>(sample.f, second->evaluate(sp, sample.wo, sample.wi), Vec3f(frac));
                sample.pdf = lerp<Float>(sample.pdf, second->evaluatePdf(sp, sample.wo, sample.wi), frac);
            }
        }

        void preprocess() override {
            bsdfA->preprocess();
            bsdfB->preprocess();
            fraction->preprocess();
        }
    };
}


#endif //MIYUKIRENDERER_MIXBSDF_TEMPLATE_H

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
#include "mixbsdf.h"
#include <miyuki.foundation/log.hpp>

namespace miyuki::core {

    Spectrum MixBSDF::evaluate(const ShadingPoint &sp, const Vec3f &wo, const Vec3f &wi) const {
        return lerp<Spectrum>(bsdfA->evaluate(sp, wo, wi),
                              bsdfB->evaluate(sp, wo, wi),
                              vec3(fraction->evaluate(sp)[0]));
    }

    Float MixBSDF::evaluatePdf(const ShadingPoint &sp, const Vec3f &wo, const Vec3f &wi) const {
        return lerp(bsdfA->evaluatePdf(sp, wo, wi),
                    bsdfB->evaluatePdf(sp, wo, wi),
                    (float) fraction->evaluate(sp)[0]);
    }

    void MixBSDF::sample(Point2f u, const ShadingPoint &sp, BSDFSample &sample) const {
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

        // evaluate whole bsdf if not sampled specular
        if ((sample.sampledType & BSDF::ESpecular) == 0) {
            sample.f = lerp<Spectrum>(sample.f, second->evaluate(sp, sample.wo, sample.wi), vec3(frac));
            sample.pdf = lerp<Float>(sample.pdf, second->evaluatePdf(sp, sample.wo, sample.wi), frac);
        }
    }

    BSDF::Type MixBSDF::getBSDFType() const { return BSDF::Type(bsdfA->getBSDFType() | bsdfB->getBSDFType()); }

    void MixBSDF::preprocess() {
        bsdfA->preprocess();
        bsdfB->preprocess();
        fraction->preprocess();
    }

} // namespace miyuki::core
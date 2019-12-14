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

namespace miyuki::core {

    Spectrum MixBSDF::evaluate(const ShadingPoint &sp, const Vec3f &wo, const Vec3f &wi) const {
        return lerp<Spectrum>(bsdfA->evaluate(sp, wo, wi), bsdfB->evaluate(sp, wo, wi), vec3(1) - fraction->evaluate(sp));
    }
    Float MixBSDF::evaluatePdf(const ShadingPoint &sp, const Vec3f &wo, const Vec3f &wi) const {
        return lerp(bsdfB->evaluatePdf(sp, wo, wi), bsdfA->evaluatePdf(sp, wo, wi), 1.0f - (float)fraction->evaluate(sp)[0]);
    }

    void MixBSDF::sample(Point2f u, const ShadingPoint &sp, BSDFSample &sample) const {
        auto frac = fraction->evaluate(sp)[0];
        BSDF *first, *second;
        if (u[0] < frac) {
            u[0] /= frac;
            first = bsdfA.get();
            second = bsdfB.get();
            frac = u[0];
        } else {
            u[0] = (u[0] - frac) / (1.0f - frac);
            first = bsdfB.get();
            second = bsdfA.get();
            frac = 1.0f - u[0];
        }
        first->sample(u, sp, sample);

        // evaluate whole bsdf if not sampled specular
        if ((sample.sampledType & BSDF::ESpecular) == 0) {
            sample.f = lerp<Spectrum>(second->evaluate(sp, sample.wo, sample.wi), sample.f, vec3(1) - Vec3f(frac));
            sample.pdf = lerp<Float>(second->evaluatePdf(sp, sample.wo, sample.wi), sample.pdf, 1 - frac);
        }
    }

    BSDF::Type MixBSDF::getBSDFType() const { return BSDF::Type(bsdfA->getBSDFType() | bsdfB->getBSDFType()); }

} // namespace miyuki::core
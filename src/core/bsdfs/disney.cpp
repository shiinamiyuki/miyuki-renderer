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

#include "disney.h"
#include <miyuki.renderer/funcs.h>
#include <miyuki.renderer/trignometry.hpp>
namespace miyuki::core {
    static inline Spectrum
    DisneyDiffuse(const Spectrum &baseColor, Float roughness, Float cosThetaH, Float cosThetaO, Float cosThetaI) {
        auto FD90 = 0.5f + 2.0f * powN<2>(cosThetaH) * roughness;
        return baseColor * InvPi
               * (1.0f + (FD90 - 1.0f) * powN<5>(1.0f - cosThetaI)) *
               (1.0f + (FD90 - 1.0f) * powN<5>(1.0f - cosThetaO));
    }

    static inline Float DisneyFrSchlick(Float F0, Float cosThetaH){
        return F0 + (1.0f - F0) * powN<5>(1.0f - cosThetaH);
    }

    static inline float DisneyDGTR(Float alpha, const Vec3f & wi, const Vec3f & wh){
        auto inv = Pi * alpha * alpha * powN<2>(Sin2Theta(wh) * (1.0f / powN<2>(alpha)) + Cos2Theta(wh));
        return 1.0f / inv;
    }

    static inline float DisneyG1(float alpha, const Vec3f & w){
        auto lambda = -0.5f + 0.5f * std::sqrt(1.0f + powN<2>(alpha) * Tan2Theta(w));
        return 1.0f / (1.0f +  lambda);
    }

    static inline float DisneyG(float alpha, const Vec3f & wo, const Vec3f & wi){
        return DisneyG1(alpha, wo) * DisneyG1(alpha, wi);
    }
    BSDF::Type DisneyBSDF::getBSDFType() const {
        return EAllButSpecular;
    }

    Spectrum DisneyBSDF::evaluate(const ShadingPoint &point, const Vec3f &wo, const Vec3f &wi) const {
        return miyuki::core::Spectrum();
    }

    Float DisneyBSDF::evaluatePdf(const ShadingPoint &point, const Vec3f &wo, const Vec3f &wi) const {
        return 0;
    }

    void DisneyBSDF::sample(Point2f u, const ShadingPoint &point, BSDFSample &sample) const {

    }

    void DisneyBSDF::preprocess() {

    }
}
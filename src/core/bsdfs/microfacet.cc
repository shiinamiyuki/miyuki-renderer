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
#include "microfacet.h"
#include <miyuki.renderer/shader.h>
#include <miyuki.renderer/trignometry.hpp>

namespace miyuki::core {
    static float SchlickWeight(float cosTheta) {
        float m = std::clamp(1.0 - cosTheta, 0.0, 1.0);
        return (m * m) * (m * m) * m;
    }
    static float Schlick(float R0, float cosTheta) { return lerp(R0, 1.0f, SchlickWeight(cosTheta)); }
    static float GGX_D(float alpha, const Vec3f &m) {
        if (m.y <= 0.0f)
            return 0.0f;
        float a2 = alpha * alpha;
        float c2 = Cos2Theta(m);
        float t2 = Tan2Theta(m);
        float at = (a2 + t2);
        return a2 / (Pi * c2 * c2 * at * at);
    }
    static float GGX_G1(float alpha, const Vec3f &v, const Vec3f &m) {
        if (dot(v,m) * v.y <= 0.0f) {
            return 0.0f;
        }
        return 2.0 / (1.0 + sqrt(1.0 + alpha * alpha * Tan2Theta(m)));
    }
    static float GGX_G(float alpha, const Vec3f &i, const Vec3f &o, const Vec3f &m) {
        return GGX_G1(alpha, i, m) * GGX_G1(alpha, o, m);
    }
    static Vec3f GGX_SampleWh(float alpha, const Vec3f &wo, const Point2f &u) {
        float phi = 2.0 * Pi * u.y;
        float t2 = alpha * alpha * u.x / (1.0 - u.x);
        float cosTheta = 1.0f / sqrt(1.0 + t2);
        float sinTheta = sqrt(std::fmax(0.0f, 1.0 - cosTheta * cosTheta));
        return Vec3f(cos(phi) * sinTheta, cosTheta, sin(phi) * sinTheta);
    }
    Spectrum MicrofacetBSDF::evaluate(const ShadingPoint &point, const Vec3f &wo, const Vec3f &wi) const {
        if (!SameHemisphere(wo, wi)) {
            return Spectrum(0);
        }
        float cosThetaO = AbsCosTheta(wo);
        float cosThetaI = AbsCosTheta(wi);
        Vec3f wh = (wo + wi);
        if (cosThetaI == 0 || cosThetaO == 0)
            return Spectrum(0);
        if (wh.x == 0 && wh.y == 0 && wh.z == 0)
            return Spectrum(0);
        wh = normalize(wh);
        float F = 1.0; // Schlick(0.4f, abs(dot(wi, wh)));
        auto R = color->evaluate(point);
        auto alpha = roughness->evaluate(point).x;
        alpha *= alpha;
        return max(Spectrum(0), R * F * GGX_D(alpha, wh) * GGX_G(alpha, wo, wi, wh) / (4.0f * cosThetaI * cosThetaO));
    }

    void MicrofacetBSDF::sample(Point2f u, const ShadingPoint &sp, BSDFSample &sample) const {
        auto alpha = roughness->evaluate(sp).x;
        alpha *= alpha;
        Normal3f wh = GGX_SampleWh(alpha, sample.wo, u);
        sample.wi = Reflect(sample.wo, wh);
        sample.f = evaluate(sp, sample.wo, sample.wi);
        sample.pdf = evaluatePdf(sp, sample.wo, sample.wi);
    }

    Float MicrofacetBSDF::evaluatePdf(const ShadingPoint &point, const Vec3f &wo, const Vec3f &wi) const {
        if (!SameHemisphere(wo, wi)) {
            return 0.0f;
        }
        auto wh = normalize(wo + wi);
        auto alpha = roughness->evaluate(point).x;
        alpha *= alpha;
        return GGX_D(alpha, wh) * AbsCosTheta(wh);
    }

} // namespace miyuki::core
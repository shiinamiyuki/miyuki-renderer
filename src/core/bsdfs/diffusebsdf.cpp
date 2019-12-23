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

#include "diffusebsdf.h"
#include <miyuki.renderer/shader.h>
#include <miyuki.renderer/sampling.h>

namespace miyuki::core {
    Spectrum DiffuseBSDF::evaluate(const ShadingPoint &point, const Vec3f &wo, const Vec3f &wi) const {
        if (wo.y * wi.y > 0)
            return Spectrum(color->evaluate(point) * InvPi);
        return {};
    }

    void DiffuseBSDF::sample(Point2f u, const ShadingPoint &sp, BSDFSample &sample) const {
        sample.wi = CosineHemisphereSampling(u);
        sample.sampledType = BSDF::Type(sample.sampledType | getBSDFType());
        if (sample.wo.y * sample.wi.y < 0) {
            sample.wi.y = -sample.wi.y;
        }
        sample.pdf = std::abs(sample.wi.y) * InvPi;
        sample.f = evaluate(sp, sample.wo, sample.wi);
        sample.sampledType = BSDF::Type(int(BSDF::Type::EReflection) | int(BSDF::Type::EDiffuse));
    }

    Float DiffuseBSDF::evaluatePdf(const ShadingPoint &point, const Vec3f &wo, const Vec3f &wi) const {
        if (wo.y * wi.y > 0)
            return std::abs(wi.y) * InvPi;
        return 0;
    }
    void DiffuseBSDF::preprocess() {
        color->preprocess();
    }
}
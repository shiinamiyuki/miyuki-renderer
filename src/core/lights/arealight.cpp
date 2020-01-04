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

#include "arealight.h"
#include <miyuki.renderer/mesh.h>

namespace miyuki::core {
    void AreaLight::setTriangle(MeshTriangle *shape) {
        this->triangle = shape;
        emission = triangle->getMaterial()->emission;
        emissionStrength = triangle->getMaterial()->emissionStrength;
    }

    Spectrum AreaLight::Li(ShadingPoint &sp) const { return emission->evaluate(sp) * emissionStrength->evaluate(sp); }

    void AreaLight::sampleLi(const Point2f &u, Intersection &isct, LightSample &sample,
                             VisibilityTester &tester) const {
        SurfaceSample surfaceSample;
        triangle->sample(u, surfaceSample);
        auto wi = surfaceSample.p - isct.p;
        auto dist2 = dot(wi,wi);
        auto dist = std::sqrt(dist2);
        wi /= dist;
        tester.shadowRay = Ray(surfaceSample.p, -1.0f * wi, RayBias / abs(dot(sample.wi, surfaceSample.normal)), dist * 0.99);
        tester.target = isct.shape;
        ShadingPoint sp;
        sp.Ng = triangle->Ng();
        sp.Ns = triangle->normalAt(surfaceSample.uv);
        sp.texCoord = triangle->texCoordAt(surfaceSample.uv);
        sample.Li = Li(sp);
        sample.wi = wi;
        sample.pdf = dist2 / (-dot(sample.wi, surfaceSample.normal)) * surfaceSample.pdf;
    }

    Float AreaLight::pdfLi(const Intersection &intersection, const Vec3f &wi) const {
        Intersection _isct;
        Ray ray(intersection.p, wi, RayBias);
        if (!triangle->intersect(ray, _isct)) {
            return 0.0f;
        }
        Float SA = triangle->area() * (-dot(wi,_isct.Ng)) / (_isct.distance * _isct.distance);
        return 1.0f / SA;
    }

    void AreaLight::sampleLe(const Point2f &u1, const Point2f &u2, LightRaySample &sample) { MIYUKI_NOT_IMPLEMENTED(); }
} // namespace miyuki::core
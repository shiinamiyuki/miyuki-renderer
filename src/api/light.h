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

#ifndef MIYUKIRENDERER_LIGHT_H
#define MIYUKIRENDERER_LIGHT_H

#include <api/spectrum.h>
#include <api/ray.h>
#include <api/entity.hpp>

namespace miyuki::core {
    struct VisibilityTester;
    struct ShadingPoint;

    class Shape;

    struct LightSample {
        Vec3f wi;
        Spectrum Li;
        float pdf;
    };

    struct LightRaySample {
        Ray ray;
        Spectrum Le;
        float pdfPos, pdfDir;
    };

    class Light : public Entity {
    public:
        virtual Spectrum Li(ShadingPoint &sp) const = 0;

        virtual void sampleLi(const Point2f &u, Intersection &isct, LightSample &sample, VisibilityTester &) const = 0;

        virtual Float pdfLi(const Intersection &intersection, const Vec3f &wi) const = 0;

        virtual void sampleLe(const Point2f &u1, const Point2f &u2, LightRaySample &sample) = 0;

    };

    class AreaLight : public Light {
    public:
        virtual void setShape(Shape *shape) = 0;
    };
}
#endif //MIYUKIRENDERER_LIGHT_H

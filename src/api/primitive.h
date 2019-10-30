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

#ifndef MIYUKIRENDERER_PRIMITIVE_H
#define MIYUKIRENDERER_PRIMITIVE_H

#include <api/spectrum.h>
#include <api/entity.hpp>
#include <api/ray.h>

namespace miyuki::core {
    class AreaLight;

    struct SurfaceSample {
        Point3f p;
        Float pdf;
        Normal3f normal;
    };


    class Primitive : public Entity {
    public:
        virtual bool intersect(const Ray &ray, Intersection &isct) const = 0;

        [[nodiscard]] virtual Bounds3f getBoundingBox() const = 0;

        [[nodiscard]] virtual AreaLight *getAreaLight() const { return nullptr; }

        virtual void sample(const Point2f &u, SurfaceSample &sample) const = 0;

        [[nodiscard]] virtual Float area() const = 0;
    };
}
#endif //MIYUKIRENDERER_PRIMITIVE_H

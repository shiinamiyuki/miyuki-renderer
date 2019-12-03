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

#ifndef MIYUKIRENDERER_AREALIGHT_H
#define MIYUKIRENDERER_AREALIGHT_H

#include <miyuki.renderer/light.h>
#include <miyuki.renderer/shader.h>
#include <miyuki.foundation/serialize.hpp>

namespace miyuki::core {
    class AreaLight final: public Light {
        MeshTriangle *triangle = nullptr;
        std::shared_ptr<Shader> emission;
    public:
        MYK_DECL_CLASS(AreaLight, "AreaLight", interface = "Light")

        Spectrum Li(ShadingPoint &sp) const override;

        void
        sampleLi(const Point2f &u, Intersection &isct, LightSample &sample, VisibilityTester &tester) const override;

        Float pdfLi(const Intersection &intersection, const Vec3f &wi) const override;

        void sampleLe(const Point2f &u1, const Point2f &u2, LightRaySample &sample) override;

        void setTriangle(MeshTriangle *shape);
    };
}
#endif //MIYUKIRENDERER_AREALIGHT_H

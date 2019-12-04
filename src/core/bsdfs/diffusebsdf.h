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

#ifndef MIYUKIRENDERER_DIFFUSEBSDF_H
#define MIYUKIRENDERER_DIFFUSEBSDF_H

#include <miyuki.renderer/bsdf.h>
#include <miyuki.foundation/serialize.hpp>

namespace miyuki::core {
    class Shader;

    class DiffuseBSDF final : public BSDF {
        std::shared_ptr<Shader> color;
    public:
        MYK_DECL_CLASS(DiffuseBSDF, "DiffuseBSDF", interface = "BSDF")

        MYK_AUTO_SER(color)

        MYK_AUTO_INIT(color)

        DiffuseBSDF() = default;

        explicit DiffuseBSDF(const std::shared_ptr<Shader> &color) : color(color) {}


        [[nodiscard]] Spectrum evaluate(const ShadingPoint &point, const Vec3f &wo, const Vec3f &wi) const override;

        void sample(Point2f u, const ShadingPoint &sp, BSDFSample &sample) const override;

        [[nodiscard]] Float evaluatePdf(const ShadingPoint &point, const Vec3f &wo, const Vec3f &wi) const override;

        [[nodiscard]] Type getBSDFType() const override {
            return Type(EDiffuse | EReflection);
        }

    };
}

#endif //MIYUKIRENDERER_DIFFUSEBSDF_H

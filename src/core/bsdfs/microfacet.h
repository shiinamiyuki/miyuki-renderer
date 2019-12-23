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
#pragma once

#include <miyuki.renderer/bsdf.h>
#include <miyuki.foundation/interfaces.h>

namespace miyuki::core {
    class Fresnel;
    class Shader;

	/*GGX microfacet reflection*/
    class MicrofacetBSDF final : public BSDF {
        std::shared_ptr<Shader> color, roughness;

      public:
        MYK_DECL_CLASS(MicrofacetBSDF, "MicrofacetBSDF", interface = "BSDF")

        MYK_SER(color, roughness)


        MicrofacetBSDF() = default;

        MicrofacetBSDF(const std::shared_ptr<Shader>& color, const std::shared_ptr<Shader> & roughness):color(color),roughness(roughness){}

        [[nodiscard]] Spectrum evaluate(const ShadingPoint &point, const Vec3f &wo, const Vec3f &wi) const override;

        void sample(Point2f u, const ShadingPoint &sp, BSDFSample &sample) const override;

        [[nodiscard]] Float evaluatePdf(const ShadingPoint &point, const Vec3f &wo, const Vec3f &wi) const override;

        [[nodiscard]] Type getBSDFType() const override { return Type(EGlossy | EReflection); }

        void preprocess()override ;
    };
} // namespace miyuki::core
#pragma once

#include <api/bsdf.h>
#include <api/serialize.hpp>


namespace miyuki::core {
    class Fresnel;
    class Shader;

	/*GGX microfacet reflection*/
    class MicrofacetBSDF final : public BSDF {
        std::shared_ptr<Shader> color, roughness;

      public:
        MYK_DECL_CLASS(MicrofacetBSDF, "MicrofacetBSDF", interface = "BSDF")

        MYK_AUTO_SER(color, roughness)

        MYK_AUTO_INIT(color, roughness)

        MicrofacetBSDF() = default;

        [[nodiscard]] Spectrum evaluate(const ShadingPoint &point, const Vec3f &wo, const Vec3f &wi) const override;

        void sample(Point2f u, const ShadingPoint &sp, BSDFSample &sample) const override;

        [[nodiscard]] Float evaluatePdf(const ShadingPoint &point, const Vec3f &wo, const Vec3f &wi) const override;

        [[nodiscard]] Type getBSDFType() const override { return Type(EGlossy | EReflection); }
    };
} // namespace miyuki::core
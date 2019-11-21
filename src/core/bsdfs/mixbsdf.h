#pragma once

#include <api/bsdf.h>
#include <api/property.hpp>
#include <api/serialize.hpp>
#include <api/shader.h>

namespace miyuki::core {
    class MixBSDF final : public BSDF {
        std::shared_ptr<BSDF> bsdfA, bsdfB;
        std::shared_ptr<Shader> fraction;

      public:
        MYK_DECL_CLASS(MixBSDF, "MixBDSF", interface = "BSDF")

        MYK_AUTO_SER(fraction, bsdfA, bsdfB)

        MYK_AUTO_INIT(fraction, bsdfA, bsdfB)

        MYK_PROP(fraction, bsdfA, bsdfB)

        [[nodiscard]] virtual Type getBSDFType() const override;

        [[nodiscard]] virtual Spectrum evaluate(const ShadingPoint &, const Vec3f &wo, const Vec3f &wi) const override;

        [[nodiscard]] virtual Float evaluatePdf(const ShadingPoint &, const Vec3f &wo, const Vec3f &wi) const override;

        virtual void sample(Point2f u, const ShadingPoint &, BSDFSample &sample) const override;
    };
} // namespace miyuki::core
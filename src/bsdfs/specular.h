//
// Created by Shiina Miyuki on 2019/2/10.
//

#ifndef MIYUKI_SPECULAR_H
#define MIYUKI_SPECULAR_H

#include "bsdf.h"
#include "fresnel.h"

namespace Miyuki {
    template<typename T>
    class SpecularBSDF : public BSDF {
    public:
        static_assert(std::is_base_of<Fresnel, T>::value,
                      "U is not derived from Fresnel");
        T fresnel;

        SpecularBSDF(const T &fresnel, const ColorMap &albedo, const ColorMap &bump = ColorMap())
                : BSDF(BSDFType((int) BSDFType::specular | (int) BSDFType::reflection),
                       albedo, bump),
                  fresnel(fresnel) {}

    protected:
        Spectrum f(const ScatteringEvent &event) const override {
            return {0, 0, 0};
        }

    public:
        Spectrum sample(ScatteringEvent &event) const override {
            event.setWi(Vec3f(-event.wo.x(), event.wo.y(), -event.wo.z()));
            event.pdf = 1;
            return Spectrum(fresnel.eval(cosTheta(event.wi)) * evalAlbedo(event) / absCosTheta(event.wi));
        }

        Float pdf(const Vec3f &wo, const Vec3f &wi) const override {
            return 0.0f;
        }
    };
}
#endif //MIYUKI_SPECULAR_H

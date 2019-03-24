//
// Created by Shiina Miyuki on 2019/3/8.
//

#include "specular.h"
#include "core/scatteringevent.h"

namespace Miyuki {

    Spectrum SpecularReflection::f(const ScatteringEvent &event) const {
        return {};
    }

    Float SpecularReflection::pdf(const ScatteringEvent &event) const {
        return 0;
    }

    Spectrum SpecularReflection::sample(ScatteringEvent &event) const {
        event.setWi(Vec3f{-event.wo.x(), event.wo.y(), -event.wo.z()});
        event.pdf = 1;
        event.bsdfLobe = lobe;
        return R / Vec3f::absDot(event.wiW, event.Ns());
    }

    Point2f SpecularReflection::invert(const Vec3f &wo, const Vec3f &wi) const {
        return Miyuki::Point2f();
    }

    Spectrum SpecularTransmission::sample(ScatteringEvent &event) const {
        bool entering = CosTheta(event.wo) > 0;
        Float etaI = entering ? etaA : etaB;
        Float etaT = entering ? etaB : etaA;
        auto n = Vec3f{0, 1, 0};
        bool reflect = false;
        if (Vec3f::dot(n, event.wo) < 0) {
            n *= -1;
        }
        Vec3f wi;
        if (!Refract(event.wo, n, etaI / etaT, &wi)) {
            reflect = true;
        }
        auto reflective = dielectric.eval(CosTheta(wi));
        auto T = 1 - reflective.max();
        if (!reflect) {
            event.setWi(wi);
            event.pdf = 1;
            Spectrum ft = T * R;
            ft /= AbsCosTheta(event.wi);
            event.bsdfLobe = BSDFLobe::transmission | BSDFLobe::specular;
            return ft;
        } else {
            event.setWi(Vec3f(-event.wo.x(), event.wo.y(), -event.wo.z()));
            event.pdf = 1;
            event.bsdfLobe = BSDFLobe::reflection | BSDFLobe::specular;
            return reflective * R / AbsCosTheta(event.wi);
        }
    }

    Point2f SpecularTransmission::invert(const Vec3f &wo, const Vec3f &wi) const {
        return Miyuki::Point2f();
    }
}
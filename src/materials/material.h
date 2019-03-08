//
// Created by Shiina Miyuki on 2019/2/28.
//

#ifndef MIYUKI_MATERIAL_H
#define MIYUKI_MATERIAL_H

#include "miyuki.h"
#include "core/spectrum.h"
#include "core/rendercontext.h"
#include "core/scatteringevent.h"

namespace Miyuki {
    struct Texture {
        Spectrum albedo;

        Texture() {}

        Texture(const Spectrum &albedo) : albedo(albedo) {}
    };
    namespace IO {
        template<>
        inline Texture deserialize<Texture>(const Json::JsonObject &object) {
            return std::move(Texture(deserialize<Vec3f>(object["albedo"])));
        }
    }
    struct MaterialInfo {
        Texture ka;
        Texture kd;
        Texture ks;
        Float roughness = 0.0f;
        Float alphaX = 0.0001f, alphaY = 0.0001f;
        Float Ni = 1.0f;
        Float Tr = 0.0f;

        MaterialInfo() {}
    };

    class Material {
    public:
        Texture emission;

        Float emissionStrength() const;

        Material(const Texture &emission) : emission(emission) {}

        virtual void computeScatteringFunction(RenderContext &ctx, ScatteringEvent &event) const = 0;

        virtual ~Material() {}
    };

    class PBRMaterial : public Material {
        MaterialInfo info;
    public:
        PBRMaterial(const MaterialInfo &info) : info(info), Material(info.ka) {}

        void computeScatteringFunction(RenderContext &ctx, ScatteringEvent &event) const override;
    };
}
#endif //MIYUKI_MATERIAL_H

//
// Created by Shiina Miyuki on 2019/2/28.
//

#ifndef MIYUKI_MATERIAL_H
#define MIYUKI_MATERIAL_H

#include "miyuki.h"
#include "core/spectrum.h"
#include "core/rendercontext.h"
#include "core/scatteringevent.h"
#include "core/texture.h"
#include <io/serialize.h>

namespace Miyuki {

    struct MaterialInfo {
        Texture ka;
        Texture kd;
        Texture ks;
        Float roughness = 0.0f;
        Float alphaX = 0.0001f, alphaY = 0.0001f;
        Float Ni = 1.0f;
        Float Tr = 0.0f;
        Float sigma = 0.0f;
        Float emission = 1;// emission strength
        MaterialInfo() {}
    };

    namespace IO {
        template<>
        inline Json::JsonObject serialize<Texture>(const Texture &texture) {
            auto result = Json::JsonObject::makeObject();
            result["albedo"] = serialize<Spectrum>(texture.albedo);
            result["texture"] = texture.image ? texture.image->filename : "";
            return result;
        }

        template<>
        inline Json::JsonObject serialize<MaterialInfo>(const MaterialInfo &info) {
            auto result = Json::JsonObject::makeObject();
            result["ka"] = serialize(info.ka);
            result["kd"] = serialize(info.kd);
            result["ks"] = serialize(info.ks);
            result["Tr"] = serialize(info.Tr);
            result["roughness"] = serialize(info.roughness);
            result["alphaX"] = serialize(info.alphaX);
            result["alphaY"] = serialize(info.alphaY);
            result["Ni"] = serialize(info.Ni);
            result["emission"] = serialize(info.emission);
            return result;
        }
    }
    class Material {
    public:
        Texture emission;

        Float emissionStrength() const;

        Material(const Texture &emission) : emission(emission) {}

        virtual void computeScatteringFunction(RenderContext &ctx, ScatteringEvent &event) const = 0;

        // used for preview
        virtual Spectrum albedo(ScatteringEvent &event) const = 0;

        virtual ~Material() {}

        virtual Json::JsonObject toJson() const = 0;
    };

    class PBRMaterial : public Material {
        MaterialInfo info;
    public:
        PBRMaterial(const MaterialInfo &info) : info(info), Material(info.ka) {}

        void computeScatteringFunction(RenderContext &ctx, ScatteringEvent &event) const override;

        Json::JsonObject toJson() const override;

        Spectrum albedo(ScatteringEvent &event) const override {
            return info.ks.evalUV(event.textureUV()) + info.kd.evalUV(event.textureUV())
                + info.ka.evalUV(event.textureUV());
        }
    };
}
#endif //MIYUKI_MATERIAL_H

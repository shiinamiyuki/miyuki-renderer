//
// Created by Shiina Miyuki on 2019/1/19.
//

#ifndef MIYUKI_MATERIAL_H
#define MIYUKI_MATERIAL_H

#include "util.h"
#include "geometry.h"
#include "spectrum.h"
#include "sampler.h"
#include "interaction.h"
#include "texture.h"
#include "reflection.h"
#include "memory.h"

namespace Miyuki {

    class BSDF;

    struct Interaction;
    enum class TransportMode {
        radiance, importance
    };

    struct ColorMap {
        Spectrum color;
        Float maxReflectance;

        std::shared_ptr<TextureMapping2D> mapping;

        ColorMap(Float x, Float y, Float z, std::shared_ptr<TextureMapping2D> m = nullptr) : color(x, y, z),
                                                                                             mapping(m) {
            maxReflectance = color.max();
        }

        ColorMap(const Spectrum &s, std::shared_ptr<TextureMapping2D> m = nullptr)
                : color(s), mapping(m) {
            maxReflectance = s.max();
        }
    };

    struct MaterialInfo {
        ColorMap ka, kd, ks;
        Float Ns;
        Float Ni;
        Float Tr;
        Float glossiness;

        MaterialInfo(const ColorMap &ka, const ColorMap &kd, const ColorMap &ks) : ka(ka), kd(kd), ks(ks) {}
    };

    class Material;

    class Material {
    protected:
        MaterialInfo materialInfo;
    public:
        Material(const MaterialInfo &info) : materialInfo(info) {}

        ColorMap Ka() const { return materialInfo.ka; }

        virtual void computeScatteringFunctions(MemoryArena &arena, Interaction &) const = 0;
    };

    inline Vec3f reflect(const Vec3f &normal, const Vec3f &i) {
        return i - 2 * Vec3f::dot(normal, i) * normal;
    }

    using MaterialPtr = std::shared_ptr<Material>;

    class MaterialFactory {
    public:
        virtual MaterialPtr operator() (const MaterialInfo &) = 0;
    };
}
#endif //MIYUKI_MATERIAL_H

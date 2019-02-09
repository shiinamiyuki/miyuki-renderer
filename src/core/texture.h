//
// Created by Shiina Miyuki on 2019/1/26.
//

#ifndef MIYUKI_TEXTURE_H
#define MIYUKI_TEXTURE_H

#include "../utils/util.h"
#include "../math/geometry.h"
#include "spectrum.h"

namespace Miyuki {
    class TextureMapping2D {
        std::vector<Spectrum> texture;
        Bound2i bound;
    public:
        TextureMapping2D() : bound({0, 0}, {0, 0}) {}

        TextureMapping2D(const std::vector<unsigned char> &pixelData, const Point2i &dimension);

        void load(const std::vector<unsigned char> &pixelData, const Point2i &dimension);

        Spectrum sample(const Point2f &uv) const;
    };

    struct ColorMap {
        Spectrum color;
        Float maxReflectance;
        bool isNull;
        std::shared_ptr<TextureMapping2D> mapping;

        ColorMap(Float x, Float y, Float z, std::shared_ptr<TextureMapping2D> m = nullptr) : color(x, y, z),
                                                                                             isNull(false),
                                                                                             mapping(m) {
            maxReflectance = color.max();
        }

        ColorMap(const Spectrum &s, std::shared_ptr<TextureMapping2D> m = nullptr)
                : isNull(false), color(s), mapping(m) {
            maxReflectance = s.max();
        }

        ColorMap() : isNull(true) {}

        operator bool() const {
            return !isNull;
        }

        bool operator!() const {
            return isNull;
        }

        Spectrum sample(const Point2f &) const;
    };
}
#endif //MIYUKI_TEXTURE_H

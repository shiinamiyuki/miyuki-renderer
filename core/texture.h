//
// Created by Shiina Miyuki on 2019/1/26.
//

#ifndef MIYUKI_TEXTURE_H
#define MIYUKI_TEXTURE_H

#include "util.h"
#include "spectrum.h"

namespace Miyuki {
    class TextureMapping2D {
        std::vector<Spectrum> texture;
        Bound2i bound;
    public:
        TextureMapping2D() : bound({0, 0}, {0, 0}) {}
        TextureMapping2D(const std::vector<unsigned char> &pixelData, const Point2i &dimension);
        void load(const std::vector<unsigned char> &pixelData, const Point2i &dimension);
        Spectrum sample(const Point2f& uv)const;
    };
}
#endif //MIYUKI_TEXTURE_H

//
// Created by Shiina Miyuki on 2019/1/26.
//

#include "../utils/util.h"
#include "texture.h"

using namespace Miyuki;

void Miyuki::TextureMapping2D::load(const std::vector<unsigned char> &pixelData, const Point2i &dimension) {
    bound = Bound2i({0, 0}, dimension);
    texture.clear();
    if (pixelData.size() != 4 * dimension.x() * dimension.y()) {
        fmt::print(stderr, "Expected texture size {} by {}, but pixelData.size() = ",
                   dimension.x(), dimension.y(), pixelData.size());
        exit(-1);
    }
    auto inv = [](Float x) {
        return pow(x, 2.2);
    };
    for (int i = 0; i < pixelData.size(); i += 4) {
        texture.emplace_back(Spectrum(inv(pixelData[i] / 255.0f),
                                      inv(pixelData[i + 1] / 255.0f),
                                      inv(pixelData[i + 2] / 255.0f)));
    }
}

Spectrum TextureMapping2D::sample(const Point2f &uv) const {
    int x = clamp<int>(lroundf(uv.x() * bound.pMax.x()), 0, bound.pMax.x() - 1);
    int y = clamp<int>(lroundf(uv.y() * bound.pMax.y()), 0, bound.pMax.y() - 1);
    return texture[x + bound.pMax.x() * y];
}

TextureMapping2D::TextureMapping2D(const std::vector<unsigned char> &pixelData, const Point2i &dimension) : bound(
        {0, 0}, {0, 0}) {
    load(pixelData, dimension);
}

Spectrum ColorMap::sample(const Point2f &uv) const {
    if (mapping) {
        return Spectrum(color * mapping->sample(uv));
    }
    return color;
}
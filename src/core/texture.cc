//
// Created by Shiina Miyuki on 2019/1/26.
//

#include "../utils/util.h"
#include "texture.h"

using namespace Miyuki;

void Miyuki::TextureMapping2D::load(const std::vector<unsigned char> &pixelData, const Point2i &dimension, bool raw) {
    bound = Bound2i({0, 0}, dimension);
    texture.clear();
    if (pixelData.size() != 4 * dimension.x() * dimension.y()) {
        fmt::print(stderr, "Expected texture size {} by {}, but pixelData.size() = ",
                   dimension.x(), dimension.y(), pixelData.size());
        exit(-1);
    }
    std::function<float(float)> inv;
    if (!raw) {
        inv = [](Float x) {
            return pow(x, 2.2);
        };
    } else {
        inv = [](auto x) { return x; };
    }
    for (int i = 0; i < pixelData.size(); i += 4) {
        texture.emplace_back(Spectrum(inv(pixelData[i] / 255.0f),
                                      inv(pixelData[i + 1] / 255.0f),
                                      inv(pixelData[i + 2] / 255.0f)));
    }
}

static inline int mod(int a, int b) {
    int ret = a % b;
    if (ret < 0)
        ret += b;
    return ret;
}

Spectrum TextureMapping2D::sample(const Point2f &uv) const {
    int x = mod(lroundf(uv.x() * bound.pMax.x()), bound.pMax.x());
    int y = mod(lroundf(uv.y() * bound.pMax.y()), bound.pMax.y());
//    CHECK(x >= 0 && y >= 0);
    return texture[x + bound.pMax.x() * y];
}

TextureMapping2D::TextureMapping2D(const std::vector<unsigned char> &pixelData, const Point2i &dimension, bool raw)
        : bound(
        {0, 0}, {0, 0}) {
    load(pixelData, dimension, raw);
}

void TextureMapping2D::bumpToNormal() {
    std::vector<Vec3f> map(bound.pMax.x() * bound.pMax.y());
    auto get = [&](int x, int y) {
        x = clamp(x, 0, bound.pMax.x() - 1);
        y = clamp(y, 0, bound.pMax.y() - 1);
        return texture[x + bound.pMax.x() * y].length();
    };
    for (int x = 0; x < bound.pMax.x(); x++) {
        for (int y = 0; y < bound.pMax.y(); y++) {
            auto dx = get(x - 1, y) - get(x + 1, y);
            auto dy = get(x, y - 1) - get(x, y + 1);
            map[x + bound.pMax.x() * y] = Vec3f(dx, dy, 1).normalized();

        }
    }
    for (int x = 0; x < bound.pMax.x(); x++) {
        for (int y = 0; y < bound.pMax.y(); y++) {
            texture[x + bound.pMax.x() * y] = map[x + bound.pMax.x() * y] * 0.5 + Vec3f{0.5, 0.5, 0.5};
        }
    }
}

void TextureMapping2D::preprocessNormal() {
    for (auto &n:texture) {
        n = Vec3f{n.x(), n.z(), n.y()};
        n *= 2;
        n -= {1, 1, 1};
    }
}

Spectrum ColorMap::sample(const Point2f &uv) const {
    if (mapping) {
        return Spectrum(color * mapping->sample(uv));
    }
    return color;
}
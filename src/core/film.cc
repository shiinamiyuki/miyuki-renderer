//
// Created by Shiina Miyuki on 2019/1/12.
//

#include "film.h"
#include "scene.h"

using namespace Miyuki;

Miyuki::Spectrum Miyuki::Film::Pixel::toInt() const {
    return L().gammaCorrection();
}

void Film::Pixel::add(const Spectrum &c, const Float &w) {
    color += c;
    filterWeightSum += w;
}

Spectrum Film::Pixel::L() const {
    auto w = filterWeightSum == 0 ? 1 : filterWeightSum;
    auto c = color;
    c += Spectrum{splatXYZ[0], splatXYZ[1], splatXYZ[2]};
    c /= w;
    return c;
}

Film::Pixel &Film::getPixel(const Point2f &p) {
    return getPixel((int32_t) p.x(), (int32_t) p.y());
}

Film::Pixel &Film::getPixel(int32_t x, int32_t y) {
    x = clamp<int32_t>(x, 0, imageBound.pMax.x() - 1);
    y = clamp<int32_t>(y, 0, imageBound.pMax.y() - 1);
    return image[x + width() * y];
}

Film::Pixel &Film::getPixel(const Point2i &p) {
    return getPixel(p.x(), p.y());
}

void Film::scaleImageColor(Float scale) {
    for (auto &i:image) {
        i.color *= scale;
    }
}

Film::Film(int32_t w, int32_t h, uint32_t _tileSize)
        : imageBound(Point2i({0, 0}), Point2i({w, h})), tileSize(_tileSize) {
    assert(w >= 0 && h >= 0);
    image.resize(w * h);
    initTiles();
}

void Film::writePNG(const std::string &filename) {
    std::vector<unsigned char> pixelBuffer;
    for (const auto &i:image) {
        auto out = i.toInt();
        pixelBuffer.emplace_back(out.r());
        pixelBuffer.emplace_back(out.g());
        pixelBuffer.emplace_back(out.b());
        pixelBuffer.emplace_back(255);
    }
    lodepng::encode(filename, pixelBuffer, (uint32_t) width(), (uint32_t) height());
}

void Film::addSample(const Point2i &pos, const Spectrum &c, Float weight) {
    getPixel(pos).add(Spectrum(c * weight), weight);
}

void Film::addSplat(const Point2i &pos, const Spectrum &c) {
    for (int i = 0; i < 3; i++)
        getPixel(pos).splatXYZ[i].add(c[i]);
//    getPixel(pos).splat += c;
}

void Film::initTiles() {
    tiles.clear();
    for (int32_t i = 0; i < width(); i += tileSize) {
        for (int32_t j = 0; j < height(); j += tileSize) {
            int32_t x = clamp<int32_t>(i + tileSize, 0, width());
            int32_t y = clamp<int32_t>(j + tileSize, 0, height());
            tiles.emplace_back(Tile{Bound2i(Point2i(i, j), Point2i(x, y))});
        }
    }
}


void Film::Tile::foreachPixel(std::function<void(const Point2i &)> f) {
    for (int32_t i = bound.pMin.x(); i < bound.pMax.x(); i++) {
        for (int32_t j = bound.pMin.y(); j < bound.pMax.y(); j++) {
            f(Point2i(i, j));
        }
    }
}

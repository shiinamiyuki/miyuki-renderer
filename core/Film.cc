//
// Created by Shiina Miyuki on 2019/1/12.
//

#include "film.h"
#include "scene.h"

using namespace Miyuki;

Miyuki::Spectrum Miyuki::Film::Pixel::toInt() const {
    auto w = filterWeightSum == 0 ? 1 : filterWeightSum;
    auto c = color;
    c /= w;
    return c.gammaCorrection();
}

void Film::Pixel::add(const Spectrum &c, const Float &w) {
    color += c;
    filterWeightSum += w;
}

Film::Pixel &Film::getPixel(const Point2f &p) {
    return getPixel((int) p.x(), (int) p.y());
}

Film::Pixel &Film::getPixel(int x, int y) {
    x = clamp<int>(x, 0, imageBound.pMax.x());
    y = clamp<int>(y, 0, imageBound.pMax.y());
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

Film::Film(int w, int h, unsigned int _tileSize)
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
    lodepng::encode(filename, pixelBuffer, (unsigned int) width(), (unsigned int) height());
}

void Film::addSample(const Point2i &pos, const Spectrum &c, Float weight) {
    //assert(!c.hasNaNs());
    getPixel(pos).add(Spectrum(c * weight), weight);
}

void Film::addSplat(const Point2i &pos, const Spectrum &c) {
    getPixel(pos).add(c, 0);
}

void Film::initTiles() {
    tiles.clear();
    for (int i = 0; i < width(); i += tileSize) {
        for (int j = 0; j < height(); j += tileSize) {
            int x = clamp<int>(i + tileSize, 0, width());
            int y = clamp<int>(j + tileSize, 0, height());
            tiles.emplace_back(Tile{Bound2i(Point2i(i, j), Point2i(x, y))});
        }
    }
}


void Film::Tile::foreachPixel(std::function<void(const Point2i &)> f) {
    for (int i = bound.pMin.x(); i < bound.pMax.x(); i++) {
        for (int j = bound.pMin.y(); j < bound.pMax.y(); j++) {
            f(Point2i(i, j));
        }
    }
}

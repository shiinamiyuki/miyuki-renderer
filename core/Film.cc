//
// Created by Shiina Miyuki on 2019/1/12.
//

#include "Film.h"

using namespace Miyuki;

Miyuki::Spectrum Miyuki::Film::Pixel::toInt() const {
    auto w = weightSum == 0 ? 1 : weightSum;
    auto c = color;
    c /= w;
    return c.gammaCorrection();
}

void Film::Pixel::add(const Spectrum &c, const Float &w) {
    color += c;
    weightSum += w;
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

void Film::writeImage(Float scale) {
    for (auto &i:image) {

    }
}

Film::Film(int w, int h) : imageBound(Point2i({0, 0}), Point2i({w, h})) {
    assert(w >= 0 && h >= 0);
    image.resize(w * h);
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

void Film::addSplat(const Point2i &pos, const Spectrum &c) {
    //TODO: replace with other filters (currently box filters)
    getPixel(pos).add(c, 1);
}



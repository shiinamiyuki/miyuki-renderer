//
// Created by xiaoc on 2019/1/12.
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
    x = clamp<int>(0, x, imageBound.pMax.x());
    y = clamp<int>(0, y, imageBound.pMax.y());
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

}



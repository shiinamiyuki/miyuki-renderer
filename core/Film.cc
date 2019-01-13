//
// Created by xiaoc on 2019/1/12.
//

#include "Film.h"

using namespace Miyuki;

Miyuki::Vec3f Miyuki::Film::Pixel::toInt() const {
    auto w = weightSum == 0 ? 1 : weightSum;
    auto f = [=](Float x) {
        return lround(pow(clamp<double>(x / w, 0, 1), 1.0 / 2.2) * 255.0);
    };
    return Vec3f(f(color.x()), f(color.y()), f(color.z()));
}

void Film::Pixel::add(const Vec3f &c, const Float& w) {
    color += c;
    weightSum += w;
}

Film::Pixel &Film::getPixel(const Point2f &p) {
    return getPixel((int)p.x(),(int)p.y());
}

Film::Pixel &Film::getPixel(int x, int y) {
    x = clamp<int>(0, x, imageBound.pMax.x());
    y = clamp<int>(0, y, imageBound.pMax.y());
    return image[x + width() * y];
}

Film::Pixel &Film::getPixel(const Point2i &p) {
    return getPixel(p.x(),p.y());
}

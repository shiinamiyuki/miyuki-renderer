//
// Created by Shiina Miyuki on 2019/2/28.
//

#include "film.h"

namespace Miyuki {
    Spectrum BufferElement::toInt() const {
        return eval().gammaCorrection();
    }

    void BufferElement::add(const Spectrum &c, const Float &w) {
        value += c;
        filterWeightSum += w;
    }

    Spectrum BufferElement::eval() const {
        auto w = filterWeightSum == 0 ? 1 : filterWeightSum;
        auto c = value;
        c /= w;
        c += Spectrum{splatXYZ[0], splatXYZ[1], splatXYZ[2]} * splatWeight;
        return c;
    }

    void BufferElement::addSplat(const Spectrum &c) {
        for (int i = 0; i < 3; i++) {
            splatXYZ[i].add(c[i]);
        }
    }

    void BufferElement::scale(Float k) {
        for (int i = 0; i < 3; i++) {
            splatWeight *= k;
        }
        value *= k;
    }

    void Pixel::scale(Float k) {
        color.scale(k);

    }

    Pixel &Film::getPixel(const Point2f &p) {
        return getPixel((int32_t) p.x(), (int32_t) p.y());
    }

    Pixel &Film::getPixel(int32_t x, int32_t y) {
        x = clamp<int32_t>(x, 0, imageBound.pMax.x() - 1);
        y = clamp<int32_t>(y, 0, imageBound.pMax.y() - 1);
        return image[x + width() * y];
    }

    Pixel &Film::getPixel(const Point2i &p) {
        return getPixel(p.x(), p.y());
    }

    void Film::scaleImageColor(Float scale) {
        for (auto &i:image) {
            i.scale(scale);
        }
    }

    Film::Film(int32_t w, int32_t h)
            : imageBound(Point2i({0, 0}), Point2i({w, h})) {
        assert(w >= 0 && h >= 0);
        image.resize(w * h);
    }

    void Film::writePNG(const std::string &filename) {
        std::vector<unsigned char> pixelBuffer;
        for (const auto &i:image) {
            auto out = i.color.toInt();
            pixelBuffer.emplace_back(out.r());
            pixelBuffer.emplace_back(out.g());
            pixelBuffer.emplace_back(out.b());
            pixelBuffer.emplace_back(255);
        }
        lodepng::encode(filename, pixelBuffer, (uint32_t) width(), (uint32_t) height());
    }

    void Film::addSample(const Point2i &pos, const Spectrum &c, Float weight) {
        getPixel(pos).color.add(Spectrum(c * weight), weight);
    }

    void Film::addSplat(const Point2i &pos, const Spectrum &c) {
        auto color = removeNaNs(c);
        getPixel(pos).color.addSplat(color);
    }


    void LightingComposition::scale(Float k) {
        direct.scale(k);
        indirect.scale(k);
    }
}
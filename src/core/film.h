//
// Created by Shiina Miyuki on 2019/2/28.
//

#ifndef MIYUKI_FILM_H
#define MIYUKI_FILM_H

#include "geometry.h"
#include "spectrum.h"
#include "utils/atomicfloat.h"

namespace Miyuki {
    struct BufferElement {
        AtomicFloat splatXYZ[3];
        Spectrum value;
        Float filterWeightSum;
        Float splatWeight = 1;

        BufferElement() : value(0, 0, 0), filterWeightSum(0) {}

        Spectrum toInt() const;

        Spectrum eval() const;

        void add(const Spectrum &c, const Float &w);

        void addSplat(const Spectrum &c);

        void scale(Float k);
    };

    struct LightingComposition {
        BufferElement albedo, direct, indirect;

        void scale(Float k);
    };

    struct Pixel {
        BufferElement color;//, depth, pos, normal;
        // LightingComposition diffuse, specular, glossy;

        void scale(Float k);
    };

    class Film {
    public:

        std::vector<Pixel> image;

    private:
        Bound2i imageBound;

    public:
        const Point2i &imageDimension() const { return imageBound.pMax; }

        int height() const { return imageBound.pMax.y(); }

        int width() const { return imageBound.pMax.x(); }

        Pixel &getPixel(const Point2f &);

        Pixel &getPixel(const Point2i &);

        Pixel &getPixel(int x, int y);

        void addSample(const Point2i &, const Spectrum &c, Float weight = 1);

        void addSplat(const Point2i &, const Spectrum &c);

        Float &splatWeight(const Point2i &pos) {
            return getPixel(pos).color.splatWeight;
        }

        void scaleImageColor(Float scale);

        Film(int w = 0, int h = 0);

        void writePNG(const std::string &filename);
    };
}
#endif //MIYUKI_FILM_H

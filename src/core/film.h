//
// Created by Shiina Miyuki on 2019/2/28.
//

#ifndef MIYUKI_FILM_H
#define MIYUKI_FILM_H

#include "geometry.h"
#include "spectrum.h"
#include "utils/atomicfloat.h"

namespace Miyuki {
    struct Pixel {
        Spectrum value;
        Float filterWeightSum;
        AtomicFloat splatXYZ[3];
        Float splatWeight = 1;

        Pixel() : value(0, 0, 0), filterWeightSum(0) {}

        Spectrum toInt() const;

        Spectrum eval() const;

        void add(const Spectrum &c, const Float &w);
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

        void addSplat(const Point2i &pos, const Spectrum &c) {
            getPixel(pos).splatXYZ[0].add(c[0]);
            getPixel(pos).splatXYZ[1].add(c[1]);
            getPixel(pos).splatXYZ[2].add(c[2]);
        }

        Float & splatWeight(const Point2i &pos){
            return getPixel(pos).splatWeight;
        }
        Film(int w = 0, int h = 0);

        void writePNG(const std::string &filename);

        void clear();
    };
}
#endif //MIYUKI_FILM_H

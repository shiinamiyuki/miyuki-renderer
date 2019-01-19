//
// Created by Shiina Miyuki on 2019/1/12.
//

#ifndef MIYUKI_FILM_H
#define MIYUKI_FILM_H

#include "util.h"
#include "geometry.h"
#include "spectrum.h"

namespace Miyuki {
    class Film {
        Bound2i imageBound;

        struct Pixel {
            Spectrum color;
            Float filterWeightSum;

            Pixel() : color(0, 0, 0), filterWeightSum(0) {}

            Spectrum toInt() const;

            void add(const Spectrum &c, const Float &w);
        };

        std::vector<Pixel> image;

    public:
        int height() const { return imageBound.pMax.y(); }

        int width() const { return imageBound.pMax.x(); }

        Pixel &getPixel(const Point2f &);

        Pixel &getPixel(const Point2i &);

        Pixel &getPixel(int x, int y);

        void addSplat(const Point2i &, const Spectrum &c, Float weight = 1);

        void scaleImageColor(Float scale);

        Film(int w = 0, int h = 0);

        void writePNG(const std::string &filename);
    };
}
#endif //MIYUKI_FILM_H

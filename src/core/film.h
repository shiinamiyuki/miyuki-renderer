//
// Created by Shiina Miyuki on 2019/1/12.
//

#ifndef MIYUKI_FILM_H
#define MIYUKI_FILM_H

#include "../utils/util.h"
#include "../utils/atomicfloat.h"
#include "../math/geometry.h"
#include "spectrum.h"
#include "memory.h"

namespace Miyuki {
    class MemoryArena;
    struct RenderContext;
    class Scene;
    class Film {
        Bound2i imageBound;
        uint32_t tileSize;
    public:
        struct Pixel {
            Spectrum color;
            Float filterWeightSum;
            AtomicFloat splatXYZ[3];
            //Spectrum splat;
            Pixel() : color(0, 0, 0), filterWeightSum(0){}

            Spectrum toInt() const;
            Spectrum L()const;
            void add(const Spectrum &c, const Float &w);
        };

        std::vector<Pixel> image;

        struct Tile {
            Bound2i bound;

            void foreachPixel(std::function<void(const Point2i &)>);
        };
    private:
        std::vector<Tile> tiles;

        void initTiles();

    public:
        int height() const { return imageBound.pMax.y(); }

        int width() const { return imageBound.pMax.x(); }

        Pixel &getPixel(const Point2f &);

        Pixel &getPixel(const Point2i &);

        Pixel &getPixel(int x, int y);

        void addSample(const Point2i &, const Spectrum &c, Float weight = 1);

        void addSplat(const Point2i &, const Spectrum &c);

        void scaleImageColor(Float scale);

        Film(int w = 0, int h = 0, uint32_t _tileSize = 16);

        void writePNG(const std::string &filename);

        std::vector<Tile> &getTiles()  { return tiles; }
    };
}
#endif //MIYUKI_FILM_H

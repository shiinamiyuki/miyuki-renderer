//
// Created by xiaoc on 2019/1/12.
//

#ifndef MIYUKI_FILM_H
#define MIYUKI_FILM_H

#include "util.h"
#include "geometry.h"
namespace Miyuki {
    class Film {
        Bound2i imageBound;
        struct Pixel{
            Vec3f color;
            Float weightSum;
            Pixel():color(0,0,0),weightSum(0){}
            Vec3f toInt()const;
            void add(const Vec3f& c, const Float& w);
        };
        std::vector<Pixel> image;
    public:
        int height()const{return imageBound.pMax.y();}
        int width()const{return imageBound.pMax.x();}
        Pixel& getPixel(const Point2f&);
        Pixel& getPixel(const Point2i&);
        Pixel& getPixel(int x,int y);
    };

}
#endif //MIYUKI_FILM_H

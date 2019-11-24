#pragma once

#include <api/math.hpp>
namespace miyuki {
    template <class T> class TImage {
        Point2i dimension;
        std::vector<T> texels;

      public:
        TImage(const Point2i &dim) : dimension(dim), texels(dim[0] * dim[1]) {}

        const T &operator()(int x, int y) const { return texels[x + y * dimension[0]]; }
        T &operator()(int x, int y) { return texels[x + y * dimension[0]]; }

        const T &operator()(const Point2i &p) const { return (*this)(p.x, p.y); }
        T &operator()(const Point2i &p) { return (*this)(p.x, p.y); }

        const T &operator()(const Point2f &p) const { return (*this)(Point2i(p * Point2f(dimension))); }
        T &operator()(const Point2f &p) { return (*this)(Point2i(p * Point2f(dimension))); }

        T *data() { return texels.data(); }
    };

    class RGBImage : public TImage<Vec3f> {
      public:
        using TImage<Vec3f>::TImage;
    };
    class RGBAImage : public TImage<Vec4f> {
        using TImage<Vec4f>::TImage;
    };

} // namespace miyuki
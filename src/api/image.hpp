// MIT License
// 
// Copyright (c) 2019 椎名深雪
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
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
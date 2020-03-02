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

#include <miyuki.foundation/math.hpp>


namespace miyuki {
    template<class T>
    class TImage {

        std::vector<T> texels;

    public:
        const Vec2i dimension;
        TImage(const Vec2i &dim) : dimension(dim), texels(dim[0] * dim[1]) {}

        const T &operator()(int x, int y) const {
            x = std::clamp(x, 0, dimension[0] - 1);
            y = std::clamp(y, 0, dimension[1] - 1);
            return texels[x + y * dimension[0]];
        }

        T &operator()(int x, int y) {
            x = std::clamp(x, 0, dimension[0] - 1);
            y = std::clamp(y, 0, dimension[1] - 1);
            return texels[x + y * dimension[0]];
        }

        const T &operator()(float x, float y) const { return (*this)(Vec2f(x, y)); }

        T &operator()(float x, float y) { return (*this)(Vec2f(x, y)); }

        const T &operator()(const Vec2i &p) const { return (*this)(p.x(), p.y()); }

        T &operator()(const Vec2i &p) { return (*this)(p.x(), p.y()); }

        const T &operator()(const Vec2f &p) const { return (*this)(Vec2i(p * Vec2f(dimension))); }

        T &operator()(const Vec2f &p) { return (*this)(Vec2i(p * Vec2f(dimension))); }

        T *data() { return texels.data(); }

        [[nodiscard]] const T *data() const { return texels.data(); }
    };

    class RGBImage : public TImage<Vec3f> {
    public:
        using TImage<Vec3f>::TImage;
    };

    class RGBAImage : public TImage<float4> {
        using TImage<float4>::TImage;
    public:
        void write(const fs::path&, Float gamma);
    };

} // namespace miyuki
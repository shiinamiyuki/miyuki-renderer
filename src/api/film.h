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

#ifndef MIYUKIRENDERER_FILM_H
#define MIYUKIRENDERER_FILM_H

#include <api/spectrum.h>

namespace miyuki::core {
    struct Pixel {
        Spectrum color;
        Float weightSum = 0.0f;

        Spectrum eval() const {
            return weightSum == 0 ? color : Spectrum(color / weightSum);
        }
    };

    struct Film {
        std::vector<Pixel> pixels;
        const size_t width, height;

        Film(size_t w, size_t h) : width(w), height(h), pixels(w * h) {}

        static float gamma(float x, float k = 1.0f / 2.2f) {
            return std::pow(std::clamp(x, 0.0f, 1.0f), k);
        }

        static int toInt(float x) {
            return std::max<uint32_t>(0, std::min<uint32_t>(255, std::lroundf(gamma(x) * 255)));
        }

        void writePPM(const std::string &filename) {
            auto f = fopen(filename.c_str(), "w");
            fprintf(f, "P3\n%d %d\n%d\n", width, height, 255);
            for (int i = 0; i < width * height; i++) {
                auto invWeight = pixels[i].weightSum == 0 ? 0.0f : 1.0f / pixels[i].weightSum;
                fprintf(f, "%d %d %d ",
                        toInt(pixels[i].color[0] * invWeight),
                        toInt(pixels[i].color[1] * invWeight),
                        toInt(pixels[i].color[2] * invWeight));
            }
        }

        void writeImage(const std::string &filename);

        Pixel &operator()(const Point2f &p) {
            return (*this)(p.x, p.y);
        }

        Pixel &operator()(float x, float y) {
            int px = std::clamp<int>(std::lround(x * width), 0, width - 1);
            int py = std::clamp<int>(std::lround(y * height), 0, height - 1);
            return pixels.at(px + py * width);
        }

        Pixel &operator()(int x, int y) {
            return pixels.at(x + y * width);
        }

        void addSample(const Point2f &p, const Spectrum &color, Float weight) {
            auto &pixel = (*this)(p);
            pixel.color += color * weight;
            pixel.weightSum += weight;
        }
    };
}
#endif //MIYUKIRENDERER_FILM_H

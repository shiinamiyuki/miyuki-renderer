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

#include <miyuki.foundation/image.hpp>
#include <stb_image.h>
#include <lodepng.h>
#include <miyuki.foundation/log.hpp>
namespace miyuki{
    void RGBAImage::write(const fs::path &path, Float gamma) {
        std::vector<unsigned char> pixelBuffer;
        auto toInt = [=](float x){
            return std::max<uint32_t>(0, std::min<uint32_t>(255, std::lroundf(std::pow(x, gamma) * 255)));
        };
        for (int i = 0; i < dimension[0] * dimension[1]; i++) {
            pixelBuffer.emplace_back(toInt(data()[i][0]));
            pixelBuffer.emplace_back(toInt(data()[i][1]));
            pixelBuffer.emplace_back(toInt(data()[i][2]));
            pixelBuffer.emplace_back(255);
        }
        auto filename = path.string();
        auto error = lodepng::encode(filename, pixelBuffer, (uint32_t) dimension[0], (uint32_t) dimension[1]);
        if (error) {
            log::log("error saving {}: {}\n", filename, lodepng_error_text(error));
        } else {
            log::log("saved to {}\n", filename);
        }
    }
}
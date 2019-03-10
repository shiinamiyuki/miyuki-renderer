//
// Created by Shiina Miyuki on 2019/3/9.
//

#include "image.h"
#define STB_IMAGE_IMPLEMENTATION
#include "thirdparty/stb/stb_image.h"

namespace Miyuki {
    namespace IO {

        Image::Image(const std::string &filename, ImageFormat format) : format(format) {
            int ch;
            auto data = stbi_load(filename.c_str(), &width, &height, &ch, 3);
            if(!data){
                throw std::runtime_error(fmt::format("Cannot load {}\n", filename).c_str());
            }
            std::function<Float(uint8_t)> f;
            if (format == ImageFormat::none) {
                f = [](uint8_t _x) -> Float {
                    double x = _x / 255.0f;
                    return std::pow(x, 2.2);
                };
            } else if (format == ImageFormat::raw) {
                f = [](uint8_t _x) -> Float {
                    return _x / 255.0;
                };
            }
            pixelData.resize(width * height);
            for (int i = 0; i < width * height; i++) {
                pixelData[i] = Spectrum(f(data[3 * i]),
                                        f(data[3 * i + 1]),
                                        f(data[3 * i + 2]));
            }
        }
    }
}
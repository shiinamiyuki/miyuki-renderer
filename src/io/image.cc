//
// Created by Shiina Miyuki on 2019/3/9.
//

#include "image.h"
#include <boost/algorithm/string/predicate.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <thirdparty/stb/stb_image.h>
#include <thirdparty/stb/stb_image_write.h>
#include <utils/thread.h>

namespace Miyuki {
    namespace IO {

        Image::Image(const std::string &filename, ImageFormat format)
                : GenericImage<Spectrum>(), format(format), filename(filename) {
            if (stbi_is_hdr(filename.c_str())) {
                LoadHDR(filename, *this);
                return;
            }
            int ch;
            auto data = stbi_load(filename.c_str(), &width, &height, &ch, 3);
            if (!data) {
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
            Thread::ParallelFor(0u, width * height, [&](uint32_t i, uint32_t threadId) {
                pixelData[i] = Spectrum(f(data[3 * i]),
                                        f(data[3 * i + 1]),
                                        f(data[3 * i + 2]));
            }, 1024);
            free(data);
        }

        void Image::save(const std::string &filename) {

            std::vector<unsigned char> pixelBuffer;
            for (const auto &i:pixelData) {
                auto out = removeNaNs(i).gammaCorrection();
                pixelBuffer.emplace_back(out.r());
                pixelBuffer.emplace_back(out.g());
                pixelBuffer.emplace_back(out.b());
                pixelBuffer.emplace_back(255);
            }
            stbi_write_png(filename.c_str(), width, height, 4, &pixelBuffer[0], width * 4);
        }

        void LoadHDR(const std::string &filename, Image &image) {
            int ch;
            auto data = stbi_loadf(filename.c_str(), &image.width, &image.height, &ch, 3);
            Assert(ch == 3);
            image.pixelData.resize(image.width * image.height);
            Thread::ParallelFor(0u, image.width * image.height, [&](uint32_t i, uint32_t threadId) {
                image.pixelData[i] = Spectrum(data[3 * i],
                                              data[3 * i + 1],
                                              data[3 * i + 2]);
            }, 1024);
            free(data);

        }
    }
}
//
// Created by Shiina Miyuki on 2019/3/9.
//

#include "image.h"
#include <boost/algorithm/string/predicate.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>

#define STB_IMAGE_IMPLEMENTATION

#include "thirdparty/stb/stb_image.h"
#include <thirdparty/stb/stb_image_write.h>
#include <utils/thread.h>
#include <utils/atomicfloat.h>

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
            switch (format) {
                case ImageFormat::none: {
                    std::vector<unsigned char> pixelBuffer;
                    for (const auto &i:pixelData) {
                        auto out = removeNaNs(i).gammaCorrection();
                        pixelBuffer.emplace_back(out.r());
                        pixelBuffer.emplace_back(out.g());
                        pixelBuffer.emplace_back(out.b());
                        pixelBuffer.emplace_back(255);
                    }
                    lodepng::encode(filename, pixelBuffer, (uint32_t) width, (uint32_t) height);
                }
                case ImageFormat::myk_binary: {

                }

                default:
                    throw NotImplemented();
            }
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

        template<size_t N>
        struct MYKBinaryImage : GenericImage<Vec<Float, N>> {
        public:
            MYKBinaryImage(const std::string &filename) {
                std::ifstream input(filename, std::ios::binary);
                std::vector<char> buffer(std::istreambuf_iterator<char>(input), {});
                auto read = [&](std::vector<char>::iterator &iter) {
                    return *iter++;
                };
                auto readInt = [&](std::vector<char>::iterator &iter) {
                    char tmp[4];
                    tmp[0] = read(iter);
                    tmp[1] = read(iter);
                    tmp[2] = read(iter);
                    tmp[3] = read(iter);
                    return *reinterpret_cast<uint32_t *>(tmp);
                };
                auto readFloat = [&](std::vector<char>::iterator &iter) {
                    return bitsToFloat(readInt(iter));
                };
                auto iter = buffer.begin();
                std::string s;
                for (int i = 0; i < 3; i++)
                    s += read(iter);
                if (s != "MYK") {
                    fmt::print(stderr, "Not a valid binary file\n");
                    return;
                }
                this->width = readInt(iter);
                this->height = readInt(iter);
                int ch = readInt(iter);
                if (ch != N) {
                    fmt::print(stderr, "expected {} channels but have {}\n");
                    return;
                }
            }

            void save(const std::string &filename) {
                std::vector<char> data;
                auto write = [&](uint32_t x) {
                    char tmp[4];
                    *reinterpret_cast<uint32_t *>(tmp) = x;
                    data.emplace_back(tmp[0]);
                    data.emplace_back(tmp[1]);
                    data.emplace_back(tmp[2]);
                    data.emplace_back(tmp[3]);
                };
                for (auto &s: std::string("MYK")) {
                    data.emplace_back(s);
                }
                write(this->width);
                write(this->height);
                write(N);
                for (const auto &i:this->pixelData) {
                    auto out = removeNaNs(i);
                    for (int k = 0; k < N; k++)
                        write(floatToBits(out[k]));

                }
                std::ofstream out(filename);
                out.write(&data[0], data.size());
            }
        };
    }
}
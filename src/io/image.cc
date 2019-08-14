//
// Created by Shiina Miyuki on 2019/3/9.
//

#include <io/image.h>

#define STB_IMAGE_IMPLEMENTATION

#include <stb/stb_image.h>
#include <stb/stb_image_write.h>
#include <utils/thread.h>
#include <utils/atomicfloat.h>
#include <iostream>
#include <utils/log.h>

namespace Miyuki {
    namespace IO {

        Image::Image(const std::string &filename, ImageFormat format)
                : GenericImage<Spectrum>(), format(format), filename(filename) {
            if (format == ImageFormat::myk_binary) {

                std::vector<float> data;
                int32_t nChannel;
                LoadMYKBinaryImage(filename, data, &width, &height, 3, &nChannel);
                pixelData.reserve(width * height);
                for (int i = 0; i < width * height; i++) {
                    pixelData.emplace_back(data[3 * i], data[3 * i + 1], data[3 * i + 2]);
                }
                return;
            }
            if (stbi_is_hdr(filename.c_str())) {
                LoadHDR(filename, *this);
                return;
            }
            int ch;
            auto data = stbi_load(filename.c_str(), &width, &height, &ch, 3);
            if (!data) {
                throw std::runtime_error(fmt::format("Cannot load {}\n", filename).c_str());
            }
			pixelData.resize(width * height);
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
           
            Thread::ParallelFor(0u, width * height, [&](uint32_t i, uint32_t threadId) {
                pixelData[i] = Spectrum(f(data[3 * i]),
                                        f(data[3 * i + 1]),
                                        f(data[3 * i + 2]));
            }, 4096);
            free(data);
        }

        void Image::save(const std::string &filename) {
            saveAsFormat(filename, format);
        }

        void Image::saveAsFormat(const std::string &filename, ImageFormat format) {
            switch (format) {
                case ImageFormat::none: {
                    std::vector<unsigned char> pixelBuffer;
                    for (const auto &i:pixelData) {
                        auto out = removeNaNs(i).toInt();
                        pixelBuffer.emplace_back(out.r);
                        pixelBuffer.emplace_back(out.g);
                        pixelBuffer.emplace_back(out.b);
                        pixelBuffer.emplace_back(255);
                    }
                    lodepng::encode(filename, pixelBuffer, (uint32_t) width, (uint32_t) height);
                    break;
                }
                case ImageFormat::myk_binary: {
                    SaveMYKBinaryImage(filename, pixelData, width, height);
                    break;
                }

                default:
                    throw NotImplemented();
            }
        }

        void LoadHDR(const std::string &filename, Image &image) {
            int ch;
            auto data = stbi_loadf(filename.c_str(), &image.width, &image.height, &ch, 3);
            Assert(ch == 3);
			Assert(data);
            image.pixelData.resize(image.width * image.height);
           // Thread::ParallelFor(0u, image.width * image.height, [&](uint32_t i, uint32_t threadId) {
			for (int i = 0; i < image.width * image.height; i++) {
				image.pixelData[i] = Spectrum(data[3 * i],
					data[3 * i + 1],
					data[3 * i + 2]);
			}
           // }, image.width);
            free(data);

        }

        bool LoadMYKBinaryImage(const std::string &filename,
                                std::vector<float> &data,
                                int32_t *width,
                                int32_t *height,
                                int32_t desiredNChannel,
                                int32_t *nChannel) {
            std::ifstream input(filename, std::ios::binary);
            std::vector<char> buffer(std::istreambuf_iterator<char>(input), {});
            auto read = [&](std::vector<char>::iterator &iter) {
                if (iter == buffer.end()) {
                    throw std::runtime_error("Read null buffer\n");
                }
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
                return false;
            }
            *width = readInt(iter);
            *height = readInt(iter);
            auto ch = readInt(iter);
            if (ch != desiredNChannel) {
                fmt::print(stderr, "expected {} channels but have {}\n", desiredNChannel, ch);
            }
            *nChannel = ch;
            data.reserve(*width * *height * ch);
            try {
                for (int i = 0; i < *width * *height * ch; i++) {
                    data.emplace_back(readFloat(iter));
                }
                return true;
            } catch (std::runtime_error &e) {
                std::cerr << e.what() << std::endl;
                return false;
            }
        }

        bool SaveMYKBinaryImage(const std::string &filename,
                                std::vector<Spectrum> &pixelData,
                                int32_t width,
                                int32_t height) {
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
            write(width);
            write(height);
            write(3);
            for (int i = 0; i < width * height; i++) {
                auto c = removeNaNs(pixelData[i]);
                for (int k = 0; k < 3; k++)
                    write(floatToBits(c[k]));

            }
            std::ofstream out(filename, std::ios::binary);
            out.write(&data[0], data.size());
            return true;
        }

        bool SaveMYKBinaryImage(const std::string &filename,
                                std::vector<float> &pixelData,
                                int32_t width,
                                int32_t height,
                                int32_t nChannel) {
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
            write(width);
            write(height);
            write(nChannel);
            for (int i = 0; i < width * height; i++) {
                for (int k = 0; k < nChannel; k++)
                    write(floatToBits(pixelData[nChannel * i + k]));

            }
            std::ofstream out(filename, std::ios::binary);
            out.write(&data[0], data.size());
            return true;
        }
    }
}
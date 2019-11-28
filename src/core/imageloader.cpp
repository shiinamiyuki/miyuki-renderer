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
#include <api/imageloader.h>
#include <unordered_map>

// #define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <api/parallel.h>

namespace miyuki {
    class ImageLoader::Impl {
        struct ImageRecord {
            std::shared_ptr<RGBAImage> image;
            fs::file_time_type lastModifiedTime;
        };

      public:
        std::unordered_map<std::string, ImageRecord> cached;
        std::shared_ptr<RGBAImage> loadRGBAImage(const fs::path &path) {
            auto iter = cached.find(fs::absolute(path).string());
            auto last = fs::last_write_time(path);

            if (iter == cached.end() || last > iter->second.lastModifiedTime) {
                auto extension = path.extension().string();
                if (extension == ".ppm") {
                    return nullptr;
                }
                int w, h;
                int comp;
                auto data = stbi_load(path.string().c_str(), &w, &h, &comp, 3);
                auto image = std::make_shared<RGBAImage>(Point2i(w, h));
                if (comp == 4) {
                    ParallelFor(
                        0, w * h,
                        [&](int i, int threadIdx) {
                            image->data()[i] =
                                Vec4f(data[4 * i + 0], data[4 * i + 1], data[4 * i + 2], data[4 * i + 3]);
                        },
                        1024);
                } else if (comp == 3) {
                    ParallelFor(
                        0, w * h,
                        [&](int i, int threadIdx) {
                            image->data()[i] = Vec4f(data[3 * i + 0], data[3 * i + 1], data[3 * i + 2], 1);
                        },
                        1024);
                } else {
                    MIYUKI_NOT_IMPLEMENTED();
				}
                stbi_image_free(data);
                cached[fs::absolute(path).string()] = ImageRecord{image, last};
                return image;
            } else {
                return iter->second.image;
            }
        }
    };
    ImageLoader::ImageLoader() : impl(new Impl()) {}
    std::shared_ptr<RGBAImage> ImageLoader::loadRGBAImage(const fs::path &path) { return impl->loadRGBAImage(path); }

} // namespace miyuki
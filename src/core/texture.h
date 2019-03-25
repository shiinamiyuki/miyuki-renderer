//
// Created by Shiina Miyuki on 2019/3/10.
//

#ifndef MIYUKI_TEXTURE_H
#define MIYUKI_TEXTURE_H

#include "miyuki.h"
#include "io/image.h"

namespace Miyuki {
    struct Texture {
        Spectrum albedo;
        std::shared_ptr<IO::Image> image;
        Float multiplier = 1;

        Texture() {}

        Texture(const Spectrum &albedo, std::shared_ptr<IO::Image> image = nullptr) : albedo(albedo),
                                                                                      image(std::move(image)) {}

        Spectrum evalUV(const Point2f &uv) const;

        Spectrum evalPixel(const Point2i &pos) const;

        Spectrum evalAlbedo()const{
            return albedo * multiplier;
        }
    };

    class ImageLoader {
        std::unordered_map<std::string, std::shared_ptr<IO::Image>> images;
    public:
        std::shared_ptr<IO::Image> load(const std::string &filename, IO::ImageFormat format = IO::ImageFormat::none);
    };
}
#endif //MIYUKI_TEXTURE_H

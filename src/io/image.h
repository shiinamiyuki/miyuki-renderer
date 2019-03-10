//
// Created by xiaoc on 2019/3/9.
//

#ifndef MIYUKI_IMAGE_H
#define MIYUKI_IMAGE_H

#include "io.h"
#include "core/spectrum.h"

namespace Miyuki {
    namespace IO {
        enum class ImageFormat{
            none,
            raw,
            bump,
            normal,
        };
        struct Image {

            int width = 0;
            int height = 0;
            std::vector<Spectrum> pixelData;
            ImageFormat format = ImageFormat::none;
            Image(int width, int height) : pixelData(width, height) {}

            Spectrum &operator()(int x, int y) {
                x = clamp(x, 0, width);
                y = clamp(y, 0, height);
                return pixelData[x + width * y];
            }

            const Spectrum &operator()(int x, int y) const {
                x = clamp(x, 0, width);
                y = clamp(y, 0, height);
                return pixelData[x + width * y];
            }
            Image(const std::string & filename, ImageFormat format = ImageFormat::none);
        };
    }
}
#endif //MIYUKI_IMAGE_H

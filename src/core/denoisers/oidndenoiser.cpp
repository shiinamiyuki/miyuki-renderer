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
#include "oidndenoiser.h"

#ifdef MYK_USE_OIDN

#include <OpenImageDenoise/oidn.hpp>
#include <miyuki.foundation/log.hpp>
#include <miyuki.foundation/spectrum.h>
#include <miyuki.foundation/parallel.h>

namespace miyuki::core {
    class OIDNDenoiser::Impl {
    public:
        static oidn::DeviceRef getDevice() {
            static oidn::DeviceRef device;
            static std::once_flag flag;
            std::call_once(flag, [&]() {
                device = oidn::newDevice();
            });
            return device;
        }

        oidn::FilterRef filter;
        RGBImage color, normal, albedo;
        Point2i dim;

        Impl(const Point2i &dim) : dim(dim), color(dim), normal(dim), albedo(dim) {

        }

        void denoise(const Film &film, RGBAImage &image) {
            image = RGBAImage(dim);

            ParallelFor(0, dim[0] * dim[1], [=, &film, &image](int i, int) {
                float invWeight = film.weight.data()[i][0];
                invWeight = invWeight == 0 ? 0 : 1.0f / invWeight;
                color.data()[i] = film.color.data()[i] * invWeight;
                albedo.data()[i] = film.albedo.data()[i] * invWeight;
                normal.data()[i] = film.normal.data()[i] * invWeight;
            }, 1024);

            filter = oidnNewFilter(getDevice().getHandle(), "RT");

            filter.setImage("color", &color.data()[0], oidn::Format::Float3, dim[0], dim[1], 0, sizeof(Spectrum));
            filter.setImage("albedo", &albedo.data()[0], oidn::Format::Float3, dim[0], dim[1], 0, sizeof(Spectrum));
            filter.setImage("normal", &normal.data()[0], oidn::Format::Float3, dim[0], dim[1], 0, sizeof(Spectrum));
            filter.setImage("output", &image.data()[0], oidn::Format::Float3, dim[0], dim[1], 0, sizeof(Spectrum));


            filter.commit();
            filter.execute();

            const char *errorMessage;
            if (oidnGetDeviceError(getDevice().getHandle(), &errorMessage) != OIDN_ERROR_NONE)
                log::log("Error: {}\n", errorMessage);
            else{
                log::log("Denoising complete\n");
            }
        }
    };

    void OIDNDenoiser::denoise(const Film &film, RGBAImage &image) {

    }
}
#else
namespace miyuki::core {
     class OIDNDenoiser::Impl {};
    void OIDNDenoiser::denoise(const Film &film, RGBAImage &image) {
         return;
    }
}
#endif
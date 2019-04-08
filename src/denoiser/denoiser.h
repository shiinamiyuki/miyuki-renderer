//
// Created by Shiina Miyuki on 2019/4/8.
//

#ifndef MIYUKI_DENOISER_H
#define MIYUKI_DENOISER_H

#include <core/shadingcontext.hpp>
#include <io/image.h>
#include <core/film.h>

namespace Miyuki {
    class Denoiser {
    public:
        typedef IO::GenericImage<ShadingContext> FeatureBuffer;
        typedef IO::Image OutputBuffer;

        void denoise(Film &film);
    };
}
#endif //MIYUKI_DENOISER_H

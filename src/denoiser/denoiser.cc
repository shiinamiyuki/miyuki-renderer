//
// Created by Shiina Miyuki on 2019/4/8.
//

#include "denoiser.h"
#include <denoiser/nlm.h>
#include <utils/thread.h>

namespace Miyuki {

    void Denoiser::denoise(Film &film) {
        CHECK(film.auxBuffer);
        if (!film.auxBuffer)return;
        FeatureBuffer &buffer = *film.auxBuffer;
        IO::Image color(buffer.width, buffer.height);
        IO::Image normal(buffer.width, buffer.height);
        IO::Image out(buffer.width, buffer.height);
        for (int i = 0; i < buffer.width; i++) {
            for (int j = 0; j < buffer.height; j++) {
                CHECK(buffer(i, j).color.valid);
                color(i, j) = buffer(i, j).color.eval();
                normal(i, j) = buffer(i, j).normal.eval();
            }
        }
        NLMeans(color, out);
        film.clear();

        for (int i = 0; i < out.width; i++) {
            for (int j = 0; j < out.height; j++) {
                film.addSample(Point2f(i, j), out(i, j));
            }
        }
    }
}
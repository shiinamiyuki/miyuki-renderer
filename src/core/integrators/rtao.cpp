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

#include "rtao.h"
#include <miyuki.renderer/camera.h>
#include <miyuki.foundation/film.h>
#include <miyuki.foundation/log.hpp>
#include <miyuki.foundation/parallel.h>
#include <miyuki.foundation/profiler.h>
#include <miyuki.renderer/sampler.h>
#include <miyuki.renderer/sampling.h>
#include <miyuki.renderer/scene.h>

namespace miyuki::core {

    void RTAO::render(const std::shared_ptr<Scene> &scene, const std::shared_ptr<Camera> &camera,
                      const std::shared_ptr<Sampler> &_sampler, Film &film) {
        scene->resetRayCounter();
        Profiler profiler;
        ParallelFor(0, film.height, [=, &film](int64_t j, uint64_t) {
            auto sampler = _sampler->clone();
            for (int i = 0; i < film.width; i++) {

                sampler->startPixel(Point2i(i, j), Point2i(film.width, film.height));
                for (int s = 0; s < spp; s++) {
                    CameraSample sample;
                    sampler->startNextSample();
                    camera->generateRay(sampler->next2D(), sampler->next2D(), Point2i(i, j),
                                        Point2i(film.width, film.height), sample);

                    Intersection isct;
                    if (scene->intersect(sample.ray, isct)) {
                        auto wo = isct.worldToLocal(-sample.ray.d);
                        auto w = CosineHemisphereSampling(sampler->next2D());
                        if (wo.y * w.y < 0) {
                            w = -w;
                        }
                        w = isct.localToWorld(w);
                        auto ray = isct.spawnRay(w);
                        ray.tMax = occludeDistance;
                        isct = Intersection();
                        if (!scene->intersect(ray, isct) || isct.distance >= occludeDistance) {
                            film.addSample(sample.pFilm, Spectrum(1), 1);
                        } else {
                            film.addSample(sample.pFilm, Spectrum(0), 1);
                        }
                    }
                }
            }
        });
        auto duration = profiler.elapsed<double>();
        log::log("Rendering done in {}secs, traced {} rays, {} M rays/sec\n", duration.count(), scene->getRayCounter(),
                 scene->getRayCounter() / duration.count() / 1e6f);
    }
} // namespace miyuki::core

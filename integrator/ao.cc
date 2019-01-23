//
// Created by Shiina Miyuki on 2019/1/19.
//

#include "ao.h"
#include "../core/scene.h"
#include "../core/sampler.h"
#include "../core/film.h"
#include "../core/geometry.h"
#include "../sampler/random.h"
using namespace Miyuki;

void AOIntegrator::render(Scene *scene) {
    fmt::print("Rendering AO\n");
    auto& film = scene->film;
    auto& seeds = scene->seeds;
    int N = scene->option.samplesPerPixel;
    auto t = runtime([&]() {
        parallelFor(0u, (unsigned int) film.width(), [&](unsigned int x) {
            for (int y = 0; y < film.height(); y++) {
                int cnt = 0;
                RandomSampler randomSampler(&seeds[x + film.width() * y]);

                for (int i = 0; i < N; i++) {
                    auto ctx = scene->getRenderContext(Point2i({(int) x, y}));
                    Intersection intersection(ctx.primary.toRTCRay());
                    intersection.intersect(*scene);
                    if (intersection.hit()) {
                        auto hit = ctx.primary.o + intersection.rayHit.ray.tfar * ctx.primary.d;
                        auto p = scene->fetchIntersectedPrimitive(intersection);
                        auto rd = cosineWeightedHemisphereSampling(p.normal[0],
                                                                   randomSampler.nextFloat(),
                                                                   randomSampler.nextFloat());
                        Ray ray(hit, rd);
                        Intersection second(ray.toRTCRay());
                        second.intersect(*scene);
                        if (!second.hit()) {
                            cnt++;
                        }
                    }
                }
                film.addSplat(Point2i({(int) x, y}), Spectrum(Vec3f(1, 1, 1) * (Float(cnt) / N)));
            }
        });
    });
    fmt::print("Rendering end in {} secs, {} M Rays/sec\n", t, N * film.width() * film.height() / t / 1e6);
}

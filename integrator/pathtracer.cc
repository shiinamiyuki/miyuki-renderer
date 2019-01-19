//
// Created by Shiina Miyuki on 2019/1/19.
//

#include "pathtracer.h"
#include "../core/scene.h"
#include "../core/sampler.h"
#include "../core/film.h"
#include "../core/geometry.h"
#include "../sampler/random.h"

using namespace Miyuki;

void PathTracer::render(Scene *scene) {
    fmt::print("Rendering\n");
    auto &film = scene->film;
    auto &seeds = scene->seeds;
    constexpr int N = 128;
    for (int i = 0; i < N; i++) {
        auto t = runtime([&]() {
            parallelFor(0u, (unsigned int) film.width(), [&](unsigned int x) {
                for (int y = 0; y < film.height(); y++) {
                    RandomSampler randomSampler(&seeds[x + film.width() * y]);
                    auto ctx = scene->getRenderContext(Point2i({(int) x, y}));
                    Ray ray = ctx.primary;
                    Spectrum color(0, 0, 0);
                    Vec3f refl(1, 1, 1);
                    for (int depth = 0; depth < 5; depth++) {
                        Intersection intersection(ray);
                        intersection.intersect(scene->sceneHandle());
                        if (!intersection.hit())
                            break;
                        ray.o += ray.d * intersection.hitDistance();
                        Interaction interaction;
                        scene->fetchInteraction(intersection, makeRef(&interaction));
                        auto &p = *interaction.primitive;
                        auto &m = *interaction.material;
                        auto tot = m.ka.max() + m.kd.max();
                        auto x = randomSampler.nextFloat() * tot;
                        if (x < m.ka.max()) {
                            color += m.ka * refl;
                            break;
                        } else {
                            refl *= m.kd;
                            ray.d = cosineWeightedHemisphereSampling(interaction.norm,
                                                                     randomSampler.nextFloat(),
                                                                     randomSampler.nextFloat());
                        }
                        if (randomSampler.nextFloat() < refl.max()) {
                            refl /= refl.max();
                        } else {
                            break;
                        }
                    }
                    film.addSplat(Point2i(x, y), color);
                }
            });
        });
        fmt::print("{}th iteration in {} secs, {} M Rays/sec\n", 1 + i, t, film.width() * film.height() / t / 1e6);
    }
}

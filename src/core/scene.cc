//
// Created by Shiina Miyuki on 2019/3/3.
//

#include "scene.h"
#include "cameras/camera.h"
#include "utils/thread.h"
#include "samplers/sampler.h"
#include "samplers/sobol.h"
#include "math/sampling.h"
#include "core/profile.h"
#include "lights/area.h"

namespace Miyuki {

    Scene::Scene() : embreeScene(new EmbreeScene()) {
        arenas.resize(Thread::pool->numThreads());
        setFilmDimension({1000, 1000});
        factory = std::make_unique<MaterialFactory>();
    }

    void Scene::setFilmDimension(const Point2i &dim) {
        film = std::make_unique<Film>(dim[0], dim[1]);
        camera = std::make_unique<PerspectiveCamera>(
                film->imageDimension(),
                parameterSet.findFloat("camera.fov", 80.0f) / 180.0f * PI,
                parameterSet.findFloat("camera.lensRadius", 0.0f),
                parameterSet.findFloat("camera.focalDistance", 0.0f));
        camera->rotateTo(parameterSet.findVec3f("camera.rotation", {}) / 180.0f * PI);
        camera->moveTo(parameterSet.findVec3f("camera.translation", {}));

        seeds.resize(film->width() * film->height());
        samplerArena.reset();
        samplers.clear();
        for (int i = 0; i < film->width() * film->height(); i++) {
            auto *temp = ARENA_ALLOC(samplerArena, RandomSampler)(&seeds[i]);
            samplers.emplace_back(temp);
        }
    }

    void Scene::loadObjMesh(const std::string &filename) {
        auto mesh = std::make_shared<Mesh>(filename);
        CHECK(meshes.find(filename) == meshes.end());
        meshes[filename] = mesh;
    }

    void Scene::loadObjMeshAndInstantiate(const std::string &name, const Transform &T) {
        loadObjMesh(name);
        instantiateMesh(name, T);
    }

    void Scene::instantiateMesh(const std::string &name, const Transform &T) {
        CHECK(meshes.find(name) != meshes.end());
        auto mesh = meshes[name]->instantiate(T);
        embreeScene->addMesh(mesh, instances.size());
        factory->applyMaterial(description["shapes"], description["materials"], *mesh);
        instances.emplace_back(mesh);
    }

    void Scene::computeLightDistribution() {
        lights.clear();
        for (const auto &mesh:instances) {
            for (auto &p:mesh->primitives) {
                if (p.material()->emission.albedo.max() > 0.0f) {
                    lights.emplace_back(std::make_shared<AreaLight>(&p));
                    p.light = lights.back().get();
                }
            }
        }
        Float *power = new Float[lights.size()];
        for (int i = 0; i < lights.size(); i++) {
            power[i] = lights[i]->power();
        }
        lightDistribution = std::make_unique<Distribution1D>(power, lights.size());
        lightPdfMap.clear();
        for (int i = 0; i < lights.size(); i++) {
            lightPdfMap[lights[i].get()] = lightDistribution->pdf(i);
        }
        delete[] power;
        fmt::print("Important lights: {} Total power: {}\n", lights.size(), lightDistribution->funcInt);
    }

    void Scene::commit() {
        setFilmDimension(Point2i{parameterSet.findInt("render.width", 500),
                                 parameterSet.findInt("render.height", 500)});
        embreeScene->commit();
        camera->computeTransformMatrix();
        fmt::print("Film dimension: {}x{}\n", film->width(), film->height());
        fmt::print("Output file: {}\n", parameterSet.findString("render.output", "scene.png"));
        computeLightDistribution();
    }

    RenderContext Scene::getRenderContext(const Point2i &raster, MemoryArena *arena) {
        int idx = raster.x() + raster.y() * film->width();
        Ray primary;
        auto sampler = samplers[idx];
        Float weight;
        sampler->start();
        camera->generateRay(*sampler, raster, &primary, &weight);
        return RenderContext(raster, primary, camera.get(), arena, sampler, weight);
    }

    bool Scene::intersect(const RayDifferential &ray, Intersection *isct) {
        *isct = Intersection(ray);
        if (!isct->intersect(embreeScene->scene))
            return false;
        isct->uv = Point2f{isct->rayHit.hit.u, isct->rayHit.hit.v};
        isct->primitive = &instances[isct->geomId]->primitives[isct->primId];
        isct->wo = -1 * ray.d;
        auto p = isct->primitive;
        isct->Ns = p->Ns(isct->uv);
        isct->Ng = p->Ng;
        isct->ref = ray.o + isct->hitDistance() * ray.d;
        return true;
    }

    void Scene::test() {
        commit();
        Point2i nTiles = film->imageDimension() / TileSize + Point2i{1, 1};
        Profiler profiler;
        for (int n = 0; n < 64; n++) {
            Thread::parallelFor2D(nTiles, [&](Point2i tile, uint32_t threadId) {
                arenas[threadId].reset();
                for (int i = 0; i < TileSize; i++) {
                    for (int j = 0; j < TileSize; j++) {
                        int x = tile.x() * TileSize + i;
                        int y = tile.y() * TileSize + j;
                        if (x >= film->width() || y >= film->height())
                            continue;
                        auto raster = Point2i{x, y};
                        auto ctx = getRenderContext(raster, &arenas[threadId]);
                        Intersection intersection;
                        if (!intersect(ctx.primary, &intersection)) {
                            film->addSample(raster, {0, 0, 0});
                        } else {
                            CoordinateSystem system(intersection.Ns);
                            Vec3f wi = system.localToWorld(CosineWeightedHemisphereSampling(ctx.sampler->get2D()));
                            auto ray = Ray{intersection.ref, wi};
                            Intersection _;
                            if (intersect(ray, &_)) {
                                film->addSample(raster, {0, 0, 0});
                            } else {
                                film->addSample(raster, {1, 1, 1});
                            }
                        }
                    }
                }
            });
        }
        fmt::print("{}secs, {}M rays/sec", profiler.elapsedSeconds(),
                   64 * film->width() * film->height() / 1e6 / profiler.elapsedSeconds());
        film->writePNG("out.png");
    }

    Light *Scene::chooseOneLight(Sampler *sampler, Float *pdf) {
        if (lights.empty()) {
            return nullptr;
        }
        auto light = lightDistribution->sampleInt(sampler->get1D());
        *pdf = lightDistribution->pdf(light);
        return lights[light].get();
    }

    void Scene::saveImage() {
        film->writePNG(parameterSet.findString("render.output", "out.png"));
    }
}
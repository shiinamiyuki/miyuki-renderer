//
// Created by Shiina Miyuki on 2019/1/12.
//

#include "scene.h"
#include "mesh.h"
#include "spectrum.h"
#include "../sampler/random.h"

using namespace Miyuki;
using namespace Mesh;

Scene::Scene() : film(1000, 1000) {
    rtcScene = rtcNewScene(GetEmbreeDevice());
    seeds.resize(film.width() * film.height());

}

Scene::~Scene() {
    rtcReleaseScene(rtcScene);
}

void Scene::commit() {
    rtcCommitScene(rtcScene);
    checkError();
}

void Scene::loadObjTrigMesh(const char *filename) {
    auto mesh = Mesh::LoadFromObj(&materialList, filename);
    if (!mesh)return;
    addMesh(mesh);
    checkError();
}

void Scene::instantiateMesh(std::shared_ptr<Mesh::TriangularMesh> mesh) {
    instances.emplace_back(Mesh::MeshInstance(mesh));
}

void Scene::addMesh(std::shared_ptr<Mesh::TriangularMesh> mesh) {
    using namespace Mesh;
    RTCGeometry rtcMesh = rtcNewGeometry(GetEmbreeDevice(), RTC_GEOMETRY_TYPE_TRIANGLE);
    auto vertices =
            (Float *) rtcSetNewGeometryBuffer(rtcMesh,
                                              RTC_BUFFER_TYPE_VERTEX,
                                              0,
                                              RTC_FORMAT_FLOAT3,
                                              sizeof(Float) * 3,
                                              mesh->vertexCount());
    auto triangles = (unsigned int *) rtcSetNewGeometryBuffer(rtcMesh,
                                                              RTC_BUFFER_TYPE_INDEX, 0, RTC_FORMAT_UINT3,
                                                              sizeof(unsigned int) * 3,
                                                              mesh->triangleCount());
    for (int i = 0; i < mesh->triangleCount(); i++) {
        for (int j = 0; j < 3; j++)
            triangles[3 * i + j] = (unsigned int) mesh->triangleArray()[i].vertex[j];
    }
    for (int i = 0; i < mesh->vertexCount(); i++) {
        for (int j = 0; j < 3; j++)
            vertices[3 * i + j] = mesh->vertexArray()[i][j];
    }
    rtcCommitGeometry(rtcMesh);
    rtcAttachGeometryByID(rtcScene, rtcMesh, instances.size());
    rtcReleaseGeometry(rtcMesh);
    instantiateMesh(mesh);
}

void Scene::writeImage(const std::string &filename) {
    film.writePNG(filename);
}

void Scene::setFilmDimension(const Point2i &dim) {
    film = Film(dim.x(), dim.y());
    seeds.resize(dim.x() * dim.y());
}

void Scene::renderAO() {
    commit();
    fmt::print("Rendering AO\n");
    constexpr int N = 640;
    auto t = runtime([&]() {
        parallelFor(0u, (unsigned int) film.width(), [&](unsigned int x) {
            for (int y = 0; y < film.height(); y++) {
                int cnt = 0;
                RandomSampler randomSampler(&seeds[x + film.width() * y]);

                for (int i = 0; i < N; i++) {
                    auto ctx = getRenderContext(Point2i({(int) x, y}));
                    Intersection intersection(ctx.primary.toRTCRay());
                    intersection.intersect(rtcScene);
                    if (intersection.hit()) {
                        auto hit = ctx.primary.o + intersection.rayHit.ray.tfar * ctx.primary.d;
                        auto p = fetchIntersectedPrimitive(intersection);
                        auto rd = cosineWeightedHemisphereSampling(p.normal[0],
                                                                   randomSampler.nextFloat(),
                                                                   randomSampler.nextFloat());
                        Ray ray(hit, rd);
                        Intersection second(ray.toRTCRay());
                        second.intersect(rtcScene);
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

void Scene::renderPreview() {
    commit();
    fmt::print("Rendering preview\n");
    auto t = runtime([&]() {
        parallelFor(0u, (unsigned int) film.width(), [&](unsigned int x) {
            for (int y = 0; y < film.height(); y++) {
                auto ctx = getRenderContext(Point2i({(int) x, y}));
                Intersection intersection(ctx.primary.toRTCRay());
                intersection.intersect(rtcScene);
                if (intersection.hit()) {
                    auto p = fetchIntersectedPrimitive(intersection);
                    Float light = std::max(Float(0.2), Vec3f::dot(p.normal[0], Vec3f(0.1, 1, 0.3).normalized()));
                    Spectrum c = Spectrum(materialList[p.materialId].kd * light);
                    film.addSplat(Point2i({(int) x, y}), c);
                }
            }
        });
    });
    fmt::print("Rendering end in {} secs, {} M Rays/sec\n", t, film.width() * film.height() / t / 1e6);
}

RenderContext Scene::getRenderContext(const Point2i &raster) {
    int x0 = raster.x();
    int y0 = raster.y();
    Float x = (2 * (Float) x0 / film.width() - 1) * static_cast<Float>(film.width()) / film.height();
    Float y = 2 * (1 - (Float) y0 / film.height()) - 1;
    Vec3f ro = camera.viewpoint;
    auto z = (Float) (2.0 / tan(camera.aov / 2));
    Float dx = 2.0 / film.height(), dy = 2.0 / film.height();
    Seed *_Xi = &seeds[(x0 + y0 * film.width())];
    Vec3f jitter = Vec3f(dx * erand48(_Xi->getPtr()), dy * erand48(_Xi->getPtr()), 0);
    Vec3f rd = Vec3f(x, y, 0) + jitter - Vec3f(0, 0, -z);
    rd.normalize();
    rd = rotate(rd, Vec3f(1, 0, 0), camera.direction.y());
    rd = rotate(rd, Vec3f(0, 1, 0), camera.direction.x());
    rd = rotate(rd, Vec3f(0, 0, 1), camera.direction.z());
    return RenderContext(Ray(ro, rd));
}

void Scene::checkError() {
    if (rtcGetDeviceError(GetEmbreeDevice()) != RTC_ERROR_NONE) {
        fmt::print("Error!\n");
        exit(-1);
    }
}

const Mesh::MeshInstance::Primitive &Scene::fetchIntersectedPrimitive(const Intersection &intersection) {
    return instances[intersection.geomID()].primitives[intersection.primID()];
}

void Scene::renderPT() {
    commit();
    fmt::print("Rendering\n");
    constexpr int N = 128;
    for (int i = 0; i < N; i++) {
        auto t = runtime([&]() {
            parallelFor(0u, (unsigned int) film.width(), [&](unsigned int x) {
                for (int y = 0; y < film.height(); y++) {
                    RandomSampler randomSampler(&seeds[x + film.width() * y]);
                    auto ctx = getRenderContext(Point2i({(int) x, y}));
                    Ray ray = ctx.primary;
                    Spectrum color(0, 0, 0);
                    Vec3f refl(1, 1, 1);
                    for (int depth = 0; depth < 5; depth++) {
                        Intersection intersection(ray);
                        intersection.intersect(rtcScene);
                        if (!intersection.hit())
                            break;
                        ray.o += ray.d * intersection.hitDistance();
                        Interaction interaction;
                        fetchInteraction(intersection, makeRef(&interaction));
                        auto& p = *interaction.primitive;
                        auto &m = materialList[p.materialId];
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

void Scene::fetchInteraction(const Intersection &intersection, Ref<Interaction> interaction) {
    interaction->primitive = makeRef<const Mesh::MeshInstance::Primitive>(&fetchIntersectedPrimitive(intersection));
    interaction->norm = interaction->primitive->normal[0];
    interaction->wi = Vec3f(intersection.rayHit.ray.dir_x,
                            intersection.rayHit.ray.dir_y,
                            intersection.rayHit.ray.dir_z);
}


void Camera::lookAt(const Vec3f &pos) {
    assert(false);
}


Vec3f Intersection::intersectionPoint() const {
    return Vec3f();
}


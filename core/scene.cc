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
    postResize();

}

Scene::~Scene() {
    rtcReleaseScene(rtcScene);
}

void Scene::computeLightDistribution() {
    std::vector<Float> power;
    for (const auto &i:lights) {
        power.emplace_back(i->power());
    }
    lightDistribution = std::make_unique<Distribution1D>(Distribution1D(power.data(), power.size()));
}

void Scene::commit() {
    rtcCommitScene(rtcScene);
    checkError();
    lights = lightList;
    for (const auto &instance : instances) {
        for (const auto &primitive : instance.primitives) {
            auto &material = materialList[primitive.materialId];
            if (material.ka.max() > 1) {
                lights.emplace_back(std::shared_ptr<Light>(new AreaLight(primitive, material.ka)));
            }
        }
    }
    //  prob(L) = power(L) / total_power
    computeLightDistribution();
    if (lightDistribution) {
        for (const auto &i: lights) {
            i->scalePower(lightDistribution->funcInt / i->power()); // scales by 1 / prob
        }
    }
    fmt::print("Important lights: {}, total power: {}\n", lights.size(), lightDistribution->funcInt);
}

class NullLight : public Light {
public:
    Spectrum sampleLi(const Point2f &u, const Interaction &interaction, Vec3f *wi, Float *pdf,
                      VisibilityTester *tester) const override {
        *pdf = 0;
        return {};
    }

    Spectrum sampleLe(const Point2f &u1, const Point2f &u2, Ray *ray, Vec3f *normal, Float *pdfPos,
                      Float *pdfDir) const override {
        *pdfPos = 0;
        return {};
    }
};

static NullLight nullLight;

Light *Scene::chooseOneLight(Sampler &sampler) const {
    if (lights.empty())
        return &nullLight;
    int idx = lightDistribution->sampleInt(sampler.randFloat());
    return lights[idx].get();
}

const std::vector<std::shared_ptr<Light>> &Scene::getAllLights() const {
    return lights;
}

void Scene::loadObjTrigMesh(const char *filename, const Transform &transform) {
    auto mesh = Mesh::LoadFromObj(&materialList, filename);
    if (!mesh)return;
    addMesh(mesh, transform);
    checkError();
}

void Scene::instantiateMesh(std::shared_ptr<Mesh::TriangularMesh> mesh) {
    instances.emplace_back(Mesh::MeshInstance(mesh));
}

void Scene::addMesh(std::shared_ptr<Mesh::TriangularMesh> mesh, const Transform &transform) {
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
        Vec3f v(mesh->vertexArray()[i][0], mesh->vertexArray()[i][1], mesh->vertexArray()[i][2]);
        v = transform.apply(v);
        for (int j = 0; j < 3; j++)
            vertices[3 * i + j] = v[j];
    }
    rtcCommitGeometry(rtcMesh);
    rtcAttachGeometryByID(rtcScene, rtcMesh, instances.size());
    rtcReleaseGeometry(rtcMesh);
    instantiateMesh(mesh);
}

void Scene::writeImage(const std::string &filename) {
    film.writePNG(filename);
}

void Scene::postResize() {
    seeds.resize(film.width() * film.height());
    samplers.clear();
    std::random_device rd;
    std::uniform_int_distribution<int> dist;
    for (int i = 0; i < seeds.size(); i++) {
        seeds[i][0] = dist(rd);
        seeds[i][1] = dist(rd);
        seeds[i][2] = dist(rd);
        samplers.emplace_back(RandomSampler(&seeds[i]));
    }

}

void Scene::setFilmDimension(const Point2i &dim) {
    film = Film(dim.x(), dim.y());
    postResize();
}

void Scene::renderPreview() {
    commit();
    fmt::print("Rendering preview\n");
    auto t = runtime([&]() {
        parallelFor(0u, (unsigned int) film.width(), [&](unsigned int x) {
            for (int y = 0; y < film.height(); y++) {
                auto ctx = getRenderContext(Point2i({(int) x, y}));
                Intersection intersection(ctx.primary.toRTCRay());
                intersection.intersect(*this);
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
    return RenderContext(Ray(ro, rd), &samplers[x0 + film.width() * y0]);
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


// TODO: coord on triangle, see Embree's documentation
void Scene::fetchInteraction(const Intersection &intersection, Ref<Interaction> interaction) {
    interaction->primitive = makeRef<const Mesh::MeshInstance::Primitive>(&fetchIntersectedPrimitive(intersection));

    interaction->wi = Vec3f(intersection.rayHit.ray.dir_x,
                            intersection.rayHit.ray.dir_y,
                            intersection.rayHit.ray.dir_z);
    interaction->hitpoint = Vec3f(intersection.rayHit.ray.org_x,
                                  intersection.rayHit.ray.org_y,
                                  intersection.rayHit.ray.org_z) + interaction->wi * intersection.hitDistance();
    interaction->uv = Point2f(intersection.rayHit.hit.u, intersection.rayHit.hit.v);
    interaction->norm = pointOnTriangle(interaction->primitive->normal[0],
                                        interaction->primitive->normal[1],
                                        interaction->primitive->normal[2],
                                        interaction->uv.x(),
                                        interaction->uv.y());
    interaction->geomID = intersection.geomID();
    interaction->primID = intersection.primID();
    interaction->material = &materialList[interaction->primitive->materialId];
}

void Scene::prepare() {
    commit();
    if (lights.empty()) {
        fmt::print(stderr, "No lights!\n");
    }
}

void Scene::foreachPixel(std::function<void(const Point2i &)> f) {
//    parallelFor(0u, (unsigned int) film.width(), [&](unsigned int x) {
//        for (int y = 0; y < film.height(); y++) {
//            f(Point2i(x, y));
//        }
//    });
    const auto &tiles = film.getTiles();
    parallelFor(0u, tiles.size(), [&](unsigned int i) {
        tiles[i].foreachPixel(f);
    });
}


void Camera::lookAt(const Vec3f &pos) {
    assert(false);
}


Vec3f Intersection::intersectionPoint() const {
    return Vec3f();
}

Option::Option() {
    rrStartDepth = 0;
    maxDepth = 5;
    samplesPerPixel = 16;
    mltLuminanceSample = 100000;
    largeStepProb = 0.3;
}

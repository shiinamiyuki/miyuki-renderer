//
// Created by Shiina Miyuki on 2019/1/12.
//

#include "scene.h"
#include "mesh.h"
#include "spectrum.h"
#include "../samplers/random.h"
#include "../samplers/stratified.h"
#include "../samplers/sobol.h"
#include "../math/distribution.h"
#include "../cameras/camera.h"

using namespace Miyuki;

Scene::Scene() : film(1000, 1000), worldBound({}, {}) {
    rtcScene = rtcNewScene(GetEmbreeDevice());
    postResize();
    camera.fov = 80 / 180.0f * M_PI;
    readImageFunc = [&](Scene &scene, std::vector<uint8_t> &pixelData) {
        if(pixelData.size() != film.width() * film.height() * 4)
            pixelData.resize(film.width() * film.height() * 4);
        for (int i = 0; i < film.width(); i++) {
            for (int j = 0; j < film.height(); j++) {
                auto out = film.getPixel(i, j).toInt();
                auto idx = i + film.width() * (film.height() - j - 1);
                pixelData[4 * idx] = out.x();
                pixelData[4 * idx + 1] = out.y();
                pixelData[4 * idx + 2] = out.z();
                pixelData[4 * idx + 3] = 255;
            }
        }
    };
    updateFunc = [](Scene &) {};
    signalFunc = []() { return true; };
}

Scene::~Scene() {
    rtcReleaseScene(rtcScene);
}

void Scene::computeLightDistribution() {
    std::vector<Float> power;
    for (const auto &i:lights) {
        CHECK(i->importance > 0);
        power.emplace_back(i->power() * i->importance);
    }
    lightDistribution = std::make_unique<Distribution1D>(Distribution1D(power.data(), power.size()));
}

void Scene::commit() {
    rtcCommitScene(rtcScene);
    checkError();
    lights = lightList;
    for (auto &instance : instances) {
        for (auto &primitive : instance.primitives) {
            if (!materialList[primitive.materialId])continue;
            auto &material = *materialList[primitive.materialId];
            if (material.Ka().maxReflectance > 0.1) {
                lights.emplace_back(std::shared_ptr<Light>(new AreaLight(primitive, material.Ka().color)));
                primitive.light = lights.back().get();
            }

        }
    }
    //  prob(L) = power(L) / total_power
    computeLightDistribution();
//    if (lightDistribution) {
//        for (const auto &i: lights) {
//            i->scalePower(lightDistribution->funcInt / i->power()); // scales by 1 / prob
//        }
//    }
    lightDistributionMap.clear();
    for (int i = 0; i < lights.size(); i++) {
        lights[i]->scalePower(1.0f / lightDistribution->pdf(i));
        lightDistributionMap[lights[i].get()] = lightDistribution->pdf(i);
    }
    infiniteLight = std::make_unique<InfiniteLight>(ColorMap(ambientLight));
    fmt::print("Important lights: {}, total power: {}\n", lights.size(), lightDistribution->funcInt);
    Point3f pMin(INF, INF, INF);
    Point3f pMax(-INF, -INF, -INF);
    for (const auto &i:instances) {
        for (const auto &p:i.primitives) {
            for (int _ = 0; _ < 3; _++) {
                pMin = min(pMin, *p.vertices[_]);
                pMax = max(pMax, *p.vertices[_]);
            }
        }
    }
    worldBound = Bound3f(pMin, pMax);
    fmt::print("World radius: {}\n", worldRadius());
}

class NullLight : public Light {
public:
    Spectrum sampleLi(const Point2f &u, const IntersectionInfo &info, Vec3f *wi, Float *pdf,
                      VisibilityTester *tester) const override {
        *pdf = 0;
        return Miyuki::Spectrum();
    }

    Spectrum sampleLe(const Point2f &u1, const Point2f &u2, Ray *ray, Vec3f *normal, Float *pdfDir,
                      Float *pdfPos) const override {
        *pdfDir = *pdfPos = 0;
        return Miyuki::Spectrum();
    }

    Float pdfLi(const IntersectionInfo &, const Vec3f &wi) const override {
        return 0;
    }

    void pdfLe(const Ray &ray, const Vec3f &normal, Float *pdfPos, Float *pdfDir) const override {
        *pdfPos = *pdfDir = 0;
    }
};

static NullLight nullLight;

Light *Scene::chooseOneLight(Sampler &sampler, Float *lightPdf) const {
    if (lights.empty())
        return &nullLight;
    int32_t idx = lightDistribution->sampleInt(sampler.nextFloat());
    if (lightPdf) {
        *lightPdf = lightDistribution->pdf(idx);
    }
    return lights[idx].get();
}

const std::vector<std::shared_ptr<Light>> &Scene::getAllLights() const {
    return lights;
}

void Scene::loadObjTrigMesh(const char *filename, const Transform &transform, TextureOption opt) {
    auto factory = BSDFFactory(this);
    auto mesh = LoadFromObj(factory, &materialList, filename, opt);
    if (!mesh)return;
    addMesh(mesh, transform);
    checkError();
}

void Scene::instantiateMesh(std::shared_ptr<TriangularMesh> mesh, const Transform &transform) {
    instances.emplace_back(MeshInstance(mesh, transform));
}

void Scene::addMesh(std::shared_ptr<TriangularMesh> mesh, const Transform &transform) {
    RTCGeometry rtcMesh = rtcNewGeometry(GetEmbreeDevice(), RTC_GEOMETRY_TYPE_TRIANGLE);
    auto vertices =
            (Float *) rtcSetNewGeometryBuffer(rtcMesh,
                                              RTC_BUFFER_TYPE_VERTEX,
                                              0,
                                              RTC_FORMAT_FLOAT3,
                                              sizeof(Float) * 3,
                                              mesh->vertexCount());
    auto triangles = (uint32_t *) rtcSetNewGeometryBuffer(rtcMesh,
                                                          RTC_BUFFER_TYPE_INDEX, 0, RTC_FORMAT_UINT3,
                                                          sizeof(uint32_t) * 3,
                                                          mesh->triangleCount());
    for (int32_t i = 0; i < mesh->triangleCount(); i++) {
        for (int32_t j = 0; j < 3; j++)
            triangles[3 * i + j] = (uint32_t) mesh->triangleArray()[i].vertex[j];
    }
    for (int32_t i = 0; i < mesh->vertexCount(); i++) {
        Vec3f v(mesh->vertexArray()[i][0], mesh->vertexArray()[i][1], mesh->vertexArray()[i][2]);
        v = transform.apply(v);
        for (int32_t j = 0; j < 3; j++)
            vertices[3 * i + j] = v[j];
    }
    rtcCommitGeometry(rtcMesh);
    rtcAttachGeometryByID(rtcScene, rtcMesh, instances.size());
    rtcReleaseGeometry(rtcMesh);
    instantiateMesh(mesh, transform);
}

void Scene::addSphere(const Vec3f &pos, Float r, int materialId) {

}

void Scene::writeImage(const std::string &filename) {
    double avg = 0;
    for (auto &i:film.image) {
        avg += luminance(i.L());
    }
    avg /= film.width() * film.height();
    fmt::print("Avg brightness: {}\n", avg);
    film.writePNG(filename);
}

void Scene::postResize() {
    seeds.resize(film.width() * film.height());
    std::random_device rd;
    std::uniform_int_distribution<int32_t> dist;
    for (int32_t i = 0; i < seeds.size(); i++) {
        seeds[i][0] = dist(rd);
        seeds[i][1] = dist(rd);
        seeds[i][2] = dist(rd);
    }
    useSampler(option.samplerType);
    camera.filmDimension = {film.width(), film.height()};
}

void Scene::setFilmDimension(const Point2i &dim) {
    film = Film(dim.x(), dim.y());
    postResize();
}

void Scene::useSampler(Option::SamplerType samplerType) {
    option.samplerType = samplerType;
    samplerArena.reset();
    samplers.clear();
    // Trust compiler optimizations
    for (int32_t i = 0; i < seeds.size(); i++) {
        Sampler *s;
        if (samplerType == Option::independent) {
            s = ARENA_ALLOC(samplerArena, RandomSampler)(&seeds[i]);
        } else if (samplerType == Option::sobol) {
            s = ARENA_ALLOC(samplerArena, SobolSampler)(&seeds[i]);
        } else {
            CHECK(samplerType == Option::stratified);
            s = ARENA_ALLOC(samplerArena, StratifiedSampler)(&seeds[i]);
        }
        samplers.emplace_back(s);
    }

}

RenderContext Scene::getRenderContext(MemoryArena &arena, const Point2i &raster) {
    int32_t x0 = raster.x();
    int32_t y0 = raster.y();
    Sampler *sampler;
    size_t idx = x0 + film.width() * y0;
    sampler = samplers[idx];
    sampler->start();
    Ray primary{{},
                {}};
    camera.generatePrimaryRay(*sampler, raster, &primary);
    return RenderContext(&camera, primary, sampler, arena, raster);
}

void Scene::checkError() {
    if (rtcGetDeviceError(GetEmbreeDevice()) != RTC_ERROR_NONE) {
        fmt::print("Error!\n");
        exit(-1);
    }
}

const Primitive &Scene::fetchIntersectedPrimitive(const Intersection &intersection) {
    return instances[intersection.geomID()].primitives[intersection.primID()];
}


// TODO: coord on triangle, see Embree's documentation
void Scene::getIntersectionInfo(const Intersection &intersection, IntersectionInfo *info) {
    info->primitive = makeRef<const Primitive>(&fetchIntersectedPrimitive(intersection));
    info->distance = intersection.hitDistance();
    info->wo = Vec3f(-intersection.rayHit.ray.dir_x,
                     -intersection.rayHit.ray.dir_y,
                     -intersection.rayHit.ray.dir_z);
    info->hitpoint = Vec3f(intersection.rayHit.ray.org_x,
                           intersection.rayHit.ray.org_y,
                           intersection.rayHit.ray.org_z) + info->wo * -intersection.hitDistance();
    info->uv = Point2f(intersection.rayHit.hit.u, intersection.rayHit.hit.v);
    info->Ng = info->primitive->Ng;
    info->normal = info->primitive->normalAt(info->uv);
    info->geomID = intersection.geomID();
    info->primID = intersection.primID();
    info->bsdf = materialList[info->primitive->materialId].get();
}

void Scene::prepare() {
    commit();
    if (lights.empty()) {
        fmt::print(stderr, "No lights!\n");
    }
    camera.initTransformMatrix();
}

void Scene::foreachPixel(std::function<void(RenderContext &)> f) {
//    parallelFor(0u, (uint32_t) film.width(), [&](uint32_t x) {
//        for (int32_t y = 0; y < film.height(); y++) {
//            f(Point2i(x, y));
//        }
//    });
    auto &tiles = film.getTiles();
    parallelFor(0u, tiles.size(), [&](uint32_t i) {
        auto arenaInfo = arenaAllocator.getAvailableArena();
        tiles[i].foreachPixel([&](const Point2i &pos) {
            auto ctx = getRenderContext(arenaInfo.arena, pos);
            f(ctx);
        });
        arenaInfo.arena.reset();
    });
}


Vec3f Intersection::intersectionPoint() const {
    return Vec3f();
}

Option::Option() {
    minDepth = 0;
    maxDepth = 5;
    samplesPerPixel = 16;
    mltLuminanceSample = 100000;
    mltNChains = 100;
    largeStepProb = 0.3;
    showAmbientLight = true;
    aoDistance = 50;
    saveEverySecond = 10;
    sleepTime = 0;
    maxRayIntensity = 10;
    samplerType = SamplerType::independent;
}

void Scene::addPointLight(const Spectrum &ka, const Vec3f &pos) {
    lightList.emplace_back(std::make_shared<PointLight>(ka, pos));
}

bool Scene::intersect(const Ray &ray, IntersectionInfo *info) {
    Intersection intersection(ray);
    intersection.intersect(*this);
    if (!intersection.hit()) {
        return false;
    }
    getIntersectionInfo(intersection, info);
    return true;
}

Float Scene::worldRadius() const {
    Float r;
    Point3f center;
    worldBound.boundingSphere(&center, &r);
    return r;
}

void Scene::readImage(std::vector<uint8_t> &pixelData) {
    readImageFunc(*this, pixelData);
}

void Scene::update() {
    updateFunc(*this);
}


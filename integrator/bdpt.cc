//
// Created by Shiina Miyuki on 2019/1/22.
//

#include "bdpt.h"
#include "../core/scene.h"

using namespace Miyuki;
void Miyuki::BDPT::render(Scene &scene){}
#if 0
void Miyuki::BDPT::render(Scene &scene) {
    fmt::print("Rendering\n");
    auto &film = scene.film;
    auto &seeds = scene.seeds;
    int N = scene.option.samplesPerPixel;
    int sleepTime = scene.option.sleepTime;
    double elapsed = 0;
    for (int i = 0; i < N; i++) {
        auto t = runtime([&]() {
            iteration(scene);
            if (sleepTime > 0) {
                std::this_thread::sleep_for(std::chrono::milliseconds(sleepTime));
            }
        });
        elapsed += t;
        fmt::print("iteration {} in {} secs, elapsed {}s, remaining {}s\n",
                   1 + i, t, elapsed, (double) (elapsed * N) / (i + 1) - elapsed);
    }
}

void Miyuki::BDPT::generateLightPath(Sampler &sampler, Scene &scene, Path &path, unsigned int maxS) {
    Vec3f throughput(1, 1, 1);
    Spectrum radiance(0, 0, 0);
    Ray ray(Vec3f(0, 0, 0), Vec3f(0, 0, 0));
    {
        auto light = scene.chooseOneLight(sampler);
        Float pdfPos, pdfDir;
        Vec3f normal;
        auto rad = light->sampleLe(Point2f(sampler.nextFloat(), sampler.nextFloat()),
                                   Point2f(sampler.nextFloat(), sampler.nextFloat()),
                                   &ray, &normal, &pdfPos, &pdfDir);
        if (pdfPos <= 0) { return; }
        radiance = rad / pdfPos;
        path.emplace_back(Vertex(Vertex::lightVertex));
        path.back().hitPoint = ray.o;
        path.back().normal = normal;
        path.back().radiance = radiance;
        path.back().sampledType = BxDFType::emission;
        radiance *= Vec3f::dot(normal, ray.d);
        radiance /= pdfDir;
    }

    for (int depth = 1; depth < maxS; depth++) {
        Intersection intersection(ray);
        intersection.intersect(scene);
        if (!intersection.hit())break;
        ray.o += ray.d * intersection.hitDistance();
        Interaction interaction;
        scene.fetchInteraction(intersection, makeRef(&interaction));
        auto &primitive = *interaction.primitive;
        auto &material = *interaction.material;
        Vec3f wi;
        Float pdf;
        BxDFType sampledType;
        auto sample = material.sampleF(sampler, interaction, ray.d, &wi, &pdf, BxDFType::all, &sampledType);
        throughput *= sample;
        if (sampledType == BxDFType::none || sampledType == BxDFType::emission) { break; }
        else {
            path.emplace_back(Vertex(Vertex::surfaceVertex));
            path.back().hitPoint = ray.o;
            path.back().normal = interaction.normal;
            path.back().radiance = radiance * throughput;
            path.back().sampledType = sampledType;
            path.back().primID = interaction.primID;
            path.back().geomID = interaction.geomID;
        }
        throughput /= pdf;
        if (sampler.nextFloat() < throughput.max()) {
            throughput /= throughput.max();
        } else {
            break;
        }
        ray.d = wi;
    }
}

void Miyuki::BDPT::generateEyePath(RenderContext &ctx, Scene &scene, Path &path, unsigned int maxT) {
    Sampler &sampler = *ctx.sampler;
    Ray ray = ctx.primary;
    Vec3f throughput(1, 1, 1);
    for (int depth = 0; depth < maxT; depth++) {
        Intersection intersection(ray);
        intersection.intersect(scene);
        if (!intersection.hit()) {
            break;
        }
        ray.o += ray.d * intersection.hitDistance();
        Interaction interaction;
        scene.fetchInteraction(intersection, makeRef(&interaction));
        auto &primitive = *interaction.primitive;
        auto &material = *interaction.material;
        Vec3f wi;
        Float pdf;
        BxDFType sampledType;
        auto sample = material.sampleF(sampler, interaction, ray.d, &wi, &pdf, BxDFType::all, &sampledType);
        if (sampledType == BxDFType::none) { break; }
        throughput *= sample;
        if (sampledType == BxDFType::emission) {
            path.emplace_back(Vertex(Vertex::lightVertex));
            path.back().hitPoint = ray.o;
            path.back().normal = interaction.normal;
            path.back().sampledType = sampledType;
            path.back().radiance = throughput;
            break;
        } else {
            path.emplace_back(Vertex(Vertex::surfaceVertex));
            path.back().hitPoint = ray.o;
            path.back().normal = interaction.normal;
            path.back().sampledType = sampledType;
            path.back().radiance = throughput;
            path.back().primID = interaction.primID;
            path.back().geomID = interaction.geomID;
        }
        throughput /= pdf;
        ray.d = wi;
        if (sampler.nextFloat() < throughput.max()) {
            throughput /= throughput.max();
        } else {
            break;
        }
    }

}

Spectrum BDPT::connectBDPT(Scene &scene, Path &L, Path &E, int s, int t) {
    if (!L[s - 1].connectable(E[t - 1]))
        return {};
    VisibilityTester visibilityTester;
    visibilityTester.targetGeomID = E[t - 1].geomID;
    visibilityTester.targetPrimID = E[t - 1].primID;
    Vec3f w = E[t - 1].hitPoint - L[s - 1].hitPoint;
    Float dist = w.lengthSquared();
    w /= sqrt(dist);
    Float cosWi = Vec3f::dot(L[s - 1].normal, w);
    if (cosWi < 0)
        return {0, 0, 0};
    Float cosWo = -Vec3f::dot(E[t - 1].normal, w);
    if (cosWo < 0)
        return {0, 0, 0};

    visibilityTester.shadowRay = Ray(L[s - 1].hitPoint, w);
    if (!visibilityTester.visible(scene))
        return {0, 0, 0};
    Float G = cosWi * cosWo / dist;
    assert(G >= 0);
    assert(!std::isnan(G));
    return Spectrum(L[s - 1].radiance * G * E[t - 1].radiance);
}

void BDPT::iteration(Scene &scene) {
    auto &film = scene.film;
    auto &seeds = scene.seeds;
    int maxDepth = scene.option.maxDepth;
    scene.foreachPixel([&](const Point2i &pos) {
        auto ctx = scene.getRenderContext(pos);
        Path L, E;
        generateEyePath(ctx, scene, E, maxDepth);
        generateLightPath(*ctx.sampler, scene, L, maxDepth);
        Spectrum LPath;
        int cnt = 0;
        for (int t = 1; t <= E.size(); t++) {
            for (int s = 0; s <= L.size(); s++) {
                cnt++;
                if (s == 0 && hasBxDFType(E[t - 1].sampledType, BxDFType::emission)) {
                    LPath += E[t - 1].radiance;
                } else if (s + t <= maxDepth) {
                    auto r = connectBDPT(scene, L, E, s, t);
                    LPath += r;
                }
            }
        }
        LPath /= cnt;
        film.addSample(pos, LPath);
    });
}

bool Vertex::connectable(const Vertex &rhs) const {
    if (hasBxDFType(rhs.sampledType, BxDFType::specular) && !hasBxDFType(rhs.sampledType, BxDFType::glossy)) {
        return false;
    }
    if (hasBxDFType(sampledType, BxDFType::specular) && !hasBxDFType(sampledType, BxDFType::glossy)) {
        return false;
    }
    if (hasBxDFType(sampledType, BxDFType::emission) && hasBxDFType(rhs.sampledType, BxDFType::emission)) {
        return false;
    }
    return true;
}

Float Vertex::convertDensity(Float pdf, const Vertex &next) const {
    // TODO: infinite light
    Vec3f w = next.hitPoint - hitPoint;
    Float invDist = 1.0f / w.lengthSquared();
    pdf *= Vec3f::dot(next.normal, w * sqrt(invDist));
    return pdf * invDist;
}

Float Vertex::pdf(const Scene &, const Vertex *, const Vertex &next) const {
    return 0;
}
#endif
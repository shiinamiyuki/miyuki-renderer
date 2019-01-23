//
// Created by Shiina Miyuki on 2019/1/22.
//

#include "bdpt.h"
#include "../core/scene.h"

void Miyuki::BDPT::render(Scene &) {

}

void Miyuki::BDPT::generateLightPath(Sampler &sampler, Scene &scene, Miyuki::BDPT::Path &path, unsigned int maxS) {
    {
        auto light = scene.chooseOneLight(sampler);
        Ray ray(Vec3f(0, 0, 0), Vec3f(0, 0, 0));
        Float pdfPos, pdfDir;
        Vec3f normal;
        auto rad = light->sampleLe(Point2f(sampler.nextFloat(), sampler.nextFloat()),
                                   Point2f(sampler.nextFloat(), sampler.nextFloat()),
                                   &ray, &normal, &pdfPos, &pdfDir);
        if (pdfPos <= 0) { return; }
        path.emplace_back(LightVertex());
        path.back().hitpoint = ray.o;
        path.back().normal = normal;
        path.back().radiance = rad;

    }
}

void Miyuki::BDPT::generateEyePath(Sampler &sampler, Scene &scene, Miyuki::BDPT::Path &path, unsigned int maxT) {

}


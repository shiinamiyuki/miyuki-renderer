//
// Created by Shiina Miyuki on 2019/3/10.
//

#include "bdpt.h"

namespace Miyuki {

    Spectrum BDPT::Li(RenderContext &ctx, Scene &scene) {
        ctx.sampler->startDimension(0);
        auto lightSubPath = generateLightSubPath(scene, ctx, minDepth, maxDepth);
        // 1 to pick light, 2 to sample pos, 2 to sample dir
        // for each iteration, 2 to sample bsdf, 1 to do r.r.
        int dim = 5 + 3 * maxDepth;
        ctx.sampler->startDimension(dim);
        auto cameraSubPath = generateCameraSubPath(scene, ctx, minDepth + 1, maxDepth + 1);
        // 2 to sample lens, 2 for pixel filter
        dim += 4 + 3 * maxDepth;
        ctx.sampler->startDimension(dim);
        for (int i = 0; i < lightSubPath.N; i++) {
            CHECK(!std::isnan(lightSubPath[i].pdfFwd) && !std::isnan(lightSubPath[i].pdfRev));
        }
        for (int i = 0; i < cameraSubPath.N; i++) {
            CHECK(!std::isnan(cameraSubPath[i].pdfFwd) && !std::isnan(cameraSubPath[i].pdfRev));
        }
        auto &film = *scene.film;
        Spectrum LPath;
        for (int t = 1; t <= cameraSubPath.N; t++) {
            for (int s = 0; s <= lightSubPath.N; s++) {
                int depth = t + s - 2;
                if ((s == 1 && t == 1) || depth < 0 || depth > maxDepth)
                    continue;
                ctx.sampler->startDimension(dim + (depth) * (depth + 1) + 2 * s);
                Point2f raster;
                auto LConnect = connectBDPT(scene, ctx, lightSubPath, cameraSubPath, s, t, &raster);
                if (t != 1)
                    LPath += LConnect;
                else if (!LConnect.isBlack()) {
                    film.addSplat(raster, LConnect);
                }

            }
        }
        return LPath;
    }

    BDPT::BDPT(const ParameterSet &set) {
        minDepth = set.findInt("integrator.minDepth", 3);
        maxDepth = set.findInt("integrator.maxDepth", 5);
        spp = set.findInt("integrator.spp", 4);
        maxRayIntensity = set.findFloat("integrator.maxRayIntensity", 10000.0f);
        progressive = set.findInt("integrator.progressive", false);
    }



    void BDPT::render(Scene &scene) {
        fmt::print("Integrator: Bidirectional Path Tracing\n");
        for (int i = 0; i < scene.filmDimension().x(); i++) {
            for (int j = 0; j < scene.filmDimension().y(); j++) {
                scene.film->splatWeight({i, j}) = 1.0f / spp;
            }
        }
        SamplerIntegrator::render(scene);
    }


}
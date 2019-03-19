//
// Created by xiaoc on 2019/3/18.
//

#include <utils/thread.h>
#include "mlt.h"

namespace Miyuki {
    MLT::MLT(const ParameterSet &set) : BDPT(set) {
        nBootstrap = set.findInt("mlt.nBootstrap", 100000);
        nDirect = set.findInt("mlt.nDirect", 16);
        nChains = set.findInt("mlt.nChains", 1000);
        minDepth = set.findInt("mlt.minDepth", 3);
        maxDepth = set.findInt("mlt.maxDepth", 5);
        spp = set.findInt("mlt.spp", 4);
        maxRayIntensity = set.findFloat("mlt.maxRayIntensity", 10000.0f);
        progressive = set.findInt("mlt.progressive", false);
        b = 0;
    }

    void MLT::generateBootstrapSamples(Scene &scene) {
        std::random_device rd;

        // compute bootstrap samples
        std::vector<Seed> bootstrapSeeds(nBootstrap);
        std::vector<Float> bootstrapWeights(nBootstrap);

        {
            std::uniform_int_distribution<unsigned short> dist;
            for (int i = 0; i < bootstrapSeeds.size(); i++) {
                bootstrapSeeds[i] = dist(rd);
            }
        }
        std::vector<MemoryArena> arenas(Thread::pool->numThreads());
        Thread::ParallelFor(0u, nBootstrap, [&](uint32_t i, uint32_t threadId) {
            arenas[threadId].reset();
            MarkovChain markovChain(bootstrapSeeds[i]);
            Point2i raster;
            auto L = newPath(scene, &arenas[threadId], markovChain, &raster);
            bootstrapWeights[i] = L.luminance();
            scene.film->addSplat(raster, L);
        }, 4096);
        Distribution1D bootstrap(bootstrapWeights.data(), nBootstrap);
        b = bootstrap.funcInt / nBootstrap;
        fmt::print("b: {}\n", b);
        scene.update();
    }

    Spectrum MLT::evalMC(Scene &scene, RenderContext &ctx, MarkovChain &markovChain, Point2i *raster) {
        Bidir::SubPath light(&markovChain.lightSubPath[0], markovChain.lightSubPath.size());
        Bidir::SubPath camera(&markovChain.cameraSubPath[0], markovChain.cameraSubPath.size());
        int depth = light.N + camera.N - 2;
        if ((camera.N == 1 && light.N == 1) || depth < 0 || depth > maxDepth)
            return {};
        return connectBDPT(scene, ctx, light, camera, light.N, camera.N, raster, false);
    }

    Spectrum MLT::newPath(Scene &scene, MemoryArena *arena, MarkovChain &markovChain, Point2i *raster) {
        RandomSampler sampler(&markovChain.seed);
        int x = clamp<int>(sampler.get1D() * scene.filmDimension().x(), 0, scene.filmDimension().x() - 1);
        int y = clamp<int>(sampler.get1D() * scene.filmDimension().y(), 0, scene.filmDimension().y() - 1);
        markovChain.imageLoc = Point2i(x, y);
        *raster = markovChain.imageLoc;
        auto ctx = scene.getRenderContext(markovChain.imageLoc, arena, &sampler);
        auto lightPath = generateLightSubPath(scene, ctx, 0, maxDepth);
        auto cameraPath = generateCameraSubPath(scene, ctx, lightPath.N == 1 ? 2 : 1, maxDepth + 1);
        markovChain.clear();
        for (int i = 0; i < lightPath.N; i++) {
            markovChain.lightSubPath.emplace_back(lightPath[i]);
        }
        for (int i = 0; i < cameraPath.N; i++) {
            markovChain.cameraSubPath.emplace_back(cameraPath[i]);
        }
        return evalMC(scene, ctx, markovChain, raster);
    }

    void MLT::render(Scene &scene) {
        generateBootstrapSamples(scene);
    }
}
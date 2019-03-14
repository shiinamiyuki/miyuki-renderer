//
// Created by Shiina Miyuki on 2019/3/13.
//

#include "mmlt.h"
#include <utils/thread.h>
#include <core/progress.h>
#include <utils/stats.hpp>

namespace Miyuki {

    // special handling for image stratification
    void MMLTSampler::ensureReadyU1U2() {
        mutate(u1, 2.0/(imageDimension.x()*imageDimension.y()), 0.1);
        mutate(u2, 2.0/(imageDimension.x()*imageDimension.y()), 0.1);
    }

    void MMLTSampler::accept() {
        MLTSampler::accept();
    }

    void MMLTSampler::reject() {
        if (u1.lastModificationIteration == currentIteration)
            u1.restore();
        if (u2.lastModificationIteration == currentIteration)
            u2.restore();
        MLTSampler::reject();

    }


    MultiplexedMLT::MultiplexedMLT(const ParameterSet &set) : BDPT(set) {
        nBootstrap = set.findInt("mlt.nBootstrap", 100000);
        nDirect = set.findInt("mlt.nDirect", 16);
        nChains = set.findInt("mlt.nChains", 256);
        minDepth = set.findInt("mlt.minDepth", 3);
        maxDepth = set.findInt("mlt.maxDepth", 5);
        spp = set.findInt("mlt.spp", 4);
        maxRayIntensity = set.findFloat("mlt.maxRayIntensity", 10000.0f);
        largeStep = 0.4;
        b = 0;
    }

    void MultiplexedMLT::render(Scene &scene) {
        auto &film = *scene.film;
        int nPixels = scene.filmDimension().x() * scene.filmDimension().y();
        nMutations = ChainsMutations(nPixels, nChains, spp);
        std::vector<Seed> mltSeeds(nChains);
        std::vector<std::shared_ptr<MMLTSampler>> samplers(nChains);

        fmt::print("Integrator: Multiplexed Metropolis Light Transport!\n");
        fmt::print("{} chains, {} mutations\n", nChains, nMutations);
        fmt::print("Generating bootstrap samples, nBootstrap={}\n", nBootstrap);
        std::random_device rd;
        {
            // compute bootstrap samples
            uint64_t nBootstrapSamples = nBootstrap * (maxDepth + 1);
            std::vector<Seed> seeds(nBootstrapSamples);
            std::vector<Seed> bootstrapSeeds(nBootstrapSamples);
            std::vector<Float> bootstrapWeights(nBootstrapSamples);

            {
                std::uniform_int_distribution<unsigned short> dist;
                for (int i = 0; i < seeds.size(); i++) {
                    seeds[i] = dist(rd);
                    bootstrapSeeds[i] = seeds[i];
                }
            }

            std::vector<MemoryArena> arenas(Thread::pool->numThreads());
            Thread::ParallelFor(0u, nBootstrap, [&](uint32_t i, uint32_t threadId) {
                for (int depth = 0; depth <= maxDepth; depth++) {
                    arenas[threadId].reset();
                    Point2i raster;
                    int seedIndex = i * (maxDepth + 1) + depth;
                    MMLTSampler sampler(&bootstrapSeeds[seedIndex], nStream, largeStep, scene.filmDimension(), depth);
                    bootstrapWeights[seedIndex] = radiance(scene, &arenas[threadId], &sampler, depth,
                                                           &raster).luminance();
                }
            }, 4096);
            Distribution1D bootstrap(bootstrapWeights.data(), nBootstrapSamples);
            b = bootstrap.funcInt * (maxDepth + 1) / nBootstrapSamples;
            fmt::print("b: {}\n", b);
            // Selecting seeds according to distribution
            std::uniform_real_distribution<Float> dist;
            for (int i = 0; i < nChains; i++) {
                auto seedIndex = bootstrap.sampleInt(dist(rd));
                auto depth = seedIndex % (maxDepth + 1);
                mltSeeds[i] = seeds[seedIndex];

                samplers[i] = std::make_shared<MMLTSampler>(&mltSeeds[i], nStream, largeStep, scene.filmDimension(),
                                                            depth);
                samplers[i]->depth = depth;
                samplers[i]->L = radiance(scene, &arenas[0], samplers[i].get(), samplers[i]->depth,
                                          &samplers[i]->imageLocation);
            }
            // Let RAII auto clean up temporary vectors
        }
        handleDirect(scene);
        scene.update();
        scene.readImageFunc = [&](std::vector<uint8_t> &pixelData) {
            auto mpp = AverageMutationPerPixel(nPixels, nChains, curIter);
            for (int i = 0; i < film.width(); i++) {
                for (int j = 0; j < film.height(); j++) {
                    film.splatWeight({i, j}) = b / mpp;
                    auto out = film.getPixel(i, j).color.toInt();
                    auto idx = i + film.width() * (film.height() - j - 1);
                    pixelData[4 * idx] = out.x();
                    pixelData[4 * idx + 1] = out.y();
                    pixelData[4 * idx + 2] = out.z();
                    pixelData[4 * idx + 3] = 255;
                }
            }
        };
        DECLARE_STATS(int32_t, acceptanceCounter);
        DECLARE_STATS(int32_t, mutationCounter);
        ProgressReporter reporter(nMutations, [&](int cur, int total) {
            static int last = -1;
            int mpp = AverageMutationPerPixel(nPixels, nChains, cur);
            if (mpp == 0)return;
            if (mpp > 4)
                mpp /= 4;
            if (cur % mpp == 0 && mpp != last) {
                last = mpp;
                fmt::print("Mutations per pixel: {}/{} Acceptance rate: {} ,Elapsed:{} Remaining:{}\n",
                           AverageMutationPerPixel(nPixels, nChains, cur),
                           spp,
                           acceptanceCounter / (double) mutationCounter,
                           reporter.elapsedSeconds(), reporter.estimatedTimeToFinish());
                scene.update();
            }
        });
        fmt::print("Start rendering\n");
        std::vector<MemoryArena> arenas(Thread::pool->numThreads());
        std::uniform_real_distribution<Float> dist;
        for (curIter = 0; curIter < nMutations && scene.processContinuable(); curIter++) {
            Thread::ParallelFor(0, nChains, [&](uint32_t idx, uint32_t threadId) {
                auto sampler = samplers[idx].get();
                sampler->startIteration();
                Point2i pProposed;
                auto LProposed = radiance(scene, &arenas[threadId], sampler, sampler->depth, &pProposed);
                Float accept = clamp(LProposed.luminance() / sampler->L.luminance(), 0.0f, 1.0f);
                auto LNew = removeNaNs(Spectrum(LProposed * accept / LProposed.luminance()));
                auto LOld = removeNaNs(Spectrum(sampler->L * (1 - accept) / sampler->L.luminance()));
                if (accept > 0) {
                    film.addSplat(pProposed, LNew);
                }
                film.addSplat(sampler->imageLocation, LOld);
                UPDATE_STATS(mutationCounter, 1);
                if (dist(rd) < accept) {
                    sampler->L = LProposed;
                    sampler->imageLocation = pProposed;
                    sampler->accept();
                    UPDATE_STATS(acceptanceCounter, 1);
                } else {
                    sampler->reject();
                }
                arenas[threadId].reset();
            }, 16);
            reporter.update();
        }
        scene.update();
        Float mpp = AverageMutationPerPixel(nPixels, nChains, curIter);
        for (int i = 0; i < film.width(); i++) {
            for (int j = 0; j < film.height(); j++) {
                film.splatWeight({i, j}) = b / mpp;
            }
        }
    }

    Spectrum
    MultiplexedMLT::radiance(Scene &scene, MemoryArena *arena, MMLTSampler *sampler, int depth, Point2i *raster) {
        auto imageLoc = sampler->sampleImageLocation();
        int s, t, nStrategies;
        sampler->startStream(cameraStreamIndex);
        if (nDirect <= 0) {
            if (depth == 0) {
                nStrategies = 1;
                s = 0;
                t = 2;
            } else {
                nStrategies = depth + 2;
                s = std::min((int) (sampler->get1D() * nStrategies), nStrategies - 1);
                t = nStrategies - s;
            }
        } else {
            // direct lighting is handled by other methods
            if (depth == 0) {
                return {};
            } else {
                // avoid generating direct lighting paths
                // infinite area lights won't be sampled
                if (depth == 1) {
                    s = 0;
                    t = 3;
                    nStrategies = 1;
                } else {
                    nStrategies = depth + 2;
                    s = std::min((int) (sampler->get1D() * nStrategies), nStrategies - 1);
                    t = nStrategies - s;
                }
            }
        }
        auto ctx = scene.getRenderContext(imageLoc, arena, sampler);
        CHECK(ctx.sampler == sampler);
        auto cameraSubPath = generateCameraSubPath(scene, ctx, t, t);
        if (cameraSubPath.N != t) {
            return {};
        }
        sampler->startStream(lightStreamIndex);
        auto lightSubPath = generateLightSubPath(scene, ctx, s, s);
        if (lightSubPath.N != s) {
            return {};
        }
        if (nDirect > 0 && s + t == 3 && s == 0) {
            if (!cameraSubPath[t - 1].isInfiniteLight())
                return {};
        }
        *raster = imageLoc;
        sampler->startStream(connectionStreamIndex);
        return removeNaNs(connectBDPT(scene, ctx, lightSubPath, cameraSubPath, s, t, raster) * nStrategies);
    }

    void MultiplexedMLT::handleDirect(Scene &scene) {
        std::unique_ptr<DirectLightingIntegrator> direct(new DirectLightingIntegrator(nDirect));
        fmt::print("Rendering direct lighting\n");
        direct->render(scene);
    }
}



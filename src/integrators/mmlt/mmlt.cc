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
        mutate(u1, 2.0f / (imageDimension.x() + imageDimension.y()), 0.1f);
        mutate(u2, 2.0f / (imageDimension.x() + imageDimension.y()), 0.1f);
    }


    MultiplexedMLT::MultiplexedMLT(const ParameterSet &set) : BDPT(set) {
        nBootstrap = set.findInt("mlt.nBootstrap", 100000);
        nDirect = set.findInt("mlt.nDirect", 16);
        nChains = set.findInt("mlt.nChains", 1000);
        minDepth = set.findInt("mlt.minDepth", 3);
        maxDepth = set.findInt("mlt.maxDepth", 5);
        spp = set.findInt("mlt.spp", 4);
        maxRayIntensity = set.findFloat("mlt.maxRayIntensity", 10000.0f);
        largeStep = set.findFloat("mlt.largeStep", 0.3f);
        maxConsecutiveRejects = set.findInt("mlt.maxConsecutiveRejects", 512);
        progressive = set.findInt("mlt.progressive", false);
        b = 0;
        twoStage = set.findInt("mlt.twoStage", false);
    }

    void MultiplexedMLT::generateBootstrapSamples(Scene &scene) {
        std::random_device rd;

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

    void MultiplexedMLT::runMC(Scene &scene, MMLTSampler *sampler, MemoryArena *arena) {
        static std::random_device rd;
        static std::uniform_real_distribution<Float> dist;
        sampler->startIteration();
        Point2i pProposed;
        auto LProposed = radiance(scene, arena, sampler, sampler->depth, &pProposed);
        Float accept = clamp(LProposed.luminance() / sampler->L.luminance(), 0.0f, 1.0f);
        // force accept mutation, this removes fireflies (somehow)
        if (sampler->rejectCount >= maxConsecutiveRejects) {
            accept = 1;
        }
        if (sampler->large() && LProposed.luminance() > 0) {
            sampler->nonZeroCount++;
        }
        auto LNew = removeNaNs(Spectrum(
                LProposed * accept / LProposed.luminance()));
        auto LOld = removeNaNs(Spectrum(
                sampler->L * (1 - accept) / sampler->L.luminance()));
        if (twoStage) {
            LNew *= approxLuminance(pProposed);
            LOld *= approxLuminance(sampler->imageLocation);
        }
        if (accept > 0) {
            scene.film->addSplat(pProposed, LNew);
        }

        scene.film->addSplat(sampler->imageLocation, LOld);
        UPDATE_STATS(mutationCounter, 1);
        if (dist(rd) < accept) {
            sampler->L = LProposed;
            sampler->imageLocation = pProposed;
            sampler->accept();
            UPDATE_STATS(acceptanceCounter, 1);
        } else {
            sampler->reject();
        }
    }


    void MultiplexedMLT::render(Scene &scene) {
        auto &film = *scene.film;
        int nPixels = scene.filmDimension().x() * scene.filmDimension().y();
        nMutations = ChainsMutations(nPixels, nChains, spp);
        mltSeeds.resize(nChains);
        samplers.resize(nChains);

        fmt::print("Integrator: Multiplexed Metropolis Light Transport!\n");
        fmt::print("{}mpp, {} chains, {} mutations\n", spp, nChains, nMutations);
        if (twoStage) {
            twoStageInit(scene);
        }
        fmt::print("Generating bootstrap samples, nBootstrap={}\n", nBootstrap);
        generateBootstrapSamples(scene);
        handleDirect(scene);
        scene.update();

        reporter = std::make_unique<ProgressReporter<uint64_t>>(nMutations * nChains, [&](int64_t cur, int64_t total) {
            static int last = -1;
            int64_t mpp = lround(cur / (double) nPixels);
            if (mpp == 0)return;
            if (mpp > 4)
                mpp /= 4;
            if (cur % mpp == 0 && mpp != last) {
                std::lock_guard<std::mutex> lockGuard(mutex);
                if (reporter->count() % mpp == 0 && mpp != last) {
                    last = mpp;
                    fmt::print("Mutations per pixel: {}/{} Acceptance rate: {} ,Elapsed:{} Remaining:{}\n",
                               lround(cur / (double) nPixels),
                               spp,
                               acceptanceCounter / (double) mutationCounter,
                               reporter->elapsedSeconds(), reporter->estimatedTimeToFinish());
                    scene.update();
                }
            }
        });
        scene.readImageFunc = [&](std::vector<uint8_t> &pixelData) {
            auto mpp = reporter->count() / (double) nPixels;
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
        fmt::print("Start rendering; progressive = {}\n", progressive);
        acceptanceCounter = 0;
        mutationCounter = 0;
        if (progressive) {
            renderProgressive(scene);
        } else {
            renderNonProgressive(scene);
        }

    }

    void MultiplexedMLT::renderProgressive(Scene &scene) {
        auto &film = *scene.film;
        int nPixels = scene.filmDimension().x() * scene.filmDimension().y();
        std::vector<MemoryArena> arenas(Thread::pool->numThreads());

        for (int64_t curIter = 0; curIter < nMutations && scene.processContinuable(); curIter++) {
            Thread::ParallelFor(0, nChains, [&](uint32_t idx, uint32_t threadId) {
                auto sampler = samplers[idx].get();
                runMC(scene, sampler, &arenas[threadId]);
                arenas[threadId].reset();
                reporter->update();
            }, 64);
        }
        scene.update();
        recoverImage(scene);
    }

    void MultiplexedMLT::renderNonProgressive(Scene &scene) {
        auto &film = *scene.film;
        int nPixels = scene.filmDimension().x() * scene.filmDimension().y();
        std::vector<MemoryArena> arenas(Thread::pool->numThreads());
        Thread::ParallelFor(0, nChains, [&](uint32_t idx, uint32_t threadId) {
            for (int64_t curIter = 0; curIter < nMutations && scene.processContinuable(); curIter++) {
                auto sampler = samplers[idx].get();
                runMC(scene, sampler, &arenas[threadId]);
                if (curIter % 16 == 0)
                    arenas[threadId].reset();
                reporter->update();
            }
        });
        scene.update();
        recoverImage(scene);
    }

    void MultiplexedMLT::recoverImage(Scene &scene) {
        int nPixels = scene.filmDimension().x() * scene.filmDimension().y();
        auto mpp = reporter->count() / (double) nPixels;
        for (int i = 0; i < scene.film->width(); i++) {
            for (int j = 0; j < scene.film->height(); j++) {
                scene.film->splatWeight({i, j}) = b / mpp;
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
        if (cameraSubPath.N < t) {
            return {};
        }
        sampler->startStream(lightStreamIndex);
        auto lightSubPath = generateLightSubPath(scene, ctx, s, s);
        if (lightSubPath.N < s) {
            return {};
        }
        if (nDirect > 0 && s + t == 3 && s == 0) {
            if (!cameraSubPath[t - 1].isInfiniteLight())
                return {};
        }
        *raster = imageLoc;
        sampler->startStream(connectionStreamIndex);
        auto Li = clampRadiance(
                removeNaNs(connectBDPT(scene, ctx, lightSubPath, cameraSubPath, s, t, raster) * nStrategies),
                maxRayIntensity);
        if (twoStage) {
            auto lum = approxLuminance(*raster);
            Li /= lum;
        }
        return Li;
    }

    void MultiplexedMLT::handleDirect(Scene &scene) {
        if (nDirect <= 0)
            return;
        std::unique_ptr<DirectLightingIntegrator> direct(new DirectLightingIntegrator(nDirect));
        fmt::print("Rendering direct lighting\n");
        direct->render(scene);
    }

    void MultiplexedMLT::twoStageInit(Scene &scene) {
        fmt::print("Two-stage MLT: test image\n");
        int nPixels = scene.filmDimension().x() * scene.filmDimension().y();
        twoStageResolution = scene.filmDimension() / twoStageSampleFactor + Point2i{1, 1};
        std::vector<MemoryArena> arenas(Thread::pool->numThreads());
        std::vector<Seed> seeds(Thread::pool->numThreads());
        for (auto &i:seeds) {
            i = rand();
        }
        twoStageTestImage.resize(twoStageResolution.x() * twoStageResolution.y());
        for (auto &i: twoStageTestImage) {
            i = 0;
        }
        ScopedAssignment<bool> a1(&twoStage, false);
        Thread::ParallelFor(0u, nPixels, [&](uint32_t id, uint32_t threadId) {
            RandomSampler rng(&seeds[threadId]);
            Float lum = 0;
            MMLTSampler sampler(&seeds[threadId], nStream, largeStep, scene.filmDimension(),
                                rng.uniformInt32() % (maxDepth + 1));
            Point2i raster;
            Spectrum Li = radiance(scene, &arenas[threadId], &sampler, sampler.depth, &raster) * (maxDepth + 1);
            Li = clampRadiance(Li, 10);
            Li = removeNaNs(Li);
            lum = Li.luminance();
            int x = raster.x();
            int y = raster.y();
            x /= twoStageSampleFactor;
            y /= twoStageSampleFactor;
            twoStageTestImage[x + y * twoStageResolution.x()].add(lum / (twoStageSampleFactor * twoStageSampleFactor));
        }, 2048);
//        Film temp(scene.filmDimension().x(), scene.filmDimension().y());
//        for (int i = 0; i < scene.filmDimension().x(); i++) {
//            for (int j = 0; j < scene.filmDimension().y(); j++) {
//                temp.addSample({i, j}, approxLuminance({i, j}));
//            }
//        }
//        temp.writePNG("twostage.png");
    }
}



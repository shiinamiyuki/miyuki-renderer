//
// Created by Shiina Miyuki on 2019/3/22.
//

#include <utils/thread.h>
#include "pssmlt.h"

namespace Miyuki {

    Spectrum PSSMLT::Li(Scene &scene, MemoryArena *arena, MLTSampler *sampler, Point2f *raster) {
        auto imageLoc = sampler->sampleImageLocation();
        *raster = Point2f(imageLoc);
        sampler->startStream(0);
        auto ctx = scene.getRenderContext(imageLoc, arena, sampler);
        auto L = VolPath::Li(ctx, scene);
        return L;
    }

    void PSSMLT::handleDirect(Scene &scene) {
        if (nDirect <= 0)
            return;
        sampleDirect = false;

        std::unique_ptr<DirectLightingIntegrator> direct(new DirectLightingIntegrator(nDirect));
        fmt::print("Rendering direct lighting\n");
        direct->render(scene);
    }

    void PSSMLT::generateBootstrapSamples(Scene &scene) {
        std::random_device rd;

        // compute bootstrap samples
        uint64_t nBootstrapSamples = nBootstrap;
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
            arenas[threadId].reset();
            Point2f raster;
            MLTSampler sampler(&bootstrapSeeds[i], 1, largeStep, scene.filmDimension(), maxDepth);
            auto L = Li(scene, &arenas[threadId], &sampler,
                        &raster);
            L = removeNaNs(L);
            bootstrapWeights[i] = L.luminance();

        }, 4096);
        Distribution1D bootstrap(bootstrapWeights.data(), nBootstrapSamples);
        b = bootstrap.funcInt / nBootstrapSamples;
        fmt::print("b: {}\n", b);
        // Selecting seeds according to distribution
        std::uniform_real_distribution<Float> dist;
        std::uniform_int_distribution<Seed> dist2;
        for (int i = 0; i < nChains; i++) {
            auto seedIndex = bootstrap.sampleInt(dist(rd));
            mltSeeds[i] = seeds[seedIndex];

            samplers[i] = std::make_shared<MLTSampler>(&mltSeeds[i], 1, largeStep, scene.filmDimension(),
                                                       maxDepth);
            samplers[i]->L = Li(scene, &arenas[0], samplers[i].get(),
                                &samplers[i]->imageLocation);
            do {
                mltSeeds[i] = dist2(rd);
            } while (mltSeeds[i] == 0);
        }
        // Let RAII auto clean up temporary vectors
    }

    void PSSMLT::runMC(Scene &scene, MLTSampler *sampler, MemoryArena *arena) {
        static std::random_device rd;
        static std::uniform_real_distribution<Float> dist;
        sampler->startIteration();
        Point2f pProposed;
        auto LProposed = Li(scene, arena, sampler, &pProposed);
        Float accept;
        if (sampler->L.luminance() == 0)
            accept = 1;
        else
            accept = clamp(LProposed.luminance() / sampler->L.luminance(), 0.0f, 1.0f);
        // force accept mutation, this removes fireflies (somehow)
        if (sampler->rejectCount >= MLTSampler::maxConsecutiveRejects) {
            accept = 1;
        }
        if (sampler->large() && LProposed.luminance() > 0) {
            sampler->nonZeroCount++;
        }
        auto LNew = removeNaNs(Spectrum(
                LProposed *
                (accept + (sampler->large() ? 1.0f : 0.0f)) / (LProposed.luminance() / b + largeStep)));
        auto LOld = removeNaNs(Spectrum(
                sampler->L *
                (1 - accept) / (sampler->L.luminance() / b + largeStep)));
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

    void PSSMLT::render(Scene &scene) {
        auto &film = *scene.film;
        int nPixels = scene.filmDimension().x() * scene.filmDimension().y();
        nMutations = ChainsMutations(nPixels, nChains, spp);
        mltSeeds.resize(nChains);
        samplers.resize(nChains);

        fmt::print("Integrator: Unidirectional Primary Sample Space Metropolis Light Transport!\n");
        fmt::print("{}mpp, {} chains, {} mutations\n", spp, nChains, nMutations);
        handleDirect(scene);
        fmt::print("Generating bootstrap samples, nBootstrap={}\n", nBootstrap);
        generateBootstrapSamples(scene);

        scene.update();

        reporter = std::make_unique<ProgressReporter<uint64_t>>(nMutations * nChains, [&](int64_t cur, int64_t total) {
            static int last = -1;
            int64_t mpp = lround(cur / (double) nPixels);
            if (mpp == 0)return;
            if (mpp != last) {
                std::lock_guard<std::mutex> lockGuard(mutex);
                if (mpp != last) {
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
                    film.splatWeight({i, j}) = 1.0 / mpp;
                    auto out = film.getPixel(i, j).toInt();
                    auto idx = i + film.width() * (film.height() - j - 1);
                    pixelData[4 * idx] = out.x();
                    pixelData[4 * idx + 1] = out.y();
                    pixelData[4 * idx + 2] = out.z();
                    pixelData[4 * idx + 3] = 255;
                }
            }
        };
        fmt::print("Start rendering;\n");
        acceptanceCounter = 0;
        mutationCounter = 0;
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


    void PSSMLT::recoverImage(Scene &scene) {
        int nPixels = scene.filmDimension().x() * scene.filmDimension().y();
        auto mpp = reporter->count() / (double) nPixels;
        for (int i = 0; i < scene.film->width(); i++) {
            for (int j = 0; j < scene.film->height(); j++) {
                scene.film->splatWeight({i, j}) = 1.0 / mpp;
            }
        }
    }
}

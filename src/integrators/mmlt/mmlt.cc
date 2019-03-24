//
// Created by Shiina Miyuki on 2019/3/13.
//

#include "mmlt.h"
#include <utils/thread.h>
#include <core/progress.h>
#include <utils/stats.hpp>

namespace Miyuki {

    const int MultiplexedMLT::cameraStreamIndex = 0;
    const int MultiplexedMLT::lightStreamIndex = 1;
    const int MultiplexedMLT::connectionStreamIndex = 2;
    const int MultiplexedMLT::nStream = 3;
    const int MultiplexedMLT::twoStageSampleFactor = 20;

    MultiplexedMLT::MultiplexedMLT(const ParameterSet &set) : BDPT(set) {
        nBootstrap = set.findInt("integrator.nBootstrap", 100000);
        nDirect = set.findInt("integrator.nDirect", 16);
        nChains = set.findInt("integrator.nChains", 8);
        largeStep = set.findFloat("integrator.largeStep", 0.25f);
        MLTSampler::maxConsecutiveRejects = set.findInt("sampler.maxConsecutiveRejects", 256);
        useKelemenWeight = true;
    }

    void MultiplexedMLT::generateBootstrapSamples(Scene &scene) {
        std::random_device rd;

        // compute bootstrap samples
        uint64_t nBootstrapSamples = nBootstrap * (maxDepth + 1);
        std::vector<std::vector<Seed>> seeds(maxDepth + 1);
        std::vector<std::vector<Seed>> bootstrapSeeds(maxDepth + 1);
        std::vector<std::vector<Float>> bootstrapWeights(maxDepth + 1);
        for (auto &i : bootstrapWeights) {
            i.resize(nBootstrap);
        }
        {
            std::uniform_int_distribution<unsigned short> dist;
            for (int k = 0; k <= maxDepth; k++) {
                bootstrapSeeds[k].resize(nBootstrap);
                seeds[k].resize(nBootstrap);
                for (int i = 0; i < nBootstrap; i++) {
                    bootstrapSeeds[k][i] = seeds[k][i] = dist(rd);
                }
            }
        }
        std::vector<MemoryArena> arenas(Thread::pool->numThreads());
        Thread::ParallelFor(0u, nBootstrap, [&](uint32_t i, uint32_t threadId) {
            for (int k = 0; k <= maxDepth; k++) {
                arenas[threadId].reset();
                Point2i raster;
                MLTSampler sampler(&bootstrapSeeds[k][i], nStream, largeStep, scene.filmDimension(), k);
                bootstrapWeights[k][i] = radiance(scene, &arenas[threadId], &sampler, k,
                                                  &raster).luminance();
            }
        }, 4096);
        b.resize(maxDepth + 1u);
        chains.clear();
        std::vector<Distribution1D> distributions;
        for (int k = 0; k <= maxDepth; k++) {
            distributions.emplace_back(&bootstrapWeights[k][0], nBootstrap);
            b[k] = distributions.back().funcInt / nBootstrap;
        }
        for (int i = 0; i < b.size(); i++) {
            fmt::print("b[{}] = {}\n", i, b[i]);
        }
        std::uniform_real_distribution<Float> dist;
        std::uniform_int_distribution<Seed> uniformIntDistribution(1, UINT64_MAX);
        for (int i = 0; i < nChains; i++) {
            std::shared_ptr<MarkovChain> markovChain(new MarkovChain(b));
            for (int k = 0; k <= maxDepth; k++) {
                Distribution1D &distribution1D = distributions[k];
                int idx = distribution1D.sampleInt(dist(rd));
                Seed seed = seeds[k][idx];
                auto sampler = std::make_shared<MLTSampler>(
                        &seed, nStream,
                        largeStep, scene.filmDimension(), k);
                sampler->depth = k;
                sampler->L = radiance(scene, &arenas[0], sampler.get(), sampler->depth,
                                      &sampler->imageLocation);
                markovChain->seeds[k] = uniformIntDistribution(rd);
                sampler->reseed(&markovChain->seeds[k]);
                markovChain->samplers[k] = sampler;
            }
            chains.emplace_back(markovChain);
        }
    }

    void MultiplexedMLT::run(MarkovChain &markovChain, Scene &scene, MemoryArena *arena) {
        static std::random_device rd;
        static std::uniform_real_distribution<Float> dist;

        int k = markovChain.pathLengthDistribution.sampleInt(dist(rd));

        Float pdfBk = markovChain.pathLengthDistribution.pdf(k);
        auto sampler = markovChain.samplers[k].get();

        sampler->startIteration();
        Point2i pProposed;
        auto LProposed = radiance(scene, arena, sampler, sampler->depth, &pProposed);
        Float accept;
        if (sampler->L.luminance() == 0)
            accept = 1;
        else
            accept = clamp(LProposed.luminance() / sampler->L.luminance(), 0.0f, 1.0f);

        if (sampler->large() && LProposed.luminance() > 0) {
            sampler->nonZeroCount++;
        }
        Float weightNew;
        Float weightOld;
        if (useKelemenWeight) {
            weightNew = 1.0f / pdfBk *
                        (accept + (sampler->large() ? 1.0f : 0.0f)) / (LProposed.luminance() / b[k] + largeStep);
            weightOld = 1.0f / pdfBk *
                        (1 - accept) / (sampler->L.luminance() / b[k] + largeStep);
        } else {
            weightNew = 1.0f / pdfBk * accept / LProposed.luminance() * b[k];
            weightOld = 1.0f / pdfBk * (1 - accept) / sampler->L.luminance() * b[k];
        }

        if (!std::isnormal(weightNew)) {
            weightNew = 0.0f;
        }
        if (!std::isnormal(weightOld)) {
            weightOld = 0.0f;
        }
        if (weightNew > 0) {
            scene.film->addSplat(pProposed, LProposed * weightNew);
        }
        // Consecutive sample filtering from ERPT
        if (weightOld > 0 && sampler->rejectCount < MLTSampler::maxConsecutiveRejects) {
            scene.film->addSplat(sampler->imageLocation, sampler->L * weightOld);
        }
        UPDATE_STATS(mutationCounter, 1);
        if (accept == 1 || dist(rd) < accept) {
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

        fmt::print("Integrator: Multiplexed Metropolis Light Transport!\n");

        fmt::print("Generating bootstrap samples, nBootstrap = {}\n", nBootstrap);
        nMutations = ChainsMutations(nPixels, nChains, spp);
        fmt::print("{}mpp, {} chains, {} mutations\n", spp, nChains, nMutations);
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
                    film.splatWeight({i, j}) = 1.0f / mpp;
                    auto out = film.getPixel(i, j).toInt();
                    auto idx = i + film.width() * (film.height() - j - 1);
                    pixelData[4 * idx] = out.x();
                    pixelData[4 * idx + 1] = out.y();
                    pixelData[4 * idx + 2] = out.z();
                    pixelData[4 * idx + 3] = 255;
                }
            }
        };
        fmt::print("Start rendering\n");
        acceptanceCounter = 0;
        mutationCounter = 0;
        std::vector<MemoryArena> arenas(Thread::pool->numThreads());
        Thread::ParallelFor(0u, chains.size(), [&](uint32_t idx, uint32_t threadId) {
            for (int64_t i = 0; i < nMutations && scene.processContinuable(); i++) {
                run(*chains[idx], scene, &arenas[threadId]);
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
                scene.film->splatWeight({i, j}) = 1.0f / mpp;
            }
        }
    }

    Spectrum
    MultiplexedMLT::radiance(Scene &scene, MemoryArena *arena, MLTSampler *sampler, int depth, Point2i *raster) {
        auto imageLoc = sampler->sampleImageLocation();
        int s, t, nStrategies;
        sampler->startStream(cameraStreamIndex);
        auto u = sampler->get1D();
        if (nDirect <= 0) {
            if (depth == 0) {
                nStrategies = 1;
                s = 0;
                t = 2;
            } else {
                nStrategies = depth + 2;
                s = std::min((int) (u * nStrategies), nStrategies - 1);
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
                    s = std::min((int) (u * nStrategies), nStrategies - 1);
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
                removeNaNs(connectBDPT(scene, ctx, lightSubPath, cameraSubPath, s, t, raster)),
                maxRayIntensity) * nStrategies;
        return Li;

    }

    void MultiplexedMLT::handleDirect(Scene &scene) {
        if (nDirect <= 0)
            return;
        std::unique_ptr<DirectLightingIntegrator> direct(new DirectLightingIntegrator(nDirect));
        fmt::print("Rendering direct lighting\n");
        direct->render(scene);
    }


}



//
// Created by Shiina Miyuki on 2019/3/3.
//

#include "renderengine.h"
#include "integrators/volpath/volpath.h"
#include <integrators/bdpt/bdpt.h>
#include <integrators/mmlt/mmlt.h>
#include <integrators/pssmlt/pssmlt.h>
#include <integrators/erpt/erpt.h>
#include <utils/thread.h>
#include "core/film.h"

namespace Miyuki {

    void printWelcome() {
        const char *welcome = R"(
        _             _    _     __                _
  /\/\ (_)_   _ _   _| | _(_)   /__\ ___ _ __   __| | ___ _ __ ___ _ __
 /    \| | | | | | | | |/ / |  / \/// _ \ '_ \ / _` |/ _ \ '__/ _ \ '__|
/ /\/\ \ | |_| | |_| |   <| | / _  \  __/ | | | (_| |  __/ | |  __/ |
\/    \/_|\__, |\__,_|_|\_\_| \/ \_/\___|_| |_|\__,_|\___|_|  \___|_|
          |___/
)";
        fmt::print("{}\n", welcome);
    }

    void RenderEngine::processCommandLine(int argc, char **argv) {
        printWelcome();
        if (argc < 2) {

        } else {
            auto filename = argv[1];
            fmt::print("Loading scene {}\n", filename);
            cxx::filesystem::path inputFile(filename);
            sceneFileName = inputFile.filename().string();
            auto parent = inputFile.parent_path();
            cxx::filesystem::current_path(parent);
            std::ifstream in(sceneFileName);
            std::string content((std::istreambuf_iterator<char>(in)),
                                (std::istreambuf_iterator<char>()));
            description = Json::parse(content);
            readDescription();

        }
    }

    int RenderEngine::exec() {
        if (integrator) {
            scene.processContinueFunc = [this](Scene &x) {
                return renderContinue == true;
            };
            if (mode == gui) {
                scene.setUpdateFunc([&](Scene &s) {
                    updateFunc();
                });
            }
            integrator->render(scene);
            scene.saveImage();
        }
        if (mode == gui) {
            while (renderContinue) {
                std::this_thread::sleep_for(std::chrono::milliseconds(250));
            }

        }
        return 0;
    }

    void RenderEngine::readDescription() {
        auto &parameters = scene.parameters();
        scene.description = description;


        loadIntegrator();
        loadCamera();
        if (description.hasKey("render")) {
            auto &render = description["render"];
            int w = 0, h = 0;
            if (render.hasKey("resolution")) {
                auto &res = render["resolution"];
                if (res.isArray()) {
                    w = res[0].getInt();
                    h = res[1].getInt();
                } else {
                    Assert(res.isString());
                    if (res.getString() == "1080p") {
                        w = 1920;
                        h = 1080;
                    } else if (res.getString() == "4k") {
                        w = 3840;
                        h = 2160;
                    } else if (res.getString() == "240p") {
                        w = 426;
                        h = 240;
                    } else if (res.getString() == "360p") {
                        w = 480;
                        h = 360;
                    } else if (res.getString() == "720p") {
                        w = 1280;
                        h = 720;
                    } else {
                        fmt::print(stderr, "Unrecognized resolution format\n");
                    }
                }
            }
            parameters.addInt("render.width", w);
            parameters.addInt("render.height", h);

            if (render.hasKey("output")) {
                parameters.addString("render.output", IO::deserialize<std::string>(render["output"]));
            }
        }
        if (description.hasKey("objects")) {
            for (const auto &obj : description["objects"].getArray()) {
                scene.loadObjMeshAndInstantiate(obj["file"].getString(), obj["name"].getString(),
                                                IO::deserialize<Transform>(obj["transform"]));
            }
        }
    }

    RenderEngine::RenderEngine() : renderContinue(true), mode(commandLine) {
        updateFunc = []() {};
    }

    void RenderEngine::stopRender() {
        renderContinue = false;
    }

    void RenderEngine::startRender() {
        renderContinue = true;
    }

    void RenderEngine::readPixelData(std::vector<uint8_t> &pixelData, int &width, int &height) {
        scene.readImage(pixelData);
        width = scene.film->width();
        height = scene.film->height();
    }

    void RenderEngine::imageSize(int &width, int &height) {
        width = scene.film->width();
        height = scene.film->height();
    }

    std::vector<MemoryArena> _arena(Thread::pool->numThreads());

    void RenderEngine::renderPreview(std::vector<uint8_t> &pixelData, int &width, int &height) {

        width = scene.film->width();
        height = scene.film->height();
        if (pixelData.size() < width * height * 4) {
            pixelData.resize(width * height * 4);
        }
        const Vec3f lightDir = Vec3f(0.1, 1, 0.1).normalized();
        Seed seed(rand());
        Thread::ParallelFor(0u, width, [&](uint32_t i, uint32_t threadId) {
            for (int j = 0; j < height; j++) {
                Spectrum out(1, 1, 1);

                NullSampler sampler(&seed);
                auto ctx = scene.getRenderContext(Point2i(i, j), &_arena[threadId], &sampler);
                Intersection intersection;
                ScatteringEvent event;
                if (scene.intersect(ctx.primary, &intersection)) {
                    Integrator::makeScatteringEvent(&event, ctx, &intersection, TransportMode::radiance);
                    auto albedo = intersection.primitive->material()->albedo(event);
                    auto lighting = std::max(0.2f, Vec3f::dot(lightDir, intersection.Ns));
                    out *= albedo * lighting;
                }
                out = out.gammaCorrection();
                auto idx = i + width * (height - j - 1);
                pixelData[4 * idx] = out.x();
                pixelData[4 * idx + 1] = out.y();
                pixelData[4 * idx + 2] = out.z();
                pixelData[4 * idx + 3] = 255;
                _arena[threadId].reset();
            }
        });
    }

    void RenderEngine::loadIntegrator() {
        auto &parameters = scene.parameters();
        scene.useDefaultReadImageFunc();
        if (description.hasKey("integrator")) {
            auto &I = description["integrator"];
            if (I.hasKey("ambientLight")) {
                parameters.addVec3f("ambientLight", IO::deserialize<Vec3f>(I["ambientLight"]));
            }
            if (I.hasKey("envMap")) {
                parameters.addString("envMap", (I["envMap"]).getString());
            }
            I.setIfHasNotKey("maxRayIntensity", 10000);
            I.setIfHasNotKey("spp", 16);
            I.setIfHasNotKey("minDepth", 0);
            I.setIfHasNotKey("maxDepth", 5);
            I.setIfHasNotKey("caustics", 1);
            I.setIfHasNotKey("adaptive", Json::JsonObject(false));
            I.setIfHasNotKey("progressive", Json::JsonObject(false));
            I.setIfHasNotKey("nDirect", 16);
            I.setIfHasNotKey("nChains", 8);
            I.setIfHasNotKey("largeStep", Json::JsonObject(0.25f));

            parameters.addFloat("integrator.maxRayIntensity", IO::deserialize<Float>(I["maxRayIntensity"]));
            parameters.addInt("integrator.spp", IO::deserialize<int>(I["spp"]));
            parameters.addInt("integrator.minDepth", IO::deserialize<int>(I["minDepth"]));
            parameters.addInt("integrator.maxDepth", IO::deserialize<int>(I["maxDepth"]));
            parameters.addInt("integrator.caustics", I["caustics"].getBool());
            parameters.addInt("integrator.adaptive", I["adaptive"].getBool());
            parameters.addInt("integrator.progressive", I["progressive"].getBool());
            parameters.addFloat("integrator.maxSampleFactor", IO::deserialize<Float>(I["maxSampleFactor"]));
            parameters.addFloat("integrator.maxError", IO::deserialize<Float>(I["maxError"]));
            parameters.addFloat("integrator.heuristic", IO::deserialize<Float>(I["heuristic"]));
            parameters.addFloat("integrator.pValue", IO::deserialize<Float>(I["pValue"]));
            parameters.addInt("integrator.nChains", IO::deserialize<int>(I["nChains"]));
            parameters.addInt("integrator.nDirect", IO::deserialize<int>(I["nDirect"]));
            parameters.addFloat("integrator.largeStep", IO::deserialize<Float>(I["largeStep"]));

            if (I.hasKey("type")) {
                auto type = I["type"].getString();
                if (type == "volpath" || type == "path") {
                    parameters.addString("integrator", "volpath");
                    integrator = std::make_unique<VolPath>(parameters);
                } else if (type == "bdpt") {
                    parameters.addString("integrator", "bdpt");
                    integrator = std::make_unique<BDPT>(parameters);
                } else if (type == "mlt" || type == "pssmlt") {
                    parameters.addString("integrator", type);
                    if (type == "mlt")
                        integrator = std::make_unique<MultiplexedMLT>(parameters);
                    else
                        integrator = std::make_unique<PSSMLT>(parameters);
                } else if (type == "erpt") {
                    parameters.addString("integrator", "erpt");
                    integrator = std::make_unique<ERPT>(parameters);
                } else {
                    fmt::print(stderr, "Unknown integrator type `{}`\n", type);
                }
            }
        }
    }

    void RenderEngine::loadCamera() {
        auto &parameters = scene.parameters();
        if (description.hasKey("camera")) {
            auto &camera = description["camera"];
            if (camera.hasKey("translation")) {
                parameters.addVec3f("camera.translation", IO::deserialize<Vec3f>(camera["translation"]));
            }
            if (camera.hasKey("rotation")) {
                parameters.addVec3f("camera.rotation", IO::deserialize<Vec3f>(camera["rotation"]));
            }
            camera.setIfHasNotKey("lensRadius", 0);
            camera.setIfHasNotKey("focalDistance", 0);
            if (camera.hasKey("fov")) {
                parameters.addFloat("camera.fov", IO::deserialize<Float>(camera["fov"]));
            }
            if (camera.hasKey("lensRadius")) {
                parameters.addFloat("camera.lensRadius", IO::deserialize<Float>(camera["lensRadius"]));
            }
            if (camera.hasKey("focalDistance")) {
                parameters.addFloat("camera.focalDistance", IO::deserialize<Float>(camera["focalDistance"]));
            }
        }
    }

    void RenderEngine::updateMaterials() {
        for (auto i: scene.instances) {
            scene.factory->applyMaterial(description["shapes"][i->name], description["materials"], *i);
        }

    }
}





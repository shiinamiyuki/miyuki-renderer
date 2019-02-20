//
// Created by Shiina Miyuki on 2019/1/29.
//
#include "rendersystem.hpp"

#include "../utils/jsonparser.hpp"
#include "../integrators/ao/ao.h"
#include "../integrators/pathtracer/pathtracer.h"
#include "../integrators/bdpt/bdpt.h"
#include "../integrators/pssmlt/pssmlt.h"
#include "../integrators/mmlt/mmlt.h"

using namespace Miyuki;
using namespace Miyuki::Json;
RenderSystem renderSystem;

void saveAtExit() {
    renderSystem.save();
}

void RenderSystem::readDescription(const std::string &filename) {
    readUnderPath(filename, [&](const std::string &file) {
        std::ifstream in(file);
        if (!in) {
            fmt::print(stderr, "Cannot open scene description {}\n", filename);
            exit(-1);
        }
        std::string content((std::istreambuf_iterator<char>(in)),
                            (std::istreambuf_iterator<char>()));
        JsonObject document = parse(content);
        using Value = JsonObject;

        auto readVec3f = [&](const Value &v) -> Vec3f {
            if (v.isArray()) {
                if (v.getArray().size() != 3) {
                    fmt::print(stderr, "three elements expected in vec3f\n");
                    return {};
                }
                return Vec3f(v[0].getFloat(), v[1].getFloat(), v[2].getFloat());
            } else if (v.isString()) { // rgb
                if (v.getString().length() != 6) {
                    fmt::print(stderr, "rgb format expected");
                    return {};
                }
                uint32_t x;
                std::stringstream ss;
                ss << std::hex << v.getString();
                ss >> x;
                uint32_t r, g, b;
                r = (x & (0xff0000)) >> 16;
                g = (x & (0x00ff00)) >> 8;
                b = x & (0x0000ff);
                return Vec3f(r, g, b) / 255.0;
            }
            return {};
        };
        auto readTransform = [&](const Value &v) -> Transform {
            // given that v is {"translate":.., "rotate":..}
            Vec3f translate, rotate;
            Float scale = 1;
            if (v.hasKey("translate")) {
                translate = readVec3f(v["translate"]);
            }
            if (v.hasKey("rotate")) {
                rotate = readVec3f(v["rotate"]) / 180.0f * M_PI;
            }
            if (v.hasKey("scale")) {
                scale = v["scale"].getFloat();
            }
            return Transform(translate, rotate, scale);
        };
        auto readMeshes = [&]() -> void {
            if (document.hasKey("meshes")) {
                const Value &meshes = document["meshes"];
                if (meshes.isArray()) {
                    for (auto i = 0; i < meshes.getArray().size(); i++) {
                        const Value &mesh = meshes[i];
                        TextureOption opt = TextureOption::use;
                        if (!mesh.isObject()) {
                            fmt::print(stderr, "mesh is required to be a object");
                        }
                        if (mesh.hasKey("texture")) {
                            std::string s = mesh["texture"].getString();
                            if (s == "use") {
                                opt = TextureOption::use;
                            }
                            if (s == "discard") {
                                opt = TextureOption::discard;
                            }
                            if (s == "raw") {
                                opt = TextureOption((int32_t) opt | (int32_t) TextureOption::raw);
                            }
                        }
                        if (mesh.hasKey("file")) {
                            auto meshFilename = mesh["file"].getString();
                            Transform transform;
                            if (mesh.hasKey("transform")) {
                                transform = readTransform(mesh["transform"]);
                            }
                            scene.loadObjTrigMesh(meshFilename.c_str(), transform, opt);
                        }
                    }
                }
            }
        };
        auto readCamera = [&]() -> void {
            if (document.hasKey("camera")) {
                const Value &camera = document["camera"];
                if (camera.hasKey("transform")) {
                    auto transform = readTransform(camera["transform"]);
                    scene.getCamera().moveTo(transform.translation);
                    scene.getCamera().rotateTo(transform.rotation);
                }
                if (camera.hasKey("fov")) {
                    scene.getCamera().fov = camera["fov"].getFloat() / 180.0 * M_PI;
                }
                if (camera.hasKey("resolution")) {
                    const Value &resolution = camera["resolution"];
                    if (resolution.isArray())
                        scene.setFilmDimension(Point2i(resolution[0].getInt(), resolution[1].getInt()));
                    else if (resolution.isString()) {
                        const std::string r = resolution.getString();
                        if (r == "1080p") {
                            scene.setFilmDimension(Point2i(1920, 1080));
                        } else if (r == "4k") {
                            scene.setFilmDimension(Point2i(3840, 2160));
                        } else if (r == "240p") {
                            scene.setFilmDimension(Point2i(426, 240));
                        } else if (r == "360p") {
                            scene.setFilmDimension(Point2i(480, 360));
                        } else if (r == "720p") {
                            scene.setFilmDimension(Point2i(1280, 720));
                        } else {
                            fmt::print(stderr, "Unrecognized resolution format\n");
                        }
                    } else {
                        fmt::print(stderr, "Unrecognized resolution format\n");
                    }
                }
            }
        };
        auto readIntegrator = [&]() -> void {
            if (document.hasKey("integrator")) {
                const Value &integratorInfo = document["integrator"];
                if (integratorInfo.hasKey("type")) {
                    integratorName = integratorInfo["type"].getString();
                    if (integratorName == "path-tracer" || integratorName == "pt") {
                        integrator = std::make_unique<PathTracer>();
                    } else if (integratorName == "ambient-occlusion") {
                        integrator = std::make_unique<AOIntegrator>();
                    } else if (integratorName == "bdpt") {
                        integrator = std::make_unique<BDPT>();
                    } else if (integratorName == "pssmlt") {
                        integrator = std::make_unique<PSSMLTUnidirectional>();
                    } else if (integratorName == "mmlt" || integratorName == "mlt") {
                        integrator = std::make_unique<MultiplexedMLT>();
                    } else {
                        fmt::print(stderr, "Unrecognized integrator: {}\n", integratorName);
                    }
                }
                if (integratorInfo.hasKey("max-depth")) {
                    scene.option.maxDepth = integratorInfo["max-depth"].getInt();
                }
                if (integratorInfo.hasKey("min-depth")) {
                    scene.option.minDepth = integratorInfo["min-depth"].getInt();
                }
                if (integratorInfo.hasKey("ambient-light")) {
                    auto ambient = readVec3f(integratorInfo["ambient-light"]);
                    scene.setAmbientLight(Spectrum(ambient.x(), ambient.y(), ambient.z()));
                }
                if (integratorInfo.hasKey("show-ambient-light")) {
                    auto ambient = integratorInfo["show-ambient-light"].getBool();
                    scene.option.showAmbientLight = ambient;
                }
                if (integratorInfo.hasKey("occlude-distance")) {
                    auto d = integratorInfo["occlude-distance"].getFloat();
                    scene.option.aoDistance = d;
                }
                if (integratorInfo.hasKey("sampler")) {
                    std::string s = integratorInfo["sampler"].getString();
                    if (s == "independent") {
                        scene.useSampler(Option::independent);
                    } else if (s == "stratified")
                        scene.useSampler(Option::stratified);
                    else if (s == "sobol")
                        scene.useSampler(Option::sobol);
                    else {
                        fmt::print(stderr, "Unrecognized sampler type: {}\n", s);
                    }
                }
            }
        };
        auto readRender = [&]() -> void {
            if (document.hasKey("render")) {
                const Value &render = document["render"];
                if (render.hasKey("output-file")) {
                    outputFile = render["output-file"].getString();
                }
                if (render.hasKey("spp")) {
                    scene.option.samplesPerPixel = render["spp"].getInt();
                }
                if (render.hasKey("sleep-time")) {
                    scene.option.sleepTime = render["sleep-time"].getInt();
                }
            }
        };
        auto readLights = [&]() -> void {
            if (document.hasKey("lights")) {
                const Value &lights = document["lights"];
                if (!lights.isArray()) {
                    fmt::print(stderr, "Expect lights to be a list\n");
                } else {
                    for (auto i = 0; i < lights.getArray().size(); i++) {
                        const Value &light = lights[i];
                        if (!light.isObject()) {
                            fmt::print(stderr, "Each light should be an object\n");
                        } else {
                            if (light.hasKey("type")) {
                                std::string type = light["type"].getString();
                                if (type == "point") {
                                    auto ka = readVec3f(light["ka"]);
                                    auto pos = readVec3f(light["position"]);
                                    Float strength = 1;
                                    if (light.hasKey("strength"))
                                        strength = light["strength"].getFloat();
                                    ka *= strength;
                                    scene.addPointLight(Spectrum(ka.x(), ka.y(), ka.z()), pos);
                                } else {
                                    fmt::print(stderr, "unrecognized light type {}\n", type);
                                }
                            }
                        }
                    }
                }
            }
        };
        readCamera();
        readMeshes();
        readIntegrator();
        readRender();
        readLights();
    });
    if (!integrator) {
        fmt::print("Using bdpt for default settings\n");
        integratorName = "bdpt";
        integrator = std::make_unique<BDPT>();
    }
    scene.prepare();

}

void RenderSystem::readImage(std::vector<uint8_t> &pixelData, int *width, int *height) {
    scene.readImage(pixelData);
    *width = scene.getResolution().x();
    *height = scene.getResolution().y();
}

void RenderSystem::GUIMode() {
    scene.setUpdateFunc([&](Scene &) {
        guiCallBack();
    });
}

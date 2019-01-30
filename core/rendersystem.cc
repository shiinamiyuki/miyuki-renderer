//
// Created by Shiina Miyuki on 2019/1/29.
//
#include "rendersystem.hpp"

#include "../lib/rapidjson/document.h"
#include "../lib/rapidjson/writer.h"
#include "../lib/rapidjson/stringbuffer.h"

using namespace rapidjson;
using namespace Miyuki;
RenderSystem renderSystem;

void saveAtExit() {
    renderSystem.save();
}

void RenderSystem::readDescription(const std::string &filename) {
    readUnderPath(filename, [&](const std::string &file) {
        std::ifstream in(file);
        std::string content((std::istreambuf_iterator<char>(in)),
                            (std::istreambuf_iterator<char>()));
        Document document;
        document.Parse(content.c_str());

        auto readVec3f = [&](const Value &v) -> Vec3f {
            if (v.IsArray()) {
                if (v.Size() != 3) {
                    fmt::print(stderr, "three elements expected in vec3f\n");
                    return {};
                }
                return Vec3f(v[0].GetFloat(), v[1].GetFloat(), v[2].GetFloat());
            } else if (v.IsString()) { // rgb
                if (v.GetStringLength() != 6) {
                    fmt::print(stderr, "rgb format expected");
                    return {};
                }
                unsigned int x;
                std::stringstream ss;
                ss << std::hex << v.GetString();
                ss >> x;
                unsigned int r, g, b;
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
            if (v.HasMember("translate")) {
                translate = readVec3f(v["translate"]);
            }
            if (v.HasMember("rotate")) {
                rotate = readVec3f(v["rotate"]) / 180.0f * M_PI;
            }
            if (v.HasMember("scale")) {
                scale = v["scale"].GetFloat();
            }
            return Transform(translate, rotate, scale);
        };
        auto readMeshes = [&]() -> void {
            if (document.HasMember("meshes")) {
                const Value &meshes = document["meshes"];
                if (meshes.IsArray()) {
                    for (SizeType i = 0; i < meshes.Size(); i++) {
                        const Value &mesh = meshes[i];
                        if (!mesh.IsObject()) {
                            fmt::print(stderr, "mesh is required to be a object");
                        }
                        if (mesh.HasMember("file")) {
                            auto meshFilename = mesh["file"].GetString();
                            Transform transform;
                            if (mesh.HasMember("transform")) {
                                transform = readTransform(mesh["transform"]);
                            }
                            scene.loadObjTrigMesh(meshFilename, transform);
                        }
                    }
                }
            }
        };
        auto readCamera = [&]() -> void {
            if (document.HasMember("camera")) {
                const Value &camera = document["camera"];
                if (camera.HasMember("transform")) {
                    auto transform = readTransform(camera["transform"]);
                    scene.getCamera().moveTo(transform.translation);
                    scene.getCamera().rotateTo(transform.rotation);
                }
                if (camera.HasMember("fov")) {
                    scene.getCamera().fov = camera["fov"].GetFloat() / 180.0 * M_PI;
                }
                if (camera.HasMember("resolution")) {
                    const Value &resolution = camera["resolution"];
                    if (resolution.IsArray())
                        scene.setFilmDimension(Point2i(resolution[0].GetInt(), resolution[1].GetInt()));
                    else if (resolution.IsString()) {
                        const std::string r = resolution.GetString();
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
            if (document.HasMember("integrator")) {
                const Value &integratorInfo = document["integrator"];
                integratorName = integratorInfo["type"].GetString();
                if (integratorName == "path-tracer") {
                    integrator = std::make_unique<PathTracer>();
                } else if (integratorName == "ambient-occlusion") {
                    integrator = std::make_unique<AOIntegrator>();
                }else if (integratorName == "bdpt") {
                    integrator = std::make_unique<BDPT>();
                }else if (integratorName == "pssmlt") {
                    integrator = std::make_unique<PSSMLTUnidirectional>();
                } else {
                    fmt::print(stderr, "Unrecognized integrator: {}\n", integratorName);
                }
                if (integratorInfo.HasMember("max-depth")) {
                    scene.option.maxDepth = integratorInfo["max-depth"].GetInt();
                }
                if (integratorInfo.HasMember("min-depth")) {
                    scene.option.minDepth = integratorInfo["min-depth"].GetInt();
                }
                if (integratorInfo.HasMember("ambient-light")) {
                    auto ambient = readVec3f(integratorInfo["ambient-light"]);
                    scene.setAmbientLight(Spectrum(ambient.x(), ambient.y(), ambient.z()));
                }
                if (integratorInfo.HasMember("show-ambient-light")) {
                    auto ambient = integratorInfo["show-ambient-light"].GetBool();
                    scene.option.showAmbientLight = ambient;
                }
                if (integratorInfo.HasMember("occlude-distance")) {
                    auto d = integratorInfo["occlude-distance"].GetFloat();
                    scene.option.aoDistance = d;
                }
            }
        };
        auto readRender = [&]() -> void {
            if (document.HasMember("render")) {
                const Value &render = document["render"];
                if (render.HasMember("output-file")) {
                    outputFile = render["output-file"].GetString();
                }
                if (render.HasMember("spp")) {
                    scene.option.samplesPerPixel = render["spp"].GetInt();
                }
                if (render.HasMember("sleep-time")) {
                    scene.option.sleepTime = render["sleep-time"].GetInt();
                }
            }
        };
        auto readLights = [&]() -> void {
            if (document.HasMember("lights")) {
                const Value &lights = document["lights"];
                if (!lights.IsArray()) {
                    fmt::print(stderr, "Expect lights to be a list\n");
                } else {
                    for (SizeType i = 0; i < lights.Size(); i++) {
                        const Value &light = lights[i];
                        if (!light.IsObject()) {
                            fmt::print(stderr, "Each light should be an object\n");
                        } else {
                            if (light.HasMember("type")) {
                                std::string type = light["type"].GetString();
                                if (type == "point") {
                                    auto ka = readVec3f(light["ka"]);
                                    auto pos = readVec3f(light["position"]);
                                    Float strength = 1;
                                    if (light.HasMember("strength"))
                                        strength = light["strength"].GetFloat();
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
    scene.prepare();

}

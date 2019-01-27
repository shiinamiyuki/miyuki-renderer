//
// Created by Shiina Miyuki on 2019/1/12.
//

#include "util.h"
#include "mesh.h"
#include "scene.h"

#include "../lib/rapidjson/document.h"
#include "../lib/rapidjson/writer.h"
#include "../lib/rapidjson/stringbuffer.h"
//#include "../lib/cxxopts.hpp"

using namespace rapidjson;
using namespace Miyuki;


class RenderSystem {
    Scene scene;
    std::unique_ptr<Integrator> integrator;
    std::string integratorName;
    std::string outputFile;
public:
    RenderSystem(int argc, char **argv) {
        Init();
        outputFile = "out.png";
    }
    void processOptions(int argc, char**argv){
        printWelcome();
        if(argc <= 1){         
          exit(0);
        }
        readDescription(argv[1]);
    }
    int render() {
        if(!integrator){return -1;}
        fmt::print(R"(
Resolution: {0}x{1}
Output File: {2}
Samples per Pixel: {3}
Integrator: {4}
)", scene.getResolution().x(),
                   scene.getResolution().y(),
                   outputFile,
                   scene.option.samplesPerPixel,
                   integratorName);
        integrator->render(scene);
        scene.writeImage(outputFile);
        return 0;
    }

    void readDescription(const std::string &filename) {
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
                        scene.getCamera().rotateTo(transform.rotation / 180.0f * M_PI);
                    }
                    if (camera.HasMember("fov")) {
                        scene.getCamera().fov = camera["fov"].GetFloat() / 180.0 * M_PI;
                    }
                    if (camera.HasMember("resolution")) {
                        const Value &resolution = camera["resolution"];
                        scene.setFilmDimension(Point2i(resolution[0].GetInt(), resolution[1].GetInt()));
                    }
                }
            };
            auto readIntegrator = [&]() -> void {
                if (document.HasMember("integrator")) {
                    const Value &integratorInfo = document["integrator"];
                    integratorName = integratorInfo["type"].GetString();
                    if (integratorName == "path-tracer") {
                        integrator = std::make_unique<PathTracer>(PathTracer());
                    } else if (integratorName == "ambient-occlusion") {
                        integrator = std::make_unique<AOIntegrator>(AOIntegrator());
                    } else {
                        fmt::print(stderr, "Unrecognized integrator: {}\n", integratorName);
                    }
                    if (integratorInfo.HasMember("max-depth")) {
                        scene.option.maxDepth = integratorInfo["max-depth"].GetInt();
                    }
                    if (integratorInfo.HasMember("min-depth")) {
                        scene.option.minDepth = integratorInfo["min-depth"].GetInt();
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
                }
            };
            readCamera();
            readMeshes();
            readIntegrator();
            readRender();
        });
        scene.prepare();
    }

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

    int exec() {
        return render();
    }

    ~RenderSystem() {
        Exit();
    }
};

int main(int argc, char **argv) {
    RenderSystem renderSystem(argc, argv);
    renderSystem.processOptions(argc, argv);
    return renderSystem.exec();
}
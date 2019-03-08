//
// Created by Shiina Miyuki on 2019/3/3.
//

#ifndef MIYUKI_SCENE_H
#define MIYUKI_SCENE_H

#include <cameras/camera.h>

#include "embreescene.h"
#include "mesh.h"
#include "rendercontext.h"
#include "film.h"
#include "core/memory.h"
#include "math/distribution.h"
#include "parameter.h"
#include "materials/materialfactory.h"

namespace Miyuki {
    class EmbreeScene;

    class Integrator;

    class VolPath;

    class Camera;

    class Sampler;

    class RenderEngine;

    class Scene {
        std::unique_ptr<EmbreeScene> embreeScene;
        std::vector<std::shared_ptr<Mesh>> instances;
        std::map<std::string, std::shared_ptr<Mesh>> meshes;
        std::unique_ptr<Camera> camera;
        std::unique_ptr<Film> film;
        std::vector<Seed> seeds;
        std::vector<Sampler *> samplers;
        std::vector<MemoryArena> arenas;
        std::vector<std::shared_ptr<Light>> lights;
        std::unique_ptr<Distribution1D> lightDistribution;
        std::unordered_map<Light *, Float> lightPdfMap;
        std::unique_ptr<MaterialFactory> factory;
        ParameterSet parameterSet;
        MemoryArena samplerArena;


        friend class Integrator;

        friend class VolPath;

        friend class RenderEngine;

        void computeLightDistribution();

        Json::JsonObject description;
    public:
        Scene();


        ParameterSet &parameters() {
            return parameterSet;
        }

        void setFilmDimension(const Point2i &);

        void loadObjMesh(const std::string &filename);

        void loadObjMeshAndInstantiate(const std::string &name, const Transform &T = Transform());

        void instantiateMesh(const std::string &name, const Transform &);

        void commit();

        bool intersect(const RayDifferential &ray, Intersection *);

        RenderContext getRenderContext(const Point2i &raster, MemoryArena *);

        void test();

        void saveImage();

        Light *chooseOneLight(Sampler *, Float *pdf);

        Float pdfLightChoice(Light *light) {
            return lightPdfMap[light];
        }
    };


}
#endif //MIYUKI_SCENE_H

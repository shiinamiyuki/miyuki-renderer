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
        std::vector<std::shared_ptr<Light>> lights;
        std::unique_ptr<Distribution1D> lightDistribution;
        std::unordered_map<Light *, Float> lightPdfMap;
        std::unique_ptr<MaterialFactory> factory;
        std::function<void(Scene&)> updateFunc;
        std::function<bool(Scene&)> processContinueFunc;
        ParameterSet parameterSet;

        friend class Integrator;

        friend class VolPath;

        friend class RenderEngine;

        void computeLightDistribution();

        Json::JsonObject description;
    public:
        Scene();

        void readImage(std::vector<uint8_t> &pixelData);

        ParameterSet &parameters() {
            return parameterSet;
        }

        void setFilmDimension(const Point2i &);

        void loadObjMesh(const std::string &filename);

        void loadObjMeshAndInstantiate(const std::string &name, const Transform &T = Transform());

        void instantiateMesh(const std::string &name, const Transform &);

        void commit();

        bool intersect(const RayDifferential &ray, Intersection *);

        RenderContext getRenderContext(const Point2i &raster, MemoryArena *,Sampler * );

        void saveImage();

        Light *chooseOneLight(Sampler *, Float *pdf);

        Float pdfLightChoice(Light *light) {
            return lightPdfMap[light];
        }

        void update();

        void setUpdateFunc(std::function<void(Scene&)> func){
            updateFunc = func;
        }

        bool processContinuable(){
            return processContinueFunc(*this);
        }
    };


}
#endif //MIYUKI_SCENE_H

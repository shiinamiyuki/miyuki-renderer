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
#include <lights/infinite.h>

namespace Miyuki {
    class EmbreeScene;

    class Integrator;

    class SamplerIntegrator;

    class VolPath;

    class BDPT;

    class MultiplexedMLT;

    class PSSMLT;

    class Camera;

    class Sampler;

    class RenderEngine;

    class ERPT;

    class Scene {
        std::unique_ptr<EmbreeScene> embreeScene;
        std::vector<std::shared_ptr<Mesh>> instances;
        std::map<std::string, std::shared_ptr<Mesh>> meshes;
        std::unique_ptr<Camera> camera;
        std::unique_ptr<Film> film;
        std::vector<std::shared_ptr<Light>> lights;
        std::unique_ptr<Distribution1D> lightDistribution;
        std::unordered_map<const Light *, Float> lightPdfMap;
        std::unique_ptr<MaterialFactory> factory;
        std::function<void(Scene &)> updateFunc;
        std::function<bool(Scene &)> processContinueFunc;
        std::function<void(std::vector<uint8_t> &)> readImageFunc;
        ParameterSet parameterSet;

        friend class Integrator;

        friend class VolPath;

        friend class RenderEngine;

        friend class SamplerIntegrator;

        friend class BDPT;

        friend class MultiplexedMLT;

        friend class ERPT;

        class PSSMLT;

        void computeLightDistribution();

        Json::JsonObject description;
    public:
        std::unique_ptr<InfiniteAreaLight> infiniteAreaLight;

        Scene();

        void readImage(std::vector<uint8_t> &pixelData);

        ParameterSet &parameters() {
            return parameterSet;
        }

        void setFilmDimension(const Point2i &);

        Point2i filmDimension() const { return {film->width(), film->height()}; }

        void loadObjMesh(const std::string &filename);

        void loadObjMeshAndInstantiate(const std::string &name, const Transform &T = Transform());

        void instantiateMesh(const std::string &name, const Transform &);

        void commit();

        bool intersect(const RayDifferential &ray, Intersection *);

        RenderContext getRenderContext(const Point2i &raster, MemoryArena *, Sampler *);

        RenderContext getRenderContext(const Point2f &raster, MemoryArena *, Sampler *);

        void saveImage();

        Light *chooseOneLight(Sampler *, Float *pdf);

        Float pdfLightChoice(const Light *light) {
            return lightPdfMap[light];
        }

        void update();

        void setUpdateFunc(std::function<void(Scene &)> func) {
            updateFunc = func;
        }

        bool processContinuable() {
            return processContinueFunc(*this);
        }
    };


}
#endif //MIYUKI_SCENE_H

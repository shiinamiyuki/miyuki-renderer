//
// Created by Shiina Miyuki on 2019/1/12.
//

#ifndef MIYUKI_SCENE_HPP
#define MIYUKI_SCENE_HPP

#include "util.h"
#include "film.h"
#include "mesh.h"
#include "sampler.h"
#include "light.h"
#include "intersection.h"
#include "ray.h"
#include "interaction.h"
#include "integrator.h"
#include "material.h"
#include "transform.h"
#include "distribution.h"
#include "../integrator/ao.h"
#include "../integrator/pathtracer.h"
#include "../integrator/bdpt.h"
#include "../integrator/pssmlt.h"
#include "../sampler/random.h"

namespace Miyuki {

    class Material;

    class MaterialList :
            public std::vector<Material> {
        std::unordered_map<std::string, int> map;
    public:

    };

    struct Camera {
        Vec3f viewpoint;
        Vec3f direction;
        double fov;

        void moveTo(const Vec3f &pos) { viewpoint = pos; }

        void rotateTo(const Vec3f &dir) {
            direction = dir;
        }

        void rotate(const Vec3f &dir) {
            direction += dir;
        }

        void lookAt(const Vec3f &pos);

        Camera(const Vec3f &v = Vec3f(0, 0, 0), const Vec3f &d = Vec3f(0, 0, 0))
                : viewpoint(v), direction(d), fov(M_PI / 2) {}
    };

    struct Ray;
    struct Intersection;
    struct Interaction;

    struct RenderContext {
        Ray primary;
        Sampler *sampler;

        RenderContext(const Ray &r, Sampler *s)
                : primary(r), sampler(s) {}
    };

    class Light;

    struct Option {
        int maxDepth;
        int minDepth;
        int samplesPerPixel;
        int mltLuminanceSample;
        Float largeStepProb;
        Float aoDistance;
        bool showAmbientLight;
        int saveEverySecond;
        int sleepTime;
        Option();
    };

    class Scene {
        friend class AOIntegrator;

        friend class BDPT;

        friend class PathTracer;

        friend class PSSMLTUnidirectional;

        Spectrum ambientLight;
        RTCScene rtcScene;
        Film film;
        MaterialList materialList;
        Camera camera;
        std::vector<Mesh::MeshInstance> instances;
        std::vector<Seed> seeds;
        std::vector<RandomSampler> samplers;
        std::vector<std::shared_ptr<Light>> lightList; // contains all user defined lights
        std::vector<std::shared_ptr<Light>> lights;    // contains all lights after commit() is called
        std::unique_ptr<Distribution1D> lightDistribution;

        // commit and preprocess scene
        void commit();

        void computeLightDistribution();

        void checkError();

        void addMesh(std::shared_ptr<Mesh::TriangularMesh>, const Transform &transform = Transform());

        void instantiateMesh(std::shared_ptr<Mesh::TriangularMesh>);

        const Mesh::MeshInstance::Primitive &fetchIntersectedPrimitive(const Intersection &);

        void fetchInteraction(const Intersection &, Ref<Interaction> interaction);

        void foreachPixel(std::function<void(const Point2i &)>);

        Light *chooseOneLight(Sampler &) const;

        const std::vector<std::shared_ptr<Light>> &getAllLights() const;

        void postResize();

    public:
        Option option;
        void addPointLight(const Spectrum& ka, const Vec3f& pos);

        void setAmbientLight(const Spectrum &s) { ambientLight = s; }

        RTCScene sceneHandle() const { return rtcScene; }

        void prepare();

        void loadObjTrigMesh(const char *filename, const Transform &transform = Transform());

        void writeImage(const std::string &filename);

        void setFilmDimension(const Point2i &);

        void renderPreview();

        Camera &getCamera() { return camera; }

        RenderContext getRenderContext(const Point2i &);

        Scene();

        ~Scene();

        RTCScene getRTCSceneHandle() const { return rtcScene; }

        Point2i getResolution() const { return Point2i(film.width(), film.height()); };
    };
}


#endif //MIYUKI_SCENE_HPP

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
#include "memory.h"
#include "../integrator/ao.h"
#include "../integrator/pathtracer.h"
#include "../integrator/bdpt.h"
#include "../integrator/pssmlt.h"
#include "../sampler/random.h"
#include "../sampler/stratified.h"

namespace Miyuki {


    class Material;

    using MaterialPtr = std::shared_ptr<Material>;

    class MaterialList :
            public std::vector<MaterialPtr> {
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
        MemoryArena &arena;
        Point2i raster;
        RenderContext(const Ray &r, Sampler *s, MemoryArena &a, const Point2i&pos)
        : arena(a), primary(r), sampler(s),raster(pos) {}
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
        enum SamplerType {
            independent,
            stratified,
        } samplerType;

        Option();
    };

    struct MeshInstance;
    enum class TextureOption {
        discard = 0,
        use = 1,
        raw = 2,
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
        std::vector<MeshInstance> instances;
        std::vector<Seed> seeds;
        std::vector<RandomSampler> uniformSamplers;
        std::vector<StratifiedSampler> stratSamplers;
        std::vector<std::shared_ptr<Light>> lightList; // contains all user defined lights
        std::vector<std::shared_ptr<Light>> lights;    // contains all lights after commit() is called
        std::unique_ptr<Distribution1D> lightDistribution;

        // commit and preprocess scene
        void commit();

        void computeLightDistribution();

        void checkError();

        void addMesh(std::shared_ptr<TriangularMesh>, const Transform &transform = Transform());

        void instantiateMesh(std::shared_ptr<TriangularMesh>, const Transform &transform);

        const Primitive &fetchIntersectedPrimitive(const Intersection &);

        void fetchInteraction(const Intersection &, Interaction* interaction);

        void foreachPixel(std::function<void(RenderContext &)>);

        Light *chooseOneLight(Sampler &) const;

        const std::vector<std::shared_ptr<Light>> &getAllLights() const;

        void postResize();

    public:
        Option option;

        void useSampler(Option::SamplerType);

        void addPointLight(const Spectrum &ka, const Vec3f &pos);

        void setAmbientLight(const Spectrum &s) { ambientLight = s; }

        RTCScene sceneHandle() const { return rtcScene; }

        void prepare();

        void loadObjTrigMesh(const char *filename,
                             const Transform &transform = Transform(),
                             TextureOption opt = TextureOption::use);

        void writeImage(const std::string &filename);

        void setFilmDimension(const Point2i &);

        Camera &getCamera() { return camera; }

        RenderContext getRenderContext(MemoryArena&,const Point2i &);

        Scene();

        ~Scene();

        RTCScene getRTCSceneHandle() const { return rtcScene; }

        Point2i getResolution() const { return Point2i(film.width(), film.height()); };
    };
}


#endif //MIYUKI_SCENE_HPP

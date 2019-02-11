//
// Created by Shiina Miyuki on 2019/1/12.
//

#ifndef MIYUKI_SCENE_HPP
#define MIYUKI_SCENE_HPP

#include "../utils/util.h"
#include "film.h"
#include "mesh.h"
#include "../samplers/sampler.h"
#include "../lights/light.h"
#include "intersection.h"
#include "ray.h"
#include "scatteringevent.h"
#include "../integrators/integrator.h"
#include "../bsdfs/bsdf.h"
#include "../math/transform.h"
#include "../math/distribution.h"
#include "memory.h"

#include "../samplers/random.h"
#include "../samplers/stratified.h"

namespace Miyuki {


    class Material;

    class BSDF;

    using MaterialPtr = std::shared_ptr<BSDF>;

    class MaterialList :
            public std::vector<MaterialPtr> {
        std::unordered_map<std::string, int32_t> map;
    public:
        void addMaterial(const std::string &name, int32_t id) { map[name] = id; }

        MaterialPtr getMaterial(const std::string &name) { return at(map[name]); }
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

    class Intersection;

    struct IntersectionInfo;

    struct RenderContext {
        Ray primary;
        Sampler *sampler;
        MemoryArena &arena;
        Point2i raster;

        RenderContext(const Ray &r, Sampler *s, MemoryArena &a, const Point2i &pos)
                : arena(a), primary(r), sampler(s), raster(pos) {}
    };

    class Light;

    struct Option {
        int32_t maxDepth;
        int32_t minDepth;
        int32_t samplesPerPixel;
        int32_t mltLuminanceSample;
        Float largeStepProb;
        Float aoDistance;
        bool showAmbientLight;
        int32_t saveEverySecond;
        int32_t sleepTime;
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

        void getIntersectionInfo(const Intersection &, IntersectionInfo *info);

        void foreachPixel(std::function<void(RenderContext &)>);

        Light *chooseOneLight(Sampler &) const;

        const std::vector<std::shared_ptr<Light>> &getAllLights() const;

        void postResize();

    public:
        MemoryArena miscArena;
        Option option;

        bool intersect(const Ray &, IntersectionInfo *);

        void addSphere(const Vec3f &pos, Float r, int materialId);

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

        RenderContext getRenderContext(MemoryArena &, const Point2i &);

        Scene();

        ~Scene();

        RTCScene getRTCSceneHandle() const { return rtcScene; }

        Point2i getResolution() const { return Point2i(film.width(), film.height()); };
    };
}


#endif //MIYUKI_SCENE_HPP

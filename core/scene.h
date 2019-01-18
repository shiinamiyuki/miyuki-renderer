//
// Created by Shiina Miyuki on 2019/1/12.
//

#ifndef MIYUKI_SCENE_HPP
#define MIYUKI_SCENE_HPP

#include "util.h"
#include "Film.h"
#include "mesh.h"
#include "sampler.h"

namespace Miyuki {
    struct Material {
        Spectrum ka, kd, ks;
        Float glossiness;

        Material() {}
    };

    class MaterialList :
            public std::vector<Material> {
        std::unordered_map<std::string, int> map;
    public:

    };

    struct Camera {
        Vec3f viewpoint;
        Vec3f direction;
        double aov;

        void moveTo(const Vec3f &pos) { viewpoint = pos; }

        void rotateTo(const Vec3f &dir) {
            direction = dir;
        }

        void rotate(const Vec3f &dir) {
            direction += dir;
        }

        void lookAt(const Vec3f &pos);

        Camera(const Vec3f &v = Vec3f(0, 0, 0), const Vec3f &d = Vec3f(0, 0, 0))
                : viewpoint(v), direction(d), aov(M_PI / 2) {}
    };

    struct Ray {
        Vec3f o, d;

        Ray(const Vec3f &_o, const Vec3f &_d) : o(_o), d(_d) {}

        Ray(const RTCRay &ray);

        RTCRay toRTCRay() const;
    };

    struct RenderContext {
        Ray primary;

        RenderContext(const Ray &r) : primary(r) {}
    };

    struct Intersection {
        RTCIntersectContext context;
        RTCRayHit rayHit;

        Intersection(const RTCRay &ray);
        Intersection(const Ray &ray);

        void intersect(RTCScene scene);

        void occlude(RTCScene scene);

        bool hit() const { return rayHit.hit.geomID != RTC_INVALID_GEOMETRY_ID; }

        unsigned int geomID() const { return rayHit.hit.geomID; }

        unsigned int primID() const { return rayHit.hit.primID; }

        Float hitDistance() const { return rayHit.ray.tfar; }

        Vec3f intersectionPoint() const;
    };

    class Scene {

        RTCScene rtcScene;
        Film film;
        MaterialList materialList;
        Camera camera;
        std::vector<Mesh::MeshInstance> instances;
        std::vector<Seed> seeds;

        void commit();

        void checkError();

        void addMesh(std::shared_ptr<Mesh::TriangularMesh>);

        // must be called after `addMesh`
        void instantiateMesh(std::shared_ptr<Mesh::TriangularMesh>);

        const Mesh::MeshInstance::Primitive &fetchIntersectedPrimitive(const Intersection &);

    public:

        void loadObjTrigMesh(const char *filename);

        void writeImage(const std::string &filename);

        void setFilmDimension(const Point2i &);

        void renderPreview();

        void renderAO();

        void renderPT();

        Camera &getCamera() { return camera; }

        RenderContext getRenderContext(const Point2i &);

        Scene();

        ~Scene();

        RTCScene getRTCSceneHandle() const { return rtcScene; }
    };
}


#endif //MIYUKI_SCENE_HPP

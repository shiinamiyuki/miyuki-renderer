//
// Created by Shiina Miyuki on 2019/1/12.
//

#ifndef MIYUKI_SCENE_HPP
#define MIYUKI_SCENE_HPP

#include "util.h"
#include "Film.h"
#include "mesh.h"
#include "sampler.h"
#include "light.h"
#include "intersection.h"
#include "ray.h"
#include "interaction.h"

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

    struct Ray;
    struct Intersection;
    struct Interaction;
    struct RenderContext {
        Ray primary;

        RenderContext(const Ray &r) : primary(r) {}
    };


    class Scene {

        RTCScene rtcScene;
        Film film;
        MaterialList materialList;
        Camera camera;
        std::vector<Mesh::MeshInstance> instances;
        std::vector<Seed> seeds;
        std::vector<Light *> lightList; // contains all user defined lights
        std::vector<Light *> lights;    // contains all lights after commit() is called

        // commit and preprocess scene
        void commit();

        void checkError();

        void addMesh(std::shared_ptr<Mesh::TriangularMesh>);

        void instantiateMesh(std::shared_ptr<Mesh::TriangularMesh>);

        const Mesh::MeshInstance::Primitive &fetchIntersectedPrimitive(const Intersection &);

        void fetchInteraction(const Intersection &, Ref<Interaction> interaction);

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

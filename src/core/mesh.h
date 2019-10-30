// MIT License
// 
// Copyright (c) 2019 椎名深雪
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#ifndef MIYUKIRENDERER_MESH_H
#define MIYUKIRENDERER_MESH_H

#include <api/shape.h>
#include <api/serialize.hpp>
#include <api/accelerator.h>

namespace miyuki::core {
    class Mesh;

    /// SOA
    /// Arranged for efficient OpenGL rendering
    struct VertexData {
        std::vector<Point3f> position;
        std::vector<Normal3f> normal;
        std::vector<Point2f> tex_coord;
    };


    struct MeshTriangle final : public Shape {
        Mesh *mesh = nullptr;
        uint16_t name_id = -1;

        MeshTriangle() = default;

        MYK_DECL_CLASS(MeshTriangle, "MeshTriangle")

        const Point3f &vertex(size_t) const;

        const Normal3f &normal(size_t) const;

        const Point2f &texCoord(size_t) const;

        bool intersect(const Ray &ray, Intersection &isct) const override {
            float u, v;
            Vec3f e1 = (vertex(1) - vertex(0));
            Vec3f e2 = (vertex(2) - vertex(0));
            auto Ng = e1.cross(e2).normalized();
            float denom = (ray.d.dot(Ng));
            float t = -(ray.o - vertex(0)).dot(Ng) / denom;
            if (denom == 0)
                return false;
            if (t < ray.tMin)
                return false;
            Vec3f p = ray.o + t * ray.d;
            double det = e1.cross(e2).length();
            auto u0 = e1.cross(p - vertex(0));
            auto v0 = Vec3f(p - vertex(0)).cross(e2);
            if (u0.dot(Ng) < 0 || v0.dot(Ng) < 0)
                return false;
            v = u0.length() / det;
            u = v0.length() / det;
            if (u < 0 || v < 0 || u > 1 || v > 1)
                return false;
            if (u + v <= 1) {
                if (t < isct.distance) {
                    isct.distance = t;
                    isct.Ng = Ng;
                    isct.Ns = Ng;
                    isct.shape = this;
                    return true;
                }
            }
            return false;
        }

        Bounds3f getBoundingBox() const override {
            return Bounds3f{
                    min(vertex(0), min(vertex(1), vertex(2))),
                    max(vertex(0), max(vertex(1), vertex(2)))
            };
        }

        void sample(const Point2f &u, SurfaceSample &sample) const override {
            Point2f uv = u;
            if (uv.x + uv.y > 1.0f) {
                uv.x = 1.0f - uv.x;
                uv.y = 1.0f - uv.y;
            }
            sample.pdf = 1 / area();
            sample.p = (1 - uv.x - uv.y) * vertex(0) + uv.x * vertex(1) + uv.y * vertex(1);
            Vec3f e1 = (vertex(1) - vertex(0));
            Vec3f e2 = (vertex(2) - vertex(0));
            sample.normal = e1.cross(e2).normalized();
        }

        Float area() const override {
            return Vec3f(vertex(1) - vertex(0)).cross(vertex(2) - vertex(0)).length();
        }

        BSDF *getBSDF() const override {
            return nullptr;
        }
    };


    class Mesh final : public Shape {
        bool _loaded = false;
    public:
        std::shared_ptr<Accelerator> accelerator;
        std::vector<MeshTriangle> triangles;
        VertexData _vertex_data;
        std::vector<Point3i> _indices;
        std::vector<std::string> _names;
        std::string filename;

        MYK_DECL_CLASS(Mesh, "Mesh", interface = "Shape")

        MYK_AUTO_SER(filename);


        bool intersect(const Ray &ray, Intersection &isct) const override {
            return accelerator->intersect(ray, isct);
        }

        [[nodiscard]] Bounds3f getBoundingBox() const override {
            return accelerator->getBoundingBox();
        }

        void sample(const Point2f &u, SurfaceSample &sample) const override {
            MIYUKI_NOT_IMPLEMENTED();
        }

        [[nodiscard]] Float area() const override {
            return accelerator->area();
        }

        [[nodiscard]] BSDF *getBSDF() const override {
            return nullptr;
        }


        // TODO: set import directory
        bool importFromFile(const std::string &filename);

        void toBinary(std::vector<char> &buffer) const;

        void fromBinary(const std::vector<char> &buffer);

        bool loadFromFile(const std::string &filename);

        void preprocess() override;

    };


    class MeshInstance final : public Shape {
    public:
        MYK_DECL_CLASS(MeshInstance, "MeshInstance", interface = "Shape")

        Transform transform, invTransform;
        std::shared_ptr<Mesh> mesh;

        MYK_AUTO_SER(transform, mesh)

        bool intersect(const Ray &ray, Intersection &isct) const override {
            auto o = invTransform(ray.o);
            auto p = invTransform(ray.o + ray.d);
            Float k = (p - o).length();
            Ray transformedRay(o, (p - o).normalized(), k * ray.tMin, k * ray.tMax);
            Intersection localIsct;
            if (mesh->intersect(transformedRay, localIsct)) {
                Float t = localIsct.distance / k;
                if (t < isct.distance) {
                    isct = localIsct;
                    isct.distance = t;
                    isct.Ng = transform(isct.Ng);
                    isct.Ns = transform(isct.Ns);
                    isct.shape = localIsct.shape;
                    return true;
                }
            }
            return false;
        }

        [[nodiscard]] Bounds3f getBoundingBox() const override {
            Bounds3f box;
            box = box.unionOf(transform(mesh->getBoundingBox().pMin));
            box = box.unionOf(transform(mesh->getBoundingBox().pMax));
            return box;
        }

        void sample(const Point2f &u, SurfaceSample &sample) const override {
            mesh->sample(u, sample);
        }

        [[nodiscard]] Float area() const override {
            return mesh->area();
        }

        [[nodiscard]] BSDF *getBSDF() const override {
            return nullptr;
        }

    protected:
        void preprocess() override {
            invTransform = transform.inverse();
        }
    };

}

#endif //MIYUKIRENDERER_MESH_H

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

#include <miyuki.renderer/accelerator.h>
#include <miyuki.renderer/material.h>
#include <miyuki.foundation/serialize.hpp>
#include <miyuki.renderer/shape.h>
#include <cereal/types/unordered_map.hpp>

namespace miyuki::core {
    class Mesh;

    class AreaLight;

    /// SOA
    /// Arranged for efficient OpenGL rendering
    struct VertexData {
        std::vector<Point3f> position;
        std::vector<Normal3f> normal;
        std::vector<Point2f> tex_coord;
    };

    struct MeshTriangle {
        AreaLight *light = nullptr;
        Mesh *mesh = nullptr;
        uint16_t name_id = -1;
        uint32_t primID = -1;

        MeshTriangle() = default;

        [[nodiscard]] const Point3f &vertex(size_t) const;

        [[nodiscard]] const Normal3f &normal(size_t) const;

        [[nodiscard]] const Point2f &texCoord(size_t) const;

        [[nodiscard]] const Vec3f Ng() const {
            Vec3f e1 = (vertex(1) - vertex(0));
            Vec3f e2 = (vertex(2) - vertex(0));
            return normalize(cross(e1, e2));
        }

        bool intersect(const Ray &ray, Intersection &isct) const {
            float u, v;
            Vec3f e1 = (vertex(1) - vertex(0));
            Vec3f e2 = (vertex(2) - vertex(0));
            auto Ng = normalize(cross(e1, e2));
            float denom = dot(ray.d, Ng);
            float t = -dot(ray.o - vertex(0), Ng) / denom;
            if (denom == 0)
                return false;
            if (t < ray.tMin)
                return false;
            Vec3f p = ray.o + t * ray.d;
            double det = length(cross(e1, e2));
            auto u0 = cross(e1, p - vertex(0));
            auto v0 = cross(Vec3f(p - vertex(0)), e2);
            if (dot(u0, Ng) < 0 || dot(v0, Ng) < 0)
                return false;
            v = u0.length() / det;
            u = v0.length() / det;
            if (u < 0 || v < 0 || u > 1 || v > 1)
                return false;
            if (u + v <= 1) {
                if (t < isct.distance) {
                    isct.distance = t;
                    isct.Ng = Ng;
                    isct.uv = Point2f(u, v);
                    isct.Ns = normalAt(isct.uv);
                    isct.shape = this;
                    return true;
                }
            }
            return false;
        }

        [[nodiscard]] Bounds3f getBoundingBox() const {
            return Bounds3f{min(vertex(0), min(vertex(1), vertex(2))), max(vertex(0), max(vertex(1), vertex(2)))};
        }

        void sample(const Point2f &u, SurfaceSample &sample) const {
            Point2f uv = u;
            if (uv.x + uv.y > 1.0f) {
                uv.x = 1.0f - uv.x;
                uv.y = 1.0f - uv.y;
            }
            sample.uv = uv;
            sample.pdf = 1 / area();
            sample.p = (1 - uv.x - uv.y) * vertex(0) + uv.x * vertex(1) + uv.y * vertex(1);
            sample.normal = Ng();
        }

        [[nodiscard]] Float area() const { return length(cross(Vec3f(vertex(1) - vertex(0)),(vertex(2) - vertex(0)))); }

        [[nodiscard]] BSDF *getBSDF() const { return nullptr; }

        [[nodiscard]] Material *getMaterial() const;

        [[nodiscard]] Vec3f positionAt(const Point2f &uv) const {
            return lerp3(vertex(0), vertex(1), vertex(2), uv[0], uv[1]);
        }

        [[nodiscard]] Normal3f normalAt(const Point2f &uv) const {
            return normalize(lerp3(normal(0), normal(1), normal(2), uv[0], uv[1]));
        }

        [[nodiscard]] Point2f texCoordAt(const Point2f &uv) const {
            return lerp3(texCoord(0), texCoord(1), texCoord(2), uv[0], uv[1]);
        }
    };

    class MeshBase : public Object {
    };

    class Mesh final : public MeshBase {

    public:
        bool _loaded = false;
        std::vector<MeshTriangle> triangles;
        VertexData _vertex_data;
        std::vector<Point3i> _indices;
        std::vector<std::string> _names;
        std::vector<std::shared_ptr<Material>> _materials;
        std::unordered_map<std::string, std::shared_ptr<Material>> materials;
        std::string filename;

        MYK_DECL_CLASS(Mesh, "Mesh", interface = "MeshBase")

        MYK_AUTO_SER(filename, materials)

        MYK_AUTO_INIT(filename, materials)

        void toBinary(std::vector<char> &buffer) const;

        void fromBinary(const std::vector<char> &buffer);

        void foreach(const std::function<void(MeshTriangle *)> &func);

        bool loadFromFile(const std::string &filename);

        void preprocess() override;

        // Alert! This changes Mesh::filename
        void writeToFile(const std::string &filename);
    };

    class MeshInstance final : public MeshBase {
    public:
        MYK_DECL_CLASS(MeshInstance, "MeshInstance", interface = "MeshBase")

        Transform transform, invTransform;
        std::shared_ptr<Mesh> mesh;

        MYK_AUTO_SER(transform, mesh)

        void foreach(const std::function<void(MeshTriangle *)> &func) { mesh->foreach(func); }

    protected:
        void preprocess() override { invTransform = transform.inverse(); }
    };

} // namespace miyuki::core

#endif // MIYUKIRENDERER_MESH_H

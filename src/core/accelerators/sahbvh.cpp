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
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include "sahbvh.h"
#include <miyuki.foundation/log.hpp>
#include <miyuki.renderer/mesh.h>
#include <miyuki.renderer/scene.h>
#include <vector>

namespace miyuki::core {
    class BVHAccelerator::BVHAcceleratorInternal final {
        struct BVHNode {
            Bounds3f box;
            uint32_t first = -1;
            uint32_t count = -1;
            int left = -1, right = -1;

            [[nodiscard]] bool isLeaf() const { return left < 0 && right < 0; }
        };

        std::vector<MeshTriangle> primitive;
        std::vector<BVHNode> nodes;

        Bounds3f boundBox;

        static Float intersectAABB(const Bounds3f &box, const Ray &ray,
                                   const Vec3f &invd) {
            Vec3f t0 = (box.pMin - ray.o) * invd;
            Vec3f t1 = (box.pMax - ray.o) * invd;
            Vec3f tMin = min(t0, t1), tMax = max(t0, t1);
            if (maxComp(tMin) <= minComp(tMax)) {
                auto t = std::max(ray.tMin + RayBias, maxComp(tMin));
                if (t >= ray.tMax + RayBias) {
                    return -1;
                }
                return t;
            }
            return -1;
        }

        int recursiveBuild(int begin, int end, int depth) {
            Bounds3f box{{MaxFloat, MaxFloat, MaxFloat},
                         {MinFloat, MinFloat, MinFloat}};
            Bounds3f centroidBound{{MaxFloat, MaxFloat, MaxFloat},
                                   {MinFloat, MinFloat, MinFloat}};


            if (end == begin)
                return -1;
            for (auto i = begin; i < end; i++) {
                box = box.unionOf(primitive[i].getBoundingBox());
                centroidBound = centroidBound.unionOf(
                        primitive[i].getBoundingBox().centroid());
            }
            if (depth == 0) {
                boundBox = box;
            }

            if (end - begin <= 4 || depth >= 32) {
                BVHNode node;

                node.box = box;
                node.first = begin;
                node.count = end - begin;
                node.left = node.right = -1;
                nodes.push_back(node);
                return nodes.size() - 1;
            } else {

                int axis = depth % 3;
                auto size = centroidBound.size();
                if (size.x() > size.y()) {
                    if (size.x() > size.z()) {
                        axis = 0;
                    } else {
                        axis = 2;
                    }
                } else {
                    if (size.y() > size.z()) {
                        axis = 1;
                    } else {
                        axis = 2;
                    }
                }
                MeshTriangle *mid = nullptr;
                if (size[axis] > 0) {
                    constexpr size_t nBuckets = 12;
                    struct Bucket {
                        size_t count = 0;
                        Bounds3f bound;

                        Bucket()
                                : bound({{MaxFloat, MaxFloat, MaxFloat},
                                         {MinFloat, MinFloat, MinFloat}}) {}
                    };
                    Bucket buckets[nBuckets];
                    for (int i = begin; i < end; i++) {
                        auto offset = centroidBound.offset(
                                primitive[i].getBoundingBox().centroid())[axis];
                        int b = std::min<int>(nBuckets - 1,
                                              std::floor(offset * nBuckets));
                        buckets[b].count++;
                        buckets[b].bound =
                                buckets[b].bound.unionOf(primitive[i].getBoundingBox());
                    }
                    Float cost[nBuckets - 1] = {0};
                    for (int i = 0; i < nBuckets - 1; i++) {
                        Bounds3f b0{{MaxFloat, MaxFloat, MaxFloat},
                                    {MinFloat, MinFloat, MinFloat}};
                        Bounds3f b1{{MaxFloat, MaxFloat, MaxFloat},
                                    {MinFloat, MinFloat, MinFloat}};
                        int count0 = 0, count1 = 0;
                        for (int j = 0; j <= i; j++) {
                            b0 = b0.unionOf(buckets[j].bound);
                            count0 += buckets[j].count;
                        }
                        for (int j = i + 1; j < nBuckets; j++) {
                            b1 = b1.unionOf(buckets[j].bound);
                            count1 += buckets[j].count;
                        }
                        cost[i] = 0.125 + (count0 * b0.surfaceArea() +
                                           count1 * b1.surfaceArea()) /
                                          box.surfaceArea();
                    }
                    int splitBuckets = 0;
                    Float minCost = cost[0];
                    for (int i = 1; i < nBuckets - 1; i++) {
                        if (cost[i] <= minCost) {
                            minCost = cost[i];
                            splitBuckets = i;
                        }
                    }
                    MIYUKI_CHECK(minCost > 0);
                    mid = std::partition(
                            &primitive[begin], &primitive[end - 1] + 1,
                            [&](MeshTriangle &p) {
                                int b = centroidBound.offset(
                                        p.getBoundingBox().centroid())[axis] *
                                        nBuckets;
                                if (b == nBuckets) {
                                    b = nBuckets - 1;
                                }
                                return b <= splitBuckets;
                            });
                } else {
                    mid = primitive.data() + (begin + end) / 2;
                }
                auto ret = nodes.size();
                nodes.emplace_back();

                BVHNode &node = nodes.back();
                node.box = box;
                node.count = -1;
                nodes.push_back(node);
                nodes[ret].left =
                        recursiveBuild(begin, mid - &primitive[0], depth + 1);
                nodes[ret].right =
                        recursiveBuild(mid - &primitive[0], end, depth + 1);

                return ret;
            }
        }

    public:

        void build(const std::vector<MeshTriangle> &primitives) {
            nodes.clear();
            primitive = primitives;
            recursiveBuild(0, primitive.size(), 0);
            log::log("BVH nodes:{}\n", nodes.size());
        }

        bool intersect(const Ray &ray, Intersection &isct) const {
            bool hit = false;
            auto invd = Vec3f(1) / ray.d;
            constexpr int maxDepth = 64;
            const BVHNode *stack[maxDepth];
            int sp = 0;
            stack[sp++] = &nodes[0];
            while (sp > 0) {
                auto p = stack[--sp];
                auto t = intersectAABB(p->box, ray, invd);

                if (t < 0 || t > isct.distance) {
                    continue;
                }
                if (p->isLeaf()) {
                    for (int i = p->first; i < p->first + p->count; i++) {
                        if (primitive[i].intersect(ray, isct)) {
                            hit = true;
                        }
                    }
                } else {
                    if (p->left >= 0)
                        stack[sp++] = &nodes[p->left];
                    if (p->right >= 0)
                        stack[sp++] = &nodes[p->right];
                }
            }
            return hit;
        }

        bool occlude(const Ray &ray) {
            Intersection isct;
            bool hit = false;
            auto invd = Vec3f(1) / ray.d;
            constexpr int maxDepth = 64;
            const BVHNode *stack[maxDepth];
            int sp = 0;
            stack[sp++] = &nodes[0];
            while (sp > 0) {
                auto p = stack[--sp];
                auto t = intersectAABB(p->box, ray, invd);

                if (t < 0 || t > isct.distance) {
                    continue;
                }
                if (p->isLeaf()) {
                    for (int i = p->first; i < p->first + p->count; i++) {
                        if (primitive[i].intersect(ray, isct)) {
                            return true;
                        }
                    }
                } else {
                    if (p->left >= 0)
                        stack[sp++] = &nodes[p->left];
                    if (p->right >= 0)
                        stack[sp++] = &nodes[p->right];
                }
            }
            return false;
        }

        [[nodiscard]] Bounds3f getBoundingBox() const { return boundBox; }
    };

    void BVHAccelerator::build(Scene &scene) {
        for (const auto &i : scene.meshes) {
            auto node = new BVHAcceleratorInternal();
            node->build(i->triangles);
            internal.emplace_back(node);
        }
    }

    bool BVHAccelerator::intersect(const Ray &ray, Intersection &isct) {
        bool hit = false;
        for (auto i : internal) {
            if (i->intersect(ray, isct))
                hit = true;
        }
        if (hit) {
            isct.p = isct.distance * ray.d + ray.o;
        }
        return hit;
    }

    bool BVHAccelerator::occlude(const Ray &ray) {
        for (auto i : internal) {
            if (i->occlude(ray))
                return true;
        }
        return false;
    }

    BVHAccelerator::~BVHAccelerator() {
        for (auto i : internal) {
            delete i;
        }
    }

    Bounds3f BVHAccelerator::getBoundingBox() const {
        Bounds3f box{{MaxFloat, MaxFloat, MaxFloat},
                     {MinFloat, MinFloat, MinFloat}};
        for (auto i : internal) {
            box = box.unionOf(i->getBoundingBox());
        }
        return box;
    }
} // namespace miyuki::core
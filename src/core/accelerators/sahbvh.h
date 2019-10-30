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

#ifndef MIYUKIRENDERER_SAHBVH_H
#define MIYUKIRENDERER_SAHBVH_H

#include <api/accelerator.h>
#include <api/serialize.hpp>


namespace miyuki::core {
    class BVHAccelerator final : public Accelerator {
        struct BVHNode {
            Bounds3f box;
            uint32_t first = -1;
            uint32_t count = -1;
            int left = -1, right = -1;

            [[nodiscard]] bool isLeaf() const {
                return left < 0 && right < 0;
            }
        };

        std::vector<Primitive *> primitive;
        std::vector<BVHNode> nodes;

        Bounds3f boundBox;

        static Float intersectAABB(const Bounds3f &box, const Ray &ray, const Vec3f &invd) {
            Vec3f t0 = (box.pMin - ray.o) * invd;
            Vec3f t1 = (box.pMax - ray.o) * invd;
            Vec3f tMin = min(t0, t1), tMax = max(t0, t1);
            if (tMin.max() <= tMax.min()) {
                auto t = std::max(ray.tMin + RayBias, tMin.max());
                if (t >= ray.tMax + RayBias) {
                    return -1;
                }
                return t;
            }
            return -1;
        }

        int recursiveBuild(int begin, int end, int depth);

    public:
        MYK_DECL_CLASS(BVHAccelerator, "BVHAccelerator", interface = "Acclerator")

        [[nodiscard]] Bounds3f getBoundingBox() const override {
            return boundBox;
        }

        void sample(const Point2f &u, SurfaceSample &sample) const override {
            MIYUKI_NOT_IMPLEMENTED();
        }

        [[nodiscard]] Float area() const override {
            MIYUKI_NOT_IMPLEMENTED();
            return 0;
        }

        void build(const std::vector<Primitive *> &primitives) override;

        bool intersect(const Ray &ray, Intersection &isct) const override {
            bool hit = false;
            auto invd = Vec3f(1) / ray.d;
            constexpr int maxDepth = 40;
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
                        if (primitive[i]->intersect(ray, isct)) {
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
    };
}
#endif //MIYUKIRENDERER_SAHBVH_H

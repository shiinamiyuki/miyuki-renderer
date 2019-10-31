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

#include <vector>
#include "sahbvh.h"
#include <api/log.hpp>

namespace miyuki::core {
    int BVHAccelerator::recursiveBuild(int begin, int end, int depth) {

        //   log::log("depth: {}, primitives:{} \n", depth, end - begin);
        Bounds3f box{{MaxFloat, MaxFloat, MaxFloat},
                     {MinFloat, MinFloat, MinFloat}};
        Bounds3f centroidBound{{MaxFloat, MaxFloat, MaxFloat},
                               {MinFloat, MinFloat, MinFloat}};


        if (end == begin)return -1;
        for (auto i = begin; i < end; i++) {
            box = box.unionOf(primitive[i]->getBoundingBox());
            centroidBound = centroidBound.unionOf(primitive[i]->getBoundingBox().centroid());
        }
        if (depth == 0) {
            boundBox = box;
        }

        if (end - begin <= 4 || depth >= 20) {
            BVHNode node;

            node.box = box;
            node.first = begin;
            node.count = end - begin;
            node.left = node.right = -1;
            nodes.push_back(node);
            return nodes.size() - 1;
        } else {

            int axis = 0;
            auto size = centroidBound.size();
            if (size.x > size.y) {
                if (size.x > size.z) {
                    axis = 0;
                } else {
                    axis = 2;
                }
            } else {
                if (size.y > size.z) {
                    axis = 1;
                } else {
                    axis = 2;
                }
            }
            if (size[axis] == 0.0f) {
                auto ret = nodes.size();
                nodes.emplace_back();

                BVHNode &node = nodes.back();
                node.box = box;
                node.count = -1;
                auto mid = (begin + end) / 2;
                nodes[ret].left = recursiveBuild(begin, mid, depth + 1);
                nodes[ret].right = recursiveBuild(mid, end, depth + 1);

                return ret;
            }
            constexpr size_t nBuckets = 12;
            struct Bucket {
                size_t count = 0;
                Bounds3f bound;

                Bucket() : bound({{MaxFloat, MaxFloat, MaxFloat},
                                  {MinFloat, MinFloat, MinFloat}}) {}
            };
            Bucket buckets[nBuckets];
            for (int i = begin; i < end; i++) {
                auto offset = centroidBound.offset(primitive[i]->getBoundingBox().centroid())[axis];
                int b = std::min<int>(nBuckets - 1, std::floor(offset * nBuckets));
                buckets[b].count++;
                buckets[b].bound = buckets[b].bound.unionOf(primitive[i]->getBoundingBox());
            }
            Float cost[nBuckets - 1] = {0};
            for (int i = 0; i < nBuckets - 1; i++) {
                Bounds3f b0, b1;
                int count0 = 0, count1 = 0;
                for (int j = 0; j <= i; j++) {
                    b0 = b0.unionOf(buckets[j].bound);
                    count0 += buckets[j].count;
                }
                for (int j = i + 1; j < nBuckets; j++) {
                    b1 = b1.unionOf(buckets[j].bound);
                    count1 += buckets[j].count;
                }
                cost[i] = 0.125 +
                          (float(count0) * b0.surfaceArea() + float(count1) * b1.surfaceArea()) / box.surfaceArea();
            }
            int splitBuckets = 0;
            Float minCost = cost[0];
            for (int i = 1; i < nBuckets - 1; i++) {
                if (cost[i] <= minCost) {
                    minCost = cost[i];
                    splitBuckets = i;
                }
            }
            auto mid = std::partition(&primitive[begin], &primitive[end - 1] + 1, [&](Primitive *p) {
                int b = centroidBound.offset(p->getBoundingBox().centroid())[axis] * nBuckets;
                if (b == nBuckets) {
                    b = nBuckets - 1;
                }
                return b <= splitBuckets;
            });
            if (mid == &primitive[begin] || mid == &primitive[end - 1] + 1) {
                log::log("empty split at {}, {}\n", depth, end - begin);
            }
            auto ret = nodes.size();
            nodes.emplace_back();

            BVHNode &node = nodes.back();
            node.box = box;
            node.count = -1;
            nodes[ret].left = recursiveBuild(begin, mid - &primitive[0], depth + 1);
            nodes[ret].right = recursiveBuild(mid - &primitive[0], end, depth + 1);

            return ret;
        }
    }

    void BVHAccelerator::build(const std::vector<Primitive *> &primitives) {
        nodes.clear();
        log::log("Building BVH\n");
        primitive = primitives;
        recursiveBuild(0, primitive.size(), 0);
        log::log("BVH nodes:{}\n", nodes.size());
    }
}
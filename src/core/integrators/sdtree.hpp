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

#ifndef MIYUKIRENDERER_SDTREE_HPP
#define MIYUKIRENDERER_SDTREE_HPP

#include <miyuki.renderer/atmoicfloat.hpp>
#include <miyuki.foundation/image.hpp>
#include <miyuki.foundation/imageloader.h>
#include <miyuki.foundation/spectrum.h>
#include <miyuki.foundation/log.hpp>
#include <miyuki.foundation/parallel.h>
#include <miyuki.foundation/rng.h>
#include <stack>


namespace miyuki::core {
    class QTreeNode {
    public:

        std::array<AtomicFloat, 4> _sum;
        std::array<int, 4> _children = {-1, -1, -1, -1};

        QTreeNode() {
            setSum(0);
        }

        void copyFrom(const QTreeNode &node) {
            _children = (node._children);
            for (int i = 0; i < 4; i++) {
                _sum[i].set(float(node._sum[i]));
            }
        }

        QTreeNode &operator=(const QTreeNode &node) {
            if (&node == this) { return *this; }
            copyFrom(node);
            return *this;
        }

        void setSum(Float val) {
            for (int i = 0; i < 4; i++) {
                _sum[i].set(val);
            }
        }

        QTreeNode *child(int i, std::vector<QTreeNode> &nodes) const {
            return _children[i] > 0 ? &nodes[_children[i]] : nullptr;
        }

        static int childIndex(const Point2f &p) {
            int x, y;
            if (p.x() < 0.5f) {
                x = 0;
            } else {
                x = 1;
            }
            if (p.y() < 0.5f) {
                y = 0;
            } else {
                y = 1;
            }
            return x + 2 * y;
        }

        [[nodiscard]] bool isLeaf(int i) const {
            return _children[i] <= 0;
        }

        [[nodiscard]] static Point2f offset(size_t i) {
            return Point2f(i & 1u, i >> 1u) * 0.5f;
        }

        [[nodiscard]] Float sum() const {
            Float v = 0;
            for (auto &i : _sum) {
                v += (float) i;
            }
            return v;
        }

        float eval(const Point2f &p, std::vector<QTreeNode> &nodes) const {
            auto idx = childIndex(p);
            if (child(idx, nodes)) {
                return 4.0f * child(idx, nodes)->eval((p - offset(idx)) * 2.0f, nodes);
            } else {
                MIYUKI_CHECK(_sum[idx].value() > 0.0f);
                return 4.0f * float(_sum[idx]);
            }
        }

        float pdf(const Point2f &p, std::vector<QTreeNode> &nodes) const {
            auto idx = childIndex(p);
            auto s = sum();
            // MIYUKI_CHECK(s > 0);
            MIYUKI_CHECK(!std::isnan(s));
            auto factor = s == 0.0f ? 0.25f : _sum[idx].value() / s;
            MIYUKI_CHECK(factor > 0);
            if (child(idx, nodes)) {
                return 4.0f * factor * child(idx, nodes)->pdf((p - offset(idx)) * 2.0f, nodes);
            } else {
                return 4.0f * factor;
            }
        }

        Point2f sample(Point2f u, std::vector<QTreeNode> &nodes) const {
            std::array<float, 4> m = {
                    float(_sum[0]),
                    float(_sum[1]),
                    float(_sum[2]),
                    float(_sum[3])
            };
            auto left = m[0] + m[2];
            auto right = m[1] + m[3];
            auto total = left + right;
            // log::log("total: {}\n", total);
            if (total == 0) {
                total = 1;
                m[0] = m[1] = m[2] = m[3] = 0.25;
                left = m[0] + m[2];
                right = m[1] + m[3];
            }

            int x, y;
            if (u[0] < left / total) {
                x = 0;
                u[0] /= left / total;
            } else {
                x = 1;
                u[0] = (u[0] - left / total) / (right / total);
            }
            auto up = m[x];
            auto down = m[2 + x];
            total = up + down;
            if (u[0] < up / total) {
                y = 0;
                u[0] /= up / total;
            } else {
                y = 1;
                u[1] = (u[1] - up / total) / (down / total);
            }
            int child = x + 2 * y;
            Point2f sampled;
            if (this->child(child, nodes)) {
                sampled = this->child(child, nodes)->sample(u, nodes);
            } else {
                sampled = u;
            }
            return Point2f(x, y) * 0.5f + sampled * 0.5f;
        }

        void deposit(const Point2f &p, Float e, std::vector<QTreeNode> &nodes) {
            int idx = childIndex(p);
            _sum[idx].add(e);
            auto c = child(idx, nodes);
            if (c) {
                c->deposit((p - offset(idx)) * 2.0f, e, nodes);
            }
        }
    };

    static Vec3f canonicalToDir(const Point2f &p) {
        const Float cosTheta = 2 * p.x() - 1;
        const Float phi = 2 * Pi * p.y();

        const Float sinTheta = sqrt(1 - cosTheta * cosTheta);
        Float sinPhi, cosPhi;
        sinPhi = sin(phi);
        cosPhi = cos(phi);

        return Vec3f(sinTheta * cosPhi, cosTheta, sinTheta * sinPhi);
    }

    static Point2f dirToCanonical(const Vec3f &d) {
        if (!std::isfinite(d.x()) || !std::isfinite(d.y()) || !std::isfinite(d.z())) {
            return {0, 0};
        }

        const Float cosTheta = std::min(std::max(d.y(), -1.0f), 1.0f);
        Float phi = std::atan2(d.z(), d.x());
        while (phi < 0)
            phi += 2.0 * Pi;

        return Point2f((cosTheta + 1) / 2, phi / (2 * Pi));
    }

    class DTree {
    public:
        AtomicFloat sum;
        AtomicFloat weight;
        std::vector<QTreeNode> nodes;

        DTree() : sum(0), weight(0) {
            nodes.emplace_back();
            _build();
        }

        DTree(const DTree &other) {
            sum.set(other.sum.value());
            weight.set(other.weight.value());
            nodes = (other.nodes);
        }

        DTree &operator=(const DTree &other) {
            if (&other == this) { return *this; }
            sum.set(other.sum.value());
            weight.set(other.weight.value());
            nodes = (other.nodes);
            return *this;
        }


        Point2f sample(const Point2f &u) {
            return nodes[0].sample(u, nodes);
        }

        Float pdf(const Point2f &p) {
            return nodes[0].pdf(p, nodes);
        }

        Float eval(const Point2f &u) {
            return nodes[0].eval(u, nodes) / weight.value();
        }

        void _build() {
            auto updateSum = [this](QTreeNode &node, auto &&update) -> double {
                for (int i = 0; i < 4; i++) {
                    if (!node.isLeaf(i)) {
                        auto c = node.child(i, nodes);
                        update(*c, update);
                        node._sum[i].set(c->sum());
                    }
                }
                //log::log("sum: {}\n",node.sum());
                return node.sum();
            };
            sum.set(updateSum(nodes.front(), updateSum));
        }

        void refine(const DTree &prev, Float threshold) {
            struct StackNode {
                size_t node = -1, otherNode = -1;
                const DTree *tree = nullptr;
            };
            std::stack<StackNode> stack;
            nodes.clear();
            nodes.emplace_back();
            stack.push({0, 0, &prev});
            sum.set(0.0f);
            auto total = prev.sum.value();
//            log::log("{} {}\n", total, threshold);
            while (!stack.empty()) {
                auto node = stack.top();
                stack.pop();
                auto &otherNode = node.tree->nodes.at(node.otherNode);
                // log::log("other index: {}, sum: {}\n",node.otherNode, otherNode.sum());
                for (int i = 0; i < 4; ++i) {

                    auto fraction = otherNode._sum[i].value() / total;
                    //log::log("{} {}\n", otherNode._sum[i].value(), fraction);
                    if (fraction > threshold) {
                        if (otherNode.isLeaf(i)) {
                            stack.push({nodes.size(), nodes.size(), this});
                        } else {
                            MIYUKI_CHECK(otherNode._children[i] > 0);
                            MIYUKI_CHECK(otherNode._children[i] != node.otherNode);
                            stack.push({nodes.size(), (size_t) otherNode._children[i], &prev});
                        }
                    }
                    nodes[node.node]._children[i] = nodes.size();
                    nodes.emplace_back();
                    nodes.back().setSum(otherNode._sum[i].value() / 4.0f);
                }

            }
//            log::log("QTreeNodes: {}\n", nodes.size());
            weight.add(1);
            _build();
        }

        void deposit(const Point2f &p, Float e) {
            sum.add(e);
            nodes[0].deposit(p, e, nodes);
        }
    };

    class DTreeWrapper {
    public:
        DTree building, sampling;

        Vec3f sample(const Point2f &u) {
            return canonicalToDir(sampling.sample(u));
        }

        Float pdf(const Vec3f &w) {
            return sampling.pdf(dirToCanonical(w));
        }

        void deposit(const Vec3f &w, Float e) {
            MIYUKI_CHECK(!building.nodes.empty());
            auto p = dirToCanonical(w);
            building.deposit(p, e);
        }

        void refine() {
            MIYUKI_CHECK(building.sum.value() > 0.0f);
            sampling = building;
//            sampling._build();
            MIYUKI_CHECK(sampling.sum.value() > 0.0f);
            building.refine(sampling, 0.01);

        }
    };

    class STreeNode {
    public:
        DTreeWrapper dTree;
        std::atomic<int> nSample = 0;
        std::array<int, 2> _children = {-1, -1};
        int axis = 0;
        bool _isLeaf = true;

        STreeNode() = default;

        STreeNode(const STreeNode &other) : nSample((int) other.nSample), _children(other._children),
                                            axis(other.axis), _isLeaf(other._isLeaf), dTree(other.dTree) {}


        [[nodiscard]] bool isLeaf() const {
            return _isLeaf;
        }

        Vec3f sample(Point3f p, const Point2f &u, std::vector<STreeNode> &nodes) {
            if (isLeaf()) {
                return dTree.sample(u);
            } else {
                if (p[axis] < 0.5f) {
                    p[axis] *= 2.0f;
                    return nodes[_children[0]].sample(p, u, nodes);
                } else {
                    p[axis] = (p[axis] - 0.5f) * 2.0f;
                    return nodes[_children[1]].sample(p, u, nodes);
                }
            }
        }

        Float pdf(Point3f p, const Vec3f &w, std::vector<STreeNode> &nodes) {
            if (isLeaf()) {
                return dTree.pdf(w);
            } else {
                if (p[axis] < 0.5f) {
                    p[axis] *= 2.0f;
                    return nodes.at(_children[0]).pdf(p, w, nodes);
                } else {
                    p[axis] = (p[axis] - 0.5f) * 2.0f;
                    return nodes.at(_children[1]).pdf(p, w, nodes);
                }
            }
        }

        void deposit(Point3f p, const Vec3f &w, Float irradiance, std::vector<STreeNode> &nodes) {
            if (isLeaf()) {
                nSample++;
                dTree.deposit(w, irradiance);
            } else {
                if (p[axis] < 0.5f) {
                    p[axis] *= 2.0f;
                    nodes[_children[0]].deposit(p, w, irradiance, nodes);
                } else {
                    p[axis] = (p[axis] - 0.5f) * 2.0f;
                    nodes[_children[1]].deposit(p, w, irradiance, nodes);
                }
            }
        }
    };

    class STree {
        std::vector<STreeNode> nodes;
    public:
        explicit STree(const Bounds3f &box) : nodes(1) {
            auto sz = maxComp(box.size()) * 0.5f;
            auto centroid = box.centroid();
            this->box = Bounds3f{centroid - Point3f(sz), centroid + Point3f(sz)};
        }

        Bounds3f box;

        Vec3f sample(const Point3f &p, const Point2f &u) {
            return nodes.at(0).sample(box.offset(p), u, nodes);
        }

        Float pdf(const Point3f &p, const Vec3f &w) {
            return nodes.at(0).pdf(box.offset(p), w, nodes) * Inv4Pi;
        }

        void deposit(Point3f p, const Vec3f &w, Float irradiance) {
            if (irradiance > 0 && !std::isnan(irradiance)) {
                nodes.at(0).deposit(box.offset(p), w, irradiance, nodes);
            }
        }

        void refine(int idx, size_t maxSample, std::vector<STreeNode> &nodes) {
//            log::log("samples: {}\n", (int) nodes[idx].nSample);
            if (nodes[idx].isLeaf() && nodes[idx].nSample > maxSample) {
//                log::log("sum {}\n", nodes[idx].dTree.building.sum.value());
                for (int i = 0; i < 2; i++) {
                    nodes[idx]._children[i] = nodes.size();
                    nodes.emplace_back();
                    auto &node = nodes.back();
                    node.nSample = maxSample / 2;
                    node.axis = (nodes[idx].axis + 1) % 3;
                    node.dTree = nodes[idx].dTree;
                    MIYUKI_CHECK(node.isLeaf());
                }
                nodes[idx]._isLeaf = false;
                nodes[idx].dTree = DTreeWrapper();

            }
            if (nodes[idx].isLeaf()) {
                nodes[idx].dTree.refine();
            } else {
                for (int i = 0; i < 2; i++) {
                    refine(nodes[idx]._children[i], maxSample, nodes);
                }
                MIYUKI_CHECK(nodes[idx]._children[0] > 0 && nodes[idx]._children[1] > 0);
            }
        }

        void refine(size_t maxSample) {
            refine(0, maxSample, nodes);
        }
    };
}
#endif //MIYUKIRENDERER_SDTREE_HPP

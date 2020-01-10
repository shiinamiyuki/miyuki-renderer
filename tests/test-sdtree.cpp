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

#include <miyuki.renderer/atmoicfloat.hpp>
#include <miyuki.foundation/image.hpp>
#include <miyuki.foundation/imageloader.h>
#include <miyuki.foundation/spectrum.h>
#include <miyuki.foundation/log.hpp>
#include <miyuki.foundation/parallel.h>
#include <miyuki.foundation/rng.h>
#include <stack>

namespace miyuki {
    class QTreeNode {
    public:

        std::array<AtomicFloat, 4> _sum;
        std::array<int, 4> _children = {-1, -1, -1, -1};
        bool _isLeaf = true;

        QTreeNode() {
            setSum(0);
        }

        QTreeNode(const QTreeNode &node) : _children(node._children) {
            for (int i = 0; i < 4; i++) {
                _sum[i].set(float(node._sum[i]));
            }
        }

        void setSum(Float val) {
            for (int i = 0; i < 4; i++) {
                _sum[i].set(val);
            }
        }

        QTreeNode *child(int i, std::vector<QTreeNode> &nodes) const {
            return _children[i] > 0 ? &nodes[_children[i]] : nullptr;
        }

        int childIndex(const Point2f &p) const {
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

        bool isLeaf(int i) const {
            return _children[i] <= 0;
        }

        Point2f offset(int i) const {
            return Point2f(i & 1, i >> 1) * 0.5f;
        }

        Float sum() const {
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
                MIYUKI_ASSERT(_sum[idx].value() > 0.0f);
                return 4.0f * float(_sum[idx]);
            }
        }

        float pdf(const Point2f &p, std::vector<QTreeNode> &nodes) const {
            auto idx = childIndex(p);
            auto s = sum();
            MIYUKI_CHECK(s > 0);
            auto factor = 4.0f * _sum[idx].value() / s;
            MIYUKI_CHECK(factor > 0);
            if (child(idx, nodes)) {
                return factor * child(idx, nodes)->pdf((p - offset(idx)) * 2.0f, nodes);
            } else {
                return factor;
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
            MIYUKI_CHECK(total > 0);

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
            if (u[1] < up / total) {
                y = 0;
                u[1] /= up / total;
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

        return {sinTheta * cosPhi, cosTheta, sinTheta * sinPhi};
    }

    static Point2f dirToCanonical(const Vec3f &d) {
        if (!std::isfinite(d.x()) || !std::isfinite(d.y()) || !std::isfinite(d.z())) {
            return {0, 0};
        }

        const Float cosTheta = std::min(std::max(d.y(), -1.0f), 1.0f);
        Float phi = std::atan2(d.z(), d.x());
        while (phi < 0)
            phi += 2.0 * Pi;

        return {(cosTheta + 1) / 2, phi / (2 * Pi)};
    }

    class DTree {
        AtomicFloat sum;
        AtomicFloat weight;
    public:
        DTree() : sum(0),weight(0) {
            nodes.emplace_back();
            _build();
        }

        std::vector<QTreeNode> nodes;

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
            updateSum(nodes.front(), updateSum);
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
            log::log("{} {}\n", total, threshold);
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
            weight.add(1);
            _build();
        }

        void deposit(const Point2f &p, Float e) {
            sum.add(e);
            nodes[0].deposit(p, e, nodes);
        }
    };


    class STreeNode {
    public:
        std::atomic<int> nSample = 0;
        Bounds3f box;
        int axis = 0;

    };

    class STree {
    public:
        Bounds3f box;
    };

    void testDTree() {
        auto image = ImageLoader::getInstance()->loadRGBAImage(
                fs::path("../data/living_room/textures/picture11-vert.jpg"));
        // image->write("test.png", 1.0f);
        DTree dTree;
        double area = (image->dimension[0] * image->dimension[1]);
        for (int iter = 0; iter < 16; iter++) {
            for (int i = 0; i < image->dimension[0]; i++) {
                for (int j = 0; j < image->dimension[1]; j++) {
                    //log::log("{} {}\n",i,j);
                    auto p = Point2f(i, j) / Point2f(image->dimension);
                    auto rgba = (*image)(p) / (float) area;
                    core::Spectrum color(rgba.x(), rgba.y(), rgba.z());
                    dTree.deposit(p, color.luminance());
                }
            }


            DTree copy = dTree;
            dTree.refine(copy, 0.01);
//            result.write(fmt::format("out{}.png", iter + 1), 1.0f / 2.2f);
        }

        RGBAImage result(image->dimension);
        ParallelFor(0, image->dimension[0], [&](int i, int) {
            for (int j = 0; j < image->dimension[1]; j++) {
                auto p = Point2f(i, j) / Point2f(image->dimension);
                result(i, j) = dTree.eval(p);
            }
        });
        result.write("out.png", 1.0f / 2.2f);
        double sum = 0;
        double cnt = 0;
        core::Rng rng;
        for (int i = 0; i < 100000; i++) {
            auto u = Point2f(rng.uniformFloat(), rng.uniformFloat());
            auto p = dTree.sample(u);
            auto pdf = dTree.pdf(p);
            sum += dTree.eval(p) / pdf;
            cnt += 1.0;
        }
        log::log("{}\n", sum / cnt);

    }
}


int main() {
    miyuki::testDTree();
}
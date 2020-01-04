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

namespace miyuki {
    class QTreeNode {
    public:
        struct Index {
            int i;

            explicit Index(int i) : i(i) {}

            QTreeNode *get() const;
        };

        Bounds2f box;
        std::array<AtomicFloat, 4> _means;
        std::array<Index, 4> _children;


        QTreeNode(const QTreeNode &node) : box(node.box), _children(node._children) {
            for (int i = 0; i < 4; i++) {
                _means[i].set(float(node._means[i]));
            }
        }

        int childIndex(const Point2f &p) const {
            int x, y;
            if (p.x < box.centroid().x) {
                x = 0;
            } else {
                x = 1;
            }
            if (p.y < box.centroid().y) {
                y = 0;
            } else {
                y = 1;
            }
            return x + 2 * y;
        }

        bool isLeaf(const Index &idx) const {
            return idx.get() == nullptr;
        }

        float eval(const Point2f &p) const {
            auto idx = childIndex(p);
            if(isLeaf(_children[idx])){
                return float(_means[idx]);
            }else{

            }
        }

        Point2f sample(Point2f u, Float *pdf) const {
            std::array<float, 4> m = {
                    float(_means[0]),
                    float(_means[1]),
                    float(_means[2]),
                    float(_means[3])
            };
            auto left = m[0] + m[2];
            auto right = m[1] + m[3];
            auto total = left + right;
            int x, y;
            float p = 1;
            if (u[0] < left / total) {
                x = 0;
                p *= left / total;
                u[0] /= left / total;
            } else {
                x = 1;
                p *= right / total;
                u[0] /= right / total;
            }
            auto up = m[x];
            auto down = m[2 + x];
            total = up + down;
            if (u[1] < up / total) {
                y = 0;
                p *= up / total;
                u[1] /= up / total;
            } else {
                y = 1;
                p *= down / total;
                u[1] /= down / total;
            }
            int child = x + 2 * y;
            if (_children[child].get()) {
                float p0;
                auto sampled = sample(u, &p0);
                *pdf = p0 * p;
                return sampled;
            } else {
                *pdf = p;
                return Point2f(lerp(box.pMin, box.pMax, u));
            }
        }


        void subdivide(const std::vector<QTreeNode> &nodes) {

        }
    };

    class QTree {
    public:
    };
}


int main() {

}
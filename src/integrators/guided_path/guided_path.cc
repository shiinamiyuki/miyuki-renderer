//
// Created by Shiina Miyuki on 2019/3/28.
//

#include "guided_path.h"
#include <utils/atomicfloat.h>
#include <utils/thread.h>

namespace Miyuki {
    class QuadTreeNode {
        std::array<AtomicFloat, 4> means;
        std::array<uint16_t, 4> children;
    public:
        QuadTreeNode() {
            children = {};
            for (auto &i:means) {
                i = 0;
            }
        }

        Float getMean(int idx) const {
            return means[idx];
        }

        void setMean(int idx, Float v) {
            means[idx] = v;
        }

        void setMean(Float meanVal) {
            for (int i = 0; i < 4; ++i) {
                setMean(i, meanVal);
            }
        }

        bool isLeaf(int idx) const {
            return children[idx] == 0;
        }

        int getChildIndex(Point2f &p) const {
            if (p.y() < 0.5) {
                p.y() *= 2;
                if (p.x() < 0.5) {
                    p.x() *= 2;
                    return 0;
                }
                p.x() = (p.x() - 0.5f) * 2.0f;
                return 1;
            } else {
                p.y() = (p.y() - 0.5f) * 2.0f;
                if (p.x() < 0.5) {
                    p.x() *= 2;
                    return 2;
                }
                return 3;
            }
        }

        size_t child(int i) const {
            return children[i];
        }

        void setChild(int i, size_t idx) {
            children[i] = idx;
        }

        int depthAt(Point2f &p, const std::vector<QuadTreeNode> &nodes) const {
            const int index = getChildIndex(p);
            if (isLeaf(index)) {
                return 1;
            } else {
                return 1 + nodes[children[index]].depthAt(p, nodes);
            }
        }

        Float eval(Point2f &p, const std::vector<QuadTreeNode> &nodes) const {
            auto idx = getChildIndex(p);
            if (isLeaf(idx)) {
                return 4 * getMean(idx);
            }
            return 4 * nodes[children[idx]].eval(p, nodes);
        }

        Float pdf(Point2f &p, const std::vector<QuadTreeNode> &nodes) const {
            auto idx = getChildIndex(p);
            if (getMean(idx) <= 0) {
                return 0;
            }
            auto factor = 4 * getMean(idx) / (getMean(0) + getMean(1) + getMean(2) + getMean(3));
            if (isLeaf(idx)) {
                return factor;
            }
            return factor * nodes[children[idx]].pdf(p, nodes);
        }

        Point2f sample(Point2f u, const std::vector<QuadTreeNode> &nodes) const {
            Float topLeft = getMean(0);
            Float topRight = getMean(1);
            Float partial = topLeft + getMean(2);
            Float total = partial + topRight + getMean(3);

            Point2f origin;
            int index = 0;
            CHECK(total > 0);

            Float boundary = partial / total;
            if (u.x() < boundary) {
                u.x() /= boundary;
                boundary = topLeft / partial;
            } else {
                origin.x() = 0.5f;
                partial = total - partial;
                boundary = topRight / partial;
                u.x() = (u.x() - boundary) / (1.0f - boundary);
                index |= 1;
            }

            if (u.y() < boundary) {
                u.y() /= boundary;
            } else {
                origin.y() = 0.5f;
                u.y() = (u.y() - boundary) / (1.0f - boundary);
                index |= 2;
            }
            if (isLeaf(index)) {
                return origin + u * 0.5f;
            }
            return origin + nodes[children[index]].sample(u, nodes) * 0.5f;
        }

        void addAtomic(int idx, Float value) {
            means[idx].add(value);
        }

        void record(Point2f &p, Float radiance, std::vector<QuadTreeNode> &nodes) {
            int idx = getChildIndex(p);
            if (isLeaf(idx)) {
                addAtomic(idx, radiance);
            } else {
                nodes[children[idx]].record(p, radiance, nodes);
            }
        }

        Float
        computeOverlappingArea(const Point2f &min1, const Point2f &max1, const Point2f &min2, const Point2f &max2) {
            float area = 1;
            for (int i = 0; i < 2; ++i) {
                area *= std::max(std::min(max1[i], max2[i]) - std::max(min1[i], min2[i]), 0.0f);
            }
            return area;
        }

        void record(const Point2f &origin,
                    float size, const Point2f &nodeOrigin, float nodeSize, float value,
                    std::vector<QuadTreeNode> &nodes) {
            auto childSize = nodeSize / 2;
            for (int i = 0; i < 4; i++) {
                Point2f childOrigin = nodeOrigin;
                if (i & 1) {
                    childOrigin[0] += childSize;
                }
                if (i & 2) {
                    childOrigin[1] += childSize;
                }
                Float w = computeOverlappingArea(origin, origin + Point2f(size),
                                                 childOrigin, childOrigin + Point2f(childSize));
                if (w > 0.0f) {
                    if (isLeaf(i)) {
                        addAtomic(i, w * value);
                    } else {
                        nodes[children[i]].record(
                                origin, size, childOrigin, childSize, value, nodes);
                    }
                }
            }

        }

        void build(std::vector<QuadTreeNode> &nodes) {
            for (int i = 0; i < 4; ++i) {
                if (!isLeaf(i)) {
                    QuadTreeNode &c = nodes[children[i]];
                    c.build(nodes);
                    Float mean = 0;
                    for (int j = 0; j < 4; ++j) {
                        mean += c.getMean(j);
                    }
                    setMean(i, mean);
                }
            }
        }
    };

    struct AtomicMeasurement {
        AtomicFloat R, G, B;
        AtomicFloat sum, statisticalWeight;
        std::atomic<size_t> nSamples;

        AtomicMeasurement()
                : R(0), G(0), B(0), nSamples(0u), sum(0), statisticalWeight(0) {}

        void record(const Spectrum &value) {
            nSamples++;
            R.add(value[0]);
            G.add(value[1]);
            B.add(value[2]);
        }

        Spectrum get() const {
            size_t n = nSamples;
            if (n == 0) { return {}; }
            return Spectrum{R, G, B} / n;
        }

        void copyFrom(const AtomicMeasurement &arg) {
            nSamples.store(arg.nSamples.load(std::memory_order_relaxed), std::memory_order_relaxed);
            R.store(arg.R);
            G.store(arg.G);
            B.store(arg.B);
            sum.store(arg.sum);
            statisticalWeight.store(arg.statisticalWeight);
        }

        AtomicMeasurement(const AtomicMeasurement &rhs) {
            copyFrom(rhs);
        }

        AtomicMeasurement &operator=(const AtomicMeasurement &rhs) {
            copyFrom(rhs);
            return *this;
        }

    };

    class DTree {
        std::vector<QuadTreeNode> nodes;
        int maxDepth;
        AtomicMeasurement measurement;
    public:
        const QuadTreeNode &operator[](size_t index) const {
            return nodes[index];
        }

        Float mean() const {
            if (measurement.statisticalWeight == 0)return 0.0f;
            auto factor = 1.0f / (4.0f * PI * measurement.statisticalWeight);
            return factor * measurement.sum;
        }

        Spectrum getMeasurement() const {
            return measurement.get();
        }

        void recordMeasurement(const Spectrum &value) {
            measurement.record(value);
        }

        void recordRadiance(Point2f p, Float radiance, Float statsWeight, bool doFilter) {
            if (std::isfinite(statsWeight) && statsWeight > 0) {
                measurement.statisticalWeight.add(statsWeight);
            }
            if (std::isfinite(radiance) && radiance > 0) {
                if (!doFilter) {
                    nodes[0].record(p, radiance, nodes);
                } else {
                    int depth = nodes[0].depthAt(p, nodes);
                    Float size = std::pow(0.5f, depth);

                    auto origin = p;
                    origin.x() -= size / 2;
                    origin.y() -= size / 2;

                    nodes[0].record(origin, size,
                                    Point2f(0, 0), 1.0f,
                                    radiance / (size * size), nodes);
                }
            }
        }

        Float evalRadiance(Point2f p) const {
            if (measurement.statisticalWeight == 0)
                return 0.0f;
            return nodes[0].eval(p, nodes) / (4 * PI * measurement.statisticalWeight);
        }

        Float evalPdf(Point2f p) const {
            if (mean() <= 0) {
                return 0.0f;
            }
            return nodes[0].pdf(p, nodes) / (4 * PI);
        }

        int depthAt(Point2f &p) const {
            return nodes[0].depthAt(p, nodes);
        }

        int depth() const {
            return maxDepth;
        }

        Point2f sampleRadiance(Point2f u) const {
            if (mean() <= 0)
                return u;
            auto res = nodes[0].sample(u, nodes);
            return res;
        }

        void build() {
            auto &root = nodes[0];
            root.build(nodes);
            Float mean = 0;
            for (int i = 0; i < 4; i++) {
                mean += root.getMean(i);
            }
            measurement.sum = mean;
        }

        void reset(const DTree &prev, int newMaxDepth, Float subdivisionThreshold) {
            measurement = AtomicMeasurement();
            maxDepth = 0;

            struct StackNode {
                size_t nodeIndex;
                size_t otherNodeIndex;
                const DTree *otherDTree;
                int depth;
            };

            std::stack<StackNode> nodeIndices;
            nodeIndices.push({0, 0, &prev, 1});

            while (!nodeIndices.empty()) {
                auto node = nodeIndices.top();
                nodeIndices.pop();

                maxDepth = std::max(maxDepth, node.depth);

                for (int i = 0; i < 4; i++) {
                    const auto &otherNode = node.otherDTree->nodes[node.otherNodeIndex];
                    const auto total = prev.measurement.sum;
                    const auto fraction = total > 0 ? otherNode.getMean(i) / total : 0.0f;

                    Assert(fraction < 1.0f + EPS);

                    if (node.depth < newMaxDepth && fraction > subdivisionThreshold) {
                        if (!otherNode.isLeaf(i)) {
                            nodeIndices.push({nodes.size(), otherNode.child(i), &prev, node.depth + 1});
                        } else {
                            nodeIndices.push({nodes.size(), nodes.size(), this, node.depth + 1});
                        }

                        nodes[node.nodeIndex].setChild(i, static_cast<uint16_t>(nodes.size()));
                        nodes.emplace_back();
                        nodes.back().setMean(otherNode.getMean(i) / 4);

                        if (nodes.size() > std::numeric_limits<uint16_t>::max()) {
                            fmt::print(stderr, "DTreeWrapper hit maximum children count.");
                            nodeIndices = std::stack<StackNode>();
                            break;
                        }
                    }
                }
            }

            for (auto &node : nodes) {
                node.setMean(0);
            }
        }

        size_t approxMemoryFootprint() const {
            return nodes.capacity() * sizeof(QuadTreeNode) + sizeof(*this);
        }

        size_t numNodes() const {
            return nodes.size();
        }

        void setStatisticalWeight(Float w) {
            measurement.statisticalWeight = w;
        }

        Float statisticalWeight() const {
            return measurement.statisticalWeight;
        }
    };

    struct DTreeWrapper {
    private:
        DTree building;
        DTree sampling;
    public:
        DTreeWrapper() {
        }

        void recordRadiance(const Vec3f &dir, Float radiance, Float statisticalWeight, bool doFilteredSplatting) {
            building.recordRadiance(dirToCanonical(dir), radiance, statisticalWeight, doFilteredSplatting);
        }

        void recordMeasurement(Spectrum m) {
            if (!m.isBlack()) {
                m = Spectrum{0.0f};
            }

            building.recordMeasurement(m);
        }

        static Vec3f canonicalToDir(Point2f p) {
            const Float cosTheta = 2 * p.x() - 1;
            const Float phi = 2 * PI * p.y();

            const Float sinTheta = std::sqrt(1.0f - cosTheta * cosTheta);
            Float sinPhi = std::sin(phi), cosPhi = std::cos(phi);

            return Vec3f{sinTheta * cosPhi, cosTheta, sinTheta * sinPhi};
        }

        static Point2f dirToCanonical(const Vec3f &d) {
            if (!std::isfinite(d.x()) || !std::isfinite(d.y()) || !std::isfinite(d.z())) {
                return {0, 0};
            }

            const Float cosTheta = std::min(std::max(d.y(), -1.0f), 1.0f);
            Float phi = std::atan2(d.z(), d.x());
            while (phi < 0)
                phi += 2.0 * M_PI;

            return Point2f{(cosTheta + 1) / 2, phi / (2 * PI)};
        }

        Float estimateRadiance(const Point2f &p) const {
            return sampling.evalRadiance(p);
        }

        Float estimateRadiance(const Vec3f &d) const {
            return estimateRadiance(dirToCanonical(d));
        }

        void build() {
            building.build();
            sampling = building;
        }

        void reset(int maxDepth, Float subdivisionThreshold) {
            building.reset(sampling, maxDepth, subdivisionThreshold);
        }

        Vec3f sampleDirection(const Point2f &sample) const {
            return canonicalToDir(sampling.sampleRadiance(sample));
        }

        Float samplePdf(const Vec3f &dir) const {
            return sampling.evalPdf(dirToCanonical(dir));
        }

        Float diff(const DTreeWrapper &other) const {
            return 0.0f;
        }

        int depth() const {
            return sampling.depth();
        }

        size_t numNodes() const {
            return sampling.numNodes();
        }

        Float meanRadiance() const {
            return sampling.mean();
        }

        Spectrum measurementEstimate() const {
            return sampling.getMeasurement();
        }

        Float statisticalWeight() const {
            return sampling.statisticalWeight();
        }

        Float statisticalWeightBuilding() const {
            return building.statisticalWeight();
        }

        void setStatisticalWeightBuilding(Float statisticalWeight) {
            building.setStatisticalWeight(statisticalWeight);
        }

        size_t approxMemoryFootprint() const {
            return building.approxMemoryFootprint() + sampling.approxMemoryFootprint();
        }

    };

    struct STreeNode {
        bool isLeaf;
        DTreeWrapper dTree;
        int axis;
        std::array<uint32_t, 2> children;

        STreeNode() {
            children = {};
            isLeaf = true;
            axis = 0;
        }

        int childIndex(Point3f &p) const {
            if (p[axis] < 0.5f) {
                p[axis] *= 2;
                return 0;
            } else {
                p[axis] = (p[axis] - 0.5f) * 2;
                return 1;
            }
        }

        int nodeIndex(Point3f &p) const {
            return children[childIndex(p)];
        }

        DTreeWrapper *dTreeWrapper(Point3f &p, Vec3f &size, std::vector<STreeNode> &nodes) {
            Assert(p[axis] >= 0 && p[axis] <= 1);
            if (isLeaf) {
                return &dTree;
            } else {
                size[axis] /= 2;
                return nodes[nodeIndex(p)].dTreeWrapper(p, size, nodes);
            }
        }

        const DTreeWrapper *dTreeWrapper() const {
            return &dTree;
        }

        int depth(Point3f &p, const std::vector<STreeNode> &nodes) const {
            Assert(p[axis] >= 0 && p[axis] <= 1);
            if (isLeaf) {
                return 1;
            } else {
                return 1 + nodes[nodeIndex(p)].depth(p, nodes);
            }
        }

        int depth(const std::vector<STreeNode> &nodes) const {
            int result = 1;

            if (!isLeaf) {
                for (auto c : children) {
                    result = std::max(result, 1 + nodes[c].depth(nodes));
                }
            }

            return result;
        }

        void forEachLeaf(
                const std::function<void(const DTreeWrapper *, const Point3f &, const Vec3f &)> &func,
                const Point3f &p, Vec3f size, const std::vector<STreeNode> &nodes) const {

            if (isLeaf) {
                func(&dTree, p, size);
            } else {
                size[axis] /= 2;
                for (int i = 0; i < 2; ++i) {
                    Point3f childP = p;
                    if (i == 1) {
                        childP[axis] += size[axis];
                    }

                    nodes[children[i]].forEachLeaf(func, childP, size, nodes);
                }
            }
        }

        Float
        computeOverlappingVolume(const Point3f &min1, const Point3f &max1, const Point3f &min2, const Point3f &max2) {
            float area = 1;
            for (int i = 0; i < 3; ++i) {
                area *= std::max(std::min(max1[i], max2[i]) - std::max(min1[i], min2[i]), 0.0f);
            }
            return area;
        }

        void recordRadiance(const Point3f &min1, const Point3f &max1, Point3f min2, Vec3f size2, const Vec3f &d,
                            Float radiance, Float statisticalWeight, bool doFilteredSplatting,
                            std::vector<STreeNode> &nodes) {
            Float w = computeOverlappingVolume(min1, max1, min2, min2 + size2);
            if (w > 0) {
                if (isLeaf) {
                    dTree.recordRadiance(d, radiance * w, statisticalWeight * w, doFilteredSplatting);
                } else {
                    size2[axis] /= 2;
                    for (int i = 0; i < 2; ++i) {
                        if (i & 1) {
                            min2[axis] += size2[axis];
                        }

                        nodes[children[i]].recordRadiance(min1, max1, min2, size2, d, radiance, statisticalWeight,
                                                          doFilteredSplatting, nodes);
                    }
                }
            }
        }
    };

    class STree {
    private:
        std::vector<STreeNode> m_nodes;
        Bound3f aabb;
    public:
        STree(const Bound3f &aabb) {
            clear();

            this->aabb = aabb;

            // Enlarge AABB to turn it into a cube. This has the effect
            // of nicer hierarchical subdivisions.
            Point3f size = this->aabb.pMax - this->aabb.pMin;
            Float maxSize = std::max(std::max(size.x(), size.y()), size.z());
            this->aabb.pMax = this->aabb.pMin + Vec3f(maxSize);
        }

        void clear() {
            m_nodes.clear();
            m_nodes.emplace_back();
        }

        void subdivideAll() {
            int nNodes = (int) m_nodes.size();
            for (int i = 0; i < nNodes; ++i) {
                if (m_nodes[i].isLeaf) {
                    subdivide(i, m_nodes);
                }
            }
        }

        void subdivide(int nodeIdx, std::vector<STreeNode> &nodes) {
            // Add 2 child nodes
            nodes.resize(nodes.size() + 2);

            if (nodes.size() > std::numeric_limits<uint32_t>::max()) {
                fmt::print(stderr, "DTreeWrapper hit maximum children count.");
                return;
            }

            STreeNode &cur = nodes[nodeIdx];
            for (int i = 0; i < 2; ++i) {
                uint32_t idx = (uint32_t) nodes.size() - 2 + i;
                cur.children[i] = idx;
                nodes[idx].axis = (cur.axis + 1) % 3;
                nodes[idx].dTree = cur.dTree;
                nodes[idx].dTree.setStatisticalWeightBuilding(nodes[idx].dTree.statisticalWeightBuilding() / 2);
            }
            cur.isLeaf = false;
            cur.dTree = {}; // Reset to an empty dtree to save memory.
        }

        DTreeWrapper *dTreeWrapper(Point3f p, Vec3f &size) {
            size = fromPoint3f(aabb.pMax - aabb.pMin);

            p.x() /= size.x();
            p.y() /= size.y();
            p.z() /= size.z();

            return m_nodes[0].dTreeWrapper(p, size, m_nodes);
        }

        DTreeWrapper *dTreeWrapper(Point3f p) {
            Vec3f size;
            return dTreeWrapper(p, size);
        }

        void forEachDTreeWrapperConst(std::function<void(const DTreeWrapper *)> func) const {
            for (auto &node : m_nodes) {
                if (node.isLeaf) {
                    func(&node.dTree);
                }
            }
        }

        void forEachDTreeWrapperConstP(
                const std::function<void(const DTreeWrapper *, const Point3f &, const Vec3f &)> &func) const {
            m_nodes[0].forEachLeaf(func, this->aabb.pMin, fromPoint3f(this->aabb.pMax - this->aabb.pMin), m_nodes);
        }

        void forEachDTreeWrapperParallel(std::function<void(DTreeWrapper *)> func) {
            int nDTreeWrappers = static_cast<int>(m_nodes.size());

            Thread::ParallelFor(0u, nDTreeWrappers, [&](uint32_t i, uint32_t) {
                if (m_nodes[i].isLeaf) {
                    func(&m_nodes[i].dTree);
                }
            }, nDTreeWrappers / 8 + 1);
        }

        void recordRadiance(const Point3f &p, const Vec3f &dTreeVoxelSize, const Vec3f &d, Float radiance,
                            Float statisticalWeight, bool doFilteredSplatting) {
            Float volume = 1;
            for (int i = 0; i < 3; ++i) {
                volume *= dTreeVoxelSize[i];
            }

            m_nodes[0].recordRadiance(p - dTreeVoxelSize * 0.5f, p + dTreeVoxelSize * 0.5f, aabb.pMin,
                                      fromPoint3f(aabb.pMax - aabb.pMin), d, radiance / volume,
                                      statisticalWeight / volume,
                                      doFilteredSplatting, m_nodes);
        }


        bool shallSplit(const STreeNode &node, int depth, size_t samplesRequired) {
            return m_nodes.size() < std::numeric_limits<uint32_t>::max() - 1 &&
                   node.dTree.statisticalWeightBuilding() > samplesRequired;
        }

        void refine(size_t sTreeThreshold, int maxMB) {
            if (maxMB >= 0) {
                size_t approxMemoryFootprint = 0;
                for (const auto &node : m_nodes) {
                    approxMemoryFootprint += node.dTreeWrapper()->approxMemoryFootprint();
                }

                if (approxMemoryFootprint / 1000000 >= (size_t) maxMB) {
                    return;
                }
            }

            struct StackNode {
                size_t index;
                int depth;
            };

            std::stack<StackNode> nodeIndices;
            nodeIndices.push({0, 1});
            while (!nodeIndices.empty()) {
                StackNode sNode = nodeIndices.top();
                nodeIndices.pop();

                // Subdivide if needed and leaf
                if (m_nodes[sNode.index].isLeaf) {
                    if (shallSplit(m_nodes[sNode.index], sNode.depth, sTreeThreshold)) {
                        subdivide((int) sNode.index, m_nodes);
                    }
                }

                // Add children to stack if we're not
                if (!m_nodes[sNode.index].isLeaf) {
                    const STreeNode &node = m_nodes[sNode.index];
                    for (int i = 0; i < 2; ++i) {
                        nodeIndices.push({node.children[i], sNode.depth + 1});
                    }
                }
            }

            // Uncomment once memory becomes an issue.
            //m_nodes.shrink_to_fit();
        }


    };

    Spectrum GuidedPath::Li(RenderContext &ctx, Scene &scene) {
        return Miyuki::Spectrum();
    }
}

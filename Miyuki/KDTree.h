#pragma once
#include "Miyuki.h"
namespace Miyuki {
	template<typename Point>
	struct DefaultPointDist {
		Float operator() (const Point&a, const Point&b) {
			return (a - b).length();
		}
	};
	template<typename Point, class DistFunc = DefaultPointDist<Point>>
	class KDTree
	{
	public:
		using PointVec = std::vector<Point>;
		struct Node {
			int axis;
			int left, right;
			Point pivot;
			
			Node(int a) { axis = a; left = right = -1; }
		};
		std::vector<Node> nodes;
		KDTree() {  nodes.reserve(1000000); }
		~KDTree() {}
		void clear() { nodes.clear(); }
		//O(n*log^2(n))
		size_t construct(PointVec& vec, int i, int j, int depth) {
			if (i >= j)return -1;
			int axis  = depth % 3;
			auto ptr = nodes.size();
			std::sort(vec.begin() + i, vec.begin() + j, [&](const Point&a, const Point&b) {
				return a.axis(axis) < b.axis(axis);
			});
			int med = (i + j) / 2;
			auto pivot = vec[med];
			nodes.emplace_back(Node(axis));
			Node n(axis);
			n.pivot = pivot;
			n.left = construct(vec, i, med, depth + 1);
			n.right = construct(vec, med + 1, j, depth + 1);
			nodes[ptr] = n;
			return ptr;
		}
	private:
		void maxHeapify(const Point& p,int idx, PointVec& result) {
			auto dist = DistFunc();
			while (true) {
				int left = 2 * idx + 1;
				int right = 2 * idx + 2;
				int largest = idx;
				if (left < result.size() && dist(p,result[left]) > dist(p,result[largest])) {
					largest = left;
				}
				if (right < result.size() && dist(p,result[right]) > dist(p,result[largest])) {
					largest = right;
				}
				if (largest != idx) {
					std::swap(result[idx], result[largest]);
					//maxHeapify(dist, largest, result);
					idx = largest;
				}
				else {
					break;
				}
			}
		}
		void buildHeap(const Point&p,PointVec& result) {
			for (int i = result.size() / 2; i >= 0; i--) {
				maxHeapify(p, i, result);
			}
		}
	public:
		Float knn(
			const Point& p,
			Float maxDist,
			unsigned int N, 
			PointVec& result) {
			result.clear();
			if (nodes.empty())return maxDist;
			int stack[128];
			int sp = 0;
			stack[sp++] = 0;
			auto dist = DistFunc();
			while (sp > 0) {
				int ptr = stack[--sp];
				if (ptr < 0)continue;
				int a = nodes[ptr].axis;
				auto& pivot = nodes[ptr].pivot;
				Float d = p.axis(a) - pivot.axis(a);
				Float realDist = dist(p, pivot);;
				if (realDist < maxDist) {
					if (result.size() < N - 1) {
						result.emplace_back(pivot);
					}
					else if (result.size() == N - 1) {
						result.emplace_back(pivot);
						buildHeap(p, result);
						maxDist = dist(p, result[0]);
					}
					else {
						result[0] = pivot;
						maxHeapify(p, 0, result);
						maxDist = dist(p, result[0]);
					}
				}
				if (d < 0) {
					if (d*d < maxDist*maxDist)
						stack[sp++] = nodes[ptr].right;
					stack[sp++] = nodes[ptr].left;
				}
				else {
					if (d*d < maxDist*maxDist)
						stack[sp++] = nodes[ptr].left;
					stack[sp++] = nodes[ptr].right;
				}
			}
			if(result.size() == N)
				return maxDist;
			else {
				maxDist = 0;
				for (auto i : result) {
					if (dist(p, i) > maxDist)
						maxDist = dist(p, i);
				}
				return maxDist;
			}
		}
		void findWithin(const Point& p,
			Float maxDist,
			PointVec& result) {
			result.clear();
			if (nodes.empty())return;
			int stack[128];
			int sp = 0;
			stack[sp++] = 0;
			auto dist = DistFunc();
			while (sp > 0) {
				int ptr = stack[--sp];
				if (ptr < 0)continue;
				int a = nodes[ptr].axis;
				auto& pivot = nodes[ptr].pivot;
				Float d = p.axis(a) - pivot.axis(a);
				Float realDist = dist(p, pivot);;
				if (realDist < maxDist) {
					result.emplace_back(pivot);
				}
				if (d < 0) {
					if (d*d < maxDist*maxDist)
						stack[sp++] = nodes[ptr].right;
					stack[sp++] = nodes[ptr].left;
				}
				else {
					if (d*d < maxDist*maxDist)
						stack[sp++] = nodes[ptr].left;
					stack[sp++] = nodes[ptr].right;
				}
			}
		}
	};
}


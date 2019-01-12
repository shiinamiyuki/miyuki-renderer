#pragma once
#include "Miyuki.h"
#include "Primitive.h"
namespace Miyuki {
	class BVH {
		AABB box;
	public:
		std::vector<Primitive*> objects;
		BVH *left, *right;
		void intersect(const Ray&ray, Intersection &);
		BVH(const AABB & _box, const std::vector<Primitive*>&v)
			:box(_box), objects(v) {
			left = right = nullptr;
		}
		BVH() {
			left = right = nullptr;
		}
		AABB dim() { return box; }
		friend BVH * makeBVH(const std::vector<Primitive*>& objects);
		~BVH();

	};
	BVH * makeBVH(const std::vector<Primitive*>& objects);
	
	class OctreeNode {
		struct Ctx {
			int i;
			Float dist;
			Ctx(int _i, Float _f) :i(_i), dist(_f) {}
			Ctx() { i = -1; dist = -1; }
		};

	public:
		AABB box;
		vec3 size;
		int begin, end;
		OctreeNode *child[8];
		OctreeNode() {
			begin = end = 0;
			for (int i = 0; i < 8; i++)child[i] = nullptr;
		}
		void intersect(const Ray&ray, Intersection &, const std::vector<Primitive*>& cache);
		bool single()const {
			int cnt = 0;
			for (int i = 0; i < 8; i++) {
				if (child[i])cnt++;
			}
			return cnt == 1;
		}
		void optimize();
	};
	class Octree {
		OctreeNode * tree;
		std::vector<Primitive*> cache;
		friend Octree * makeOctree(const std::vector<Primitive*>& objects);
		Octree() {}
	public:
		void optimize() { tree->optimize(); }
		void intersect(const Ray&ray, Intersection &);
		AABB dim() { return tree[0].box; }
	};
	Octree * makeOctree(const std::vector<Primitive*>& objects);
}


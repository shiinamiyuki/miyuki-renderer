#include "Octree.h"

using namespace Miyuki;
AABB getBoundBox(const std::vector<Primitive*> objects) {
	vec3 a{ inf,inf,inf }, b(-inf, -inf, -inf);
	for (auto i : objects) {
		auto box = i->getBoundBox();
		a = min(a, box.min);
		b = max(b, box.max);
	}
	vec3 d{ eps,eps,eps };
	auto result =  AABB(a - d, b + d);
	for (auto i : objects) {
		auto box = i->getBoundBox();
		assert(AABB::intersect(box, result));
	}
	return result;
}


BVH::~BVH()
{
}



void Miyuki::BVH::intersect(const Ray & ray, Intersection &intersction)
{
	BVH * stack[256];
	int sp = 0;
	stack[sp++] = this;
	while (sp > 0) {
		BVH * tree = stack[--sp];
		if (tree->box.intersect(ray) < eps) {
			continue;
		}
		if (tree->objects.empty()) {
			if (tree->left)
				stack[sp++] = tree->left;
			if (tree->right)
				stack[sp++] = tree->right;
			continue;
		}
		for (auto i : tree->objects) {
			i->intersect(ray, intersction);
		}
	}
}

BVH * Miyuki::makeBVH(const std::vector<Primitive*>& objects)
{
	int D = 20;
	BVH * root = new BVH[pow(2,D)];
	auto box = getBoundBox(objects);
	root->box = box;
	BVH * cur = root;
	std::function< BVH*(const AABB&  , std::vector<Primitive*>&,int)>
		build = [&](const AABB& b,std::vector<Primitive*> &vec,int depth)->BVH* {
		
		if (vec.size() > 16 && depth < D) {
			auto box = getBoundBox(vec);
			auto s = (box.max - box.min) / 2;
			auto axis = s.x() > s.y() ? (s.x() > s.z() ? 0 : 2) : (s.y() > s.z() ? 1 : 2);
			std::sort(vec.begin(), vec.end(), [=](Primitive * a, Primitive * b) {
				return a->getBoundBox().center().axis(axis) < b->getBoundBox().center().axis(axis);
			});

			std::vector<Primitive*> left, right;
			for (auto i = 0; i < vec.size(); i++) {
				if (i <vec.size() / 2)
					left.emplace_back(vec[i]);
				else
					right.emplace_back(vec[i]);
			}
			auto result = cur++;
			result->box = b;
			result->left = build(getBoundBox(left), left, depth + 1);
			result->right = build(getBoundBox(right), right, depth + 1);
			return result;
		}
		else{
			cur->objects = vec;
			cur->box = b;
			cur->left = cur->left = nullptr;
			return cur++;
		}
	};
	return build(box, std::vector<Primitive*>(objects),0);
}

Octree * Miyuki::makeOctree(const std::vector<Primitive*>& objects) {
	int D = 6;
	Octree * tree = new Octree();
	OctreeNode * root = new OctreeNode[pow(8, D)];
	auto box = getBoundBox(objects);
	root->box = box;
	OctreeNode * cur = root;
	std::function< OctreeNode*(AABB , std::vector<Primitive*>, int)>
		build = [&](AABB  b, std::vector<Primitive*> vec, int depth)->OctreeNode* {
		bool criteria = true;
		criteria = criteria && vec.size() > 16 && depth < 12;
		if (criteria) {
			auto result = cur++;
			auto v1 = b.min;
			auto v2 = b.center();
			auto size = (b.max - b.min) / 2;
			auto dx = vec3(size.x(), 0, 0);
			auto dy = vec3(0, size.y(), 0);
			auto dz = vec3(0, 0, size.z());
			vector<vector<Primitive*>>v;
			vector<AABB>boxs;
			boxs.resize(8);
			v.resize(8);
			for (int i = 0; i < 2; i++) {
				for (int j = 0; j < 2; j++) {
					for (int k = 0; k < 2; k++) {
						auto offset = i * dx + j * dy + k * dz;
						boxs[4 * i + 2 * j + k] = AABB(v1 + offset, v2 + offset);
					}
				}
			}
			for (auto o : vec) {
				for (int i = 0; i < 8; i++) {
					if (AABB::intersect(boxs[i], o->getBoundBox())) {
						v[i].push_back(o);
					}
				}
			}
			int cnt = 0;
			for (auto i : v) {
				cnt += i.size();
			}
			if (cnt < 3 * vec.size()) {
				result->box = b;
				for (int i = 0; i < 8; i++) {
					result->child[i] = build(boxs[i], v[i], depth + 1);
				}
				return result;
			}
		}
		if (!vec.empty()) {
			//fmt::print("Depth = {},  {}\n", depth, vec.size());
			cur->begin = tree->cache.size();
			cur->end = cur->begin + vec.size();
			for (auto i : vec) {
				tree->cache.emplace_back(i);
			}
			cur->box = b;
			return cur++;
		}return nullptr;
	};
	tree->tree =  build(box, std::vector<Primitive*>(objects), 0);
	return tree;
}



void Miyuki::OctreeNode::intersect(const Ray & ray, Intersection &intersection, const std::vector<Primitive*>& cache)
{
	OctreeNode * stack[8 * 20];
	Ctx vec[8];
	int sp = 0;
	stack[sp++] = this;
	while (sp > 0) {
		auto tree = stack[--sp];
		Float dist2 = -1;
		Float dist = tree->box.intersect(ray, &dist2);
		if (dist < eps)
			continue;
		if (tree->begin == tree->end) {
			int tail = 0;
			for (int i = 0; i < 8; i++) {
				if (tree->child[i]) {
					Float dist = tree->child[i]->box.intersect(ray);
					if (dist > eps) {
						vec[tail].i = i;
						vec[tail++].dist = dist;
					}
				}
			}
			//is this good?
			std::sort(vec, vec + tail, [&](auto a, auto b) {return a.dist > b.dist; });
			for (int i = 0; i < tail; i++) {
				stack[sp++] = tree->child[vec[i].i];
			}
		}
		else {
			for (int i = tree->begin; i < tree->end;i++) {
				cache[i]->intersect(ray, intersection);
			}
			if (intersection.hit())
				return;
		}
	}
}

void Miyuki::OctreeNode::optimize()
{
	for (auto i : child) {
		if (i)
			i->optimize();
	}
	for (int i = 0; i < 8; i++) {
		if (child[i] && child[i]->single()) {
			for (int j = 0; j < 8; j++) {
				if (child[i]->child[j]) {
					child[i] = child[i]->child[j];
					break;
				}
			}
		}
	}
	size = (box.max - box.min) / 2;
}

void Miyuki::Octree::intersect(const Ray & ray, Intersection &i)
{
	tree->intersect(ray, i,cache);
}

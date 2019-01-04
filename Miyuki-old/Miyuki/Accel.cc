#include "Accel.h"
using namespace Miyuki;
AABB getBoundBox(const std::vector<Primitive*> objects) {
	vec3 a{ inf,inf,inf }, b(-inf, -inf, -inf);
	for (auto i : objects) {
		auto box = i->getBoundBox();
		a = min(a, box.min);
		b = max(b, box.max);
	}
	vec3 d{ eps,eps,eps };
	auto result = AABB(a - d, b + d);
	for (auto i : objects) {
		auto box = i->getBoundBox();
		assert(AABB::intersect(box, result));
	}
	return result;
}

void Miyuki::BVH::intersect(const Ray & _ray, Intersection & isct)
{
	
	int stack[40];
	int sp = 0;
	int root = 0;
	Ray ray(_ray.o, _ray.d);
	if (!nodes[root].box.intersect(ray))
		return;
	SIMDRay simdRay(ray);
	stack[sp++] = 0;
	while (sp > 0) {
		InternalNode * node = nodes.data() + stack[--sp];
		if (!node->leaf) {
			if (node->left>0&&nodes[node->left].box.intersect(ray)) {
				stack[sp++] = node->left;
			}
			if (node->right > 0 && nodes[node->right].box.intersect(ray)) {
				stack[sp++] = node->right;
			}
		}
		else {
#ifdef ENABLE_SIMD
			for (int i = node->simdBegin; i != node->simdEnd; i++) {
				simdTrigs[i].intersect(simdRay, isct);
			}
#endif
			for (int i = node->begin; i != node->end; i++) {
				cache[i]->intersect(ray, isct);
			}
		}
	}
}

void Miyuki::BVH::construct(const std::vector<Primitive*>& objects)
{
	auto box = getBoundBox(objects);
	std::function<int(const AABB&, std::vector<Primitive*>&, int)>
		build = [&](const AABB& b, std::vector<Primitive*> &vec, int depth)->int {
		int ptr = nodes.size();
		if (vec.size() > 16 && depth < 20) {
			nodes.emplace_back(InternalNode());
			auto box = getBoundBox(vec);
			auto s = (box.max - box.min) / 2;
			auto axis = s.x() > s.y() ? (s.x() > s.z() ? 0 : 2) : (s.y() > s.z() ? 1 : 2);
			std::sort(vec.begin(), vec.end(), [=](Primitive * a, Primitive * b) {
				return a->getBoundBox().center().axis(axis) < b->getBoundBox().center().axis(axis);
			});

			std::vector<Primitive*> left, right;
			for (auto i = 0; i < vec.size(); i++) {
				if (i < vec.size() / 2)
					left.emplace_back(vec[i]);
				else
					right.emplace_back(vec[i]);
			}
			auto L = build(getBoundBox(left), left, depth + 1);
			auto R = build(getBoundBox(right), right, depth + 1);
			nodes[ptr].leaf = false;
			nodes[ptr].left = L;
			nodes[ptr].right = R;
			nodes[ptr].box = b;
			nodes[ptr].begin = nodes[ptr].end = 0;
			return ptr;
		}
		else if(!vec.empty()){
			nodes.emplace_back(InternalNode());
			nodes[ptr].leaf = true;
#ifdef ENABLE_SIMD
			std::vector<Triangle*> trigs;
			std::vector<Primitive*> others;
			for (Primitive* i : vec) {

				if (i->type() == Triangle().type()) {
					trigs.emplace_back(static_cast<Triangle*>(i));
				}
				else {
					others.emplace_back(i);
				}
			}

		/*	while (trigs.size() % simdVec::width() != 0) {
				others.emplace_back(trigs.back());
				trigs.pop_back();
			}*/

			nodes[ptr].begin = cache.size();
			nodes[ptr].end = cache.size() + others.size();

			for (auto i : others) {
				cache.emplace_back(i);
			}

			nodes[ptr].simdBegin = simdTrigs.size();

			for (int i = 0; i < trigs.size(); i++) {
				assert(trigs[i]->type() == Triangle().type());
			}
			std::vector<Triangle*> v;
			int cnt = 0;
			for (int i = simdVec::width(); i <= trigs.size(); i+= simdVec::width()) {
				v.clear();
				for (int j = i - simdVec::width(); j < i; j++) {
					v.emplace_back(trigs[j]);
					cnt++;
				}
				simdTrigs.emplace_back(SIMDTriangle(v));
			}
			if ((cnt < trigs.size())) {
				v.clear();
				while (cnt < trigs.size()) {
					v.emplace_back(trigs[cnt++]);
				}
				simdTrigs.emplace_back(SIMDTriangle(v));
			}
			nodes[ptr].simdEnd = simdTrigs.size();
#else
			nodes[ptr].begin = cache.size();
			nodes[ptr].end = cache.size() + vec.size();
			for (auto i : vec) {
				cache.emplace_back(i);
			}
#endif
			nodes[ptr].box = b;
			nodes[ptr].left = -1;
			nodes[ptr].right = -1;
			return ptr;
		}
		else {
			return -1;
		}
	};
	build(box, std::vector<Primitive*>(objects), 0);
}

void Miyuki::BVH::clear()
{
#ifdef ENABLE_SIMD
	simdTrigs.clear();
#endif
	nodes.clear();
	cache.clear();
}

std::string Miyuki::BVH::getBuildInfo() const
{
	return fmt::format("BVH nodes:{}\nSIMD triangles:{}\nother:{}\n",
		nodes.size(),
		simdTrigs.size(),
		cache.size());
}

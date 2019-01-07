#include "Accel.h"
#include "Render.h"

AABB makeBoundBox(const QVector3D&a, const QVector3D&b) {
	AABB box;
	box.min = toVec3(a);
	box.max = toVec3(b);
	return box;
}
AABB makeBoundBox(const Vector&a, const Vector&b) {
	AABB box;
	box.min = a;
	box.max = b;
	return box;
}
AABB getBoundBox(const Primitive& p) {
	QVector3D d(0.001, 0.001, 0.001);
	if (p.type == TYPE_TRIANGLE) {
		auto a = fromVec3(min(p.triangle.vertex0, min(p.triangle.vertex1, p.triangle.vertex2)));
		auto b = fromVec3(max(p.triangle.vertex0, max(p.triangle.vertex1, p.triangle.vertex2)));
		return makeBoundBox(a - d, b + d);
	}
	else {
		auto c = fromVec3(p.sphere.center);
		auto r = p.sphere.radius;
		QVector3D v(r, r, r);
		return makeBoundBox(c - v, c + v);
	}
}
AABB merge(const AABB&a, const AABB&b) {
	return makeBoundBox(min(a.min, b.min), max(a.max, b.max));
}
AABB getBoundBox(const std::vector<Primitive>& objects, const std::vector<int>& v) {
	auto a = vec3(inf, inf, inf);
	auto b = vec3(-inf, -inf, -inf);
	for (auto i : v) {
		auto box = getBoundBox(objects[i]);
		a = min(box.min, a);
		b = max(box.max, b);
	}
	return makeBoundBox(a, b);
}
Vector centroid(const AABB&a) {
	return (a.min + a.max) / 2;
}
Vector size(const AABB&a) {
	return ((a.max) - (a.min)) / 2;
}
bool intersect(const AABB &a, const AABB &b) {
	for (unsigned int i = 0; i < 3; i++) {
		if (abs(centroid(a).s[i] - centroid(b).s[i])<= size(a).s[i] + size(b).s[i] + 0.01) {}
		else { return false; }
	}
	return true;
}

bool contains(const AABB &a, const AABB &b) {
	return a.min.x <= b.min.x && b.max.x <= a.max.x
		&&a.min.y <= b.min.y && b.max.y <= a.max.y
		&&a.min.z <= b.min.z && b.max.z <= a.max.z;
}



int BVH::chooseAxis(const std::vector<int> rest)
{
	Vector a = vec3(inf, inf, inf), b = vec3(-inf, -inf, -inf);
	for (auto i : rest) {
		const auto box = boxes[i];
		a = min(a, centroid(box));
		b = max(b, centroid(box));
	}
	Vector dim;
	vsub(b, a, dim);
	if (dim.x > dim.y) {
		if (dim.x > dim.z) {
			return 0;
		}return 2;
	}
	else {
		if (dim.y > dim.z) {
			return 1;
		}return 2;
	}
}

AABB BVH::getBoundBox(const std::vector<int> rest)
{
	AABB box = makeBoundBox(vec3(inf, inf, inf), vec3(-inf, -inf, -inf));
	for (auto i : rest) {
		box = merge(box, boxes[i]);
	//	box.min = min(box.min, boxes[i].min);
	//	box.max = max(box.max, boxes[i].max);
	}
	Vector d=vec3(0.001,0.001,0.001);
	vsub(box.min, d, box.min);
	vadd(box.max,d, box.max);
	for (auto i : rest) {
		assert(contains(box, boxes[i]));
	}
	return box;
}

void BVH::construct(const std::vector<Primitive>&v)
{
	std::vector<int> rest;
	boxes.resize(v.size());
	for (auto& i : v) {
		boxes[i.id] = (::getBoundBox(i));
		rest.emplace_back(i.id);
	}
	nodes.reserve(400);
	construct(v, rest);
}

int BVH::construct(const std::vector<Primitive>& objects,  std::vector<int>& rest)
{
	int idx = nodes.size();
	if (rest.empty())return -1;
	if (rest.size() <= 16 ) {
		nodes.emplace_back(BVHNode());
		nodes[idx].begin = prims.size();
		nodes[idx].end = prims.size() + rest.size();
		nodes[idx].box = getBoundBox(rest);
		nodes[idx].left = nodes[idx].right = -1;
		for (auto i : rest) {
			prims.emplace_back(i);
			assert(contains(nodes[idx].box, boxes[i]));
		}
		
		return idx;
	}
	else {
		nodes.emplace_back(BVHNode());
		
		int axis = chooseAxis(rest);
		auto& vbox = boxes;
		auto box = getBoundBox(rest);;
		std::sort(rest.begin(), rest.end(), [&](int a, int b) {
			return centroid(vbox[a]).s[axis] < centroid(vbox[b]).s[axis];
		});
		int mid = rest.size() / 2;
		std::vector<int> v1, v2;
		for (int i = 0; i < mid; i++) {
			v1.emplace_back(rest[i]);
		}
		for (int i = mid; i < rest.size(); i++) {
			v2.emplace_back(rest[i]);
		}
		int left = construct(objects, v2);
		int right = construct(objects, v1);

		nodes[idx].begin = nodes[idx].end = 0;
		nodes[idx].left = left;
		nodes[idx].right = right;
		nodes[idx].box = box;
		for (auto i : rest) {
			assert(contains(nodes[idx].box, boxes[i]));
		}
		return idx;
	}
}

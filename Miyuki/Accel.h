#pragma once
#include "util.h"
#include "kernel/trace.h"
class Render;

class BVH {
	friend class Render;
	std::vector<int> prims;
	std::vector<BVHNode> nodes;
	std::vector<AABB>boxes;
	int chooseAxis(const std::vector<int> rest);
	AABB getBoundBox(const std::vector<int> rest);
	void construct(const std::vector<Primitive>&);
	int construct(const std::vector<Primitive>&objects,  std::vector<int>& rest);
public:
	BVH() {}
	BVH(const std::vector<Primitive>&v) { construct(v); }
	~BVH() {}
};


 #pragma once
#include "Miyuki.h"
#include "Primitive.h"
#include "Intersection.h"
namespace Miyuki {
	class AccelerationStructure {
	public:
		AccelerationStructure() {}
		virtual void intersect(const Ray&ray, Intersection&isct) = 0;
		virtual void construct(const std::vector<Primitive*>&) = 0;
		virtual void clear() = 0;
		virtual std::string getBuildInfo() const{ return ""; }
		virtual ~AccelerationStructure() {}
		virtual Float dim()const { return 0; }
	};
	class BVH : public AccelerationStructure {
		struct InternalNode {
			AABB box;
			int begin, end;
			int left, right;
#ifdef ENABLE_SIMD
			int simdBegin, simdEnd;
#endif
			bool leaf;
		};
		std::vector<InternalNode> nodes;
		std::vector<Primitive*> cache;
#ifdef ENABLE_SIMD
		std::vector<SIMDTriangle> simdTrigs;
#endif
	public:
		BVH() {}
		void intersect(const Ray&ray, Intersection&isct)override;
		void construct(const std::vector<Primitive*>&) override;
		void clear()override;
		std::string getBuildInfo()const override;
		Float dim()const override{ return (nodes[0].box.max - nodes[0].box.min).max(); }
	};
}
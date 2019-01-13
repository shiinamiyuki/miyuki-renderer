#pragma once
#include "Miyuki.h"
#include "Ray.h"
#include "Intersection.h"
#include "Material.h"
#include "aabb.h"
#include "Float4.h"
#include "Sampler.h"
namespace Miyuki {
	class Scene;
	struct Material;
	struct RenderContext {
		Ray ray;
		Intersection intersection;
		vec3 color;
		vec3 throughput;
		Seed * Xi;
		Scene * scene;
		Sampler sampler;
		bool sampleEmit;
		bool directLighting;
		int depth;
		RenderContext(Scene *_s,const Ray&_r, Seed*_Xi)
			:scene(_s),depth(0), ray(_r),Xi(_Xi),throughput(vec3(1,1,1)), sampleEmit(true){
			sampler.Xi = _Xi;
			directLighting = true;
		}
		Float rand() {
			return sampler.sample();
		}
	};
	class Primitive
	{
	protected:
		Material material;
	public:
		unsigned int id;
		Primitive();
		virtual void intersect(const Ray&, Intersection&) = 0;
		virtual vec3 randomPointOnObject(Seed*) = 0;
		virtual vec3 getNormOfPoint(const vec3&) = 0;
		virtual AABB getBoundBox() = 0;
		virtual Float area()const = 0;
		virtual int type()const = 0;
		virtual ~Primitive();
		const Material& getMaterial()const { return material; }
	};
	class Sphere :public Primitive {
		Float radius;
		vec3 center;
	public:
		int type()const override { return 1; }
		Sphere() {}
		Sphere(const vec3&v, Float r, const Material& m)
			:center(v), radius(r) {
			material = m;
		}
		void intersect(const Ray&, Intersection&)override;
		vec3 getNormOfPoint(const vec3&)override;
		vec3 randomPointOnObject(Seed*)override;
		AABB getBoundBox()override;
		Float area()const override { return radius * radius*pi; }
	};
	class SIMDTriangle;
	class Triangle : public Primitive {
		vec3 vertices[3];
		vec3 norm;
		Float A;
		friend class SIMDTriangle;
	public:
		int type()const override { return 2; }
		Triangle() {}
		Triangle(const vec3 &_a, const vec3 &_b, const vec3 &_c) {
			vertices[0] = _a;
			vertices[1] = _b;
			vertices[2] = _c;
			norm =vec3::crossProduct(_b - _a, _c - _a);
			A = norm.length() / 2;
			norm.normalize();
		}
		Triangle(const vec3 &_a, const vec3 &_b, const vec3 &_c, Material m){
			material = m;
			vertices[0] = _a;
			vertices[1] = _b;
			vertices[2] = _c;
			norm = vec3::crossProduct(_b - _a, _c - _a);
			A = norm.length() / 2;
			norm.normalize();
		}
		vec3 getNormOfPoint(const vec3&)override{ return norm; }
		AABB getBoundBox() override {
			vec3 a = { inf, inf, inf };
			vec3 b = { -inf, -inf, -inf };
			for (unsigned int i = 0; i < 3; i++) {
				a = min(a, vertices[i]);
				b = max(b, vertices[i]);
			}
			auto boundBox = AABB(a, b);
			for (unsigned int i = 0; i < 3; i++) {
				assert(boundBox.contains(vertices[i]));
			}
			return boundBox;
		}
		Float area()const override { return A; }
		void intersect(const Ray &ray, Intersection &intersection)override;
		vec3 randomPointOnObject(Seed*)override;
	};
	class SIMDTriangle {
		simdVec vertices[3];
		simdVec norm;
		Triangle * trigs[8];
	public:
		SIMDTriangle() {}
		SIMDTriangle(std::vector<Triangle*> list);
		void intersect(const SIMDRay &ray, Intersection &intersection);
	};
}


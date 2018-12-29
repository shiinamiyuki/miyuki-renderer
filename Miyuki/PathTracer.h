#pragma once
#include "Miyuki.h"
#include "Logger.h"
#include "Primitive.h"
#include "Accel.h"
#include "Camera.h"
#include "Logger.h"
#include "Integrator.h"
namespace Miyuki {
	class Scene;
	class PathTracer : public Integrator{
		friend class Scene;
	protected:
		Logger * logger;
		Camera camera;
		Scene * scene;
	public:
		PathTracer(Scene*s) :scene(s),logger(nullptr){}
		vec3 sampleLights(RenderContext&ctx);
		vec3 sampleLights(const Ray&ray,Intersection&isct,Seed*	Xi);
		void intersect(const Ray& ray, Intersection&);
		void prepare();
		vec3 trace(int x0, int y0);
		vec3 raycast(int x0, int y0);
		void render(Scene *)override;
	};
	class BDPT : public Integrator {
	protected:
		Logger * logger;
		Camera camera;
		Scene * scene;
		void intersect(const Ray& ray, Intersection&);
		struct LightVertex {
			vec3 normal;
			vec3 wi, wo;
			vec3 throughput;	// the accumlated throughput until this vertex is reached
			vec3 radiance;		// the radiance throughput until this vertex is reached
			vec3 hitpoint;
			Primitive * object;
			BxDFType type;
			LightVertex(){}
		};
		struct Path : public std::vector<LightVertex> {};
		void traceLightPath(Seed*Xi,Path&);
		void traceEyePath(RenderContext& ctx,Path&);
		vec3 connectPath(Path&,Path&);
		vec3 trace(int x, int y);
		bool visiblity(const LightVertex& v1, const LightVertex&v2);
		Float contribution(const LightVertex& v1, const LightVertex&v2);
		std::vector<Path> paths;
		void generateLightPath();
	public:
		static inline Float G(const vec3&n1, const vec3& d, const vec3&n2, const vec3&x1, const vec3&x2) {
			return fabs(vec3::dotProduct(n1, d) * vec3::dotProduct(n2, d)) / (0.001 + (x1 - x2).lengthSquared());
		}
		void render(Scene *)override;

	};
}
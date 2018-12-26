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
		vec3 sampleLights(const Ray&ray,Intersection&isct,Seed*Xi);
		void intersect(const Ray& ray, Intersection&);
		void prepare();
		vec3 trace(int x0, int y0);
		vec3 raycast(int x0, int y0);
		void render(Scene *)override;
	};
}
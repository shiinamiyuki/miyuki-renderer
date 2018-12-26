#pragma once
#include "Miyuki.h"
#include "KDTree.h"
#include "Accel.h"
#include "Logger.h"
#include "Integrator.h"

namespace Miyuki {
	class Scene;
	struct Photon {
		vec3 pos;
		vec3 dir;
		vec3 flux;
		const Float x()const { return pos.x(); }
		const Float y()const { return pos.y(); }
		const Float z()const { return pos.z(); }
		const Float axis(unsigned int a)const { return pos.axis(a); }
		Photon(const vec3&p, const vec3&d, const vec3&f) :pos(p), dir(d),flux(f) {}
		Photon() {}
	};
	struct PhotonDist {
		Float operator() (const Photon& a, const Photon& b) {
			return (a.pos - b.pos).length();
		}
	};
	class PhotonMapRenderer : public Integrator{
		struct Parameter {
			Float initalRadius;
			Float radius;
			int maxPhoton;
			int usePhoton;
			Parameter() {
				initalRadius = 30;
				maxPhoton = 500;
			}
		}parameter;
		friend class Scene;
		Scene * scene;
		Logger *logger;
		std::vector<vec3> indirectCache;
		std::vector<Photon> photons;
		std::vector<Float> powerIntergral;
		KDTree<Photon, PhotonDist> photonMap;
		Float power;
		void computeTotalPower();
		Primitive * searchLight(Float powerInt);
		void emitPhoton(Float total,vec3& flux,Ray&ray,Seed*Xi);
		void tracePhotonIdx(Float total, unsigned int,Seed*Xi);
		void tracePhoton(unsigned long long N);
		void intersect(const Ray& ray, Intersection&);
		void buildPhotonMap();
		vec3 estimateRadiance(const vec3&p,const vec3&norm);
		vec3 traceEyePath(int x, int y);
		void trace();
		void prepare();
	public:
		PhotonMapRenderer(Scene * s=nullptr) :scene(s),logger(nullptr) { }
		void reset();
		void render();
		void render(Scene * s)override;
	};
}
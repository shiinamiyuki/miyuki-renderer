#pragma once
#include "Miyuki.h"
#include "Integrator.h"
#include "PhotonMap.h"
#include "Logger.h"
namespace Miyuki {
	class SPPM : public Integrator
	{
		Scene * scene;
		Logger * logger;
		long long Ne;
		struct Hitpoint {
			Hitpoint() {}
		};
		struct Region {
			std::vector<Hitpoint> hitpoints;
			vec3 flux;
			Float radius;
			vec3 radiance;
			vec3 nonDiffuse;
			Float alpha;
			int N;
			Region() :flux(0, 0, 0), nonDiffuse(0,0,0){
				radius = 1;
				N = 1;
			}
			vec3 estimateRadiance(SPPM * sppm,const vec3& refl,const vec3&p, const vec3&norm);
		};
		std::vector<Region> image;
		std::vector<Photon> photons;
		std::vector<Float> powerIntergral;
		KDTree<Photon, PhotonDist> photonMap;
		Float power;
		void init(int w,int h);		
		void computeTotalPower();
		Primitive * searchLight(Float powerInt);
		void emitPhoton(Float total, vec3& flux, Ray&ray, Seed*Xi);
		void tracePhoton(Float N, Seed*Xi);
		void intersect(const Ray& ray, Intersection&isct);
		void photonPass(int N);
		void cameraPass();
		void buildPhotonMap();
	public:
		SPPM();
		void render(Scene *)override;
		~SPPM();
	};
}


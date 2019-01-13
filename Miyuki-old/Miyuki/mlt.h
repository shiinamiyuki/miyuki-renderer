#pragma once
#include "Miyuki.h"
#include "Integrator.h"
#include "PathTracer.h"
namespace Miyuki {
	class MLT;
	class MLTSampler : public SamplerBase {
		friend class MLT;
		struct PrimarySample {
			Float value;
			int lastModifyIter;
			PrimarySample() { lastModifyIter = 0; }
		};
		int idx;
		std::vector<PrimarySample> samples;
		std::vector<std::pair<int, PrimarySample>> stack;
		Seed  seed[3];
		int curIter,lastLargeStepIter;
		int largeStep;
		Float largeStepProb;
	public:
		struct Sample {
			float weight;
			vec3 contrib;
			vec3 pos;
			Sample() { weight = 0; pos = vec3(0, 0, 0); contrib = vec3(0, 0, 0); }
		};
		float b;
		float oldI;
		Sample oldSample, newSample, contribSample;
		void start() { 
			idx = 0;
			largeStep = erand48(seed) < largeStepProb ? 1 : 0;	
		}
		MLTSampler();
		Float sample(Seed* Xi)override;
		Float mutate(Seed*Xi,Float x, Float n);
		Float primarySample(int i,Seed*Xi);
		void mutate();
		void update(Scene*, const vec3& p, const vec3& Fy);
		void accept();
		void reject();
		void newSeed();
		void finishBoostrap();
		void assignSeed(const std::vector<Float>&);
	};
	class MLT : public PathTracer
	{
		double b;
		std::vector<MLTSampler> sampler;
		std::vector<int> mut;
		vec3 trace(MLTSampler&, int x, int y, bool boot=false);
		void bootstrap();
	public:
		void render(Scene *)override;
		MLT();
		~MLT();
	};
}

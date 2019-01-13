#pragma once
#include "Miyuki.h"
namespace Miyuki {
	class SamplerBase
	{
	public:
		SamplerBase();
		virtual Float sample(Seed* Xi);
		virtual ~SamplerBase();
	};
	
	struct Sampler {
		SamplerBase * s;
		Seed * Xi;
		Sampler() :Xi(nullptr), s(nullptr) {}
		Float sample() {
			return s->sample(Xi);
		}
		static int toInt(Float x,int max) {
			x = static_cast<int>(x * max);
			if (x >= max) {
				x = max - 1;
			}
			return x;
		}
	};
}


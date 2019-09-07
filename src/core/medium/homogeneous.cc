#include <core/medium/homogeneous.h>
#include <core/samplers/sampler.h>
#include <core/memory.h>
#include <core/phases/phase.h>

namespace Miyuki {
	namespace Core {
		static Spectrum exp(const Spectrum& s) {
			return Spectrum(std::exp(s.r), std::exp(s.g), std::exp(s.b));
		}
		Spectrum HomogeneousMedium::Tr(const Ray& ray, Sampler& sampler)const {
			// ray.d is always normalized
			Float d = ray.tMax;// *ray.d.length();
			auto tau = sigma_t * -d;
			auto Tr =  removeNaNs(exp( tau));
			//fmt::print("{}\n", Tr.max());
			return Tr;
		}

		Spectrum HomogeneousMedium::sample(MediumSample& sample)const {
			int channel = std::min(Spectrum::dimension() - 1, (int)(sample.sampler->get1D() * Spectrum::dimension()));
			Float dist = -std::log(1 - sample.sampler->get1D()) / sigma_t[channel];
			Float t = dist;// *sample.ray.d.length();
			bool sampledMedium = t < sample.ray.tMax;
			if (sampledMedium) {
				sample.phase = sample.arena->New<HenyeyGreenstein>(g);
				sample.origin = sample.ray.o + t * sample.ray.d;
				
			}
			Spectrum Tr = removeNaNs(exp(sigma_t * -t));
			Spectrum density = sampledMedium ? (sigma_t * Tr) : Tr;
			Float pdf = 0.0f;
			for (int i = 0; i < Spectrum::dimension(); i++) {
				pdf += density[i];
			}
			pdf /= Spectrum::dimension();
		
			//fmt::print("sample medium {} {}\n",t,(Tr * sigma_s / pdf).max());
			return sampledMedium ? Tr * sigma_s / pdf : Tr / pdf;
		}

		void HomogeneousMedium::doPreprocess() {
			sigma_t = sigma_a + sigma_s;
		}
	}
}
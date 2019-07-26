#include <core/bsdfs/bsdf.h> 

namespace Miyuki {
	namespace Core {
		void BSDF::sample(
			BSDFSample& sample
		)const {
			if (impl->match(sample.lobe)) {
				impl->sample(sample);
			}
		}

		// evaluate bsdf according to wo, wi
		Spectrum  BSDF::evaluate(
			const Vec3f& wo,
			const Vec3f& wi,
			BSDFSampleOption option,
			BSDFLobe lobe
		)const {
			if (impl->match(lobe)) {
				return impl->evaluate(wo, wi, option, lobe);
			}
			else {
				return {};
			}
		}

		// evaluate pdf according to wo, wi
		Float  BSDF::evaluatePdf(
			const Vec3f& wo,
			const Vec3f& wi,
			BSDFSampleOption option,
			BSDFLobe lobe
		)const {
			if (impl->match(lobe)) {
				return impl->evaluatePdf(wo, wi, option, lobe);
			}
			else {
				return 0.0f;
			}
		}		
	}
}
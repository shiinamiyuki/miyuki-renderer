#ifndef MIYUKI_BSDF_SCATTERINGFUNCTION_HPP
#define MIYUKI_BSDF_SCATTERINGFUNCTION_HPP

#include <core/probablity/commonpdf.hpp>

namespace Miyuki {
	namespace Core {
		// Conventions:
		// sampled directions w.r.t local frame
		// sampled direction is always incoming direction (wi)
		// pdf w.r.t solid angle
		struct ScatteringFunctionSample {
			Point2f u;
			Vec3f wo;
			Vec3f wi; // sampled direction
			Float pdf = 0; // pdf of the sampled direction
			Spectrum f; // sampled bsdf
		};

		/*
		*	A single scattering function
		*/
		template<class ImplT>
		struct ScatteringFunction {
			void sample(
				ScatteringFunctionSample& sample)const {
				This().sampleImpl(sample);
			}

			Spectrum evaluate(const Vec3f& wo, const Vec3f& wi)const {
				return This().evaluateImpl(wo, wi);
			}

			// evaluate pdf according to wo, wi
			Float evaluatePdf(const Vec3f& wo, const Vec3f& wi)const {
				return This().evaluatePdfImpl(wo, wi);
			}
		private:
			const ImplT& This()const {
				return *static_cast<const ImplT*>(this);
			}
		};
	}
}


#endif